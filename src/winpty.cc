// Copyright (c) 2011-2012 Ryan Prichard
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <string.h>
#include <vector>
#include <iostream>
#include <stdint.h>
#include <sstream>
#include <io.h>
#include "Buffer.h"
#include "winpty.h"


#define AGENT_EXE L"winpty-agent.exe"

static volatile LONG consoleCounter;


static HMODULE getCurrentModule() {
  HMODULE module;
  GetModuleHandleExW(6, (LPCWSTR)getCurrentModule, &module);
  return module;
}

static std::wstring getModuleName(HMODULE module) {
  const int bufsize = 4096;
  wchar_t path[bufsize];
  int size = GetModuleFileNameW(module, path, bufsize);
  if (size != 0 && size != bufsize) {
    return std::wstring(path);
  } else {
    return std::wstring();
  }
}

static std::wstring directoryName(const std::wstring &path) {
  std::wstring::size_type pos = path.find_last_of(L"\\/");
  if (pos == std::wstring::npos)
    return L"";
  else
    return path.substr(0, pos);
}

static bool pathExists(const std::wstring &path) {
  return GetFileAttributesW(path.c_str()) != 0xFFFFFFFF;
}


static std::wstring getObjectName(HANDLE object) {
  BOOL success;
  DWORD lengthNeeded = 0;
  GetUserObjectInformationW(object, UOI_NAME, NULL, 0, &lengthNeeded);
  if (lengthNeeded % sizeof(wchar_t) == 0) {
    wchar_t *tmp = new wchar_t[lengthNeeded / 2];
    success = GetUserObjectInformationW(object, UOI_NAME, tmp, lengthNeeded, NULL);
    std::wstring ret = tmp;
    delete [] tmp;
    return ret;
  } else {
    return std::wstring();
  }
  return NULL;
}

static std::wstring getDesktopFullName() {
  HWINSTA station = GetProcessWindowStation();
  HDESK desktop = GetThreadDesktop(GetCurrentThreadId());
  if (station != NULL && desktop != NULL) {
    return getObjectName(station) + L"\\" + getObjectName(desktop);
  } else {
    return NULL;
  }
}

static BackgroundDesktop setupBackgroundDesktop() {
  BackgroundDesktop ret;
  ret.originalStation = GetProcessWindowStation();
  ret.station = CreateWindowStationW(NULL, 0, WINSTA_ALL_ACCESS, NULL);
  bool success = !!SetProcessWindowStation(ret.station);
  if (success) {
    ret.desktop = CreateDesktopW(L"Default", NULL, NULL, 0, GENERIC_ALL, NULL);
    if (ret.originalStation != NULL && ret.station != NULL && ret.desktop != NULL) {
        ret.desktopName = getObjectName(ret.station) + L"\\" + getObjectName(ret.desktop);
    }
  }
  return ret;
}

static void restoreOriginalDesktop(const BackgroundDesktop &desktop) {
  SetProcessWindowStation(desktop.originalStation);
  CloseDesktop(desktop.desktop);
  CloseWindowStation(desktop.station);
}



// ##################
// ### PTYPipe ###
// ##################

PTYPipe::PTYPipe(std::wstring &name, bool overlapped)
:overlapped_(overlapped),
 connected_(false),
 name_(name) {
  handle_ = CreateNamedPipeW(name_.c_str(), PIPE_ACCESS_DUPLEX |
                                            FILE_FLAG_FIRST_PIPE_INSTANCE |
                                            (overlapped ? FILE_FLAG_OVERLAPPED : 0), 0, 1, 0, 0, 3000, NULL);
}

PTYPipe::~PTYPipe(){
  CloseHandle(handle_);
}


bool PTYPipe::IsValid(){
  return handle_ != INVALID_HANDLE_VALUE;
}

bool PTYPipe::Connect() {
  OVERLAPPED over, *pover = NULL;
  if (overlapped_) {
    pover = &over;
    memset(&over, 0, sizeof(over));
    over.hEvent = CreateEvent(NULL, true, FALSE, NULL);
    if (over.hEvent == NULL) {
      return false;
    }
  }

  bool success = !!ConnectNamedPipe(handle_, pover);
  if (overlapped_ && !success && GetLastError() == ERROR_IO_PENDING) {
    DWORD actual;
    success = !!GetOverlappedResult(handle_, pover, &actual, true);
  }
  if (!success && GetLastError() == ERROR_PIPE_CONNECTED) {
    success = true;
  }
  if (overlapped_) {
    CloseHandle(over.hEvent);
  }
  connected_ = true;
  return success;
}

bool PTYPipe::WritePacket(const WriteBuffer &packet) {
  std::string payload = packet.str();
  int32_t payloadSize = payload.size();
  DWORD actual;
  bool success = !!WriteFile(handle_, &payloadSize, sizeof(int32_t), &actual, NULL);
  if (success && actual == sizeof(int32_t)) {
    success = !!WriteFile(handle_, payload.c_str(), payloadSize, &actual, NULL);
  }
  return success;
}

int32_t PTYPipe::ReadInt32() {
  int32_t result;
  DWORD actual;
  BOOL success = !!ReadFile(handle_, &result, sizeof(int32_t), &actual, NULL);
  if (success && actual == sizeof(int32_t)) {
    return result;
  } else {
    return NULL;
  }
}

int PTYPipe::FD(){
  if (fd_ == NULL) fd_ = _open_osfhandle((intptr_t)handle_, 0);
  return fd_;
}


// ##############
// ### WinPTY ###
// ##############


WinPTY::WinPTY(int cols, int rows)
: control_(NULL),
  data_(NULL),
  cols_(cols),
  rows_(rows) { Open(); }

WinPTY::WinPTY()
: control_(NULL),
  data_(NULL){ Open(80, 30); }


WinPTY::~WinPTY() {
  delete control_;
  delete data_;
}

std::wstring WinPTY::FindAgent(){
  std::wstring progDir = directoryName(getModuleName(getCurrentModule()));
  std::wstring ret = progDir + L"\\" + AGENT_EXE;
  if (pathExists(ret)) {
    return ret;
  } else {
    return NULL;
  }
}


int WinPTY::HandleToFD(HANDLE handle) {
  return _open_osfhandle((intptr_t)handle, 0);
}

HANDLE WinPTY::FDToHandle(int fd) {
  return (HANDLE)_get_osfhandle(fd);
}

bool WinPTY::StartAgent(const BackgroundDesktop &desktop) {
  bool success;

  std::wstring agentProgram = WinPTY::FindAgent();
  std::wstringstream agentCmdLineStream;
  agentCmdLineStream << L"\"" << agentProgram << L"\" "
                     << control_->name_ << " " << data_->name_ << " "
                     << cols_ << " " << rows_;
  std::wstring agentCmdLine = agentCmdLineStream.str();

  // Start the agent.
  STARTUPINFOW sui;
  memset(&sui, 0, sizeof(sui));
  sui.cb = sizeof(sui);
  sui.lpDesktop = (LPWSTR)desktop.desktopName.c_str();
  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(pi));
  std::vector<wchar_t> cmdline(agentCmdLine.size() + 1);
  agentCmdLine.copy(&cmdline[0], agentCmdLine.size());
  cmdline[agentCmdLine.size()] = L'\0';
  success = !!CreateProcessW(agentProgram.c_str(), &cmdline[0],
                            NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi);
  if (success) {
    agent_pid_ = pi.dwProcessId;
    agent_tid_ = pi.dwThreadId;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
  return success;
}

bool WinPTY::Open(int cols, int rows) {
  cols_ = cols;
  rows_ = rows;
  return Open();
}

bool WinPTY::Open() {
  std::wstringstream pipeName;
  pipeName << L"\\\\.\\pipe\\winpty-" << GetCurrentProcessId() << L"-" << InterlockedIncrement(&consoleCounter);
  std::wstring controlName = pipeName.str() + L"-control";
  std::wstring dataName = pipeName.str() + L"-data";
  control_ = new PTYPipe(controlName, false);
  data_ = new PTYPipe(dataName, true);

  if (!control_->IsValid()) return false;
  if (!data_->IsValid())    return false;

  BackgroundDesktop desktop = setupBackgroundDesktop();

  if (!StartAgent(desktop)) return false;
  if (!control_->Connect()) return false;
  if (!data_->Connect())    return false;


  WriteBuffer packet;
  packet.putInt(AgentMsg::Ping);
  control_->WritePacket(packet);
  if (control_->ReadInt32() != 0)  return false;
  return true;
}

int WinPTY::Fork(const wchar_t *file, const wchar_t *argv, const wchar_t *cwd, const wchar_t *env) {
  WriteBuffer packet;
  packet.putInt(AgentMsg::StartProcess);
  packet.putWString(file ? file : L"");
  packet.putWString(argv ? argv : L"");
  packet.putWString(cwd ? cwd : L"");
  std::wstring envStr;
  if (env != NULL) {
    const wchar_t *p = env;
    while (*p != L'\0') {
      p += wcslen(p) + 1;
    }
    p++;
    envStr.assign(env, p);
    envStr.push_back(L'\0');
  }
  packet.putWString(envStr);
  packet.putWString(getDesktopFullName());
  control_->WritePacket(packet);
  return control_->ReadInt32();
}

int WinPTY::GetExitCode() {
  WriteBuffer packet;
  packet.putInt(AgentMsg::GetExitCode);
  control_->WritePacket(packet);
  return control_->ReadInt32();
}

int WinPTY::GetFD() {
  return data_->FD();
}

int WinPTY::GetColumns() {
  return cols_;
}

int WinPTY::GetRows() {
  return rows_;
}

std::wstring WinPTY::GetName(){
  return data_ ? data_->name_ : std::wstring(L"");
}

void WinPTY::SetColumns(int cols) {
  cols_ = cols;
  Resize(cols_, rows_);
}
void WinPTY::SetRows(int rows) {
  rows_ = rows;
  Resize(cols_, rows_);
}

int WinPTY::Resize(int cols, int rows) {
  cols_ = cols;
  rows_ = rows;
  WriteBuffer packet;
  packet.putInt(AgentMsg::SetSize);
  packet.putInt(cols);
  packet.putInt(rows);
  control_->WritePacket(packet);
  return control_->ReadInt32();
}

void WinPTY::Close() {
  delete this;
}
