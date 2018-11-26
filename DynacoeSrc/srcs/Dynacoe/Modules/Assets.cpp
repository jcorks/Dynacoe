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

#include <Dynacoe/Modules/Assets.h>

#include <Dynacoe/Image.h>
#include <Dynacoe/Dynacoe.h>
#include <Dynacoe/Util/Filesys.h>
#include <Dynacoe/Util/Iobuffer.h>
#include <Dynacoe/Modules/Sound.h>
#include <vorbis/vorbisfile.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/FontAsset.h>
#include <Dynacoe/RawData.h>
#include <Dynacoe/Decoders/Decoder.h>
#include <Dynacoe/Encoders/Encoder.h>
#include <Dynacoe/AudioBlock.h>
#include <Dynacoe/Model.h>

using namespace std;
using namespace Dynacoe;
using namespace Dynacoe;



static void notFoundError(Assets::Type, const std::string & name);
static std::string typeToString(Assets::Type);

static std::map<std::string, Decoder *> decoders;
static std::vector<Asset *> errorInstances;
static std::vector<std::map<std::string, Encoder *>> encoders;
static AssetID storeGen(const std::string &, Asset *, Assets::Type);
static void LoadDecoders();
static void LoadDecoder(Decoder *);
static Decoder * GetDecoder(const std::string &);
static void LoadEncoders();
static void LoadEncoder(Encoder *);
static bool Encode(int, Asset *, const std::string & ext, const std::string &);

// all storage buckets
static std::vector<Asset *> assetList[AssetID::NUMTYPES];
// zombie buckets of indices
static std::stack<int> deadList[AssetID::NUMTYPES];
static std::unordered_map<std::string, AssetID> assetMap[AssetID::NUMTYPES];
static void storeSystemImages();



static Asset * CreateAsset(Assets::Type type, const std::string & str);

// Dynacoe's Asset Registration Process

// The string representing the data is
// hashed and mapped to an index. The index
// refers to where it is placed in the vector.
//
// Registration:
//      - path is hashed to check is already existed        (O(path.length()))
//      - if it does AND != -1,
//                    then done.                            done
//      - else,
//          get the next immediate free index.
//          - if !deadList.empty()                          (O(1))
//              - then deadList.pop()                       (O(1))
//              - use this index as the free index
//              - chunkList[free index] = AudioBlock        (O(1))
//          - else
//              - chunkList.push_back(Asset)           (average = O(1), worst = O(n))
//              - use size() - 1 as the free index
//      - hash(path, free index)                            (O(path.length()))
//      - return index
// Retrieval (index)
//      - if index < -1, retrun NULL
//      - return chunkList[index]                           (O(1))
// Retrieval (string)
//      - if (hash(path) == DNE || -1) return NULL;         (O(path.length))
//      - else return index
// Removal
//      - hash(string, -1);                                 (O(path.length())
//      - deadList.push(old index);                         (O(1))
//      - chunkList[old index] == NULL;                     (O(1))
//      - * free data *                                     (worst = O(Asset.size())???)



// Private Members








static size_t assetCounter;








// Private Methods


static void initBase();
/* Image storage */









static const int DYNACOE_SAMPLE_RATE =        44100;       // (Samples processed per second)


Asset::~Asset(){}

static std::string fSearch(const std::string &);

void Assets::Init() {



    for(int i = 0; i < (int)Type::NoType; ++i) {
        encoders.push_back(std::map<std::string, Encoder*>());
        errorInstances.push_back(nullptr);
    }

    LoadDecoders();
    LoadEncoders();
}








AssetID Assets::Load(const string & ext, const string & name, bool standard) {
    string path;



    Decoder * dec = GetDecoder(ext);
    if (!dec) return AssetID();


    if (standard) {
        path = (fSearch(name.c_str())).c_str();
        if (path == "") {
            notFoundError(dec->GetType(), name);
            return AssetID();
        }
    } else
        path = name;


    InputBuffer * f = new InputBuffer;
    f->Open(path.c_str());
    if (!f->Size()) {
        Console::Error() << "[Dynacoe::Assets]: Failed to load file " << name<< Console::End();
        delete f;
        return AssetID();
    }

    auto out =  LoadFromBuffer(
        ext,
        name,
        f->ReadBytes(f->Size())
    );

    delete f;
    return out;
}




AssetID Assets::LoadFromBuffer(
    const string & ext,
    const string & name,
    const std::vector<uint8_t> & buffer) {

    Decoder * dec = GetDecoder(ext);
    if (!dec) return AssetID();


    // query the asset
    AssetID index = Query(dec->GetType(), name);
    if (index.Valid()) {
        return index;
    }

    Asset * out = NULL;


    out = (*dec)(
        name.c_str(),
        ext,
        &buffer[0],
        buffer.size()
    );



    if (!out) {
        Console::Error()  << "[Dynacoe::Assets]: Failed to load "<< typeToString(dec->GetType())
                 << " \"" << name << "\" !" << Console::End();
        return AssetID();
    }


    return storeGen(name, out, dec->GetType());
}

bool Assets::Write(AssetID id, const std::string & ext, const std::string & name) {
    if (!id.Valid()) {
        Console::Error()  << "[Dynacoe::Assets]: Failed to write asset: asset not found!"<< Console::End();
        return false;
    }
    Asset * asset = assetList[id.GetType()][id.GetHandle()];
    if (!asset) {
        Console::Error()  << "[Dynacoe::Assets]: Failed to write asset: asset not found!"<< Console::End();
        return false;
    }
    return Encode(id.GetType(), asset, ext, name);
}


AssetID Assets::Query(Type type, const string & name) {
    auto it = assetMap[(int) type].find(name);
    return (it == assetMap[(int) type].end() ? AssetID() : it->second);

}


AssetID Assets::New(Type type, const string & id) {
    std::string actual = id;
    if (id == "") {
        static uint32_t i = 0;
        Chain name = "_DynacoeAsset_"; name << i;
        actual = name;
        i++;
    }
    if (assetMap[(int) type].find(actual) != assetMap[(int)type].end()) {
        Console::Error()<<("[Dynacoe::Assets]: Cannot create asset: asset with this name already exists!")<< Console::End();
        return AssetID();
    }

    return storeGen(actual, CreateAsset(type, actual), type);
}

Asset * CreateAsset(Assets::Type type, const string & str) {
    switch(type) {
        case Assets::Type::Image:    return new Image(str);
        case Assets::Type::Audio:    return new AudioBlock(str);
        case Assets::Type::Sequence: return new Sequence(str);
        case Assets::Type::Model:     return new Model(str);
        default:
            Console::Error()<<("[Dynacoe::Assets]: It is not currently possible to create a new " + typeToString(type) + " \nasset during runtime; it must be loaded\n");
            return nullptr;
        break;
    }
}

bool Assets::Remove(AssetID id) {
    if (id.GetType() == AssetID::NO_TYPE) {
        Console::Error()<<("[Dynacoe::Assets]: Couldn't remove asset. The given ID refers to no existing asset.")<< Console::End();
        return false;
    }

    if (!assetList[id.GetType()][id.GetHandle()]) {
        Console::Error()<<("[Dynacoe::Assets]: Attempted to retrieve asset that was removed.")<< Console::End();
        return false;
    }


    assetMap[id.GetType()].erase(assetList[id.GetType()][id.GetHandle()]->GetAssetName());
    delete assetList[id.GetType()][id.GetHandle()];
    assetList[id.GetType()][id.GetHandle()] = NULL;


    deadList[id.GetType()].push(id.GetHandle());
    return true;
}


/* Image storing data flow
                              if not found
   stringName -> ImageQuery  ------------> Resolve name               OK
                    |                      to find fully qualified ------{
                    | if found             path name
                    |                           |
       return id<----                           | if DNE
                                   return fail<--




         Convert format            OK                                   OK                   OK
}------> to 24-bit uncompressed  ------->  Apply Image Transformations --> Store in cache  -----> return ID
        (insert alpha channel if none)       (Permanent colorization,          |
               |                               tilesetting, etc)               | if cache error
               |  if unsupported format              |                         |
return fail <---                                     | if not possible         ---->return fail
                                      return fail<----


*/

AssetID storeGen(const string & path, Asset * asset, Assets::Type type) {
    if (!asset) {
        Console::Error()<<("[Dynacoe::Assets]: Cannot add asset! Invalid data.")<< Console::End();
        return AssetID();
    }


    // First, use a known empty slot if any
    if (!deadList[(int)type].empty()) {
        int freeIndex = deadList[(int)type].top();
        assetList[(int)type][freeIndex] = asset;
        deadList[(int)type].pop();
        return AssetID(freeIndex, (int)type);
    }

    // If the deadList is empty, add as normal
    assetList[(int)type].push_back(asset);

    AssetID out =  AssetID(assetList[(int)type].size() - 1, (int)type);
    assetMap[(int)type][path] = out;
    return out;
}

Decoder * Assets::GetDecoder(const string & ext) {
    auto it = decoders.find(ext);
    if (it == decoders.end()) {
        Console::Error()  << "[Dynacoe::Assets]: Could not find a loader supporting the filetype " << ext<< Console::End();

        return nullptr;
    }
    return it->second;
}

bool Encode(int type, Asset * asset, const std::string & ext, const std::string & str) {
    map<std::string, Encoder*> localEncs = encoders[type];
    auto iter = localEncs.find(ext);
    if (iter == localEncs.end()) {
        return false;
    }

    return (*iter->second)(asset, ext, str);

}


void notFoundError(Assets::Type type, const std::string & name) {
    Console::Error()  << "[Dynacoe::Assets]: Could not find " << typeToString(type)
             << " \"" << name << "\""<< Console::End();
}


string typeToString(Assets::Type type) {
    switch(type) {
        case Assets::Type::Image:    return "Image";
        case Assets::Type::Font:     return "Font";
        case Assets::Type::Audio:    return "Audio";
        case Assets::Type::Sequence: return "Sequence";
        case Assets::Type::Model:    return "Model";
        case Assets::Type::Particle: return "Particle";
        case Assets::Type::RawData:  return "RawData";

    }
    return "RawData";
}


vector<string> Assets::SupportedLoadExtensions(Assets::Type type) {
    vector<string> out;
    for(auto it = decoders.begin(); it != decoders.end(); ++it) {
        if ((it->second)->GetType() == type) {
            vector<std::string> exts = (it->second)->GetExtensions();
            for(auto nt = exts.begin(); nt != exts.end(); ++nt) {
                out.push_back(*nt);
            }
        }
    }
    return out;
}

vector<string> Assets::SupportedWriteExtensions (Assets::Type type) {
    vector<string> out;
    map<string, Encoder*> localEncoder = encoders[(int)type];
    for(auto it = localEncoder.begin(); it != localEncoder.end(); ++it) {
        out.push_back((it->second)->GetExtension());
    }
    return out;
}

void Assets::LoadDecoder(Decoder * dec) {
    if (!dec) return;
    vector<std::string> exts = dec->GetExtensions();
    for(uint32_t i = 0; i < exts.size(); ++i) {


        // TODO: possible security risk here by commenting out, but allowing
        // users to override is COOL... Coolness factor increase.. or security... hmmm
        /*
        if (decoders.count(dec->GetExtension())) {
            Console::Info()  << "[Dynacoe::Assets]: New Loader handling already handled file type extension. Ignoring.";
            return;
        } */
        decoders[exts[i]] = dec;
    }
}

void Assets::LoadEncoder(Encoder * enc) {
    if (!enc) return;
    encoders[(int)enc->GetType()][enc->GetExtension()] = enc;
}


string Assets::Name(AssetID id) {
    if (!id.Valid()) {
        Console::Error()  << "[Dynacoe::Assets]: Failed to get name of asset: asset not found!"<< Console::End();
        return "";
    }

    Asset * asset = assetList[id.GetType()][id.GetHandle()];
    if (!asset) {
        Console::Error()  << "[Dynacoe::Assets]: Failed to get name of asset: asset not found!"<< Console::End();
        return "";
    }
    return asset->GetAssetName();
}


Asset * Assets::GetRaw(const AssetID & id) {
    if (!id.Valid()) {
        Console::Error() << ("Error getting from cache: Non-existent ID\n");
        return errorInstances[id.GetType()];
    }   
    return (assetList[id.GetType()][id.GetHandle()]);


}


Asset * Assets::GetGeneric(Assets::Type type) {
    return errorInstances[(int)type];
}











string fSearch(const string & file) {
    Filesys fs;
    string cd = fs.GetCWD();
    fs.ChangeDir(Engine::GetBaseDirectory());
    string fullPath = fs.FindFile(file);
    fs.ChangeDir(cd);
    return fullPath;
}
