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

static const int DYNACOE_SAMPLE_RATE =        44100;       // (Samples processed per second)

// What a mouthful!
vector<Asset *> Assets::assetList[AssetID::NUMTYPES];
stack<int> Assets::deadList[AssetID::NUMTYPES];
unordered_map<string, AssetID> Assets::assetMap[AssetID::NUMTYPES];
size_t Assets::assetCounter;



vector<Asset*> Assets::errorInstances;
map<string, Decoder*> Assets::decoders;
vector<map<string, Encoder*>> Assets::encoders;


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

void Assets::InitAfter (){}
void Assets::RunBefore( ){}
void Assets::RunAfter()  {}
void Assets::DrawBefore(){}
void Assets::DrawAfter() {}







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
        Console::Error() << "[Dynacoe::Assets]: Failed to load file " << name<< Console::End;
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
                 << " \"" << name << "\" !" << Console::End;
        return AssetID();
    }


    return storeGen(name, out, dec->GetType());
}

bool Assets::Write(AssetID id, const std::string & ext, const std::string & name) {
    if (!id.Valid()) {
        Console::Error()  << "[Dynacoe::Assets]: Failed to write asset: asset not found!"<< Console::End;
        return false;
    }
    Asset * asset = assetList[id.type][id.handle];
    if (!asset) {
        Console::Error()  << "[Dynacoe::Assets]: Failed to write asset: asset not found!"<< Console::End;
        return false;
    }
    return Encode(id.type, asset, ext, name);
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
        Console::Error()<<("[Dynacoe::Assets]: Cannot create asset: asset with this name already exists!")<< Console::End;
        return AssetID();
    }

    return storeGen(actual, CreateAsset(type, actual), type);
}

Asset * Assets::CreateAsset(Type type, const string & str) {
    switch(type) {
        case Type::Image:    return new Image(str);
        case Type::Audio:    return new AudioBlock(str);
        case Type::Sequence: return new Sequence(str);
        case Type::Model:     return new Model(str);
        default:
            Console::Error()<<("[Dynacoe::Assets]: It is not currently possible to create a new " + typeToString(type) + " \nasset during runtime; it must be loaded\n");
            return nullptr;
        break;
    }
}

bool Assets::Remove(AssetID id) {
    if (id.type == AssetID::NO_TYPE) {
        Console::Error()<<("[Dynacoe::Assets]: Couldn't remove asset. The given ID refers to no existing asset.")<< Console::End;
        return false;
    }

    if (!assetList[id.type][id.handle]) {
        Console::Error()<<("[Dynacoe::Assets]: Attempted to retrieve asset that was removed.")<< Console::End;
        return false;
    }


    assetMap[id.type].erase(assetList[id.type][id.handle]->GetAssetName());
    delete assetList[id.type][id.handle];
    assetList[id.type][id.handle] = NULL;


    deadList[id.type].push(id.handle);
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

AssetID Assets::storeGen(const string & path, Asset * asset, Assets::Type type) {
    if (!asset) {
        Console::Error()<<("[Dynacoe::Assets]: Cannot add asset! Invalid data.")<< Console::End;
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
        Console::Error()  << "[Dynacoe::Assets]: Could not find a loader supporting the filetype " << ext<< Console::End;

        return nullptr;
    }
    return it->second;
}

bool Assets::Encode(int type, Asset * asset, const std::string & ext, const std::string & str) {
    map<std::string, Encoder*> localEncs = encoders[type];
    auto iter = localEncs.find(ext);
    if (iter == localEncs.end()) {
        return false;
    }

    return (*iter->second)(asset, ext, str);

}


void Assets::notFoundError(Assets::Type type, const std::string & name) {
    Console::Error()  << "[Dynacoe::Assets]: Could not find " << typeToString(type)
             << " \"" << name << "\""<< Console::End;
}


string Assets::typeToString(Assets::Type type) {
    switch(type) {
        case Type::Image:    return "Image";
        case Type::Font:     return "Font";
        case Type::Audio:    return "Audio";
        case Type::Sequence: return "Sequence";
        case Type::Model:    return "Model";
        case Type::Particle: return "Particle";
        case Type::RawData:  return "RawData";

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
        Console::Error()  << "[Dynacoe::Assets]: Failed to get name of asset: asset not found!"<< Console::End;
        return "";
    }

    Asset * asset = assetList[id.type][id.handle];
    if (!asset) {
        Console::Error()  << "[Dynacoe::Assets]: Failed to get name of asset: asset not found!"<< Console::End;
        return "";
    }
    return asset->GetAssetName();
}
















Backend * Assets::GetBackend() {
    return nullptr;
}


string Assets::fSearch(const string & file) {
    Filesys fs;
    string cd = fs.GetCWD();
    fs.ChangeDir(Engine::GetBaseDirectory());
    string fullPath = fs.FindFile(file);
    fs.ChangeDir(cd);
    return fullPath;
}
