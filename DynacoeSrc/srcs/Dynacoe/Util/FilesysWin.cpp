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


#include <iostream>

using namespace std;

#ifdef _WIN32


using namespace Dynacoe;
bool Filesys::ChangeDir(const std::string & dir) {
    if (!SetCurrentDirectory(dir.c_str())) {
        return false;
    }
    currentPath = dir;
	// clear out old directory objects
	currentPathObjs.clear();


    // gather new objects

    WIN32_FIND_DATA objData;
    HANDLE dirHandle = INVALID_HANDLE_VALUE;
    TCHAR szDir[DIRECTORY_SIZE_LIMIT];
    std::string sDir = dir + "\\*";
    for(int i = 0; i < sDir.length(); ++i) {
	    szDir[i] = sDir[i];
    }
    szDir[sDir.length()] = '\0';
    LARGE_INTEGER fileSize;

    dirHandle = FindFirstFile(szDir, &objData);
    Object newObj;
    newObj.name = objData.cFileName;
    newObj.file = !(objData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    fileSize.LowPart = objData.nFileSizeLow;
    fileSize.HighPart = objData.nFileSizeHigh;
    newObj.size = fileSize.QuadPart;
    currentPathObjs.push_back(newObj);

    while(FindNextFile(dirHandle, &objData)) {
        newObj.name = objData.cFileName;
        newObj.file = !(objData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        fileSize.LowPart = objData.nFileSizeLow;
        fileSize.HighPart = objData.nFileSizeHigh;
        newObj.size = fileSize.QuadPart;
        currentPathObjs.push_back(newObj);
    }


	return true;
}

Filesys::Directory Filesys::QueryDirectory() {
	Directory out(currentPathObjs);
	return out;
}

std::string Filesys::GetCWD() {
	char str[DIRECTORY_SIZE_LIMIT];
	GetCurrentDirectory(DIRECTORY_SIZE_LIMIT, str);
	currentPath = str;
	return currentPath;
}

bool Filesys::GoToParent() {
	std::string d = GetCWD();
	int index = d.length() - 1;
	while(d[index] != '\\') index--;
	d = d.substr(0, index);
	return ChangeDir(d);
}

bool Filesys::GoToChild(const std::string & target) {
	return ChangeDir(GetCWD() + "\\" + target);
}

bool Filesys::CreateDir(const std::string & dir) {
	std::string newDir = GetCWD() + "\\" + dir;
	return CreateDirectory(newDir.c_str(), NULL);
}

#endif
