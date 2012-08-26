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

  Local<Object> options = args[0]->ToObject();

  wchar_t* file;
  wchar_t* cmdline;
  wchar_t* cwd;
  wchar_t* env;

  Local<Value> columns_ = Get(options, "columns");
  Local<Value> rows_ = Get(options, "rows");
  int columns = columns_->IsInt32() ? columns_->Int32Value() : 80;
  int rows = rows_->IsInt32() ? rows_->Int32Value() : 30;

  Local<Object> file_v = Call(exports, "formatFile", Get(options, "file"));
  if (Buffer::HasInstance(file_v)) {
    file = reinterpret_cast<wchar_t*>(Buffer::Data(file_v));
  }

  Local<Object> argv_v = Call(exports, "formatArgs", Get(options, "args"));
  if (Buffer::HasInstance(argv_v)) {
    cmdline = reinterpret_cast<wchar_t*>(Buffer::Data(argv_v));
  }

  Local<Object> cwd_v = Call(exports, "formatCwd", Get(options, "cwd"));
  if (Buffer::HasInstance(cwd_v)) {
    cwd = reinterpret_cast<wchar_t*>(Buffer::Data(cwd_v));
  }

  Local<Object> env_v = Call(exports, "formatEnv", Get(options, "env"));
  if (Buffer::HasInstance(env_v)) {
    env = reinterpret_cast<wchar_t*>(Buffer::Data(env_v));
  }

  WinPTY* pty = new WinPTY();
  args.This()->SetPointerInInternalField(0, pty);
  pty->Open(columns, rows);
  int result = pty->StartProcess(file, cmdline, cwd, env);

  Set(args.This(), "writable", true);
  Set(args.This(), "readable", true);
  Set(args.This(), "status", result);
  return scope.Close(args.This());
}

Handle<Value> PTYWrap::NewInstance(const Arguments& args) {
  HandleScope scope;
  Handle<Value> argv[1] = { args[0] };
  return scope.Close(constructor->NewInstance(1, argv));
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
  return Integer::New(pty->GetExitCode());
}

Handle<Value> PTYWrap::GetDataPipe(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  return Integer::New(_fileno((FILE*)pty->GetDataPipe()));
}


Handle<Value> PTYWrap::SetSize(const Arguments& args) {
  WinPTY* pty = ObjectWrap::Unwrap<WinPTY>(args.This());
  return Integer::New(pty->SetSize(args[0]->Int32Value(), args[1]->Int32Value()));
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
  SetPrototypeMethod(ctor, "getExitCode", GetExitCode);
  SetPrototypeMethod(ctor, "getHandle", GetDataPipe);
  SetPrototypeMethod(ctor, "resize", SetSize);
  SetPrototypeMethod(ctor, "close", Close);
  constructor = Persistent<Function>::New(ctor->GetFunction());
  Set(exports, constructor);
}


void init(Handle<Object> target) {
  PTYWrap::Init(target);
}
NODE_MODULE(winpty, init);
