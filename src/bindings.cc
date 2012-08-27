#include "bindings.h"
#include "winpty.h"
#include <node.h>
#include <node_buffer.h>
#include <vector>
#include <string>
#include <v8_templates.h>

using namespace v8;
using namespace node;


Persistent<Object> PTYWrap::exports;
Persistent<Function> PTYWrap::constructor;

PTYWrap::PTYWrap(){}
PTYWrap::~PTYWrap(){}


Handle<Value> PTYWrap::FDToHandle(const Arguments& args) {
  HandleScope scope;
  return scope.Close(Integer::New((intptr_t)WinPTY::FDToHandle(args[0]->IntegerValue())));
}

Handle<Value> PTYWrap::HandleToFD(const Arguments& args) {
  HandleScope scope;
  return scope.Close(Integer::New(WinPTY::HandleToFD((HANDLE)args[0]->IntegerValue())));
}

Handle<Value> PTYWrap::New(const Arguments& args) {
  HandleScope scope;
  WinPTY* pty;
  if (args[0]->IsNumber() && args[1]->IsNumber()) {
    pty = new WinPTY(args[0]->IntegerValue(), args[1]->IntegerValue());
  } else {
    pty = new WinPTY();
  }

  args.This()->SetPointerInInternalField(0, pty);
  return scope.Close(args.This());
}

Handle<Value> PTYWrap::NewInstance(const Arguments& args) {
  HandleScope scope;
  Handle<Value> argv[2] = { args[0], args[1] };
  return scope.Close(constructor->NewInstance(2, argv));
}


template <typename dataType>
void GetBuffer(dataType &ref, Local<Object> buff){
  if (Buffer::HasInstance(buff)) {
    ref = reinterpret_cast<dataType>(Buffer::Data(buff));
  }
}

Handle<Value> PTYWrap::Fork(const Arguments& args) {
  HandleScope scope;
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());

  Local<Object> file_v = Call(exports, "formatFile", args[0]->ToString());
  Local<Object> argv_v = Call(exports, "formatArgs", Local<Array>::Cast(args[1]->ToObject()));
  Local<Object> cwd_v = Call(exports, "formatCwd", args[2]->ToString());
  Local<Object> env_v = Call(exports, "formatEnv", args[3]->ToObject());
  wchar_t* file = L"";
  wchar_t* argv = L"";
  wchar_t* cwd = L"";
  wchar_t* env = L"";
  GetBuffer(file, file_v);
  GetBuffer(argv, argv_v);
  GetBuffer(cwd, cwd_v);
  GetBuffer(env, env_v);

  int result = pty->Fork(file, argv, cwd, env);

  return scope.Close(ToV8(result));
}


Handle<Value> PTYWrap::Open(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  if (args[0]->IsNumber() && args[1]->IsNumber()) {
    return ToV8(pty->Open(args[0]->Int32Value(), args[1]->Int32Value()));
  } else {
    return ToV8(pty->Open());
  }
}

Handle<Value> PTYWrap::Close(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  pty->Close();
  Set(args.This(), "closed", true);
  return Undefined();
}

Handle<Value> PTYWrap::GetExitCode(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  return ToV8(pty->GetExitCode());
}

Handle<Value> PTYWrap::FindAgent(const Arguments& args) {
  return ToV8(WinPTY::FindAgent());
}


Handle<Value> PTYWrap::Resize(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  return ToV8(pty->Resize(args[0]->Int32Value(), args[1]->Int32Value()));
}




Handle<Value> PTYWrap::GetFD(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  if (pty->GetFD()) {
    return ToV8(pty->GetFD());
  } else {
    return Null();
  }
}

Handle<Value> PTYWrap::GetColumns(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  return ToV8(pty->GetColumns());
}

Handle<Value> PTYWrap::GetRows(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  return ToV8(pty->GetRows());
}

Handle<Value> PTYWrap::GetName(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  return ToV8(pty->GetName());
}

Handle<Value> PTYWrap::GetAgentPID(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  if (pty->agent_pid_) {
    return ToV8(pty->agent_pid_);
  } else {
    return Null();
  }
}

Handle<Value> PTYWrap::SetColumns(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  pty->SetColumns(args[0]->IntegerValue());
  return args.This();
}

Handle<Value> PTYWrap::SetRows(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  pty->SetRows(args[0]->IntegerValue());
  return args.This();
}


void PTYWrap::Init(Handle<Object> target) {
  HandleScope scope;
  exports = Persistent<Object>::New(target);
  Local<FunctionTemplate> ctor = JSClass("WinPTY", New, 1);
  SetMethod(ctor, "handleToFD", HandleToFD);
  SetMethod(ctor, "fdToHandle", FDToHandle);
  SetMethod(ctor, "agentPath", FindAgent);
  //SetMethod(ctor, "isTTY", );
  //SetMethod(ctor, "guessHandleType", );

  SetPrototypeMethod(ctor, "open", Open);
  SetPrototypeMethod(ctor, "fork", Fork);
  SetPrototypeMethod(ctor, "close", Close);
  SetPrototypeMethod(ctor, "resize", Resize);
  SetPrototypeMethod(ctor, "getExitCode", GetExitCode);
  SetPrototypeMethod(ctor, "setRows", SetRows);
  SetPrototypeMethod(ctor, "setColumns", SetColumns);
  SetPrototypeMethod(ctor, "getRows", GetRows);
  SetPrototypeMethod(ctor, "getColumns", GetColumns);
  SetPrototypeMethod(ctor, "agentPID", GetAgentPID);
  SetPrototypeMethod(ctor, "name", GetName);
  SetPrototypeMethod(ctor, "fd", GetFD);
  //SetPrototypeMethod(ctor, "unref", );
  //SetPrototypeMethod(ctor, "readStart", );
  //SetPrototypeMethod(ctor, "readStop", );
  //SetPrototypeMethod(ctor, "writeBuffer", );
  //SetPrototypeMethod(ctor, "writeAsciiString", );
  //SetPrototypeMethod(ctor, "writeUtf8String", );
  //SetPrototypeMethod(ctor, "writeUcs2String", );
  //SetPrototypeMethod(ctor, "getWindowSize", );
  //SetPrototypeMethod(ctor, "setRawMode", );

  constructor = Persistent<Function>::New(ctor->GetFunction());
  Set(exports, constructor);
}


void init(Handle<Object> target) {
  PTYWrap::Init(target);
}
NODE_MODULE(winpty, init);
