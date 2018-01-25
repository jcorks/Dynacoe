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


#include <Dynacoe/Decoders/DecodeOBJ.h>
#include <Dynacoe/Mesh.h>
#include <Dynacoe/Util/Vector.h>
#include <Dynacoe/Util/Math.h>
#include <Dynacoe/Util/Iobuffer.h>
#include <Dynacoe/Util/Filesys.h>
using namespace Dynacoe;
using std::vector;
using std::string;

using Dynacoe::Chain;
using Dynacoe::Vector;
using Dynacoe::InputBuffer;
using Dynacoe::Filesys;


Asset * DecodeOBJ::operator()(
    const std::string & fname,
    const std::string &,   
    const uint8_t * buffer,
    uint64_t size
) {
    return nullptr;//TODO;
    // Each current mesh object should actually be a new aspect
    
    /*
    Model * out = new Model(fname.c_str);
    
    AssetID meshID = Assets::New(Assets::Type::Mesh);
    Mesh * obj = &Assets::Get<Mesh>(meshID);



    InputBuffer in;
    in.OpenBuffer(std::vector<uint8_t>(buffer, buffer+size));
    Chain src = in.ReadString(in.Size()).c_str();
    src.SetDelimiters("\n\r");
    int numValues;
    Chain token;
    if (!src.Length()) {
        Console::Error() << ("[DecodeOBJ]: The file could not be opened\n");
        delete out;
        Assets::Remove(meshID);
        return nullptr;
    }
    vector<float> OBJvertexList;
    vector<float> OBJtexCoords;
    vector<float> OBJnormals;
    vector<float> OBJspaceCoords;


    vector<Vector> vertPos;
    vector<Vector> vertUVs;
    vector<Vector> vertNorms;
    Mesh::MeshObject meshObj;

    bool usingMat = false;
    


    src.NextLink();
    while(src.LinksLeft()) {
        Chain line;
        line.SetDelimiters(" ");
        line = src.GetLink().c_str();
        //Console::Info()  << "Currentline: {" << line << "}" ;

        if (line.GetLink()[0] == '#') {
            // Read comment
            //Console::Info()  << "Reading a comment..." ;
            src++;

        } else if (line.GetLink() == "v") {
            // Read vertices
            Chain vertexLine;
            vertexLine.SetDelimiters(" \t");
            vertexLine  = line.c_str();


            numValues = 0;
            //Console::Info()  << "Read vertex {";
            vertexLine++; // skip v
            while(vertexLine.LinksLeft()) {


                numValues++;
                OBJvertexList.push_back(atof(vertexLine.GetLink().c_str()));
                //Console::Info() << vertexLine.GetLink().c_str() << ", ";
                vertexLine++;
            }


            if (numValues == 3) {
                OBJvertexList.push_back(1.f);
                //Console::Info()  << "1.f";
            }

            //Console::Info()  << "}" ;

            src++;

        } else if (line.GetLink() == "vt") {
            // texture coords
            Chain textureLine;
            textureLine.SetDelimiters(" \t");
            textureLine = line.c_str();

            int numCoords = 0;
            //Console::Info()  << "Read texCoords {";
            textureLine++; // skip vt
            while(textureLine.LinksLeft() && numCoords < 2) {
                float val = atof(textureLine.GetLink().c_str());
                Math::Clamp(val, 0, 1);

                //Console::Info()  << textureLine.GetLink().c_str() << ", ";
                OBJtexCoords.push_back(val);
                numCoords++;

                textureLine++;
                // discard optional 3rd coordinate
            }

            //Console::Info()  << "}\n";
            src++;
        } else if (line.GetLink() == "vn") {
            // normals
            Chain normalLine;
            normalLine.SetDelimiters(" \t");
            normalLine = line.c_str();
            numValues = 0;
            //Console::Info()  << "Read normal {";
            normalLine++; // skip vn
            while(normalLine.LinksLeft()){
                //Console::Info()  << normalLine.GetLink().c_str() << ", ";
                OBJnormals.push_back(atof(normalLine.GetLink().c_str()));
                normalLine++;
                numValues++;
            }

            while(numValues++ < 4) {
                OBJnormals.push_back(1.f);
            }
            //Console::Info()  << "}" ;

            src++;

        } else if (line.GetLink() == "f") {

            // the face definitions
            int link = 0,
                iter = 0,
                vertices = 0;
            Chain faceDef;
            Chain faceLine = line.c_str();
            faceLine.SetDelimiters(" \t");
            float * current;
            faceLine++; // exhaust 'f'
  

            link = 0;
            int index = faceLine.GetLinkPos();
            Chain num, vertexDef;
            //Console::Info()  << faceLine.c_str();

            // for each vertex in the faceLine....
            while(faceLine.LinksLeft()) {
                vertexDef = faceLine.GetLink();
                int index = 0;
                link = 0;
                num = "";



                // parse the vertex
                while(index <= vertexDef.Length()) {
                    
                    
                    if (vertexDef[index] != '/' && index < vertexDef.Length()) {
                        num+=vertexDef[index++];
                        continue;
                    } 

                    iter = atoi(num.c_str())-1;
                    
                    if (link == 0) {// vertices
                        current = &OBJvertexList[iter*4];
                        vertPos.push_back(Vector(current[0], current[1], current[2]));



                    }

                    if (link == 1) { //texCoords
                        if (OBJtexCoords.size() && num.Length()) {
                            current = &OBJtexCoords[iter*2];
                            vertUVs.push_back(Vector(current[0], current[1]));
                        } else {
                            vertUVs.push_back(Vector());
                        }
                    }


                    if (link == 2) { // normals
                        if (OBJnormals.size()&& num.Length()) {
                            current = &OBJnormals[iter*4];
                            vertNorms.push_back(Vector(current[0], current[1], current[2]));
                        } else {
                            vertNorms.push_back(Vector());
                        }
                        //Console::Info()  << "Would have Read normal (src = " << token.c_str() << ")" ;
                    }
                
                    index++;
                    link++;
                    num = "";
                }

                // then register and go to next vertex
                faceLine++;
                meshObj.faceList.push_back(vertPos.size()-1);    
            }
            
            

            src++;

        } else if (line.GetLink() == "mtllib"){
            // material library external file
            // loadMaterial(filename);
            // add material properties to vertexData
            ///  so texture names, light reflection vals, etc.
            ///  when Graphics gets a well-formed vertexStream,
            ///  it will store the textures
            line++;
            Chain mtlName = line.GetSubstr(line.GetLinkPos(), line.Length()-1);
            Filesys filesys;
            loadMtllib(filesys.FindFile(mtlName.c_str()).c_str());

            src++;
        } else if (line.GetLink() == "usemtl") {
            // material external file
            // if alReady working on a mesh object, push its instance
            line++;
            
            if (usingMat) {
                obj->AddObject(meshObj);
                //Console::Info()  << "Added " << (int)meshObj.faceList.size() << " vertices for mesh object";
            }
            string cmp = line.GetLink().c_str();
            auto it = mtls.find(cmp);
            if (it == mtls.end()) {
                Console::Warning()  << "Unknown material reference \"" << line.GetLink().c_str() << "\"\n";
                return nullptr;
            }
            ResourceMaterial & resource = *it->second;     
            meshObj.material = resource.mat;
            meshObj.textures = resource.textures;
            meshObj.faceList.clear();
            usingMat = true;
            
            
            src++;
        } else if (line.GetLink() == "s") {
            src++;
        } else if (line.GetLink() == "o") {
            src++;
        } else if (line.GetLink() == "vp") {
            src++;
        } else if (line.GetLink() == "g") {
            src++;
        } else {

                Chain out;
                out << "Unexpected string " << src.GetLink() << ". Failed to load OBJ\n";
                Console::Error()  <<  "Unexpected string " << src.GetLink().c_str() << ". Failed to load OBJ\n" ;
                return nullptr;
        }
    }
    obj->SetVertexCount(vertPos.size());
    obj->DefineVertices(Mesh::VertexAttribute::Position, vertPos);
    obj->DefineVertices(Mesh::VertexAttribute::UV,       vertUVs);
    if (vertNorms.size()) {
        //Console::Info()  << "[DecodeOBJ]: Adding optional normals...";
        obj->DefineVertices(Mesh::VertexAttribute::Normal, vertNorms);
    }
    // add the last object
    obj->AddObject(meshObj);
    

    return obj;
    */
}



bool DecodeOBJ::loadMtllib(const Chain & filename) {

    InputBuffer in;
    in.Open(filename);
    Chain src = in.ReadString(in.Size());
    if (!src.ToString().size()) {
        Console::Error()  << "[DecodeOBJ]: The material " << filename << " could not be opened";
        return false;
    }
    src.SetDelimiters("\n\r");

    ResourceMaterial * curMat = nullptr;

    while(src.LinksLeft()) {
        Chain line = src.GetLink();
        line.SetDelimiters(" \t");
        if (line.GetLink()[0] == '#') {
            // Read comment
            //Console::Info()  << "Reading a comment..." ;
        } else if (line.GetLink() == "newmtl") {
            line++;
            
            curMat = new ResourceMaterial();
            mtls[line.GetLink()] = curMat;
            //Console::Info()  << " New material: " << line.GetLink().c_str();

        } else if (line.GetLink() == "Ns") {

            

        } else if (line.GetLink() == "Ni") {

            

        } else if (line.GetLink() == "Ka") {
            Vector p;
            p.x = atof(line.GetLink().c_str()); line++;
            p.y = atof(line.GetLink().c_str()); line++;
            p.z = atof(line.GetLink().c_str()); line++;
            Color next = {1.f, 1.f, 1.f, 1.f};
            curMat->mat.state.ambient = next;
        } else if (line.GetLink() == "Kd") {
            Vector p;
            p.x = atof(line.GetLink().c_str()); line++;
            p.y = atof(line.GetLink().c_str()); line++;
            p.z = atof(line.GetLink().c_str()); line++;
            Color next = {p.x, p.y, p.z, 1.f};
            curMat->mat.state.diffuse = next;
        } else if (line.GetLink() == "Ks") {
            Vector p;
            p.x = atof(line.GetLink().c_str()); line++;
            p.y = atof(line.GetLink().c_str()); line++;
            p.z = atof(line.GetLink().c_str()); line++;
            Color next = {p.x, p.y, p.z, 1.f};
            curMat->mat.state.specular = next;
        } else if (line.GetLink() == "Ke") {


        } else if (line.GetLink() == "d") {

        } else if (line.GetLink() == "Tr") {

        } else if (line.GetLink() == "Tf") {
        
        } else if (line.GetLink() == "map_Kd") {
            line++;
            curMat->textures.push_back({0, Assets::Load("png", line.GetLink())});
            
        } else if (line.GetLink() == "map_Ks") {
            line++;
            curMat->textures.push_back({0, Assets::Load("png", line.GetLink())});
            
        } else if (line.GetLink() == "map_Ka") {
            line++;
            curMat->textures.push_back({0, Assets::Load("png", line.GetLink())});
            
        } else if (line.GetLink() == "illum") {

        } else {
                
                Console::Error()  <<  "Unexpected string " << src.GetLink() 
   << ". Failed to load OBJ mtllib\n" ;
           return false; 
        }
     
        src++;   
    }
    return true;
}


