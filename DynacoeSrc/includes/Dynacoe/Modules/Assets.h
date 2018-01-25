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
class Graphics;
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
/// Handles all files loaded from and written to disk.
///

class Assets : public Module {
  public:

    /// \brief A classification of individual Assets.
    enum class Type {
        Placeholder,
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
    static AssetID Load(const std::string & fileType, const std::string & file, bool  nameOnly=true);


    /// \brief Loads the specified asset from memory into, err, memory.
    ///
    /// Once run, LoadFromMemory will load the asset
    /// as if it were called from Load().
    /// @param fileType The registered file type. See output from SupportedLoadExtensions() to see which strings are valid.
    /// @param name The unique identifier of the asset, allowing for use with Query() calls.
    /// @param data The bytes consisting of the Asset's data.
    static AssetID LoadFromBuffer(const std::string & filetype, const std::string & name, const std::vector<uint8_t> & data);


    /// \brief Returns a list of supported file types that Dynacoe can load.
    ///
    /// These extensions should be passed as the fileType argument to Assets::Load()
    /// @param type The type of asset to ask available extensions for.
    static std::vector<std::string> SupportedLoadExtensions(Assets::Type type);

    /// \brief Returns the AssetID associated with the name and type
    ///
    /// For the query to be successful, both the type and name must match.
    /// @param type The type of asset to ask about.
    /// @param name The name of the asset to check.
    static AssetID Query(Assets::Type type, const std::string & name);

    /// \brief Generates a new instance of the specified Asset type and returns its ID.
    ///
    /// An invalid AssetID is returned if unsuccessful.
    /// @param type The type of the new asset.
    /// @param name The unique identifier of the new asset.
    static AssetID New(Assets::Type type, const std::string & name = "");

    /// \brief Returns the Asset reference associated with the given ID.
    ///
    /// @param id The asset to retrieve.
    template<typename T>
    static T & Get(AssetID id);

    /// \brief Removes the specified Asset.
    ///
    /// After removal, all remaining AssetID copies are made invalid.
    /// @param id The asset to remove.
    static bool    Remove(AssetID id);

    /// \brief Attempts to dump the asset to a file
    ///
    /// The encoder extension specifies which encoder
    /// should handle writing the file. If the extension is not supported, the write will fail
    /// and false will be returned. True is returned if the dump was reported as successful
    /// by the encoder.
    /// @param id The asset to write.
    /// @param enconderExtension The type to write the asset as.
    /// @param outputName The namem of the file.
    static bool    Write(AssetID id, const std::string & encoderExtension, const std::string & outputName);

    /// \brief Returns a vector of strings containing the
    /// currently supported types that can be written to.
    ///
    /// @param type The type of asset to ask about.
    static std::vector<std::string> SupportedWriteExtensions(Assets::Type type);

    /// \brief Returns the name associated with the Asset.
    ///
    /// @param type The asset to ask about.
    static std::string Name(AssetID type);






  private:
    friend class Graphics;

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


    //Attempts to store the image(s) specified by the 32-bit pixel buffer given.
    //
    // Hidden denotes whether
    // or not the compiled Image is stored in the standard image list or
    // the private System one.
    static AssetID storePixels(std::vector<uint32_t*> &, std::string ID, int w, int h, bool hidden);

    static Asset * CreateAsset(Type type, const std::string & str);

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

    static std::string fSearch(const std::string &);









  public:
    std::string GetName() { return "Asset Manager"; }
    void Init(); void InitAfter(); void RunBefore(); void RunAfter(); void DrawBefore(); void DrawAfter();
    Backend * GetBackend();
};



template<typename T>
T & Assets::Get(AssetID id) {
    if (!id.Valid()) {
        Console::Error() << ("Error getting from cache: Non-existent ID\n");
        return *((T*)errorInstances[id.type]);
    }
    Asset * i;
    i = dynamic_cast<T*>(assetList[id.type][id.handle]);

    if (i) return *((T*)i);
    if (!assetList[id.type][id.handle])
        Console::Error() <<("Error getting from cache: Used deleted ID\n");
    else
        Console::Error() <<("Asset exists, but is a different type than the template given.\n");
    return *((T*)errorInstances[id.type]);
}

}



#endif
