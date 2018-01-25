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



#ifndef H_DC_DATA_TABLE_INCLUDED
#define H_DC_DATA_TABLE_INCLUDED

#include <Dynacoe/Component.h>
#include <unordered_map>
#include <vector>
#include <string>

namespace Dynacoe {
    
/// \brief A container meant to share values across program invocations
///
/// Often with programs, there is a need to preserve data beyond the execution of the 
/// program. Sometimes, it is enough to simply write to a text file, but this isn't 
/// always flexible, and has little resistance against device errors, forms 
/// of corruption, and user tampering. DataTable provides a non-sequential, corruption-resistant 
/// way to store data. Using the Write functions, you can store named variables 
/// , data, and even other DataTables in a safe manner. Then WriteState() can be 
/// used to retrieve a data buffer that represents the DataTable's exact state,
/// which can then be used later in another instance.
///
/// Supported component events:
/// - on-read: after reading a piece of data. (args: name);
/// - on-write: before writing a piece of data. The default handler actually performs the write, so stopping propogation will prevent writing.(args: name);
/// - on-remove: before removing a piece of data. The default handler actually performs the removal, so stopping propogation will prevent removal. (args: name);
/// - on-read-state: after successfully reading a state
class DataTable : public Component {
  public:
    DataTable();
      
    
    /// \name Writing 
    /// Associates a datum with a name.
    ///
    /// @param name The name that the data will be associated with 
    /// @param i    The data to store 
    ///\{
    DataTable & Write(const std::string & name,  int      i);
    DataTable & Write(const std::string & name,  uint32_t i);
    DataTable & Write(const std::string & name,  uint64_t i);
    DataTable & Write(const std::string & name,  float    i);
    DataTable & Write(const std::string & name,  double   i);
    DataTable & Write(const std::string & name,  const std::string & i);
    DataTable & Write(const std::string & name,  const std::vector<uint8_t> & i);
    DataTable & Write(const std::string & name,  DataTable & i);
    ///\}
    
    /// \name Reading
    /// Retrieves a datum associated with a name. If there is no such association,
    /// the input reference is left untouched.
    /// 
    /// @param name The name associated with the data.
    /// @param i    A reference to retrieve the datum with.
    ///\{
    DataTable & Read(const std::string & name, int         & i);
    DataTable & Read(const std::string & name, uint32_t    & i);
    DataTable & Read(const std::string & name, uint64_t    & i);
    DataTable & Read(const std::string & name, float       & i);
    DataTable & Read(const std::string & name, double      & i);
    DataTable & Read(const std::string & name, std::string & i);
    DataTable & Read(const std::string & name, std::vector<uint8_t> & i);
    DataTable & Read(const std::string & name, DataTable & i);
    ///\}
    
    /// \brief Removes any datum-name association.
    ///
    void Remove(const std::string &);
    
    /// \brief Returns whether there is a datum-name association 
    /// with the given name.
    ///
    bool Query(const std::string &) const;


    /// \brief Writes a pure data-form of the table's contents. This can then be 
    /// passed into ReadState() turn the calling object into an exact copy 
    /// of this one.
    ///
    /// WriteState()/ReadState() were intended to be used in conjuction to preserve data contents of 
    /// the DataTable across program invocations. Along with writing the exact state of 
    /// the DataTable, the data includes header and checksum data to improve parity across 
    /// execution states.
    std::vector<uint8_t> WriteState();
    
    
    /// \brief Sets the DataTable state to reflect the state
    /// defined by the input buffer.
    ///
    /// The buffer is implied to have been produced from a WriteState call.
    /// If not, or corruption on some device has occurred, 
    /// there is a good chance that this function will detect it before applying the data. 
    /// In the case that an error does occur, false will be returned and no action taken.
    bool ReadState(const std::vector<uint8_t> &);
    
    
    /// \brief Removes all stored associations and data entries, restoring 
    /// the container back to a default state.
    ///
    void Clear();
    

  private:  
    struct DataEntry {
        DataEntry() :
            begin(0),
            count(0)
        {}
        
        DataEntry(uint64_t b, uint64_t c) :
            begin(b),
            count(c){}
        uint64_t begin;
        uint64_t count;
    };
  
    std::unordered_map<std::string, uint64_t> nameTable; 
    std::vector<DataEntry> entryTable;
    std::vector<uint8_t> data;
    
    DataEntry & Find(const std::string &, uint64_t);
    uint64_t GetNewBlock(uint64_t);
    void Consolidate();
};
    
    
    
}



#endif