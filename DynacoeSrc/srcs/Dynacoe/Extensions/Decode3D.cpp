#ifdef DC_EXTENSION_EXTRA_DECODERS


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


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Dynacoe/Extensions/Decode3D.h>
#include <Dynacoe/Model.h>
#include <Dynacoe/Mesh.h>
#include <Dynacoe/Image.h>
using namespace Dynacoe;

static float wrapUV(float f) {
    while(f > 1) f-=1;
    while(f < 0) f+=1;
    return f;
}

Asset * Decode3D::operator()(
    const std::string & fname,
    const std::string & ext,
    const uint8_t * buffer,
    uint64_t size
) {
    Assimp::Importer importer;
    const aiScene * scene =  importer.ReadFileFromMemory(
        (void*)buffer,
        size,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType|
        aiProcess_GenUVCoords |
        aiProcess_TransformUVCoords,
        ext.c_str()
    );

    if (!scene) {
        Console::Error() << "[Decode3D (assimp)]: Could not successfully load " << fname << "!\n";
        return nullptr;
    }

    Console::Info() <<
        "3D file contains the following info:\n" <<
        (scene->HasMeshes() ? "Mesh data\n" : "") <<
        (scene->HasMaterials() ? "Material data\n" : "") <<
        (scene->HasLights() ? "Light data\n" : "") <<
        (scene->HasTextures() ? "Texture data\n" : "") <<
        (scene->HasCameras() ? "Camera data\n" : "") <<
        (scene->HasAnimations() ? "Animation data\n" : "")<<
        "\n";



    Model * out = new Model(fname);

    // materials
    std::vector<Material> materials;
    for(uint32_t i = 0; i < scene->mNumMaterials; ++i) {
        Material mat;
        aiMaterial * srcMat = scene->mMaterials[i];

        // basic colors
        aiColor4t<float> dest;
        Color destC;
        aiGetMaterialColor(srcMat, AI_MATKEY_COLOR_DIFFUSE, &dest);
        destC = Color(dest.r, dest.g, dest.b, dest.a);
        mat.state.diffuse = destC;

        aiGetMaterialColor(srcMat, AI_MATKEY_COLOR_SPECULAR, &dest);
        destC = Color(dest.r, dest.g, dest.b, dest.a);
        mat.state.specular = destC;

        aiGetMaterialColor(srcMat, AI_MATKEY_COLOR_AMBIENT, &dest);
        destC = Color(dest.r, dest.g, dest.b, dest.a);
        mat.state.ambient = destC;

        mat.SetProgram(Material::CoreProgram::Basic);//Material::CoreProgram::Lighting);


        materials.push_back(mat);
    }

    std::vector<AssetID> textures;
    // Textures
    for(uint32_t n = 0; n < scene->mNumTextures; ++n) {

        Chain assetName = fname.c_str();
        assetName << "_tex_";
        assetName << (int)n;
        aiTexture * tex = scene->mTextures[n];
        AssetID id;
        // so, if mHeight is non-zero, the data is given
        // directly as pixels in ARGB
        if (tex->mHeight) {
            id = Assets::New(Assets::Type::Image, assetName);
            Image * im = &Assets::Get<Image>(id);
            std::vector<uint8_t> texData;
            texData.resize(tex->mWidth*tex->mHeight*4);

            // reorder for RGBA
            aiTexel * iter = tex->pcData;
            for(uint32_t k = 0; k < tex->mWidth; ++k) {
                texData[k+0] = iter->r;
                texData[k+1] = iter->g;
                texData[k+2] = iter->b;
                texData[k+3] = iter->a;
                iter ++;
            }

            im->frames.push_back(Image::Frame(tex->mWidth, tex->mHeight, texData));
        } else {
            std::vector<uint8_t> texData((uint8_t*)tex->pcData, (uint8_t*)tex->pcData+tex->mWidth);
            id = Assets::LoadFromBuffer(tex->achFormatHint, assetName, texData);
        }
        textures.push_back(id);
    }


    for(uint32_t i = 0; i < scene->mNumMeshes; ++i) {
        Chain assetName = fname.c_str();
        assetName << "_mesh_";
        assetName << (int)i;

        out->AddSection();
        Mesh & mesh = out->SectionMesh(out->GetSectionCount()-1);
        aiMesh * src = scene->mMeshes[i];
        // first: vertices

        mesh.SetVertexCount(src->mNumVertices);
        for(uint32_t n = 0; n < src->mNumVertices; ++n) {
            mesh.SetVertex(n, Mesh::VertexAttribute::Position,
                {
                    src->mVertices[n].x,
                    src->mVertices[n].y,
                    src->mVertices[n].z
                }
            );
        }

        // normals
        for(uint32_t n = 0; n < src->mNumVertices; ++n) {
            mesh.SetVertex(n, Mesh::VertexAttribute::Normal,
                {
                    src->mNormals[n].x,
                    src->mNormals[n].y,
                    src->mNormals[n].z
                }
            );
        }

        // UVs
        for(uint32_t n = 0; n < src->mNumVertices && src->mTextureCoords[0]; ++n) {
            mesh.SetVertex(n, Mesh::VertexAttribute::UV,
                    {
                        wrapUV(src->mTextureCoords[0][n].x),
                        wrapUV(src->mTextureCoords[0][n].y),
                        wrapUV(src->mTextureCoords[0][n].z)
                    }
                );
        }



        // faces! (For now, only triangle support)
        if (!(src->mPrimitiveTypes & aiPrimitiveType_TRIANGLE)) {
            Dynacoe::Console::Error() << "[Decode3D (assimp)]: Read failed, mesh is not triangulated!!\n";
            return nullptr;
        }
        Mesh::MeshObject * meshObject = mesh.Get(mesh.AddObject());
        aiFace * face;
        for(uint32_t n = 0; n < src->mNumFaces; ++n) {
            face = src->mFaces+n;
            for(uint8_t k = 0; k < face->mNumIndices; ++k)
                meshObject->faceList.push_back(face->mIndices[k]);
        }


        // textures
        aiMaterial * srcMat = scene->mMaterials[src->mMaterialIndex];
        aiTextureType supportedTypes[] = {
            aiTextureType_DIFFUSE,
            aiTextureType_NORMALS,
            aiTextureType_SHININESS,
            aiTextureType_UNKNOWN
        };
        aiTextureType * iter = supportedTypes;
        aiTextureType n;
        while(((n = *(iter++)) != aiTextureType_UNKNOWN)) {
            aiString path;
            aiTextureMapping mapping;
            uint32_t index = UINT32_MAX;
            for(uint32_t k = 0; k < srcMat->GetTextureCount(n); ++k) {
                /*
                if (srcMat->GetTexture((aiTextureType)n, k, &path, &mapping, &index) == 0) {
                    switch(n) {
                      case aiTextureType_DIFFUSE:   obj->AddTexture(AspectMesh::TextureSlot::Color,  textures[index]); break;
                      case aiTextureType_NORMALS:   obj->AddTexture(AspectMesh::TextureSlot::Normal, textures[index]); break;
                      case aiTextureType_SHININESS: obj->AddTexture(AspectMesh::TextureSlot::Shiny,  textures[index]); break;
                    }
                }
                */
                if (srcMat->GetTexture((aiTextureType)n, k, &path, &mapping) == 0) {
                    Chain fnameC = fname.c_str();
                    fnameC.SetDelimiters("/\\");
                    Chain newPath = "/";

                    while(fnameC.LinksLeft()) {
                        Chain token = fnameC.GetLink();
                        fnameC++;
                        if (fnameC.LinksLeft()) newPath << "/" << token;
                    }
                    newPath <<"/";
                    newPath << path.C_Str();


                    Material * matLocal = &materials[src->mMaterialIndex];


                    AssetID id = Assets::Load("png", newPath, false);
                    Mesh::MeshObject * visible = mesh.Get(0);
                    switch(n) {
                      case aiTextureType_DIFFUSE:   matLocal->AddTexture(0, id); break; // color
                      case aiTextureType_NORMALS:   matLocal->AddTexture(1, id); break; // normals
                      case aiTextureType_SHININESS: matLocal->AddTexture(2, id); break; // shiny
                    }
                }

            }

        }

        // TODO: tangents? wha?

        // finalize and upload
        //if (i % 2 == 0)
        out->SectionMaterial(out->GetSectionCount()-1) = (materials[src->mMaterialIndex]);
    }




    return out;

}
#endif
