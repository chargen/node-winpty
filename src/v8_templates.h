#ifndef V8_TEMPLATES_H
#define V8_TEMPLATES_H

#include <v8.h>
#include <io.h>


#define LT Local
#define HT Handle
#define PT Persistent
#define V Value
#define F Function
#define S String
#define C(k) String::NewSymbol(k)
#define OBJ(o) (o)->ToObject()


#define PLANE(T, Z, TT) \
void Set(HT<T> o, HT<S> k, TT v) { OBJ(o)->Set(k, Z::New(v)); }      \
void Set(PT<T> o, HT<S> k, TT v) { OBJ(o)->Set(k, Z::New(v)); }      \
void Set(HT<T> o, PT<S> k, TT v) { OBJ(o)->Set(k, Z::New(v)); }      \
void Set(PT<T> o, PT<S> k, TT v) { OBJ(o)->Set(k, Z::New(v)); }      \
void Set(HT<T> o, char* k, TT v) { OBJ(o)->Set(C(k), Z::New(v)); }   \
void Set(PT<T> o, char* k, TT v) { OBJ(o)->Set(C(k), Z::New(v)); }   \
//void Set(LT<T> o, LT<S> k, TT v) { OBJ(o)->Set(k, Z::New(v)); }      \
//void Set(LT<T> o, HT<S> k, TT v) { OBJ(o)->Set(k, Z::New(v)); }      \
//void Set(LT<T> o, PT<S> k, TT v) { OBJ(o)->Set(k, Z::New(v)); }      \
//void Set(LT<T> o, char* k, TT v) { OBJ(o)->Set(C(k), Z::New(v)); }   \
//void Set(HT<T> o, LT<S> k, TT v) { OBJ(o)->Set(k, Z::New(v)); }      \
//void Set(PT<T> o, LT<S> k, TT v) { OBJ(o)->Set(k, Z::New(v)); }      \


#define BOAT(T, TT) \
void Set(HT<T> o, HT<S> k, TT v) { OBJ(o)->Set(k,v); }    \
void Set(PT<T> o, HT<S> k, TT v) { OBJ(o)->Set(k,v); }    \
void Set(HT<T> o, PT<S> k, TT v) { OBJ(o)->Set(k,v); }    \
void Set(PT<T> o, PT<S> k, TT v) { OBJ(o)->Set(k,v); }    \
void Set(HT<T> o, char* k, TT v) { OBJ(o)->Set(C(k),v); } \
void Set(PT<T> o, char* k, TT v) { OBJ(o)->Set(C(k),v); } \
//void Set(HT<T> o, LT<S> k, TT v) { OBJ(o)->Set(k,v); }    \
//void Set(PT<T> o, LT<S> k, TT v) { OBJ(o)->Set(k,v); }    \
//void Set(LT<T> o, LT<S> k, TT v) { OBJ(o)->Set(k,v); }    \
//void Set(LT<T> o, HT<S> k, TT v) { OBJ(o)->Set(k,v); }    \
//void Set(LT<T> o, PT<S> k, TT v) { OBJ(o)->Set(k,v); }    \
//void Set(LT<T> o, char* k, TT v) { OBJ(o)->Set(C(k),v); } \


#define WHEEL(T) \
LT<V> Get(HT<T> o, char* k) { return OBJ(o)->Get(C(k)); }  \
LT<V> Get(PT<T> o, char* k) { return OBJ(o)->Get(C(k)); }  \
LT<V> Get(HT<T> o, HT<S> k) { return OBJ(o)->Get(k); }     \
LT<V> Get(PT<T> o, HT<S> k) { return OBJ(o)->Get(k); }     \
BOAT(T, HT<V>) \
BOAT(T, PT<V>) \
PLANE(T, Boolean, bool) \
PLANE(T, Int32, int) \
PLANE(T, Number, float) \
void Set(HT<T> o, HT<F> v) { OBJ(o)->Set(v->GetName(), v); } \
void Set(PT<T> o, HT<F> v) { OBJ(o)->Set(v->GetName(), v); } \
void Set(HT<T> o, PT<F> v) { OBJ(o)->Set(v->GetName(), v); } \
void Set(PT<T> o, PT<F> v) { OBJ(o)->Set(v->GetName(), v); } \

//void Set(HT<T> o, LT<F> v) { OBJ(o)->Set(v->GetName(), v); } \
//void Set(PT<T> o, LT<F> v) { OBJ(o)->Set(v->GetName(), v); } \
//LT<V> Get(HT<T> o, LT<S> k) { return OBJ(o)->Get(k); }     \
//LT<V> Get(PT<T> o, LT<S> k) { return OBJ(o)->Get(k); }     \
//BOAT(T, LT<V>) \
//LT<V> Get(LT<T> o, LT<S> k) { return OBJ(o)->Get(k); }     \
//LT<V> Get(LT<T> o, char* k) { return OBJ(o)->Get(C(k)); }  \
//LT<V> Get(LT<T> o, HT<S> k) { return OBJ(o)->Get(k); }     \
//void Set(LT<T> o, HT<F> v) { OBJ(o)->Set(v->GetName(), v); } \
//void Set(LT<T> o, LT<F> v) { OBJ(o)->Set(v->GetName(), v); } \
//void Set(LT<T> o, PT<F> v) { OBJ(o)->Set(v->GetName(), v); } \


Handle<String> ToV8(const std::wstring& val) {
  return String::New(reinterpret_cast<uint16_t*>(const_cast<wchar_t*>(val.c_str())), val.length());
}

Handle<String> ToV8(const std::string& val) {
  return String::New(val.c_str());
}


Handle<Integer> ToV8(long val) {
  return Integer::New(val);
}

Handle<Integer> ToV8(uint32_t val) {
  return Integer::NewFromUnsigned(val);
}

Handle<Number> ToV8(double val) {
  return Number::New(val);
}

Handle<Boolean> ToV8(bool val) {
  return Boolean::New(val);
}

Handle<Integer> ToV8(intptr_t val) {
  return Integer::New(_open_osfhandle(val, 0));
}







WHEEL(Value)
WHEEL(Object)
WHEEL(Function)
WHEEL(Array)







template <typename t_t, typename v_t>
Local<Object> Call(t_t o, char* key, v_t arg) {
  HandleScope scope;
  Local<Object> obj = o->ToObject();
  Local<Function> func = Local<Function>::Cast(obj->Get(String::NewSymbol(key)));
  Local<Value> args[] = { arg };
  return scope.Close(func->Call(obj, 1, args))->ToObject();
}

Local<Object> Call(Local<Value> o, char* key, char* arg) {
  HandleScope scope;
  Local<Object> obj = o->ToObject();
  Local<Function> func = Local<Function>::Cast(obj->Get(String::NewSymbol(key)));
  Local<Value> args[] = { String::New(arg) };
  return scope.Close(func->Call(obj, 1, args))->ToObject();
}


LT<FunctionTemplate> JSClass(char* name, InvocationCallback ctor, int internalFields) {
  LT<FunctionTemplate> constructor = FunctionTemplate::New(ctor);
  if (internalFields > 0) {
    constructor->InstanceTemplate()->SetInternalFieldCount(internalFields);
  }
  constructor->SetClassName(String::NewSymbol(name));
  return constructor;
}



#endif
