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

#include <Dynacoe/Decoders/DecodeOGG.h>
#include <vorbis/vorbisfile.h>
#include <Dynacoe/Modules/Sound.h>
#include <Dynacoe/Util/Iobuffer.h>
#include <Dynacoe/AudioBlock.h>

using std::string;
using std::vector;
using namespace Dynacoe;
using Dynacoe::InputBuffer;


struct OVDataSource {
    const uint8_t * buffer;
    uint64_t size;
    uint64_t position;
};


static size_t ov_memory_read_func(void * ptr, size_t size, size_t nmemb, void * datasource);
//static int    ov_memery_seek_func(void * datasource, ogg_int64_t offset, int whence);
//static int    ov_memory_close_func(void * datasource);
//static int    ov_memory_tell_func(void * datasource);






struct _OggBlock {
    _OggBlock(){
        size = 4096;
        data = new char [4096];
    }
    ~_OggBlock() {
        delete[] data;
    }
    char * data;
    long size;
};
Asset * DecodeOGG::operator()(
    const std::string & path,
    const std::string &,
    const uint8_t * buffer,
    uint64_t sizeB

) {
    // Uses vorbisfile library to decode
    OggVorbis_File oggFile;

    OVDataSource datasource = {
        buffer,
        sizeB,
        0
    };
    int status = ov_open_callbacks(
        &datasource,
        &oggFile,
        NULL,
        0,
        ov_callbacks{
            ov_memory_read_func,
            nullptr,
            nullptr,
            nullptr
        }
    );

    /*
    int status = ov_open_callbacks(
            fopen(path.c_str(), "rb"),
            &oggFile,
            NULL,
            0,
            OV_CALLBACKS_DEFAULT
    );
    */
    if (status != 0) {
        Console::Error()  << "[OGG] Couldn't read ogg " << path
                << "    ->";
        if (status == OV_EREAD)      Console::Error() << "Media read error\n";
        if (status == OV_ENOTVORBIS) Console::Error() << "File is not a Vorbis Ogg file.\n";
        if (status == OV_EVERSION)   Console::Error() << "Vorbis version mismatch\n";
        if (status == OV_EBADHEADER) Console::Error() << "Invalid Vorbis header. File possibly corrupt...\n";
        if (status == OV_EFAULT)     Console::Error() << "Internal logic fault. SOMETHING REALLY BAD HAPPENED\n";

        return NULL;

    }

    // next, make sure it is compatible with our format.
    vorbis_info * info = ov_info(&oggFile, -1);
    int numChannels = info->channels;
    int rate = info->rate;

    if (numChannels != 2) {
        Console::Warning()  << "[OGG] Reading " << path.c_str()
                << "   -> Number of channels is not 2. Attempting to compensate...\n";
    }

/*
    if (rate != DYNACOE_SAMPLE_RATE) {
        Console::Info()  << "[OGG] Reading " << path.c_str()
                << "   -> Sample rate is different than Dynacoe's. Expect altered playback speed.";
    }
*/





    // now read
    vector<_OggBlock *> blockList;
    int section = 0;
    int totalSize = 0;
    std::vector<char> bufferDecoded;
    _OggBlock nextBlock;
    
    while(true) {
        int size = ov_read(&oggFile,
                           nextBlock.data,
                           nextBlock.size,
                           0, 2, 1, &section);
        if (!size) {
            //Console::Info()  << "Done reading..\n";
            break;
        }
        nextBlock.size = size;
        if (totalSize+size > bufferDecoded.size()) {
            bufferDecoded.resize(bufferDecoded.size()+1024*1024);
        }
        memcpy(&bufferDecoded[totalSize], nextBlock.data, size);
        totalSize += size;
    }

    AudioBlock * out = new AudioBlock(path);
    /*char * data = new char[totalSize];
    uint32_t size = totalSize;

    int bufferPos = 0;
    for(int i = 0; i < (int) blockList.size(); ++i) {
        _OggBlock * block = blockList[i];
        memcpy(data + bufferPos, block->data, block->size);
        bufferPos += block->size;
        delete block;
    }*/
    
    out->Define((AudioSample*)&bufferDecoded[0], totalSize/sizeof(AudioSample));
    out->SetVolume(.9);
    //delete[] data;
    //out->volume = 255;

    return out;
}






///// statics
size_t ov_memory_read_func(void * ptr, size_t size, size_t nmemb, void * datasource) {
    OVDataSource * data = (OVDataSource*)(datasource);
    uint64_t bytesLeft = data->size - data->position;
    size_t   numRead = (bytesLeft < size*nmemb ? bytesLeft : size*nmemb);
    memcpy(ptr, data->buffer+data->position, numRead);
    data->position += numRead;
    return numRead;
}









