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


#ifdef __unix__

#include <iostream>
#include <cstring>
using namespace Dynacoe;

bool Filesys::ChangeDir(const std::string & dir) {

    char pathTemp[PATH_MAX];
  	if (!realpath(dir.c_str(), pathTemp)) 
        return false;


	if (chdir(dir.c_str()) == -1)
		return false;


    currentPath = pathTemp;


	// clear out old directory objects

	int numFiles;
	currentPathObjs.clear();




	// gather new path objects
	dirent **ls;

	numFiles = scandir(currentPath.c_str(),
						   &ls, NULL,
						   alphasort);

	// dump file names in base vector;
    
	Object newObj;
	for(int i = 0; i < numFiles; ++i) {
        if (!strcmp(ls[i]->d_name, ".") ||
            !strcmp(ls[i]->d_name, "..")) continue;

		if (ls[i]->d_type == DT_DIR)  // directory detected
			newObj.file = false;    
		else
			newObj.file = true	;

		newObj.name = ls[i]->d_name;
		//newObj->size = ls[i]->d_reclen;
		currentPathObjs.push_back(newObj);
	}

	return true;

}

Filesys::Directory Filesys::QueryDirectory() {
	Directory out(currentPathObjs);
	return out;
}

std::string Filesys::GetCWD() {
	char dir[DIRECTORY_SIZE_LIMIT];
	currentPath = getcwd(dir, sizeof(dir));

	return currentPath;
}

bool Filesys::GoToParent() {

	std::string d = GetCWD();
	int index = d.length() - 1;
	while (d[index] != '/') index--;
	d = d.substr(0, index);
	//std::cout << d << std::endl;
	return ChangeDir(d);


}

bool Filesys::GoToChild(const std::string & target) {
	return ChangeDir(GetCWD() + "/" + target);
}

bool Filesys::CreateDir(const std::string & dir) {
	std::string newDir = GetCWD() + "/" + dir;
	return mkdir(newDir.c_str(), 0777);
}

#endif
