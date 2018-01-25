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


#include <Dynacoe/Util/Table.h>
#include <unordered_map>



using namespace Dynacoe;
using std::vector;


struct Library {
    Library() {TableIDpool = 1;} // table 0 never refers to a valid table.

    std::unordered_map<uint64_t, void *> id2inst;
    std::unordered_map<void *, std::vector<uint64_t>> inst2id;// ids owned by each table instance
    std::unordered_map<void *, std::unordered_map<uint64_t, uint64_t>> inst2dataEntry;// lookup reference ids owned by each table instance
    uint64_t TableIDpool;
    std::unordered_map<uint64_t, uint64_t> id2lookupPool; 
};


static Library * src = nullptr;


void LookupLibrary::AssertOpen() {
    if (!src) {
        src = new Library;
    }
}

bool LookupLibrary::IsTableAlias(uint64_t a, uint64_t b) {
    return (a == b || src->id2inst.find(a) == src->id2inst.find(b));
}

uint64_t LookupLibrary::AcquireTableID(void * table) {
    src->id2inst[src->TableIDpool] = table;
    src->inst2id[table].push_back(src->TableIDpool);
    return src->TableIDpool++;
}

void * LookupLibrary::CurrentTableID(uint64_t id) {
    return src->id2inst[id];
}


void LookupLibrary::RemoveTable(void * tableInst) {
    auto idsIter = src->inst2id.find(tableInst);
    vector<uint64_t> ids = idsIter->second;
    src->inst2id.erase(idsIter);

    for(uint64_t i = 0; i < ids.size(); ++i) {
        src->id2inst.erase(src->id2inst.find(ids[i]));
    }

    src->inst2dataEntry.erase(tableInst);
}


uint64_t LookupLibrary::MapTableData(uint64_t tableID, uint64_t dataSlot) {


    // get new id for this data
    auto lookupIDpool = src->id2lookupPool.find(tableID);
    if (lookupIDpool == src->id2lookupPool.end()) {
        src->id2lookupPool[tableID] = 0;
        lookupIDpool = src->id2lookupPool.find(tableID);
    }

    // apply data to data entry pair
    src->inst2dataEntry[src->id2inst[tableID]][(lookupIDpool->second)] = dataSlot;
    return (lookupIDpool->second)++;
}

void LookupLibrary::UnmapTableData(uint64_t tableID, uint64_t lookupID) {
    src->inst2dataEntry[src->id2inst[tableID]].erase(lookupID);
}

bool LookupLibrary::IsDataMapped(uint64_t tableID, uint64_t lookupID) {
    auto tableInstIter = src->id2inst.find(tableID);
    if (tableInstIter == src->id2inst.end()) return false;


    auto dataRefMapIter = src->inst2dataEntry.find(tableInstIter->second);
    if (dataRefMapIter == src->inst2dataEntry.end()) return false;


    return (dataRefMapIter->second).find(lookupID) != (dataRefMapIter->second).end();
}


uint64_t LookupLibrary::GetDataSlot(uint64_t tableID, uint64_t lookupID) {
    return src->inst2dataEntry[src->id2inst[tableID]][lookupID];
}







LookupID::LookupID() :
    TableID(0),
    lookupID(0){

}

bool LookupID::operator==(const LookupID & c) const {
    return c.TableID == TableID && c.lookupID == lookupID;
}

bool LookupID::operator!=(const LookupID & c) const {
    return c.TableID != TableID || c.lookupID != lookupID;
}


bool LookupID::Valid() const{
    return LookupLibrary::IsDataMapped(TableID, lookupID);
}





