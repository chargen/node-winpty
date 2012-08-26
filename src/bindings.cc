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



Handle<Value> PTYWrap::New(const Arguments& args) {
  HandleScope scope;
  WinPTY* pty = new WinPTY();
  args.This()->SetPointerInInternalField(0, pty);
  return scope.Close(args.This());
}

Handle<Value> PTYWrap::NewInstance(const Arguments& args) {
  HandleScope scope;
  Handle<Value> argv[1] = { args[0] };
  return scope.Close(constructor->NewInstance(1, argv));
}


Handle<Value> PTYWrap::FindAgent(const Arguments& args) {
  return ToV8(WinPTY::FindAgent());
}


Handle<Value> PTYWrap::Open(const Arguments& args) {
  HandleScope scope;
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());

  int columns = args[0]->IsInt32() ? args[0]->Int32Value() : 80;
  int rows = args[1]->IsInt32() ? args[1]->Int32Value() : 30;

  return scope.Close(ToV8(pty->Open(columns, rows)));
}


Handle<Value> PTYWrap::StartProcess(const Arguments& args) {
  HandleScope scope;
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());

  Local<Object> options = args[0]->ToObject();
  wchar_t* file;
  wchar_t* cmdline;
  wchar_t* cwd;
  wchar_t* env;

  Local<Object> file_v = Call(exports, "formatFile", args[0]->ToString());
  if (Buffer::HasInstance(file_v)) {
    file = reinterpret_cast<wchar_t*>(Buffer::Data(file_v));
  }

  Local<Object> argv_v = Call(exports, "formatArgs", Local<Array>::Cast(args[0]->ToObject()));
  if (Buffer::HasInstance(argv_v)) {
    cmdline = reinterpret_cast<wchar_t*>(Buffer::Data(argv_v));
  }

  Local<Object> cwd_v = Call(exports, "formatCwd", args[0]->ToString());
  if (Buffer::HasInstance(cwd_v)) {
    cwd = reinterpret_cast<wchar_t*>(Buffer::Data(cwd_v));
  }

  Local<Object> env_v = Call(exports, "formatEnv", args[0]->ToObject());
  if (Buffer::HasInstance(env_v)) {
    env = reinterpret_cast<wchar_t*>(Buffer::Data(env_v));
  }

  int result = pty->StartProcess(file, cmdline, cwd, env);

  Set(args.This(), "writable", true);
  Set(args.This(), "readable", true);
  Set(args.This(), "status", result);

  return scope.Close(args.This());
}


Handle<Value> PTYWrap::Close(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  pty->Close();
  Set(args.This(), "writable", false);
  Set(args.This(), "readable", false);
  Set(args.This(), "closed", true);
  return Undefined();
}

Handle<Value> PTYWrap::GetExitCode(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  return ToV8(pty->GetExitCode());
}

Handle<Value> PTYWrap::GetDataPipe(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  return ToV8(pty->GetDataPipe());
}


Handle<Value> PTYWrap::Resize(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  return ToV8(pty->Resize(args[0]->Int32Value(), args[1]->Int32Value()));
}


// Handle<Value> PTYWrap::GetColumns(Local<String> property, const AccessorInfo &info) {
//   WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(info.Holder());
//   return Integer::New(pty->columns_);
// }

// Handle<Value> PTYWrap::GetRows(Local<String> property, const AccessorInfo &info) {
//   WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(info.Holder());
//   return Integer::New(pty->rows_);
// }

// void PTYWrap::SetColumns(Local<String> property, Local<Value> value, const AccessorInfo& info) {
//   WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(info.Holder());
//   pty->SetColumns(value->Int32Value());
// }

// void PTYWrap::SetRows(Local<String> property, Local<Value> value, const AccessorInfo& info) {
//   WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(info.Holder());
//   pty->SetRows(value->Int32Value());
// }




void PTYWrap::Init(Handle<Object> target) {
  HandleScope scope;
  exports = Persistent<Object>::New(target);
  Local<FunctionTemplate> ctor = JSClass("WinPTY", New, 1);
  SetMethod(ctor, "findAgent", FindAgent);
  SetPrototypeMethod(ctor, "startProcess", StartProcess);
  SetPrototypeMethod(ctor, "getExitCode", GetExitCode);
  SetPrototypeMethod(ctor, "getHandle", GetDataPipe);
  SetPrototypeMethod(ctor, "resize", Resize);
  SetPrototypeMethod(ctor, "close", Close);
  SetPrototypeMethod(ctor, "open", Open);
  constructor = Persistent<Function>::New(ctor->GetFunction());
  Set(exports, constructor);
}


void init(Handle<Object> target) {
  PTYWrap::Init(target);
}
NODE_MODULE(winpty, init);
