/*

Copyright (c) 2018, Johnathan Corkery. (jcorkery@umich.edu)
All rights reserved.

This file is part of the Dynacoe project (https://github.com/jcorks/Dynacoe)
Dynacoe was released under the MIT License, as detailed below.



Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is furnished 
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall
be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.



*/


#include <Dynacoe/Variable.h>




using namespace Dynacoe;

static bool SetFromBool  (const std::string &, void *, Variable::Type);
static bool SetFromInt   (const std::string &, void *, Variable::Type);
static bool SetFromFloat (const std::string &, void *, Variable::Type);
static bool SetFromString(const std::string &, void *, Variable::Type);
static bool SetFromVector (const std::string &, void *, Variable::Type);


Variable::Variable() {
    name = "";
    type = Type::Unknown;
    data = nullptr;
}

Variable::Variable(const std::string & n, bool & v) :
    name(n),
    data(&v),
    type(Type::Boolean) {}

Variable::Variable(const std::string & n, int & v) :
    name(n),
    data(&v),
    type(Type::Int) {}

Variable::Variable(const std::string & n, float & v) :
    name(n),
    data(&v),
    type(Type::Float) {}

Variable::Variable(const std::string & n, std::string & v) :
    name(n),
    data(&v),
    type(Type::String) {}

Variable::Variable(const std::string & n, void * v) :
    name(n),
    data(v),
    type(Type::Address) {}

Variable::Variable(const std::string & n, Vector & v) :
    name(n),
    data(&v),
    type(Type::Vector) {}


std::string Variable::GetName() {
    return name;
}

Variable::Type Variable::GetType() {
    return type;
}


std::string Variable::ToString() {
    Vector * dummy;
    static char buffer[256];
    switch(type) {
      case Type::Boolean: return (*(bool*) data) ? "True" : "False";
      case Type::Int:   return (Chain() << *((int*)data));
      case Type::Float: return (Chain() << *((float*)data));
      case Type::Vector:
        dummy = (Vector*)data;
        return(Chain() << "{" << dummy->x << ", " << dummy->y << ", " << dummy->z << "}");
      case Type::Address:
        sprintf(buffer, "%p", data);
        return (Chain() << buffer); break;
      case Type::String:  return *((std::string *) data);
      default:;
    }
    return "";
}



void Variable::Set(const std::string & str) {
    if (SetFromBool  (str, data, type)) return;
    if (SetFromInt   (str, data, type)) return;
    if (SetFromFloat (str, data, type)) return;
    if (SetFromVector (str, data, type)) return;
    if (SetFromString(str, data, type)) return;
}





bool Dynacoe::operator<(const Variable & a, const Variable & b) {
    return a.name < b.name;
}




















///////////// Statics / privates

static bool SetFromBool  (const std::string & str, void * dest, Variable::Type type) {
    bool val;
    if (str.size() > 6) return false;
    std::string valSrc = str;
    for(int i = 0; i < valSrc.size(); ++i) {
        valSrc[i] = tolower(valSrc[i]);
    }
    if (valSrc == "true") {
        val = true;
    } else if (valSrc == "false") {
        val = false;
    } else {
        return false;
    }
    


    switch(type) {
      case Variable::Type::Boolean:
        *((bool*)dest) = val; return true;

      case Variable::Type::Int:
        *((int*)dest) = val; return true;

      case Variable::Type::Float:
        *((float*)dest) = val; return true;

      case Variable::Type::String:
        *((std::string*) dest) = valSrc;
      default:;
    }
    return false;
}


static bool SetFromInt (const std::string & str, void * dest, Variable::Type type) {
    int val;

    if (!sscanf(str.c_str(), "%d", &val)) return false;
    Chain c;


    switch(type) {
      case Variable::Type::Boolean:
        *((bool*) dest) = val; return true;

      case Variable::Type::Int:
        *((int*) dest) = val; return true;

      case Variable::Type::Float:
        *((float*) dest) = val; return true;

      case Variable::Type::String:
        *((std::string*)dest) = Chain() << *(std::string*)dest << val;
        return true;
      default:;

    }
    return false;
}

static bool SetFromFloat(const std::string & str, void * dest, Variable::Type type) {

    float val;

    if (!sscanf(str.c_str(), "%f", &val)) return false;
    Chain c;


    switch(type) {
      case Variable::Type::Boolean:
        *((bool*) dest) = val; return true;

      case Variable::Type::Int:
        *((int*) dest) = val; return true;

      case Variable::Type::Float:
        *((float*) dest) = val; return true;

      case Variable::Type::String:
        *((std::string*) dest) = Chain() << *(std::string*)dest << val;
        return true;
      default:;

    }
    return false;
}


static bool SetFromString(const std::string & str, void * dest, Variable::Type type) {


    switch(type) {
      case Variable::Type::String:
        *((std::string*)dest) = str;
        return true;
      default:;


    }
    return false;
}


static bool SetFromVector(const std::string & str, void * dest, Variable::Type type) {
    Vector var;
    if (!sscanf(str.c_str(), "{%f, %f, %f}", &var.x, &var.y, &var.z)) return false;

    switch(type) {
      case Variable::Type::Vector:
        *((Vector*)dest) = var;
        return true;
      default:;

    }
    return false;
}


