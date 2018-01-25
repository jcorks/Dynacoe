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

/*  Input / OutputBuffer classes:
 *  A complete and concise solution for binary data with data streams.
 *

 */

#ifndef H_IOBUFFER_INCLUDED
#define H_IOBUFFER_INCLUDED

#include <string>
#include <cstring>
#include <vector>



namespace Dynacoe {
/**
 *\brief Binary file reading utility.

 * InputBuffer, as the name implies, handles input data. Once instantiated
 * with a target, the InputBuffer can then easily read and process binary data.
 * InputBuffers are the most effective for unpacking data, especially data
 * that is packed in accordance to OutputBuffer routines, as all
 * OutputBuffer writing functions mirror the reading functions of InputBuffer.
 * In the event that one wishes to processes text, the InputBuffer features
 * a handy token grabber as well as the ability to process embedded C-Style
 * strings, making them a viable alternative to other input streams.
 */

class InputBuffer {


  public:

    InputBuffer();
    ~InputBuffer();

    /// \brief Opens a raw buffer to read from.
    ///
    void OpenBuffer(const std::vector<uint8_t> & buffer);

    /// \brief Opens a file to read from.
    ///
    /// file should contain a path to a valid file.
    void Open(const std::string & file);

    /// \brief  Returns the next nBytes as a string.
    ///
    std::string ReadString(unsigned int nBytes);

    /// \brief Returns the next n bytes.
    /// 
    /// If there are less than numBytes remaining,
    /// the remaining bytes are returned.
    std::vector<uint8_t> ReadBytes(uint32_t numBytes);

    /// \brief  Reads the next sizeof<T> bytes adn returns a T object with
    /// its state set to the read contents.
    ///
    /// Note: If used with user objects, be aware that this only performs a shallow copy
    /// If not enough bytes are left to acoomodate the data object, the contents are undefined.
    template<typename T>
    T Read();

    /// \brief Reads the next sizeof(T) bytes into the given reference
    ///    
    /// While similar to the other Read functions, this will likely yeild
    /// the best performance.
    ///
    template<typename T>
    void Read(T &);

    /// \brief  Set the buffer position byte.
    ///
    /// If this operation operation were to go beyond the first index of data,
    /// it will move to the very first index.
    /// If a fastforward() operation were to go beyond the last valid index,
    /// it will move to the very last index.
    void GoToByte(int nBytes);


    /// \brief  returns the size of the buffer in bytes.
    ///
    int Size();

    /// \brief Returns the number of bytes left in the buffer.
    ///
    int BytesLeft();

    /// \brief Returns whether or not all data has been read.
    ///
    bool ReachedEnd();
  private:
    uint8_t * buffer;
    int bufferPos;
    long size;
    void * readN(int num);
    void readN(void *, uint32_t);
    uint8_t * immediateBuffer;

};

/**
 * 
 * \brief Binary file writing utility.
 *
 * OutputBuffers are the complement to InputBuffers. Once instantiated, the
 * user can easily and quickly write data to the buffer in a variety of
 * formats verbatim. Once the user is ready, they can write the buffer to
 * either a new file or either write over or append an existing file.
 * It is also possible to view already written data to the buffer
 * via getData(). If they so wish, the user may also clear the entire
 * buffer that they have written to as well.
 */

class OutputBuffer {
  public:


    OutputBuffer();
    ~OutputBuffer();

    /// \brief  Stores the written data to the specified file.
    ///
    bool CommitToFile(std::string fileName, bool append);

    /// \name Writing Data
    ///
    /// Write various kinds of data to the queued buffer.
    /// The variations are equivalent to InputBuffer's reading.
    ///\{
    template<typename T>
    void Write(const T &);
    void WriteBytes(const std::vector<uint8_t> & data);
    void WriteString(std::string str);
    ///\}

    /// \brief Set the buffer position to the specifified byte
    ///
    void GoToByte(int n);

    /// \brief Returns a copy of the queued data.
    ///    
    std::vector<uint8_t> const GetData();

    /// \brief Returns the size of the queued data in bytes.
    ///    
    int Size();

    /// \brief Resets the queued data buffer.
    ///    
    void Clear();

  private:
    uint8_t * buffer;
    int bufferPos;
    long size;
    long operationalSize;

    void resize();




};
}









template <typename T>
void Dynacoe::OutputBuffer::Write(const T & obj) {
    uint8_t * block = (uint8_t *) &obj;
    std::vector<uint8_t> data(block, block+sizeof(T));
    WriteBytes(data);
}


template <typename T>
T Dynacoe::InputBuffer::Read() {
    T object;
    std::vector<uint8_t> data = ReadBytes(sizeof(T));
    if (!data.size()) return object;
    memcpy(&object, &data[0], sizeof(object));
    return object;
}


template <typename T>
void Dynacoe::InputBuffer::Read(T & o) {
    readN(&o, sizeof(o));
}
#endif
