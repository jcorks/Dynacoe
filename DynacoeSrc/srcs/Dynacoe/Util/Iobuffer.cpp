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


#include <Dynacoe/Util/Iobuffer.h>
#include <fstream>
#include <algorithm>
using namespace Dynacoe;
using namespace std;

const int initSize_c = 64;
const int immediate_size_c = 1024;

OutputBuffer::OutputBuffer() {
    bufferPos = 0;
    buffer = new uint8_t[initSize_c];
    operationalSize = 0;
    size = initSize_c;

}


OutputBuffer::~OutputBuffer() {
    delete[] buffer;

}

void OutputBuffer::resize() {

    uint32_t newSize = (size+1)*2;
    uint8_t * newBuffer = new uint8_t[newSize];
    memset(newBuffer, size, 0);
    memcpy(newBuffer, buffer, size);

    size = newSize;
    delete[] buffer;
    buffer = newBuffer;
}

void OutputBuffer::WriteBytes(const std::vector<uint8_t> & data) {
    while(data.size() + bufferPos >= size) {
        //cout << "Needs to resize to " << size*2 << endl;
        resize();
    }

    for(int i = 0; i < data.size(); i++) {
        buffer[bufferPos] = data[i];
        ++bufferPos;
    }
    operationalSize += data.size();
}





std::vector<uint8_t> OutputBuffer::GetData() {
    std::vector<uint8_t> b(operationalSize);
    memcpy(&b[0], buffer, operationalSize);
    return b;
}

int OutputBuffer::Size() {
    return operationalSize;
}

void OutputBuffer::Clear() {
    delete[] buffer;
    buffer = NULL;
    size = 0;
    operationalSize = 0;
    bufferPos = 0;
}

void OutputBuffer::WriteString(std::string str) {
    for(unsigned int i = 0; i < str.size(); ++i) {
        Write<uint8_t>(str[i]);
    }
}


bool OutputBuffer::CommitToFile(std::string fileName, bool append) {

	std::ofstream out_file;
	std::filebuf *out_ptr;
    if (append)
        out_file.open(fileName.c_str(), std::ios::app | std::ios::binary);
    else
        out_file.open(fileName.c_str(), std::ios::binary);

	out_ptr = out_file.rdbuf();

	bool good = out_file.good();


	// put the sequence of [size] uint8_ts into the new file
	out_ptr->sputn((char*)buffer, bufferPos);

	out_file.close();

	return good;
}



void OutputBuffer::GoToByte(int byte) {
    bufferPos = byte;
    if (bufferPos > size - 1)
        bufferPos = size - 1;
    if (bufferPos < 0)
        bufferPos = 0;

}



InputBuffer::InputBuffer() {
    size = 0;
    buffer = nullptr;
    bufferPos = 0;
    immediateBuffer = new uint8_t[immediate_size_c];
}

void InputBuffer::Open(const std::string & newFile) {
    // First check if we even have a valid file.
    // If we don't don't modify the state
    std::filebuf file;
    file.open(newFile, std::ios::in | std::ios::binary);
    if (!file.is_open())  {
        return;
    }



    // We now have a valid file, but we need to make sure reading is okay
	int newSize = file.pubseekoff(0, std::ios::end, std::ios::in);
	file.pubseekpos(0, std::ios::in);

    if (newSize <= 0) {
        return;
    }


    // At this point we can
	// create space for the .CIM file

	uint8_t * newBuffer = new uint8_t[newSize];

	// stores entire file as array of bytes.
	// "sequence: get n uint8_tacters" => sgetn
	file.sgetn((char*)newBuffer, newSize);


    
    // Now we can guarantee everything will work
    if (buffer) delete[] buffer;    
    buffer = newBuffer;
    size = newSize;
	bufferPos = 0;
    file.close();

}

void InputBuffer::OpenBuffer(const vector<uint8_t> & rawInput) {
    if (!rawInput.size()) return;

    bufferPos = 0;
    size = rawInput.size();
    if (buffer) delete[] buffer;
    buffer = new uint8_t[size];
    memcpy(buffer, &rawInput[0], size);

}




bool InputBuffer::ReachedEnd() {
    return (bufferPos >= size);
}



int InputBuffer::BytesLeft() {
    return size - bufferPos;
}

void InputBuffer::readN(void * d, uint32_t num) {
    if (bufferPos + num > size) return;
    memcpy(d, buffer + bufferPos, num);
    bufferPos+=num;
}

void * InputBuffer::readN(int num) {
    if (bufferPos + num > size) return NULL;

    uint8_t * out;
    if (num <= immediate_size_c)
        out = immediateBuffer;
    else 
        out = new uint8_t[num];


    memcpy(out, buffer + bufferPos, num);
    bufferPos+=num;

    return ((void*) out);

}

std::vector<uint8_t> InputBuffer::ReadBytes(uint32_t n) {
    int bytesToRead = std::min(n, (uint32_t) size - bufferPos); 
    uint8_t * target = (uint8_t *) readN(bytesToRead);
    if (!target) return std::vector<uint8_t>();
    std::vector<uint8_t> out;
    out.resize(bytesToRead);
    memcpy(&out[0], target, bytesToRead);
    if (bytesToRead > immediate_size_c)
        delete[] target;
    return out;
    
}


void InputBuffer::GoToByte(int n) {
    if (!size) return;
    bufferPos = n;
    if (bufferPos > size - 1)
        bufferPos = size - 1;
    if (bufferPos < 0)
        bufferPos = 0;

}


InputBuffer::~InputBuffer() {
    if(buffer)
        delete[] buffer;
    delete[] immediateBuffer;

}

int InputBuffer::Size() {
    return size;
}

std::string InputBuffer::ReadString(unsigned int nBytes) {
    if (nBytes > size) nBytes = size;

    std::string out;

    for(unsigned int i = 0; i < nBytes; ++i) {
        out += Read<uint8_t>();
    }

    return out;
}




