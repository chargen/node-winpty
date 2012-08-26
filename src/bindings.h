#ifndef WINPTY_BINDINGS_H
#define WINPTY_BINDINGS_H

#include <v8.h>
#include <node.h>
#include "winpty.h"

using namespace v8;

void init(Handle<Object> target);

class PTYWrap : public node::ObjectWrap {
 public:
  PTYWrap();
  ~PTYWrap();

  WinPTY* handle_;

  static void Init(Handle<Object> target);
  static Handle<Value> New(const Arguments& args);
  static Handle<Value> FindAgent(const Arguments& args);
  static Handle<Value> NewInstance(const Arguments& args);
  static Handle<Value> StartProcess(const Arguments& args);
  static Handle<Value> GetExitCode(const Arguments& args);
  static Handle<Value> GetDataPipe(const Arguments& args);
  static Handle<Value> Resize(const Arguments& args);
  static Handle<Value> Close(const Arguments& args);
  static Handle<Value> Open(const Arguments& args);

  static Persistent<Object> exports;
  static Persistent<Function> constructor;
};


#endif
