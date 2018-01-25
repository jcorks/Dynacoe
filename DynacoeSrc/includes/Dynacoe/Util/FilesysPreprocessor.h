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

#ifndef FILESYS_PREPROCESSOR_H_INCLUDED
#define FILESYS_PREPROCESSOR_H_INCLUDED




	#ifdef _WIN32
		#include <windows.h>
	 	#define __FSYS__ WINDOWS
		#define	__FSYS__UNSUPPORTED_OS false;
	
	#elif __unix__
		#include <unistd.h>
	 	#include <dirent.h>
		#include <sys/stat.h>
		#include <sys/types.h>
		#define __FSYS__ UNIX_VARIANT
	#endif
	#if !defined(__FSYS__)
		#define __FSYS__UNSUPPORTED_OS true;
		#define __FSYS__ UNKNOWN
		#error Filesys: Unknown or unsupported operating system. Make sure you compile with your operating system specified. Supported OS defines: _WIN32 __unix__	
	#endif




#endif
