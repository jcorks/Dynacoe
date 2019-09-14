#!/bin/bash


#Copyright (c) 2018, Johnathan Corkery. (jcorkery@umich.edu)
#All rights reserved.

#This file is part of the Dynacoe project (https://github.com/jcorks/Dynacoe)
#Dynacoe was released under the MIT License, as detailed below.



#Permission is hereby granted, free of charge, to any person obtaining a copy 
#of this software and associated documentation files (the "Software"), to deal 
#in the Software without restriction, including without limitation the rights 
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
#copies of the Software, and to permit persons to whom the Software is furnished 
#to do so, subject to the following conditions:

#The above copyright notice and this permission notice shall
#be included in all copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
#EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
#MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
#IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
#DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
#ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
#DEALINGS IN THE SOFTWARE.


# Configure script for Dynacoe



NULL_STR_C="__xNULLx__"


version=0.3
depSubsystems="X11 Win32 Alsa Pulse DirectSound"
buildTypes="Static Dynamic"


# Backends.
# Ordered in terms of capability, where the most capable / powerful
# backends are at the top
renderers="
ShaderGL_X11
GLES_X11
ShaderGL_Win32
NoRender
"

displays="
OpenGLFramebuffer_X11
GLESFramebuffer_X11
OpenGLFramebuffer_Win32
NoDisplay
"

inputManagers="
X11Input_X11
GainputX11
GainputWin32
NoInput
"

audioManagers="
RtAudioAlsa
RtAudioOSS
RtAudioDirectSound
NoAudio
"

extensions="
ExtraDecoders
Done
"



# Backend Dependencies
# Subsystems provide the resources from the operating system
# to the backend
# Every backend must be bound to a subsystem. If there is no real
# subsystem to be bound to ,i.e. it is portable to any system that
# supports c++11, use the special 'ANY' subsystem
backendSystemDeps="
BEGIN Dynacoe ANY END
BEGIN ShaderGL_X11 X11 END
BEGIN GLES_X11 X11 END
BEGIN ShaderGL_Win32 Win32 END
BEGIN OpenGLFramebuffer_X11 X11 END
BEGIN GLESFramebuffer_X11 X11 END
BEGIN OpenGLFramebuffer_Win32 Win32 END
BEGIN X11Input_X11 X11 END
BEGIN GainputX11 X11 END
BEGIN GainputWin32 Win32 END
BEGIN RtAudioAlsa Alsa END
BEGIN RtAudioOSS Pulse END
BEGIN RtAudioDirectSound DirectSound END
BEGIN NoInput ANY END
BEGIN NoDisplay ANY END
BEGIN NoRender ANY END
BEGIN NoAudio ANY END
BEGIN ExtraDecoders ANY END
BEGIN Done ANY END
"

# The actual linking dependency of each backend and subsystem
# ANy additional required libraries aside from the ones brought in by the
# bound subsystem should be here
Dependencies="
BEGIN Dynacoe END
BEGIN ShaderGL_X11    -lGLEW -lGL  END
BEGIN GLES_X11    -lGLESv2  END
BEGIN ShaderGL_Win32  -lglew-dc -lOpenGL32 END
BEGIN OpenGLFramebuffer_X11    -lGLEW -lGL  END
BEGIN GLESFramebuffer_X11    -lEGL -lGLESv2  END
BEGIN OpenGLFramebuffer_Win32  -lglew-dc -lOpenGL32  END
BEGIN X11Input_X11 END
BEGIN GainputX11   -lgainputstatic END
BEGIN GainputWin32 -lgainputstatic-dc -lkernel32 -luser32 -lgdi32 -lXinput9_1_0 END
BEGIN RtAudioAlsa -lpthread END
BEGIN RtAudioOSS  END
BEGIN RtAudioDirectSound END
BEGIN NoInput END
BEGIN NoRender END
BEGIN NoDisplay END
BEGIN NoAudio END
BEGIN Win32 -lmingw32 -lole32 -lwinmm -lWsock32 -lgdi32 -lm -lpng-dc -lz-dc -lfreetype-dc -lvorbisfile-dc -lvorbis-dc -logg-dc  END
BEGIN X11 -lpthread -lX11 -lm -lpng -lz -lfreetype -lvorbisfile -lvorbis -logg -levdev END
BEGIN Alsa -lasound END
BEGIN DirectSound -ldsound END
BEGIN ExtraDecoders -lassimp END
BEGIN Done END
BEGIN ANY  END
"

#TODO: make specific option equivalents for multiple compilers
CompilerOpts="
BEGIN Windows -std=c++11 -lpthread  -static -static-libgcc -static-libstdc++ -mwindows END
BEGIN Linux -std=c++11  -lpthread -I/usr/include/freetype2 -I/usr/include/freetype2/freetype END
BEGIN Debug  -ggdb -pg END
BEGIN Release  -O2 END
BEGIN UnixLike -std=c++11 END
BEGIN amd64 -m64 END
BEGIN x86 -m32 END
"

#Dynacoe provides some 3rd party dependencies for you already if you have a supported OS
#These paths are relative to the Dynacoe Root folder
libPath="
BEGIN Windows /lib/win/64/ END
BEGIN Linux END
BEGIN UnixLike END
BEGIN UnknownOS END
"

incPath="
BEGIN Linux END
BEGIN UnixLike END
BEGIN Windows /include/win/ END
BEGIN UnknownOS END
"

#These paths deonte external assets that are absolutely necessary for dynacoe to compile
libPathEssential="
BEGIN Windows /lib/win/64/ END
BEGIN Linux END
BEGIN UnixLike END
BEGIN UnknownOS END
"

incPathEssential="
BEGIN Linux END
BEGIN UnixLike END
BEGIN Windows /include/win/ END
BEGIN UnknownOS END
"



defines="
BEGIN Dynacoe END
BEGIN ShaderGL_X11                -DDC_BACKENDS_SHADERGL_X11 END
BEGIN GLES_X11                -DDC_BACKENDS_GLES_X11 END
BEGIN ShaderGL_Win32              -DDC_BACKENDS_SHADERGL_WIN32 END
BEGIN OpenGLFramebuffer_X11     -DDC_BACKENDS_OPENGLFRAMEBUFFER_X11 END
BEGIN GLESFramebuffer_X11     -DDC_BACKENDS_GLESFRAMEBUFFER_X11 END
BEGIN OpenGLFramebuffer_Win32   -DDC_BACKENDS_OPENGLFRAMEBUFFER_WIN32 END
BEGIN X11Input_X11               -DDC_BACKENDS_X11INPUT_X11 END
BEGIN GainputX11                 -DDC_BACKENDS_GAINPUTX11 END
BEGIN GainputWin32               -DDC_BACKENDS_GAINPUTWIN32 END
BEGIN RtAudioAlsa                -DDC_BACKENDS_RTAUDIO_ALSA END
BEGIN RtAudioOSS                 -DDC_BACKENDS_RTAUDIO_OSS END
BEGIN RtAudioDirectSound         -DDC_BACKENDS_RTAUDIO_WIN32 END
BEGIN NoInput                    -DDC_BACKENDS_NOINPUT END
BEGIN NoDisplay                  -DDC_BACKENDS_NODISPLAY END
BEGIN NoRender                   -DDC_BACKENDS_NORENDER END
BEGIN NoAudio                    -DDC_BACKENDS_NOAUDIO END
BEGIN Win32                      -DDC_SUBSYSTEM_WIN32 END
BEGIN X11                        -DDC_SUBSYSTEM_X11 END
BEGIN Alsa                       -DDC_SUBSYSTEM_ALSA END
BEGIN Pulse                      -DDC_SUBSYSTEM_PULSE END
BEGIN DirectSound                -DDC_SUBSYSTEM_DIRECTSOUND END
BEGIN Linux                      -DDC_OS_LINUX END
BEGIN Windows                    -DDC_OS_WINDOWS  END
BEGIN UnixLike                   -DDC_OS_UNIXLIKE   END
BEGIN UnknownOS                  -DDC_OS_UNKNOWNOS  END
BEGIN ExtraDecoders              -DDC_EXTENSION_EXTRA_DECODERS END
BEGIN Done                                              END
BEGIN ANY END
"



# User variables
userAutoSelect="t"
userBuildType=$NULL_STR_C
userSubsystems=""
userRenderer=""
userDisplay=""
userInputManager=""
userAudioManager=""
userOS=""

# config

userSystemPathsOnly="f"


#makefile variables
libMakefileDefines=""
libMakefileIncpaths=""
libMakefileLibpaths=""
libCompilerOpts=""
buildMakefileLibs=""







ScriptError() {
    echo -e "[ERROR] $1"
    exit
}





isInList="f"
inputList=""
IsInList() {
    for iistr in $inputList; do
        if [ $iistr == $1 ]; then
            isInList="t"
            return
        fi
    done
    isInList="f"
}



# Extracts a defined list corresponding to a given key
# the lists are expected to be set up as
inputList=""
extractedList=""
ListExtract() {
    # $inputList -> the List
    # $1 -> the item
    extractedList=""
    tempCheck="f"
    tempListBegin="f"
    for str in $inputList; do
        if [ $tempListBegin == "t" ]; then
            if [ $str == "END" ]; then
                return
            fi
            extractedList="$extractedList $str"
        fi

        if [ $tempCheck == "t" ]; then
            if [ "$str" == "$1" ]; then
                tempListBegin="t"
            fi
        fi


        if [ $str == "BEGIN" ]; then
            tempCheck="t"
        fi
    done

    ScriptError "Internal: $1 has no entry in the given list"
}







### Bases ###

#Prompts the user to select from a list of things
#Will not return until the user inputs a valid string
#Input
userSelectList=$NULL_STR_C
#output
userChoice=$NULL_STR_C
UserSelect() {



    userChoice=$NULL_STR_C
    tempHasChoices="f"
    for str in $userSelectList; do
        if [ $userAutoSelect == "t" ]; then
            echo -e "[Auto-Selecting \"$str\"]"
            userChoice=$str
            return
        fi

        tempHasChoices="t"
        echo ">  $str"
    done
    if [ $tempHasChoices == "f" ]; then
        userChoice=$NULL_STR_C
        return
    fi
    tempResp=$NULL_STR_C

    read tempResp

    tempValid="n"
    for str in $userSelectList; do
        if [ $str == $tempResp ]; then
            tempValid="y"
        fi
    done

    if [ $tempValid == "n" ]; then
        echo "Sorry, I'm not familiar with $tempResp"
        UserSelect
    fi


    userChoice=$tempResp
}










### Tests ####

testResult=FAIL
testInput=NULL_STR_C
TestCompiler() {
    echo -en "Looking for g++ with c++11 support..."
    echo     "Looking for g++ with c++11 support..." >> configure_log
    rm -rf ./.tempdir
    mkdir ./.tempdir
    cd ./.tempdir
    echo "
#include <algorithm>
#include <iostream>

int main() {
    auto l = []() {
        std::cout << \"OK\" << std::endl;
    };
    l();
    return 0;

}
" > .compilertest.cpp
    g++ --std=c++11 .compilertest.cpp &> ../configure_log
    if [ $? == "0" ]; then
        echo "[OK]"
        testResult="PASS"
    else
        echo "[FAIL]"
        testResult="FAIL"
    fi
    cd ..
    rm -rf ./.tempdir
}


TestSystemX11() {
    echo -n "Checking if X11 exists..."
    echo    "Checking if X11 exists..." >> configure_log
    rm -rf ./.tempdir
    mkdir ./.tempdir
    cd ./.tempdir
    echo "
#include <X11/Xlib.h>

int main() {
    XOpenDisplay(NULL);
    return 0;

}
" >> .x11test.cpp
    g++ --std=c++11 .x11test.cpp -lX11 >> ../configure_log 2>&1
    if [ $? == "0" ]; then
        echo "[OK]"
        testResult="PASS"
    else
        echo "[FAIL]"
        testResult="FAIL"
    fi
    cd ..
    rm -rf ./.tempdir
}

TestSystemWin32() {
    echo -n "Checking if Win32 exists..."
    echo    "Checking if Win32 exists..." >> configure_log
    rm -rf ./.tempdir
    mkdir ./.tempdir
    cd ./.tempdir
    echo "
#include <windows.h>

int main() {
    return 0;

}
" >> .wintest.cpp
    g++ --std=c++11 .wintest.cpp -lole32 -lwinmm -lWsock32 >> ../configure_log 2>&1
    if [ $? == "0" ]; then
        echo "[OK]"
        testResult="PASS"
    else
        echo "[FAIL]"
        testResult="FAIL"
    fi
    cd ..
    rm -rf ./.tempdir
}

TestSystemAlsa() {
    echo -n "Checking if Alsa exists..."
   	echo    "Checking if Alsa exists..." >> configure_log
    rm -rf ./.tempdir
    mkdir ./.tempdir
    cd ./.tempdir
    echo "
#include <alsa/asoundlib.h>

int main() {
    return 0;

}
" >> .alsatest.cpp
    g++ --std=c++11 .alsatest.cpp -lasound >> ../configure_log 2>&1
    if [ $? == "0" ]; then
        echo "[OK]"
        testResult="PASS"
    else
        echo "[FAIL]"
        testResult="FAIL"
    fi
    cd ..
    rm -rf ./.tempdir

}

TestSystemDirectSound() {
    echo -n "Checking if DirectSound exists..."
    echo    "Checking if DirectSound exists..." >> configure_log
    rm -rf ./.tempdir
    mkdir ./.tempdir
    cd ./.tempdir
    echo "
#include <windows.h>
#include <mmsystem.h>

int main() {
    return 0;

}
" >> .dsoundtest.cpp
    g++ -ldsound --std=c++11 .dsoundtest.cpp >> ../configure_log 2>&1
    if [ $? == "0" ]; then
        echo "[OK]"
        testResult="PASS"
    else
        echo "[FAIL]"
        testResult="FAIL"
    fi
    cd ..
    rm -rf ./.tempdir
}


TestSystemLinkingBasic() {
    echo -n "Checking if all the required linking libraries exist..."
    echo    "Checking if required libs exist exists..." >> configure_log
    rm -rf ./.tempdir
    mkdir ./.tempdir
    cd ./.tempdir
    echo "
#include<iostream>

int main() {
    return 0;

}
" >> .link.cpp
    for tempStrTest in $libMakefileLibpaths; do
        lib_libpaths_appended="$lib_libpaths_appended -L..$tempStrTest"
    done

    echo "g++ $lib_libpaths_appended --std=c++11 .link.cpp $buildMakefileLibs" >> ../configure_log 2>&1
    g++ $lib_libpaths_appended --std=c++11 .link.cpp $buildMakefileLibs >> ../configure_log 2>&1
    if [ $? == "0" ]; then
        echo "[OK]"
        testResult="PASS"
    else
        echo "[FAIL]"
        testResult="FAIL"
    fi
    cd ..
    rm -rf ./.tempdir
}















### RETRIEVERS ####

AddDependencies() {
    inputList=$Dependencies
    ListExtract $1
    tempList=$extractedList

    inputList=$Dependencies
    for tempad in $tempSubSysList; do
        ListExtract $tempad
        tempList="$tempList $extractedList"
    done
    inputList=$buildMakefileLibs

    for eachDep in $tempList; do
        IsInList $eachDep
        if [ $isInList == "f" ]; then
            buildMakefileLibs="$buildMakefileLibs $eachDep"
        fi
    done



    # also add to the makefile info
    inputList=$defines
    ListExtract $1
    libMakefileDefines="$libMakefileDefines $extractedList"

}

AddSystemDependencies() {

    inputList=$backendSystemDeps
    ListExtract $1
    tempSubSysList=$extractedList

    inputList=$Dependencies
    for tempad in $tempSubSysList; do
        ListExtract $tempad
        tempList="$tempList $extractedList"
    done
    inputList=$buildMakefileLibs

    for eachDep in $tempList; do
        IsInList $eachDep
        if [ $isInList == "f" ]; then
            buildMakefileLibs="$buildMakefileLibs $eachDep"
        fi
    done



    # also add to the makefile info
    inputList=$defines
    ListExtract $1
    libMakefileDefines="$libMakefileDefines $extractedList"

}


GetOS() {
    echo -n "Checking the environment's OS.."
    rm -rf ./.tempdir
    mkdir ./.tempdir
    cd ./.tempdir
    echo -e "
#include <iostream>

using namespace std;

int main() {
    #if (defined __linux__ || defined linux || defined __linux)
        cout << \"Linux\";
        cout.flush();
        return 0;
    #endif

    #if (defined _WIN32 || defined _WIN64 || defined __WINDOWS__)
        cout << \"Windows\";
        cout.flush();
        return 0;
    #endif

    #if (defined __unix__ || defined __unix)
        cout << \"UnixLike\";
        cout.flush();
        return 0;
    #endif

    cout << \"Unknown\" << endl;
}
" >> .ostest.cpp
    g++ .ostest.cpp -o .ostest >> ../configure_log 2>&1

    if [ $? == "0" ]; then
        echo -n ". "
    else
        echo "[FAIL]"
        ScriptError "OS retrieval failed: couldn't compile test."
    fi
    ./.ostest > .temposout
    userOS=$(cat .temposout)
    echo $userOS
    cd ..
    rm -rf ./.tempdir

    #try to filter out whitespace
    tempFilter=$userOS
    for str in $tempFilter; do
        userOS=$str
    done


    if [ $userSystemPathsOnly = "f" ]; then
        inputList=$libPath
        ListExtract $userOS
        libMakefileLibpaths="$libMakefileLibpaths $extractedList"

        inputList=$incPath
        ListExtract $userOS
        libMakefileIncpaths="$libMakefileIncpaths $extractedList"
    else
        echo ""
        echo "*****"
        echo "NOTE: Not adding packaged Dynacoe dependency paths as specified in configure invocation directives."
        echo "*****"
        echo ""
    fi

    inputList=$libPathEssential
    ListExtract $userOS
    libMakefileLibpaths="$libMakefileLibpaths $extractedList"

    inputList=$incPathEssential
    ListExtract $userOS
    libMakefileIncpaths="$libMakefileIncpaths $extractedList"


    inputList=$defines
    ListExtract $userOS
    libMakefileDefines="$libMakefileDefines $extractedList"



    inputList=$CompilerOpts
    ListExtract $userOS
    libCompilerOpts="$libCompilerOpts $extractedList"

}


GetCompiler() {
    TestCompiler

    if [ $testResult == "FAIL" ]; then
       ScriptError "g++ doesn't seem to be installed properly. Make sure it is in your path"
    fi
}


GetSystem() {

    TestSystemX11
    if [ $testResult == "PASS" ]; then
        userSubsystems="$userSubsystems X11"
    fi

    TestSystemWin32
    if [ $testResult == "PASS" ]; then
        userSubsystems="$userSubsystems Win32"
    fi

    TestSystemAlsa
    if [ $testResult == "PASS" ]; then
        userSubsystems="$userSubsystems Alsa"
    fi

    TestSystemDirectSound
    if [ $testResult == "PASS" ]; then
        userSubsystems="$userSubsystems DirectSound"
    fi


    echo "Your environment supports these subsystems: "
    echo $userSubsystems
    userSubsystems="$userSubsystems ANY"

    inputList=$defines
    for tempStr in $userSubsystems; do
        ListExtract $tempStr
        libMakefileDefines="$libMakefileDefines $extractedList"
    done
}


GetBuildType() {
    #echo "Which kind of build do want Dynacoe to have?"

    #userSelectList=$buildTypes
    #UserSelect
    #userBuildType=$userChoice

    echo "Will build statically (only one supported)"
}


# Retrieves all the available backends to the user
supportedList=""
fullBackendList=""
GetSupportedList() {
    supportedList=""
    for backend in $fullBackendList; do
        echo -n "Checking for $backend support..."
        inputList=$backendSystemDeps
        ListExtract $backend
        userCanSupportBackend="t"
        for backendDep in $extractedList; do
            inputList=$userSubsystems
            IsInList $backendDep
            # We've encountered a backEnd that the user can't support
            if [ $isInList == "f" ]; then
                userCanSupportBackend="f"
            fi
        done

        if [ $userCanSupportBackend == "t" ]; then
            supportedList="$supportedList $backend"
            echo [Supported]
        else
            echo [Unsupported]
        fi
    done
}

GetAvailableBackends() {
    echo "Retrieving available backends..."


    # first, tack on engine requirements
    AddDependencies "Dynacoe"




    fullBackendList=$renderers
    GetSupportedList
    userRenderer=$supportedList

    fullBackendList=$displays
    GetSupportedList
    userDisplay=$supportedList

    fullBackendList=$inputManagers
    GetSupportedList
    userInputManager=$supportedList

    fullBackendList=$audioManagers
    GetSupportedList
    userAudioManager=$supportedList

    fullBackendList=$extensions
    GetSupportedList
    userExtensions=$supportedList

    echo "Choose an input manager:"
    userSelectList=$userInputManager
    UserSelect
    if [ $userChoice == $NULL_STR_C ]; then
        ScriptError "No input manager is supported for your system. Unable to build Dynacoe"
    fi
    userInputManager=$userChoice
    AddDependencies $userInputManager

    echo "Choose a renderer:"
    userSelectList=$userRenderer
    UserSelect
    if [ $userChoice == $NULL_STR_C ]; then
        ScriptError "No renderer is supported for your system. Unable to build Dynacoe"
    fi
    userRenderer=$userChoice
    AddDependencies $userRenderer

    echo "Choose a display:"
    userSelectList=$userDisplay
    UserSelect
    if [ $userChoice == $NULL_STR_C ]; then
        ScriptError "No Display is supported for your system. Unable to build Dynacoe"
    fi
    userDisplay=$userChoice
    AddDependencies $userDisplay


    echo "Choose an audio manager:"
    userSelectList=$userAudioManager
    UserSelect
    if [ $userChoice == $NULL_STR_C ]; then
        ScriptError "No audio manager is supported for your system. Unable to build Dynacoe"
    fi
    userAudioManager=$userChoice
    AddDependencies $userAudioManager


    echo "Choose an extension to add:"
    userSelectList=$userExtensions
    userExtensions=""
    UserSelect
    while [ $userChoice != "Done"  ] && [ $userChoice != "" ]; do
        finishloop="f"

        for str in $userExtensions; do
            if [ $userChoice == $str ]; then
                finishloop="t"
                break;
            fi
        done

        if [ $finishloop == "t" ]; then
            break;
        fi
        userExtensions="$userExtensions $userChoice"
        AddDependencies $userChoice
        UserSelect
    done

    for user in $userExtensions; do
        AddDependencies $user     
    done
    AddSystemDependencies $userAudioManager
    AddSystemDependencies $userDisplay
    AddSystemDependencies $userRenderer
    AddSystemDependencies $userInputManager
    AddSystemDependencies "Dynacoe"

}


CursoryCompileCheck() {
	TestSystemLinkingBasic
	if [ $testResult = "FAIL" ]; then
		ScriptError "One or more libraries could not be found or are unfit to build Dynacoe. Please check the configure_log."
	fi
}



ConfirmSetup() {
    echo
    echo
    echo "Dynacoe will be setup up with:"
    echo "Renderer:     $userRenderer"
    echo "Display:      $userDisplay"
    echo "InputManager: $userInputManager"
    echo "AudioManager: $userAudioManager"
    echo "extensions:   $userExtensions"
    echo
    echo "Link Dependencies:"
    echo $buildMakefileLibs
    echo
    echo "Makefile Defines (lib):"
    echo $libMakefileDefines
    echo
    echo "Paths"
    echo $libMakefileLibpaths $libMakefileIncpaths
    echo
    echo "Compiler options:"
    echo $libCompilerOpts
}



# Special

PrintHelp() {
    echo
    echo -e "Dynacoe $version - Config"
    echo
    echo -e "This configure script will let you choose what"
    echo -e "backends Dynacoe should build with"
    echo -e "and its dependencies it will be linked to."
    echo
    echo Options:
    echo -e "   --help\tGets you here"
    echo -e "   --auto\tChooses the \"best\" options for what you have. This is the default"
    echo -e "   --manual\tLets you choose each backend."
    echo -e "   --debug\tSets up the configuration with additional debugging compiler flags."
    echo -e "   --release\tSets up the configuration with additional release compiler flags."
    echo -e "   --x86\tAssures that the build will result in a 32-bit binary library and link against only 32-bit binaries."
    echo -e "   --amd64\tAssures the same as --x86, instead 64-bit."
    echo -e "   --system-paths-only\tConfigures builds to not rely on any built-in binaries or headers for backends."
    exit
}





ProcessArgs() {

    if [ $1 == "--help" ]; then
        PrintHelp
    elif [ $1 == "--manual" ]; then
        userAutoSelect="f"
    elif [ $1 == "--debug"  ]; then
        temp="Debug"
        inputList=$CompilerOpts
        ListExtract $temp
        libCompilerOpts="$libCompilerOpts $extractedList"
    elif [ $1 == "--release" ]; then
        temp="Release"
        inputList=$CompilerOpts
        ListExtract $temp
        libCompilerOpts="$libCompilerOpts $extractedList"
    elif [ $1 == "--x86" ]; then
        temp="x86"
        inputList=$CompilerOpts
        ListExtract $temp
        libCompilerOpts="$libCompilerOpts $extractedList"
    elif [ $1 == "--amd64" ]; then
        temp="amd64"
        inputList=$CompilerOpts
        ListExtract $temp
        libCompilerOpts="$libCompilerOpts $extractedList"
    elif [ $1 == "--system-paths-only" ]; then
        userSystemPathsOnly="t"
    else
        ScriptError "Unrecognized option \"$1\""
    fi
}


WriteHelperFiles() {
    cd ./build/lib
    echo $libMakefileDefines > lib_defines
    echo $libMakefileLibpaths > lib_libpaths
    echo $libMakefileIncpaths > lib_incpaths
    echo $libCompilerOpts > lib_compileropts
    echo $buildMakefileLibs > build_libs
}











### Here, the actual script begins ###

rm -f ./configure_log
touch configure_log
if [ $# -ne "0" ]; then
    while(("$#")); do
        ProcessArgs $1
        shift
    done
fi


GetCompiler
GetOS
GetSystem
GetBuildType
GetAvailableBackends
CursoryCompileCheck
ConfirmSetup
WriteHelperFiles



exit
