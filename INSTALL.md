Building Dynacoe
================
Dynacoe does not have any hard-coded dependencies, but rather has
major components that link to a variety of system backends choosable at copmile time 
of the library.

This help file will guide you through setting up a working environment
for Dynacoe. Most provided libraries were for 64-bit systems only.





Prerequisites
-------------

While Dynacoe does not have any strict dependencies, building has a few environment requirements:
    - GNU Make
    - bash
    - A version of g++ that is c++11 compliant AND can produce 64-bit binaries
    

Although not necessary, if you are running on a Linux machine, having the following will give you the most flexibility for Dynacoe backends
    - libfreetype
    - libMesa (or any OpenGL implementation)
    - libalsa
    - libogg
    - libvorbis
    - libvorbisfile
    - libX11
		


    



Windows users can simply can use the libraries / backends provided with the Dynacoe installation





1.	If on Linux, Make should already be installed on your system, if not.
	search your distributions' package manager for a version. 
	
	If on Windows, MSYS2 is the best way to go. To install msys, check the "Installing MSYS2" guide below.




2.	Open Dynacoe's toplevel directory. In a terminal emulator of your choice (Windows users:
 	use the MSYS shell), enter the following
		
		./configure.sh


	This configure Dynacoe for your system to prepare the building process. The configure script will also
    choose which backends Dynacoe will link against. If Dynacoe is unable to provide a system backend for each of its major
    components, Dynacoe will fail to configure and will not be able to build.


3. While still in the top-level directory, enter:

		make 
	
	This will build both the library and demos. If everything worked without error, you are done.
	Feel free to run the demo! Also, checkout the makfile in this directory.
	You can use this as a base for starting off. You just need to add your source files 
	on the SRCS line. Once thats done, the makefile will compile your project for you via
	the `make' command.
	





    
Installing MSYS2
----------------

Grab the installer from here at the top and install.

https://msys2.github.io/

Once installed, run open the "Mingw-w64 Win64 Shell" shortcut that it installs.
(it opens msys2 configured to build with the 64bit toolchain without path shinanegens)

In that new term, run msys2-prep.sh provided by Dynacoe and answer 'y' to any prompts.











	

