Downloads {#downloads} 
=========

Here you can download various sets of things to best fit your needs:





Windows C++ Toolkit
-------------------

[Toolkit + Tools + Prereqs Installer (direct link)](https://jcorks.github.io/Dynacoe/dynacoe.msi)

[visit the usage section to learn about how to use these tools.](@ref toolkit)

The full MSI will install:
- [MSYS2](http://msys2.github.io/) - giving you the compiler you'll need and the assets needed to 
  interact with the utilities pertaining to Dynacoe. 
- A handful of tools to manage Dynacoe projects, magically 
  get release builds from the internet, and more.

Note that this installer requires connection to the internet to finish. The MSI itself is really tiny, but 
MSYS2 which it retrieves is most definitely not. All in all, the total install will be
around 1 GB.

Any bugs with the installer should be filed in the bugs section [here](https://github.com/jcorks/dynacoe-installer/issues).


Linux C++ Toolkit 
-----------------

[Toolkit + SH installer (direct link)](https://jcorks.github.io/Dynacoe/dynacoe-install.tar.xz)

Contains an install script to grant you access to the same tools as the windows toolkit, all used from your terminal.
As such, you can [visit the usage section to learn about how to use these tools.](@ref toolkit) once you get it installed. Note that unlike most other \*nix style installs, this is meant to be installed in a directory local
to each user. The actual produced directory tree is extremely tiny, so having a few unique setups is at basically no cost,
and you wont have to worry about permissions. Simply unpack it, and run the install sh script.


Javascript/ECMAScript Bindings Toolkit
--------------------------------------

The [Sandboxe Project](https://jcorks.github.io/sandboxe) provides a standalone executable 
with bindings to most Dynacoe symbols and features. 



Windows Library
---------------

[Release build + headers + dependencies(direct link)](https://jcorks.github.io/Dynacoe/release/win/dynacoe.tar.xz)


Only tested with MinGW GCC. Anything else, you'll need to compile the prereqs and do the 
linking yourself.

Linux Library
-------------

[Release build + headers (direct link)](https://jcorks.github.io/Dynacoe/release/linux/dynacoe.tar.xz)


Everyone else
-------------

For all other systems, you may wish to try your hand at manually building Dynacoe from source.
[Visit the project page at GitHub.](http://github.com/jcorks/Dynacoe)
