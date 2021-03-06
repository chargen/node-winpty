/*
 * Copyright (c) 2011-2012 Ryan Prichard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef WINPTY_H
#define WINPTY_H

#include <windows.h>
#include <string>
#include <Buffer.h>

struct AgentMsg {
  enum Type {
    Ping,
    StartProcess,
    SetSize,
    GetExitCode
  };
};


struct BackgroundDesktop {
  HWINSTA originalStation;
  HWINSTA station;
  HDESK desktop;
  std::wstring desktopName;
};


class PTYPipe {
 public:
  PTYPipe(std::wstring &name, bool overlapped);
  ~PTYPipe();

  bool IsValid();
  bool Connect();
  bool WritePacket(const WriteBuffer &packet);
  int32_t ReadInt32();
  int FD();

  HANDLE handle_;
  std::wstring &name_;
  int fd_;
  bool overlapped_;
  bool connected_;
};


class WinPTY {
 public:
  WinPTY(int cols, int rows);
  WinPTY();
  ~WinPTY();

  static std::wstring FindAgent();

  bool StartAgent(const BackgroundDesktop &desktop);
  static int HandleToFD(HANDLE handle);
  static HANDLE FDToHandle(int fd);
  bool Open();
  bool Open(int cols, int rows);
  void Close();
  int Fork(const wchar_t *file, const wchar_t *argv, const wchar_t *cwd, const wchar_t *env);
  int Resize(int cols, int rows);
  int GetExitCode();
  int GetFD();
  int GetColumns();
  int GetRows();
  std::wstring GetName();
  void SetColumns(int cols);
  void SetRows(int rows);
  int cols_;
  int rows_;
  int agent_pid_;
  int agent_tid_;
  PTYPipe* control_;
  PTYPipe* data_;
};


/*
 * winpty API.
 */

/*
 * Starts a new winpty instance with the given size.
 *
 * This function creates a new agent process and connects to it.
 */

/*
 * Start a child process.  Either (but not both) of appname and cmdline may
 * be NULL.  cwd and env may be NULL.  env is a pointer to an environment
 * block like that passed to CreateProcess.
 *
 * This function never modifies the cmdline, unlike CreateProcess.
 *
 * Only one child process may be started.  After the child process exits, the
 * agent will scrape the console output one last time, then close the data pipe
 * once all remaining data has been sent.
 *
 * Returns 0 on success or a Win32 error code on failure.
 */

/*
 * Returns the exit code of the process started with winpty_start_process,
 * or -1 none is available.
 */

/*
 * Returns an overlapped-mode pipe handle that can be read and written
 * like a Unix terminal.
 */

/*
 * Change the size of the Windows console.
 */
/*
 * Closes the winpty.
 */


#endif /* WINPTY_H */
