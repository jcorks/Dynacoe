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

#ifndef H_COEDEUTIL_DICTIONARY_INCLUDED
#define H_COEDEUTIL_DICTIONARY_INCLUDED

#include <cstddef>
#include <vector>
#include <stack>
#include <utility>
#include <cstdint>

#include <iostream>
namespace Dynacoe {






class LookupID;

/// \brief An associative container that
/// issues each member an ID guaranteed to be unique for the duration of
/// the program. 
///
/// Every operation is average-case constant time.
template<typename T>
class Table {
  public:
    Table();
    ~Table();

    /// \brief
    /// Adds an entry to the lookup table.
    /// A lookupID is returned.
    LookupID Insert(T);

    /// \brief Dissasociates an ID from an entry.
    ///
    void Remove(const LookupID &);


    /// \brief Returns whehter or not the ID is owned by this table.
    ///
    bool Query(const LookupID &);

    /// \brief Finds an entry based on the ID.
    ///
    /// THe ID must be owned by the table. If it does not, the results are undefined (but usually a 
    /// null object which is implementation-defined in its handling)
    T & Find(LookupID);
    
    /// \brief Returns a std::vector holding each member added
    ///
    std::vector<T> List();


  private:
    uint64_t thisID;

    std::stack<size_t> deadList;
    std::vector<T> entryList;
    std::vector<bool>  entryValid;


};




class LookupID {
public:
    LookupID();


    // Returns whether or not this LookupID refers to a valid data entry in a table
    bool Valid() const;

    bool operator==(const LookupID & c) const;
    bool operator!=(const LookupID & c) const;

    friend bool operator<(const LookupID & l, const LookupID & r) {
        if (l.TableID == r.TableID) {
            return l.lookupID < r.lookupID;
        }
        return false;
    }

  private:
    template<typename T>
    friend class Table;


    uint64_t TableID;
    uint64_t lookupID;

    LookupID(size_t TableID_, size_t id_) :
        TableID(TableID_),
        lookupID(id_){}
};






////// Implementation!



// private container class for Table<T>
class LookupLibrary {

    template<typename T>
    friend class Table;

    friend class LookupID;

    // initializes the LookupLibrary assets. If already inited, does nothing
    static void AssertOpen();

    // Returns if two ids refer to the same physical table
    static bool IsTableAlias(uint64_t a, uint64_t b);
    
    // Assign a new id for the table pointer to by the pointer
    static uint64_t AcquireTableID(void *);

    // Returns the Table instance for the ID given.
    static void * CurrentTableID(uint64_t);

    // dissasociates all assets of the table
    static void RemoveTable(void *);


    // Claims the data slot for a new piece fo data. The lookupID belonging to the 
    // table id is returned
    static uint64_t MapTableData(uint64_t TableID, uint64_t dataSlot);

    // Dissasociates the lookup id from the table id
    static void UnmapTableData(uint64_t TableID, uint64_t lookupID);

    // Returns whether or not the tableID-lookupID combo refers to a slot held by a Table instance.
    static bool IsDataMapped(uint64_t TableID, uint64_t lookupID);

    // Returns the data slot of the owning table-lookupID combo 
    static uint64_t GetDataSlot(uint64_t TableID, uint64_t lookupID);



};



template<typename T>
Table<T>::Table() {
    LookupLibrary::AssertOpen();
    thisID = LookupLibrary::AcquireTableID(this);
}

template<typename T>
Table<T>::~Table() {
    LookupLibrary::RemoveTable(this);
}

template<typename T>
LookupID Table<T>::Insert(T item) {

    if (!deadList.empty()) {
        size_t id = deadList.top();
        entryList[id]  = item;
        entryValid[id] = true;
        deadList.pop();
        return LookupID(thisID, LookupLibrary::MapTableData(thisID,  id));
    }
    entryList.push_back(item);
    entryValid.push_back(true);
    LookupID out(thisID, LookupLibrary::MapTableData(thisID, entryList.size() - 1));



    return out;
}

template<typename T>
void Table<T>::Remove(const LookupID & id) {
    if (!id.Valid()) return;


    uint64_t vectorSlot = LookupLibrary::GetDataSlot(id.TableID, id.lookupID);
    entryValid[vectorSlot] = false;
    deadList.push(vectorSlot);
    LookupLibrary::UnmapTableData(id.TableID, id.lookupID);
}


template<typename T>
bool Table<T>::Query(const LookupID & id) {
    if (!id.Valid()) return false;
    
    return LookupLibrary::IsTableAlias(id.TableID, thisID);
}

template<typename T>
T & Table<T>::Find(LookupID id) {
    if (!id.Valid()) {
        T * out = 0x0;
        return *out;
    }; // have fun, you dummy
    return entryList[LookupLibrary::GetDataSlot(thisID, id.lookupID)];
}



template<typename T>
std::vector<T> Table<T>::List() {
    std::vector<T> out;
    for(size_t i = 0; i < entryList.size(); ++i) {
        if (entryValid[i]) {
            out.push_back(entryList[i]);
        }
    }
    return out;
}


}

#endif
