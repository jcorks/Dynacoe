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


#include <Dynacoe/Settings.h>
#include <unordered_map>

using namespace Dynacoe;

struct Setting {
    Settings::GetCallback getter;
    Settings::SetCallback setter;
    std::string info; 
};

class Dynacoe::SettingsData {
  public:
    std::unordered_map<std::string, Setting> settings;
    std::vector<std::string> settingNames;
};


Settings::Settings() {
    X = new SettingsData;
}

Settings::~Settings() {
    delete X;
}


void Settings::Add(
    const std::string & name, 
    Settings::GetCallback getter, 
    Settings::SetCallback setter,
    const std::string & info) {

    // already exists. Cannot overwrite!
    if (X->settings.find(name) != X->settings.end()) return;

    // no no getter? Bad 
    if (!getter) {
        return;
    }


    Setting s;
    s.getter = getter;
    s.setter = setter; // null setter == read-only
    s.info = info;

    X->settings[name] = s;
    X->settingNames.push_back(name);

}

void Settings::AddReadOnly(const std::string & name, GetCallback getter, const std::string & info) {
    Add(name, getter, nullptr, info);
}


void Settings::Set(const std::string & name, const std::string & value) {
    auto iter = X->settings.find(name);
    if (iter == X->settings.end()) return;

    Setting & val = iter->second;
    if (!val.setter) return;
    val.setter(name, value);
}

std::string Settings::Get(const std::string & name) const {
    auto iter = X->settings.find(name);
    if (iter == X->settings.end()) return "";

    Setting & val = iter->second;
    return val.getter(name);
}

std::string Settings::GetInfo(const std::string & name) const {
    auto iter = X->settings.find(name);
    if (iter == X->settings.end()) return "";

    Setting & val = iter->second;
    return val.info;
}


const std::vector<std::string> & Settings::List() const {
    return X->settingNames;
}

std::string Settings::GetSummary() const {
    std::string out = "";
    uint32_t size = X->settingNames.size();
    for(uint32_t i = 0; i < size; ++i) {
        Setting & val = X->settings[X->settingNames[i]];
        out += X->settingNames[i] + ":" + val.getter(X->settingNames[i]) + (val.setter?"\n" : "[Read-only]\n");
        out += val.info;
        out += "\n\n";
    }
    return out;
}









