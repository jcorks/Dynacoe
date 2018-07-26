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


#include <Dynacoe/Image.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Dynacoe.h>
#include <Dynacoe/Util/RefBank.h>
#include <cstdint>
#include <cmath>


using namespace std;
using namespace Dynacoe;

static int badTexture = -1;
class FreeTexture : public RefBank<int>::AccountRemover {
  public:
    void operator()(const int & id) {
        if (id != badTexture)
            Graphics::GetRenderer()->RemoveTexture(id);
    }
};

static RefBank<int> * textureBank = nullptr;
static FreeTexture  * textureRemover = nullptr;


Image::Frame::Frame(int textureObject) {
    if (!textureBank) {
        badTexture = Graphics::GetRenderer()->AddTexture(16, 16, nullptr);
        textureBank = new RefBank<int>();
        textureRemover = new FreeTexture;
        textureBank->SetAccountRemover(textureRemover);

        if (textureObject == -1)
            textureObject = badTexture;
    }


    textureBank->Deposit(textureObject);
    object = textureObject;
}

Image::Frame::Frame() : Frame(badTexture) {}
Image::Frame::Frame(uint32_t w, uint32_t h) :
              Frame(Graphics::GetRenderer()->AddTexture(w, h, nullptr)) {}
Image::Frame::Frame(uint32_t w, uint32_t h, const std::vector<uint8_t> & data) : Frame(w, h) {
    SetData(data);
}

Image::Frame::Frame(const Frame & other) : Frame(other.object) {}

Image::Frame::~Frame() {
    if (object != -1)
        textureBank->Withdraw(object);
}


Image::Frame & Image::Frame::operator=(const Image::Frame & other) {
    textureBank->Deposit(other.object);
    object = other.object;
    return *this;
}




std::vector<uint8_t> Image::Frame::GetData() const {
    std::vector<uint8_t> out;
    // Original texture size may not be current image size.
    uint32_t w = Width();
    uint32_t h = Height();
    out.resize(h * w * 4);
    Graphics::GetRenderer()->GetTexture(object, &out[0]);
    return out;
}

void Image::Frame::SetData(const std::vector<uint8_t> & data) {
    uint32_t w, h;
    w = Width();
    h = Height();
    if (data.size() != w*h*4) {
        Console::Error() << "Image::Frame::SetData : given data does not match image dimensions. Ignoring request.\n";
        return;
    }

    Graphics::GetRenderer()->UpdateTexture(object, &data[0]);
}

uint32_t Image::Frame::Width() const {
    return Graphics::GetRenderer()->GetTextureWidth(object);
}

uint32_t Image::Frame::Height() const {
    return Graphics::GetRenderer()->GetTextureHeight(object);
}

std::vector<Image::Frame> Image::Frame::Detilize(uint32_t ws, uint32_t hs) const {
    uint32_t w = Width();
    uint32_t h = Height();

    uint32_t countW = ceil(w / (float)ws);
    uint32_t countH = ceil(h / (float)hs);

    std::vector<uint8_t> subImage;
    subImage.resize(ws*hs*4);
    const std::vector<uint8_t> & sourceImage = GetData();



    uint8_t * subData = &subImage[0];
    uint8_t * subIter = subData;

    const uint8_t * srcData = &sourceImage[0];
    const uint8_t * srcIter = srcData;

    uint32_t srcIndex;
    uint32_t srcX; // pixel x of src image
    uint32_t srcY; // pixel y or src image
    std::vector<Frame> out;
    for(uint32_t y = 0; y < countH; ++y) {
        for(uint32_t x = 0; x < countW; ++x) {

            subIter = subData;

            srcX = x*ws;
            srcY = y*hs;

            for(uint32_t subY = 0; subY < hs; ++subY, ++srcY) {
                srcX = x*ws;
                for(uint32_t subX = 0; subX < ws; ++subX, ++srcX) {
                    if (srcY >= h) continue;
                    if (srcX >= w) continue;

                    srcIter = srcData + (srcX + srcY*w)*4;

                    subIter[0] = srcIter[0];
                    subIter[1] = srcIter[1];
                    subIter[2] = srcIter[2];
                    subIter[3] = srcIter[3];

                    subIter += 4;
                }
            }
            out.push_back(Frame(ws, hs, subImage));


        }
    }
    return out;

}


int Image::Frame::GetHandle() const {
    return object;
}


class FrameCounter : public Entity {
  public:
    FrameCounter() {
        frame = 0;
    }
    void OnStep() {
        frame++;
    }

    uint64_t frame;

};

static FrameCounter * frameCounter = nullptr;
Image::Image(const std::string & n) : Asset(n) {
    if (!frameCounter) {
        frameCounter = Entity::CreateReference<FrameCounter>();
        Engine::AttachManager(frameCounter->GetID());
    }
    index = frameCounter->frame;
}

Image::Frame & Image::CurrentFrame(){
    if (!frames.size()) {
        static Frame bad;
        return bad;
    }
    uint64_t frame = frameCounter->frame - index;
    return frames[frame%frames.size()];
}
