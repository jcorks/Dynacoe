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


#include <Dynacoe/Components/DataTable.h>
#include <algorithm>
#include <cstring>
using namespace Dynacoe;


// extremely simple hash to make sure the data given to the ReadState function is sound
static uint64_t Checksum(const std::vector<uint8_t> & data, uint32_t begin, uint32_t count) {
    uint8_t partitions[8];
    partitions[0] = 0;
    partitions[1] = 0;
    partitions[2] = 0;
    partitions[3] = 0;
    partitions[4] = 0;
    partitions[5] = 0;
    partitions[6] = 0;
    partitions[7] = 0;
    for(uint64_t i = begin; i < count; ++i) {
        partitions[i%8] += (uint8_t)data[i];
    }
    
    uint64_t out;
    
    // Endianness?
    ((uint8_t*)(&out))[0] = partitions[0];
    ((uint8_t*)(&out))[1] = partitions[1];
    ((uint8_t*)(&out))[2] = partitions[2];
    ((uint8_t*)(&out))[3] = partitions[3];
    ((uint8_t*)(&out))[4] = partitions[4];
    ((uint8_t*)(&out))[5] = partitions[5];
    ((uint8_t*)(&out))[6] = partitions[6];
    ((uint8_t*)(&out))[7] = partitions[7];
    return out;
}


// utility class for storing generic data items in
// a vector. It serves as a "function set extension"
// that modifies the vector for its duration
class SafeWriteBuffer {
  public:
    SafeWriteBuffer(std::vector<uint8_t> & v) : data(v) {}
  
    // Put things in the buffer. It resizes data 
    // as need be.
    void Push(const void * d, uint64_t size) {
        uint64_t begin = data.size();
        data.resize(data.size()+size);
        memcpy(&data[begin], d, size);        
    }
    
  private:
    std::vector<uint8_t> & data;
};

/// utility class that adds a feature set to 
// conveniently and safely read from a vector
class SafeReadBuffer {

  public:
    SafeReadBuffer(const std::vector<uint8_t> & v) : data(v), iter(0) {}
  
    
    // reads bytes into d.
    // If reading bytes would have read beyond the buffer, 
    // d is left untouched.
    void Scan(void * d, uint64_t size) {
        memcpy(d, &data[iter], iter + size <= data.size() ? size : 0);
        iter += size;
    }
    
    // Scans chars until it hits a nul character or the end of the buffer 
    // then returns a string containing the scanned dta
    std::string ScanString() {
        std::string out;
        while(iter < data.size() && data[iter]) out += (char)data[iter++];
        if (iter < data.size()) iter++;
        return out;
    }
    
    bool AtEnd() {
        return iter == data.size();
        
    }
    
  private:
    uint64_t iter;
    const std::vector<uint8_t> & data;
};



DataTable & DataTable::Write(const std::string & name, int i) {
    if (EmitEvent("on-write", Entity::ID(), {name})) {
        DataEntry & e = Find(name, sizeof(i));
        memcpy(&data[e.begin], &i, sizeof(i));
    }
    return *this;

}

DataTable & DataTable::Write(const std::string & name, uint32_t i) {
    if (EmitEvent("on-write", Entity::ID(), {name})) {
        DataEntry & e = Find(name, sizeof(i));
        memcpy(&data[e.begin], &i, sizeof(i));
    }
    return *this;
}

DataTable & DataTable::Write(const std::string & name, uint64_t i) {
    if (EmitEvent("on-write", Entity::ID(), {name})) {
        DataEntry & e = Find(name, sizeof(i));
        memcpy(&data[e.begin], &i, sizeof(i));
    }
    return *this;
}


DataTable & DataTable::Write(const std::string & name, float i) {
    if (EmitEvent("on-write", Entity::ID(), {name})) {
        DataEntry & e = Find(name, sizeof(i));
        memcpy(&data[e.begin], &i, sizeof(i));
    }
    return *this;

}

DataTable & DataTable::Write(const std::string & name, double i) {
    if (EmitEvent("on-write", Entity::ID(), {name})) {
        DataEntry & e = Find(name, sizeof(i));
        memcpy(&data[e.begin], &i, sizeof(i));
    }
    return *this;
}


DataTable & DataTable::Write(const std::string & name, const std::string & str) {
    if (EmitEvent("on-write", Entity::ID(), {name})) {
        DataEntry & e = Find(name, str.size());
        memcpy(&data[e.begin], &str[0], str.size());
    }
    return *this;
}

DataTable & DataTable::Write(const std::string & name, const std::vector<uint8_t> & str) {
    if (EmitEvent("on-write", Entity::ID(), {name})) {
        DataEntry & e = Find(name, str.size());
        memcpy(&data[e.begin], &str[0], str.size());
    }
    return *this;
}

DataTable & DataTable::Write(const std::string & name, DataTable & i) {
    return DataTable::Write(name, i.WriteState());
}





DataTable & DataTable::Read(const std::string & name, int & i) {
    auto iter = nameTable.find(name);
    if (iter == nameTable.end()) return *this;
    int f;
    memcpy(&f, &data[entryTable[iter->second].begin], sizeof(i));
    i = f;
    EmitEvent("on-read", Entity::ID(), {name});
    return *this;
}

DataTable & DataTable::Read(const std::string & name, uint32_t & i) {
    auto iter = nameTable.find(name);
    if (iter == nameTable.end()) return *this;
    int f;
    memcpy(&f, &data[entryTable[iter->second].begin], sizeof(i));
    i = f;
    EmitEvent("on-read", Entity::ID(), {name});
    return *this;
}

DataTable & DataTable::Read(const std::string & name, uint64_t & i) {
    auto iter = nameTable.find(name);
    if (iter == nameTable.end()) return *this;
    int f;
    memcpy(&f, &data[entryTable[iter->second].begin], sizeof(i));
    i = f;
    EmitEvent("on-read", Entity::ID(), {name});
    return *this;
}



DataTable & DataTable::Read(const std::string & name, float & i) {
    auto iter = nameTable.find(name);
    if (iter == nameTable.end()) return *this;
    float f;
    memcpy(&f, &data[entryTable[iter->second].begin], sizeof(i));
    i = f;
    EmitEvent("on-read", Entity::ID(), {name});
    return *this;
}


DataTable & DataTable::Read(const std::string & name, double & i) {
    auto iter = nameTable.find(name);
    if (iter == nameTable.end()) return *this;
    float f;
    memcpy(&f, &data[entryTable[iter->second].begin], sizeof(i));
    i = f;
    EmitEvent("on-read", Entity::ID(), {name});
    return *this;
}


DataTable & DataTable::Read(const std::string & name, std::string & i) {
    auto iter = nameTable.find(name);
    if (iter == nameTable.end()) return *this;
    uint64_t size = entryTable[iter->second].count;
    i.resize(size);
    memcpy(&i[0], &data[entryTable[iter->second].begin], size);
    EmitEvent("on-read", Entity::ID(), {name});
    return *this;
}

DataTable & DataTable::Read(const std::string & name, std::vector<uint8_t> & i) {
    auto iter = nameTable.find(name);
    if (iter == nameTable.end()) return *this;
    uint64_t size = entryTable[iter->second].count;
    i.resize(size);
    memcpy(&i[0], &data[entryTable[iter->second].begin], size);
    EmitEvent("on-read", Entity::ID(), {name});
    return *this;
}

DataTable & DataTable::Read(const std::string & name, DataTable & i) {
    std::vector<uint8_t> dataCopy;
    Read(name, dataCopy);
    i.ReadState(dataCopy);
    return *this;
}

void DataTable::Remove(const std::string & name) {
    if (EmitEvent("onr-remove", Entity::ID(), {name})) {
        auto iter = nameTable.find(name);
        if (iter == nameTable.end()) return;
        nameTable.erase(iter);
    }
}

bool DataTable::Query(const std::string & name) const {
    return (nameTable.find(name) != nameTable.end());
}




std::vector<uint8_t> DataTable::WriteState() {
    // crush any memory gaps so that writeState gives the smallest
    // possible representation
    
    Consolidate();
    
    // Data format: -Header-
    //              [1byte version number, "COEBEEF"]
    //              [8B numEntries]
    //              [8B dataBufferSize]
    
    //              -Meaningful Data-    
    //              [string1][0][string2][0]...[string[numEntries]][0]
    //              [8B begin1][8B count1][8B begin2][8B count2]...
    //              [dataBlock]

    //              -Footer-
    //              [checksum (on all previous data: 0, filesize-8)(8B)]
    std::vector<uint8_t> out;
    SafeWriteBuffer b(out);

        
        
    // Header
    const char header[]  = {'\1', 'C', 'O', 'E', 'B', 'E', 'E', 'F'};
    b.Push(&header, 8);
    
    uint64_t count = entryTable.size();
    b.Push(&count, sizeof(count));
    
    count = data.size();
    b.Push(&count, sizeof(count));
    
    
    
    
    
    
    // meaningfulData
    for(auto i = nameTable.begin(); i != nameTable.end(); ++i) {
        b.Push(i->first.c_str(), i->first.size()+1); 
    }
    DataEntry entry;
    for(auto i = nameTable.begin(); i != nameTable.end(); ++i) {
        entry = entryTable[i->second];
        b.Push(&entry.begin, sizeof(uint64_t)); 
        b.Push(&entry.count, sizeof(uint64_t)); 
    }
    
    b.Push(&data[0], data.size());
    
    
    
    
    // footer
    uint64_t checksum = Checksum(out, 0, out.size());
    b.Push(&checksum, sizeof(uint64_t));
    
    return out;
}

bool DataTable::ReadState(const std::vector<uint8_t> & state) {
    if (state.size() < 8) return false;
        // Header
    if (state[0] != '\1' ||
        state[1] != 'C' ||
        state[2] != 'O' ||
        state[3] != 'E' ||
        state[4] != 'B' ||
        state[5] != 'E' ||
        state[6] != 'E' ||
        state[7] != 'F') {
        return false;
    }

    // checksum 
    uint64_t checksum = Checksum(state, 0, state.size()-8);
    if (
         ((uint8_t *) &checksum)[0] != state[state.size()-8] ||
         ((uint8_t *) &checksum)[1] != state[state.size()-7] ||
         ((uint8_t *) &checksum)[2] != state[state.size()-6] ||
         ((uint8_t *) &checksum)[3] != state[state.size()-5] ||
         ((uint8_t *) &checksum)[4] != state[state.size()-4] ||
         ((uint8_t *) &checksum)[5] != state[state.size()-3] ||
         ((uint8_t *) &checksum)[6] != state[state.size()-2] ||
         ((uint8_t *) &checksum)[7] != state[state.size()-1]) {                 
         
        return false;
    }
    
    DataTable table;
    
    SafeReadBuffer b(state);
    
    // Header
    uint64_t d;
    b.Scan(&d, sizeof(uint64_t));
    uint64_t numEntries;
    b.Scan(&numEntries, sizeof(uint64_t));
    uint64_t bufferSize;
    b.Scan(&bufferSize, sizeof(uint64_t));
    
    
    // meaningfulData
    std::string str;
    for(uint64_t i = 0; i < numEntries; ++i) {
        table.nameTable[b.ScanString()] = i;
    }
    
    table.entryTable.resize(numEntries);
    DataEntry entry;
    for(uint64_t i = 0; i < numEntries; ++i) {
        b.Scan(&entry.begin, sizeof(uint64_t));
        b.Scan(&entry.count, sizeof(uint64_t));
        table.entryTable[i] = entry;
    }
    
    table.data.resize(bufferSize);
    b.Scan(&table.data[0], bufferSize);
    
    
    // swap working contents with self to set
    std::swap(nameTable, table.nameTable);
    std::swap(data, table.data);
    std::swap(entryTable, table.entryTable);
    
    EmitEvent("on-read-state");
    return true;
    
}

void DataTable::Clear() {
    nameTable.clear();
    entryTable.clear();
    data.clear();
    
}



DataTable::DataEntry & DataTable::Find(const std::string & str, uint64_t size) {
    auto iter = nameTable.find(str);
    if (!(iter != nameTable.end() &&
          entryTable[iter->second].count == size)) { // uh-oh, new var associated with name
        DataEntry out;
        out.begin = GetNewBlock(size);
        out.count = size;
        nameTable[str] = entryTable.size();
        entryTable.push_back(out);
        return entryTable[entryTable.size()-1];
    } 
    
    return entryTable[iter->second];
}

uint64_t DataTable::GetNewBlock(uint64_t d) {
    uint64_t out = data.size();
    data.resize(data.size() + d);
    return out;
}


void DataTable::Consolidate() {
    if (entryTable.size() == nameTable.size()) return;
    DataTable table;
    
    uint64_t dataIndex = 0;
    // presize the vector. Will always be smaller than original
    table.data.reserve(data.size());
    
    for(auto iter = nameTable.begin(); iter != nameTable.end(); ++iter) {
        DataEntry d = entryTable[iter->second];
  
  
        // reform entry into new table
        table.nameTable[iter->first] = table.entryTable.size();
        for(uint64_t i = 0; i < d.count; ++i)
            table.data.push_back(data[d.begin+i]);
        
        d.begin = dataIndex; // reset index
        table.entryTable.push_back(d);
        dataIndex += d.count;
    }
    
    std::swap(nameTable,  table.nameTable);
    std::swap(data,       table.data);
    std::swap(entryTable, table.entryTable);
}

DataTable::DataTable() : Component() {
    SetTag("DataTable");
    InstallEvent("on-write");
    InstallEvent("on-read");
    InstallEvent("on-remove");
    InstallEvent("on-read-state");
}



