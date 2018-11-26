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

#ifndef H_DASSETS_INCLUDED
#define H_DASSETS_INCLUDED




#include <Dynacoe/AssetID.h>
#include <Dynacoe/Modules/Module.h>
#include <Dynacoe/Modules/Console.h>

#include <vector>
#include <string>
#include <cctype>
#include <stack>
#include <unordered_map>
#include <map>






namespace Dynacoe {
class Image;
class Particle;
class AudioBlock;
class Sequence;
class Particle;
class FontAsset;
class Decoder;
class Encoder;




class Asset {
    public:
        Asset(const std::string & n) : name{n}{}
        std::string GetAssetName() const {return name;}
        virtual ~Asset() = 0;
    private:
        std::string name;

};


///
/// \brief
/// Handles loading media.
///
/// When working with a multimedia project, you often will need to load in 
/// different kinds of media with varying filetypes and sources. Assets allows 
/// you to work with a common framework for all these types. In specifying the filename 
/// and kind of media, Dynacoe will find the media, load it, and let you start working 
/// with it right away. 
///
/// Any Dynacoe build could have additional supported media types as well. The supported 
/// media types may always be queried using the SupportedLoadExtensions() function.
///
/// As well as loading, Dynacoe also supports exporting for some types. SupportedWriteExtensions()
/// may be used for that purpose.
///

namespace Assets {

    /// \brief A classification of individual Assets.   
    ///
    enum class Type {
        Placeholder,    ///< Not an action Type.
        Image,          ///< An Image.
        Font,           ///< A FontAsset.
        Audio,          ///< An AudioBlock.
        Sequence,       ///< A Sequence.
        Particle,       ///< A Particle.
        RawData,        ///< A data buffer.
        Model,          ///< Collection of meshes, animations, and/or materials
        NoType
    };




    /// \brief Loads the specifed asset into memory
    ///
    /// The first argument refers to what kind of file Dynacoe should attempt
    /// to load it as. This also determines its type. For example, If the
    /// "png" format is supported, loading it will associate a Dynaoce::Image with the asset
    /// Load() will also compare the path with known saved Asset paths.
    /// If the path is already known,  the id to the already saved duplicate is returned.
    /// if there is no match, the asset is added to the list.
    /// If unsuccessful, an invalid AssetID will be returned.
    /// @param fileType The registered file type. See output from SupportedLoadExtensions() to see which strings are valid.
    /// @param file The path to the file to be loaded.
    /// @param nameOnly If true, Dynacoe treats the file argument as a name of a file and recursively searches
    /// for the file in the current working directory. Else, the file argument is treated as a
    /// full path to the file.
    AssetID Load(const std::string & fileType, const std::string & file, bool  nameOnly=true);


    /// \brief Loads the specified asset from memory into, err, memory.
    ///
    /// Once run, LoadFromMemory will load the asset
    /// as if it were called from Load().
    /// @param fileType The registered file type. See output from SupportedLoadExtensions() to see which strings are valid.
    /// @param name The unique identifier of the asset, allowing for use with Query() calls.
    /// @param data The bytes consisting of the Asset's data.
    AssetID LoadFromBuffer(const std::string & filetype, const std::string & name, const std::vector<uint8_t> & data);


    /// \brief Returns a list of supported file types that Dynacoe can load.
    ///
    /// These extensions should be passed as the fileType argument to Assets::Load()
    /// @param type The type of asset to ask available extensions for.
    std::vector<std::string> SupportedLoadExtensions(Assets::Type type);

    /// \brief Returns the AssetID associated with the name and type
    ///
    /// For the query to be successful, both the type and name must match.
    /// @param type The type of asset to ask about.
    /// @param name The name of the asset to check.
    AssetID Query(Assets::Type type, const std::string & name);

    /// \brief Generates a new instance of the specified Asset type and returns its ID.
    ///
    /// An invalid AssetID is returned if unsuccessful.
    /// @param type The type of the new asset.
    /// @param name The unique identifier of the new asset.
    AssetID New(Assets::Type type, const std::string & name = "");

    /// \brief Returns the Asset reference associated with the given ID.
    ///
    /// @param id The asset to retrieve.
    template<typename T>
    T & Get(const AssetID & id);

    /// \brief Removes the specified Asset.
    ///
    /// After removal, all remaining AssetID copies are made invalid.
    /// @param id The asset to remove.
    bool    Remove(AssetID id);

    /// \brief Attempts to dump the asset to a file
    ///
    /// The encoder extension specifies which encoder
    /// should handle writing the file. If the extension is not supported, the write will fail
    /// and false will be returned. True is returned if the dump was reported as successful
    /// by the encoder.
    /// @param id The asset to write.
    /// @param enconderExtension The type to write the asset as.
    /// @param outputName The namem of the file.
    bool    Write(AssetID id, const std::string & encoderExtension, const std::string & outputName);

    /// \brief Returns a vector of strings containing the
    /// currently supported types that can be written to.
    ///
    /// @param type The type of asset to ask about.
    std::vector<std::string> SupportedWriteExtensions(Assets::Type type);

    /// \brief Returns the name associated with the Asset.
    ///
    /// @param type The asset to ask about.
    std::string Name(AssetID type);







    void Init();

    // internal functions (Need cleanup!)

    // Gets the asset 
    Asset * GetRaw(const AssetID & id);

    // Gets the default asset for the given type. 
    Asset * GetGeneric(Assets::Type type);


    void LoadDecoders();
    void LoadDecoder(Decoder *);
    Decoder * GetDecoder(const std::string &);
    void LoadEncoders();
    void LoadEncoder(Encoder *);
    
}



template<typename T>
T & Assets::Get(const AssetID & id) {
    T * i =dynamic_cast<T*>(Assets::GetRaw(id));
    if (i) return *i;
    if (!GetRaw(id))
        Console::Error() <<("Error getting from cache: Used deleted ID\n");
    else
        Console::Error() <<("Asset exists, but is a different type than the template given.\n");
    return *(T*)GetGeneric((Assets::Type)id.GetType());
}

}



#endif
