// fast-xml2js.cpp
#include <node.h>
#include <nan.h>

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
using v8::NewStringType;

using namespace rapidxml;

Local<String> str(Isolate* isolate, const char *value) {
  return String::NewFromUtf8(
    isolate,
    value,
    NewStringType::kNormal,
    static_cast<int>(strlen(value))
  ).ToLocalChecked();
}


Local<String> strNode(Isolate* isolate, xml_node<> *node) {
  return str(isolate, node->name());
}

void ParseString(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if(args.Length() != 2)
  {
    isolate->ThrowException(Exception::TypeError(
      str(isolate, "Wrong number of arguments")));
    return;
  }

  if(!args[0]->IsString())
  {
    isolate->ThrowException(Exception::TypeError(
      str(isolate, "First argument must be a string")));
    return;
  }

  if(!args[1]->IsFunction())
  {
    isolate->ThrowException(Exception::TypeError(
      str(isolate, "Second argument must be a callback")));
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

          bool hasProperty = obj->HasOwnProperty(context, strNode(isolate, node)).FromMaybe(false);

          if(hasProperty)
          {
            lst = Local<Array>::Cast(Nan::Get(obj, strNode(isolate, node)).ToLocalChecked());
            Nan::Set(lst, str(isolate, "length"), Number::New(isolate, lst->Length() + 1));
          }
          else
          {
            lst = Array::New(isolate, 1);
            Nan::Set(obj, strNode(isolate, node), lst);
          }

          Nan::Set(lst, lst->Length()-1, newObj);
        }
        else
        {
          Nan::Set(obj, strNode(isolate, node), newObj);
        }
      }
      else
      {
        Local<Array> lst;
        if(node != doc.first_node())
        {
          bool hasProperty = obj->HasOwnProperty(context,  strNode(isolate, node)).FromMaybe(false);
          if(hasProperty)
          {
            lst = Local<Array>::Cast(Nan::Get(obj, str(isolate, node->name())).ToLocalChecked());
            Nan::Set(lst, str(isolate, "length"), Number::New(isolate, lst->Length() + 1));
          }
          else
          {
            lst = Array::New(isolate, 1);
            Nan::Set(obj, strNode(isolate, node), lst);
          }

          if(node->first_attribute()) {
            Local<Object> attrObj = Object::New(isolate);
            Nan::Set(newObj, str(isolate, "_"), str(isolate, node->first_node()->value()));
            Nan::Set(newObj, str(isolate, "$"), attrObj);

            for(xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
            {
              Nan::Set(attrObj, str(isolate, attr->name()), str(isolate, attr->value()));
            }

            Nan::Set(lst, lst->Length()-1, newObj);
          }
          else {
            Nan::Set(lst, lst->Length()-1, str(isolate, node->first_node()->value()));
          }
        }
        else
        {
          Nan::Set(obj, str(isolate, node->name()), str(isolate, node->first_node()->value()));
        }
      }

      nodeStack.pop();
      nodeStack.push(node->next_sibling());

      if(hasChild) {

        if(node->first_attribute())
        {
          Local<Object> attrObj = Object::New(isolate);
          Nan::Set(newObj, str(isolate, "$"), attrObj);

          for(xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
          {
            Nan::Set(attrObj, str(isolate, attr->name()), str(isolate, attr->value()));
          }
        }

        nodeStack.push(node->first_node());
        objStack.push(newObj);
      }

    }

  }
  catch (const std::runtime_error& e)
  {
    errorString = str(isolate, e.what());
  }
  catch (const rapidxml::parse_error& e)
  {
    errorString = str(isolate, e.what());
  }
  catch (const std::exception& e)
  {
    errorString = str(isolate, e.what());
  }
  catch (const Local<Value>& e)
  {
    errorString = e;
  }
  catch (...)
  {
    errorString = str(isolate, "An unknown error occurred while parsing.");
  }

  delete[] xml;

  Local<Function> cb = Local<Function>::Cast(args[1]);
  const unsigned argc = 2;
  Local<Value> argv[argc] = { errorString, obj };

  Nan::Callback callback(cb);
  Nan::Call(callback, argc, argv);
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "parseString", ParseString);
}

NODE_MODULE(fastxml2js, init)

}  // namespace fastxml2js
