
#include <node.h>
#include <uv.h>
#include <node_object_wrap.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <algorithm>

#include "Caffelib.hpp"

using namespace std;
using namespace v8;


/**
 This is the main class to represent a caffe net
*/

class WrappedCaffe : public node::ObjectWrap
{
  public:
    /*
       Initialize the prototype of class Array. Called when the module is loaded.
       ALl the methods and attributes are defined here.
    */
    static void Init(v8::Local<v8::Object> exports, Local<Object> module) {
      Isolate* isolate = exports->GetIsolate();

      // Prepare constructor template and name of the class
      Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
      tpl->SetClassName(String::NewFromUtf8(isolate, "Caffe"));
      tpl->InstanceTemplate()->SetInternalFieldCount(1);

      // Prototype - methods. These can be called from javascript
      NODE_SET_PROTOTYPE_METHOD(tpl, "toString", ToString);
      NODE_SET_PROTOTYPE_METHOD(tpl, "inspect", Inspect);
      NODE_SET_PROTOTYPE_METHOD(tpl, "setMeanFile", SetMeanFile);
      NODE_SET_PROTOTYPE_METHOD(tpl, "setTrainedFile", SetTrainedFile ) ;
      NODE_SET_PROTOTYPE_METHOD(tpl, "setLabelFile", SetLabelFile ) ;
      NODE_SET_PROTOTYPE_METHOD(tpl, "processImageFile", ProcessImageFile ) ;

      NODE_SET_METHOD(exports, "net", Net);

      // define how we access the attributes
      tpl->InstanceTemplate()->SetAccessor(String::NewFromUtf8(isolate, "name"), GetCoeff, SetCoeff);
      tpl->InstanceTemplate()->SetAccessor(String::NewFromUtf8(isolate, "mode"), GetCoeff, SetCoeff);

      constructor.Reset(isolate, tpl->GetFunction());
      exports->Set(String::NewFromUtf8(isolate, "Caffe"), tpl->GetFunction());
    }
    /*
	The nodejs constructor. 
	@see New
    */
    static Local<Object> NewInstance(const FunctionCallbackInfo<Value>& args);

  private:
   /*
	The C++ constructor.
   */
    explicit WrappedCaffe( const char *protofile ) {
      name_ = NULL ;
      net_ = caffenet::create( protofile ) ;
    }
    /*
	The destructor needs to free anything
    */
    ~WrappedCaffe() { 
        delete name_ ;
	delete net_ ;
    }

    /*
	The javascript constructor.
    */
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args) {
      if (args.IsConstructCall()) {
        if( !args[0]->IsUndefined() ) {
          Isolate* isolate = args.GetIsolate();
          Local<Context> context = isolate->GetCurrentContext() ;
          Local<String> string = args[0]->ToString(context).ToLocalChecked() ;
  	  char *c = new char[ string->Utf8Length() + 16 ] ;
	  string->WriteUtf8( c ) ;
          WrappedCaffe* self = new WrappedCaffe( c ) ;
          self->Wrap( args.This() ) ;
	  delete c ;
        }
        args.GetReturnValue().Set( args.This() ) ;
      }
    }

/*-----------------------------------------------
 ATTRIBUTES    
-----------------------------------------------*/
    char *name_  ; 	/*< Name of this net */
    caffenet::CaffeNet *net_;  	/*< the actual caffe net */


/*-----------------------------------------------
 JS METHODS    
-----------------------------------------------*/

    static void ToString(const FunctionCallbackInfo<Value>& args);
    static void Inspect(const FunctionCallbackInfo<Value>& args);
    static void Net(const FunctionCallbackInfo<Value>& args );
    static void SetMeanFile(const FunctionCallbackInfo<Value>& args );
    static void SetTrainedFile( const FunctionCallbackInfo<v8::Value>& args ) ;
    static void SetLabelFile( const FunctionCallbackInfo<v8::Value>& args ) ;
    static void ProcessImageFile( const FunctionCallbackInfo<v8::Value>& args ) ;

    static void GetCoeff(Local<String> property, const PropertyCallbackInfo<Value>& info);
    static void SetCoeff(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void>& info);

    static v8::Persistent<v8::Function> constructor; /**< a nodejs constructor for this object */
} ;
Persistent<Function> WrappedCaffe::constructor;



Local<Object> WrappedCaffe::NewInstance(const FunctionCallbackInfo<Value>& args)
{
printf( "In NewInstance\n" ) ;

  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext() ;
  EscapableHandleScope scope(isolate) ; ;

  const unsigned argc = 2;
  Local<Value> argv[argc] = { args[0], args[1] };
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  MaybeLocal<Object> instance = cons->NewInstance(context, argc, argv);

  return scope.Escape(instance.ToLocalChecked() );
}

/** 
	returns a string representation of the target 
*/
void WrappedCaffe::ToString( const v8::FunctionCallbackInfo<v8::Value>& args )
{
  Isolate* isolate = args.GetIsolate();
//  Local<Context> context = isolate->GetCurrentContext() ;

  WrappedCaffe* self = ObjectWrap::Unwrap<WrappedCaffe>(args.Holder());
  char *rc = new char[ 1000 ] ;   // allocate a big array.
  if( self->name_ != NULL ) {
  	sprintf( rc, "[ %s ] ", self->name_ ) ;
  } else if( self->net_ != NULL ) {
	sprintf( rc, self->net_->getName() ) ;
  } else {
	sprintf( rc, "?? UNNAMED ??" ) ;
  }
  args.GetReturnValue().Set( String::NewFromUtf8( isolate, rc) );
  delete  rc ;
}

/** internally calls ToString. @see ToString */
void WrappedCaffe::Inspect( const v8::FunctionCallbackInfo<v8::Value>& args )
{
  ToString( args ) ;
}



void WrappedCaffe::Net( const FunctionCallbackInfo<v8::Value>& args )
{
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext() ;
  EscapableHandleScope scope(isolate) ; ;


printf( "In Net( %d)\n", args[0]->IsUndefined()  ) ;
        if( !args[0]->IsUndefined() ) {
          Isolate* isolate = args.GetIsolate();
          Local<Context> context = isolate->GetCurrentContext() ;
          Local<String> string = args[0]->ToString(context).ToLocalChecked() ;
  	  char *c = new char[ string->Utf8Length() + 16 ] ;
	  string->WriteUtf8( c ) ;
          WrappedCaffe* self = new WrappedCaffe( c ) ;
          self->Wrap( args.This() ) ;
          args.GetReturnValue().Set( args.This() ) ;
	  delete c ;
        }

  const unsigned argc = 0;
  Local<Value> argv[argc] = {};
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked() ;

  scope.Escape(instance);

  WrappedCaffe* self = node::ObjectWrap::Unwrap<WrappedCaffe>( instance ) ;

  args.GetReturnValue().Set( instance );
}



void WrappedCaffe::SetMeanFile( const FunctionCallbackInfo<v8::Value>& args )
{
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext() ;
  EscapableHandleScope scope(isolate) ; ;

  WrappedCaffe* self = node::ObjectWrap::Unwrap<WrappedCaffe>( args.Holder() ) ;

  if( !args[0]->IsUndefined() ) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext() ;
    Local<String> string = args[0]->ToString(context).ToLocalChecked() ;
    char *c = new char[ string->Utf8Length() + 16 ] ;
    string->WriteUtf8( c ) ;
    self->net_->setMeanFile( c ) ;
    args.GetReturnValue().Set( args.Holder() ) ;
    delete c ;
  }
}



void WrappedCaffe::ProcessImageFile( const FunctionCallbackInfo<v8::Value>& args )
{
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext() ;
  EscapableHandleScope scope(isolate) ; ;

  WrappedCaffe* self = node::ObjectWrap::Unwrap<WrappedCaffe>( args.Holder() ) ;

  if( !args[0]->IsUndefined() ) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext() ;
    Local<String> string = args[0]->ToString(context).ToLocalChecked() ;
    char *c = new char[ string->Utf8Length() + 16 ] ;
    string->WriteUtf8( c ) ;
    Local<Array> arr = Array::New( isolate, 5 ) ;
    char **rc = self->net_->processImageFile( c, 5 ) ;
    for( int i=0 ; i<5 ; i++ ) {
	arr->Set( context, i, String::NewFromUtf8(isolate, rc[i] ) ) ;
	delete rc[i] ;
    }
    delete rc ;
    args.GetReturnValue().Set( arr ) ;
    delete c ;
  }
}



void WrappedCaffe::SetTrainedFile( const FunctionCallbackInfo<v8::Value>& args )
{
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext() ;
  EscapableHandleScope scope(isolate) ; ;

  WrappedCaffe* self = node::ObjectWrap::Unwrap<WrappedCaffe>( args.Holder() ) ;

  if( !args[0]->IsUndefined() ) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext() ;
    Local<String> string = args[0]->ToString(context).ToLocalChecked() ;
    char *c = new char[ string->Utf8Length() + 16 ] ;
    string->WriteUtf8( c ) ;
    self->net_->setTrainedFile( c ) ;
    args.GetReturnValue().Set( args.Holder() ) ;
    delete c ;
  }
}



void WrappedCaffe::SetLabelFile( const FunctionCallbackInfo<v8::Value>& args )
{
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext() ;
  EscapableHandleScope scope(isolate) ; ;

  WrappedCaffe* self = node::ObjectWrap::Unwrap<WrappedCaffe>( args.Holder() ) ;

  if( !args[0]->IsUndefined() ) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext() ;
    Local<String> string = args[0]->ToString(context).ToLocalChecked() ;
    char *c = new char[ string->Utf8Length() + 16 ] ;
    string->WriteUtf8( c ) ;
    self->net_->setLabelFile( c ) ;
    args.GetReturnValue().Set( args.Holder() ) ;
    delete c ;
  }
}






/*
	This is a nodejs defined method to get attributes. 
*/
void WrappedCaffe::GetCoeff(Local<String> property, const PropertyCallbackInfo<Value>& info)
{
  Isolate* isolate = info.GetIsolate();
  //Local<Context> context = isolate->GetCurrentContext() ;
  WrappedCaffe* obj = ObjectWrap::Unwrap<WrappedCaffe>(info.This());

  v8::String::Utf8Value s(property);
  std::string str(*s);

  if (str == "name" && obj->name_ != NULL ) {
    info.GetReturnValue().Set( String::NewFromUtf8(isolate, obj->name_) ) ;
  } else if (str == "mode" && obj->net_ != NULL ) {
    info.GetReturnValue().Set( String::NewFromUtf8(isolate, obj->net_->isGpuMode() ? "GPU" : "CPU" ) ) ;
  }

}




/*
	This is a nodejs defined method to set writeable attributes. 
*/
void WrappedCaffe::SetCoeff(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
{
  WrappedCaffe* self = ObjectWrap::Unwrap<WrappedCaffe>(info.This());

  v8::String::Utf8Value s(property);
  std::string str(*s);

  if ( str == "name" ) {
    char *c = new char[value->ToString()->Length()] ;
    value->ToString()->WriteUtf8( c, 32 ) ;
    delete self->name_ ;
    self->name_ = c;
  } else if ( str == "mode" ) {
    char c[10] ;
    value->ToString()->WriteUtf8( c, 1 ) ;
    if( self->net_ ) {
      self->net_->setGpuMode( c[0]=='G' ) ;
    }
  }


}

/*
	The module init script - called by nodejs at load time
*/
void InitCaffe(Local<Object> exports, Local<Object> module)
{
  WrappedCaffe::Init(exports, module);
}


NODE_MODULE(caffenet, InitCaffe)


