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
#include <Dynacoe/Backends/Display/NoDisplay_Multi.h>


using namespace Dynacoe;
using std::string;
using std::vector;

NoDisplay::NoDisplay(){}
NoDisplay::~NoDisplay(){}


std::string NoDisplay::Name() {return "NoDisplay";}
std::string NoDisplay::Version() {return "v1.0";}
bool NoDisplay::Valid() {return true;}



void NoDisplay::Resize(int, int){}
void NoDisplay::SetPosition(int, int){}
void NoDisplay::Fullscreen(bool){}
void NoDisplay::Hide(bool){}
void NoDisplay::LockClientResize(bool){}
void NoDisplay::LockClientPosition(bool){}
void NoDisplay::SetViewPolicy(ViewPolicy){}


int NoDisplay::Width(){return 0;}
int NoDisplay::Height(){return 0;}
int NoDisplay::X(){return 0;}
int NoDisplay::Y(){return 0;}


void NoDisplay::SetName(const string &){}
void NoDisplay::AddResizeCallback(NoDisplay::ResizeCallback*){}
void NoDisplay::RemoveResizeCallback(NoDisplay::ResizeCallback*){}
void NoDisplay::AddCloseCallback(NoDisplay::CloseCallback*){}
void NoDisplay::RemoveCloseCallback(NoDisplay::CloseCallback*){}

bool NoDisplay::IsCapable(NoDisplay::Capability){return false;}
Dynacoe::Display::DisplayHandleType NoDisplay::GetSystemHandleType(){return DisplayHandleType::Unknown;}
void *  NoDisplay::GetSystemHandle(){return nullptr;}
Dynacoe::Display::DisplayEventType NoDisplay::GetSystemEventType(){return DisplayEventType::Unknown;}
void * NoDisplay::GetLastSystemEvent(){return nullptr;}
