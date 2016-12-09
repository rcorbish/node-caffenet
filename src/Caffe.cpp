
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
      NODE_SET_PROTOTYPE_METHOD(tpl, "train", Train ) ;
      NODE_SET_PROTOTYPE_METHOD(tpl, "load", Load ) ;

      //NODE_SET_METHOD(exports, "net", Net);

      // define how we access the attributes
      tpl->InstanceTemplate()->SetAccessor(String::NewFromUtf8(isolate, "name"), GetCoeff, SetCoeff);
      tpl->InstanceTemplate()->SetAccessor(String::NewFromUtf8(isolate, "mode"), GetCoeff, SetCoeff);

      constructor.Reset(isolate, tpl->GetFunction());
      exports->Set(String::NewFromUtf8(isolate, "Caffe"), tpl->GetFunction());
    }

  private:
   /*
	The C++ constructor.
   */
    explicit WrappedCaffe( const char *protofile=NULL) {
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
	The javascript constructor. N = new c.Caffe( .... ) 
    */
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args) {
      if (args.IsConstructCall()) {
        if( args.Length()>0 && !args[0]->IsUndefined() ) {
          Isolate* isolate = args.GetIsolate();
          Local<Context> context = isolate->GetCurrentContext() ;
          Local<String> string = args[0]->ToString(context).ToLocalChecked() ;
  	  char *c = new char[ string->Utf8Length() + 16 ] ;
	  string->WriteUtf8( c ) ;
          WrappedCaffe* self = new WrappedCaffe( c ) ;
          self->Wrap( args.This() ) ;
	  delete c ;
        } else {
          WrappedCaffe* self = new WrappedCaffe() ;
          self->Wrap( args.This() ) ;
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
    static void SetMeanFile(const FunctionCallbackInfo<Value>& args );
    static void SetTrainedFile( const FunctionCallbackInfo<v8::Value>& args ) ;
    static void SetLabelFile( const FunctionCallbackInfo<v8::Value>& args ) ;
    static void ProcessImageFile( const FunctionCallbackInfo<v8::Value>& args ) ;
    static void Train( const FunctionCallbackInfo<v8::Value>& args ) ;
    static void Load( const FunctionCallbackInfo<v8::Value>& args ) ;

    static void GetCoeff(Local<String> property, const PropertyCallbackInfo<Value>& info);
    static void SetCoeff(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void>& info);

    static v8::Persistent<v8::Function> constructor; /**< a nodejs constructor for this object */
} ;
Persistent<Function> WrappedCaffe::constructor;



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
	sprintf( rc, "* %s *", self->net_->getName() ) ;
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



void WrappedCaffe::Train( const FunctionCallbackInfo<v8::Value>& args )
{
  Isolate* isolate = args.GetIsolate();
  //Local<Context> context = isolate->GetCurrentContext() ;
  EscapableHandleScope scope(isolate) ; ;

  WrappedCaffe* self = node::ObjectWrap::Unwrap<WrappedCaffe>( args.Holder() ) ;

  if( !args[0]->IsUndefined() ) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext() ;
    Local<String> string = args[0]->ToString(context).ToLocalChecked() ;
    char *c = new char[ string->Utf8Length() + 16 ] ;
    string->WriteUtf8( c ) ;
    self->net_->train( c ) ;
    args.GetReturnValue().Set( args.Holder() ) ;
    delete c ;
  }
}



void WrappedCaffe::Load( const FunctionCallbackInfo<v8::Value>& args )
{
  Isolate* isolate = args.GetIsolate();
  //Local<Context> context = isolate->GetCurrentContext() ;
  EscapableHandleScope scope(isolate) ; ;

  WrappedCaffe* self = node::ObjectWrap::Unwrap<WrappedCaffe>( args.Holder() ) ;

  if( !args[0]->IsUndefined() ) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext() ;
    Local<String> string1 = args[0]->ToString(context).ToLocalChecked() ;
    char *c1 = new char[ string1->Utf8Length() + 16 ] ;
    string1->WriteUtf8( c1 ) ;
    
    char *c2 = NULL ;
    if( !args[1]->IsUndefined() ) {
      Local<String> string2 = args[1]->ToString(context).ToLocalChecked() ;
      c2 = new char[ string2->Utf8Length() + 16 ] ;
      string2->WriteUtf8( c2 ) ;
    }
    
    self->net_->load( c1, c2 ) ;
    args.GetReturnValue().Set( args.Holder() ) ;
    delete c2 ;
    delete c1 ;
  }
}





void WrappedCaffe::SetMeanFile( const FunctionCallbackInfo<v8::Value>& args )
{
  Isolate* isolate = args.GetIsolate();
  //Local<Context> context = isolate->GetCurrentContext() ;
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


const char *Labels[] = { "airplane", "automobile", "bird", "cat", "deer", "dog", "frog", "horse", "ship", "truck" } ;


void WrappedCaffe::ProcessImageFile( const FunctionCallbackInfo<v8::Value>& args )
{
  Isolate* isolate = args.GetIsolate();
  //Local<Context> context = isolate->GetCurrentContext() ;
  EscapableHandleScope scope(isolate) ; ;

  WrappedCaffe* self = node::ObjectWrap::Unwrap<WrappedCaffe>( args.Holder() ) ;

  if( !args[0]->IsUndefined() ) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext() ;
    Local<String> string = args[0]->ToString(context).ToLocalChecked() ;
    
    char *c = new char[ string->Utf8Length() + 16 ] ;
    string->WriteUtf8( c ) ;
    
    int rc = self->net_->processImageFile( c, 5 ) ;
    args.GetReturnValue().Set( String::NewFromUtf8( isolate, Labels[rc]) ) ;
    
    delete c ;
  }
}



void WrappedCaffe::SetTrainedFile( const FunctionCallbackInfo<v8::Value>& args )
{
  Isolate* isolate = args.GetIsolate();
  //Local<Context> context = isolate->GetCurrentContext() ;
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
  //Local<Context> context = isolate->GetCurrentContext() ;
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
void InitCaffe(Local<Object> exports, Local<Object> module) {
  caffenet::Init() ;
  WrappedCaffe::Init(exports, module);
}


NODE_MODULE(caffenet, InitCaffe)


