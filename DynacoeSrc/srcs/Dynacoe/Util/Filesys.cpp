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


#include <Dynacoe/Util/Filesys.h>

using namespace std;
using namespace Dynacoe;

Filesys::Filesys() {
	_init();
	ChangeDir(GetCWD());
}

Filesys::Filesys(const std::string & initDir) {
	_init();
	ChangeDir(initDir);
}

void Filesys::_init() {
	home = GetCWD();
	type = __FSYS__;
}
Filesys::Object::Object() {
	file = false;
	size = 0;
	name = "";
}

__OS Filesys::GetType() {
	return type;
}


Filesys::Directory::Directory(const std::vector<Object> & src) {
	vec.assign(src.begin(), src.end());
	numFiles = src.size();
	pos = 0;

}

std::string Filesys::Directory::GetNextName() {
	if (!AtEnd())
		return vec[pos++].name;
	else
		return "";
}

bool Filesys::Directory::operator()() {
	return AtEnd();
}

bool Filesys::Directory::AtEnd() {
	return(pos >= numFiles);
}
void Filesys::Directory::ResetPosition() {
	pos = 0;
}

Filesys::Directory & Filesys::Directory::operator>>(std::string & str) {
	str = GetNextName();
	return (*this);
}

int Filesys::Directory::Size() {
	return numFiles;
}

bool Filesys::Directory::Query(const std::string & target) {
	for(int i = 0; i < numFiles; ++i) {
		if (vec[i].name == target)
			return true;
	}
	return false;
}

bool Filesys::Directory::IsFile(const std::string & obj) {
	for(int i = 0; i < numFiles; ++i) {
		if (vec[i].name == obj)
			return vec[i].file;
	}
	return false;
}


string Filesys::FindFile(const string & name) {
    string originalDir = GetCWD();
    string out = FFH(name, ".", true);
    ChangeDir(originalDir); // return to n
    return out;
}

std::string Filesys::FindDirectory(const std::string & name) {
    string originalDir = GetCWD();
    string out = FFH(name, ".", false);
    ChangeDir(originalDir); // return to n
    return out;
}

string Filesys::FFH(const string & target, const string & entryPoint, bool file) {
    string path = "";

    bool canChange = ChangeDir(entryPoint);
	if (!canChange) return path;


    Filesys::Directory d = QueryDirectory();
    string current;
    for(int i = 0; i < d.Size(); ++i) {
        current = d.GetNextName();

        // get the file is the one we are looking for
        if ((file ? d.IsFile(current) : !d.IsFile(current)) && current == target) {
            path = GetCWD() + "/" + current;
            return path;
        }

        // search recursively on subdirectories
        if (!d.IsFile(current) && current != "." &&
                                  current != "..") {


            string oldPath = GetCWD();
            string res = FFH(target, GetCWD() + "/" + current, file);
            if (res != "") return res;
            ChangeDir(oldPath);

        }

    }

    return ""; // the case where it was not found
}

vector<string> Filesys::FindAllFiles(const string & name) {
    string originalDir = GetCWD();
    vector<string> out = FAFH(name, ".", vector<string>());
    ChangeDir(originalDir); // return to n
    return out;
}

#include <iostream>
vector<string> Filesys::FAFH(const string & target,
			     const string & entryPoint,
			     vector<string> paths) {
    string path = "";

    std::cout << endl << entryPoint;
    bool canChange = ChangeDir(entryPoint);
	if (!canChange) return paths;


    Filesys::Directory d = QueryDirectory();
    string current;
    for(int i = 0; i < d.Size(); ++i) {
        current = d.GetNextName();

        // get the file is the one we are looking for
        if (d.IsFile(current) && current == target) {
            path = GetCWD() + "/" + current;
            paths.push_back(path);
        }

        // search recursively on subdirectories
        if (!d.IsFile(current) && current != "." &&
                                  current != "..") {
            string oldPath = GetCWD();
            paths =              FAFH(target,
                                      GetCWD() + "/" + current,
                                      paths);
            ChangeDir(oldPath);

        }

    }

    return paths; // the case where it was not found
}

