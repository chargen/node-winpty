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

  static void Init(Handle<Object> target);
  WinPTY* handle_;

  static Handle<Value> New(const Arguments& args);
  static Handle<Value> NewInstance(const Arguments& args);
  static Handle<Value> FDToHandle(const Arguments& args);
  static Handle<Value> HandleToFD(const Arguments& args);
  static Handle<Value> GetExitCode(const Arguments& args);
  static Handle<Value> Resize(const Arguments& args);
  static Handle<Value> Close(const Arguments& args);
  static Handle<Value> Open(const Arguments& args);
  static Handle<Value> Fork(const Arguments& args);
  static Handle<Value> FindAgent(const Arguments& args);
  static Handle<Value> GetFD(const Arguments& args);
  static Handle<Value> GetColumns(const Arguments& args);
  static Handle<Value> GetRows(const Arguments& args);
  static Handle<Value> GetName(const Arguments& args);
  static Handle<Value> GetAgentPID(const Arguments& args);
  static Handle<Value> SetColumns(const Arguments& args);
  static Handle<Value> SetRows(const Arguments& args);
  static Persistent<Object> exports;
  static Persistent<Function> constructor;
};


#endif
