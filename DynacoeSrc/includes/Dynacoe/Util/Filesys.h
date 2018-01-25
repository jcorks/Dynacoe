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

/*
 *  A lightweight, cross platform solution to accessing filesystems.
 *
 *  Johnathan Corkery, 2013
 */

#ifndef FILESYS_H_INCLUDED
#define FILESYS_H_INCLUDED


const int DIRECTORY_SIZE_LIMIT 		= 256;


#include <string>
#include <vector>



namespace Dynacoe {

enum __OS {
	UNIX_VARIANT,
	WINDOWS,
	UNKNOWN
};


/**
 *
 * \brief Cross-Platform filesystem access.
 *
 *
 *
 *  Unfortunately, to make this work properly, so called
 *  "preprocessor magic" is necessary for Filesys to operate on
 *  multiple platforms.
 */
class Filesys {
 	struct Object {
		bool file;
		int size;
		std::string name;
		Object();
	};

	__OS type;


	std::string currentPath;
	std::vector<Object> currentPathObjs;
	std::string home;
	void _init();
	std::string FFH(const std::string &, const std::string &, bool);
	std::vector<std::string> FAFH(const std::string &,
				      const std::string &,
				      std::vector<std::string>);
    
  	__OS GetType();

  public:


	/// \brief  The container for managing directory objects.
	///
	class Directory {
		std::vector<Object> vec;
		int numFiles;
		int pos;


	  public:
        /// \brief Pulls the next directory name from the list.
        /// The order of the elements in the list depends on the operating
        /// system, but it will most likely be alphabetical.
        ///
	  	std::string GetNextName();

	  	/// \brief Resets the reading position to the first name on the list.
	  	///
	  	void ResetPosition();

	  	/// \brief Returns the number of names contained within the directory.
	  	///
	  	int Size();

	  	/// \brief Equivalent to getNextName().
	  	///
	  	Directory & operator >> (std::string & str);

	  	/// \brief Returns if all files have been exhausted
	  	///
	  	bool AtEnd();

	  	/// \brief Equivalent to atEnd()
        ///
	  	bool operator()();


  		/// \brief Returns whether or not target exists
  		///
  		bool Query(const std::string & target);

	  	/// \brief Returns whether or nor obj is a file.
	  	/// PROTIP: if the Filesys::Object is not a file,
	  	/// it is most likely a directory!
	  	bool IsFile(const std::string & obj);

	 	Directory(const std::vector<Object> & src);
	};

    /// \name Init
    ///
    /// Filesys needs an entry point. If none is specified, then it
    /// is set to the current working directory.
    ///\{
  	Filesys();
  	Filesys(const std::string & initDir);
    ///\}

  	/// \brief Change to the specified directory.
  	/// Returns true on success.
  	bool ChangeDir(const std::string &);

	/// \brief Change to the specified child directory.
	/// NOTICE: you only need to specify the name of the directory
	/// Ex) if you query the directory and it returns true, then
	/// this function will succeed as well.
    ///	
    bool GoToChild(const std::string &);


	/// \brief Change directory to the parent directory.
	///
	bool GoToParent();

  	/// \brief return the current working directory.
  	///
  	std::string GetCWD();

	/// \brief Creates a directory with the given name.
	/// The directory is relative to the current working directory.
    ///
	bool CreateDir(const std::string &);


  	/// \brief return all the object names in the current working directory as a directory instance.
  	/// See the Directory object for more details. Special directories are not included
    ///  	
    Directory QueryDirectory();

	/// \brief Searches for the file of the specified name.
	/// The search is done in the
	/// current directory and all sub directories.
	/// If successful, return the full path of the
	/// file, else returns an empty string.
    ///	
    std::string FindFile(const std::string & name);

    /// \brief Searches for the directory of the specified 
    /// name. THe search is done in the current and all sub 
    /// directories. If sucessful, the directory's full path is 
    /// returned. Else, an empty string is returned.
    std::string FindDirectory(const std::string & name);


	/// \brief Similar to findFile, but returns a vector of paths.
	/// The paths coresponding to all instances of name that exist in
	/// the current and sub-directories of the current.
    ///	
    std::vector<std::string> FindAllFiles(const std::string & name);





};
};

#include "FilesysPreprocessor.h"

#endif

