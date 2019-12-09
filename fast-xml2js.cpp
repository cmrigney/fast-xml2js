// fast-xml2js.cpp
#include <node.h>
#include "rapidxml/rapidxml_utils.hpp"
#include <stack>
#include <cstring>

namespace fastxml2js {

using v8::Function;
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::Context;
using v8::String;
using v8::Number;
using v8::Value;
using v8::Array;

using namespace rapidxml;

void ParseString(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if(args.Length() != 2)
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong number of arguments")));
    return;
  }

  if(!args[0]->IsString())
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "First argument must be a string")));
    return;
  }

  if(!args[1]->IsFunction())
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Second argument must be a callback")));
    return;
  }

  Local<Context> context = Context::New(isolate);

  auto param1 = args[0]->ToString(context).ToLocalChecked();
  char *xml = new char[param1->Utf8Length(isolate) + 1];
  param1->WriteUtf8(isolate, xml);

  Local<Object> obj = Object::New(isolate);
  Local<Value> errorString = Null(isolate);

  try
  {

    xml_document<> doc;
    doc.parse<0>(xml);

    std::stack<xml_node<> *> nodeStack;
    std::stack<Local<Object>> objStack;

    nodeStack.push(doc.first_node());
    objStack.push(obj);

    while(nodeStack.size() > 0)
    {
      xml_node<> *node = nodeStack.top();
      if(!node)
      {
        nodeStack.pop();
        objStack.pop();
        continue;
      }

      Local<Object> obj = objStack.top();

      Local<Object> newObj = Object::New(isolate);

      bool hasChild = false;

      //Need to reduce duplicate code here
      if(!node->first_node() || (node->first_node() && node->first_node()->type() != node_cdata && node->first_node()->type() != node_data))
      {
        hasChild = true;

        Local<Array> lst;
        if(node != doc.first_node())
        {

          bool hasProperty = obj->HasOwnProperty(context, String::NewFromUtf8(isolate, node->name())).FromMaybe(false);
          if(hasProperty)
          {
            lst = Local<Array>::Cast(obj->Get(String::NewFromUtf8(isolate, node->name())));
            lst->Set(String::NewFromUtf8(isolate, "length"), Number::New(isolate, lst->Length() + 1));
          }
          else
          {
            lst = Array::New(isolate, 1);
            obj->Set(String::NewFromUtf8(isolate, node->name()), lst);
          }

          lst->Set(lst->Length()-1, newObj);
        }
        else
        {
          obj->Set(String::NewFromUtf8(isolate, node->name()), newObj);
        }
      }
      else
      {
        Local<Array> lst;
        if(node != doc.first_node())
        {
          bool hasProperty = obj->HasOwnProperty(context, String::NewFromUtf8(isolate, node->name())).FromMaybe(false);
          if(hasProperty)
          {
            lst = Local<Array>::Cast(obj->Get(String::NewFromUtf8(isolate, node->name())));
            lst->Set(String::NewFromUtf8(isolate, "length"), Number::New(isolate, lst->Length() + 1));
          }
          else
          {
            lst = Array::New(isolate, 1);
            obj->Set(String::NewFromUtf8(isolate, node->name()), lst);
          }

          if(node->first_attribute()) {
            Local<Object> attrObj = Object::New(isolate);
            newObj->Set(String::NewFromUtf8(isolate, "_"), String::NewFromUtf8(isolate, node->first_node()->value()));
            newObj->Set(String::NewFromUtf8(isolate, "$"), attrObj);

            for(xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
            {
              attrObj->Set(String::NewFromUtf8(isolate, attr->name()), String::NewFromUtf8(isolate, attr->value()));
            }

            lst->Set(lst->Length()-1, newObj);
          }
          else {
            lst->Set(lst->Length()-1, String::NewFromUtf8(isolate, node->first_node()->value()));
          }
        }
        else
        {
          obj->Set(String::NewFromUtf8(isolate, node->name()), String::NewFromUtf8(isolate, node->first_node()->value()));
        }
      }

      nodeStack.pop();
      nodeStack.push(node->next_sibling());

      if(hasChild) {

        if(node->first_attribute())
        {
          Local<Object> attrObj = Object::New(isolate);
          newObj->Set(String::NewFromUtf8(isolate, "$"), attrObj);

          for(xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
          {
            attrObj->Set(String::NewFromUtf8(isolate, attr->name()), String::NewFromUtf8(isolate, attr->value()));
          }
        }

        nodeStack.push(node->first_node());
        objStack.push(newObj);
      }

    }

  }
  catch (const std::runtime_error& e)
  {
    errorString = String::NewFromUtf8(isolate, e.what());
  }
  catch (const rapidxml::parse_error& e)
  {
    errorString = String::NewFromUtf8(isolate, e.what());
  }
  catch (const std::exception& e)
  {
    errorString = String::NewFromUtf8(isolate, e.what());
  }
  catch (const Local<Value>& e)
  {
    errorString = e;
  }
  catch (...)
  {
    errorString = String::NewFromUtf8(isolate, "An unknown error occurred while parsing.");
  }

  delete[] xml;

  Local<Function> cb = Local<Function>::Cast(args[1]);
  const unsigned argc = 2;
  Local<Value> argv[argc] = { errorString, obj };

  cb->Call(context, Null(isolate), argc, argv);
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "parseString", ParseString);
}

NODE_MODULE(fastxml2js, init)

}  // namespace fastxml2js
