#include <iostream>

#include "texture.h"

std::vector<Texture2D*> Texture2D::allTexs;


Texture2D::Texture2D()
    : Size(), Internal_Format(GL_RGBA), Image_Format(GL_RGBA),Data_Type(GL_UNSIGNED_BYTE), Wrap_S(GL_CLAMP_TO_EDGE), Wrap_T(GL_CLAMP_TO_EDGE),
    /*
    Filter_Min(GL_NEAREST), Filter_Max(GL_NEAREST)
    */
    Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR)
{
    glGenTextures(1, &(this->ID));
    allTexs.push_back(this);
}

void Texture2D::Setup(uint internal_f, uint img_f, uint data_t, uint wrap_s, uint wrap_t, uint filter_min, uint filter_max)
{
    if (internal_f != SKIP) Internal_Format = internal_f;
    if (img_f != SKIP) Image_Format = img_f;
    if (data_t != SKIP) Data_Type = data_t;
    if (wrap_s != SKIP) Wrap_S = wrap_s;
    if (wrap_t != SKIP) Wrap_T = wrap_t;
    if (filter_min != SKIP) Filter_Min = filter_min;
    if (filter_max != SKIP) Filter_Max = filter_max;
}

void Texture2D::Generate(Point2D<float> size, unsigned char* data)
{
    Size = size;
    Data = data;
    // create Texs
    glBindTexture(GL_TEXTURE_2D, this->ID);
    glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, (GLsizei)Size.X, (GLsizei)Size.Y, 0, this->Image_Format, Data_Type, data);
    // set Texs wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
    // unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Bind() const
{
    glBindTexture(GL_TEXTURE_2D, this->ID);
}

uint Texture2D::GenDepthTex(Point2D<float> size)
{
    unsigned int depthTexture;
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, (GLsizei)size.X, (GLsizei)size.Y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    return depthTexture;
}

uint Texture2D::Generate(uint internal_f, uint img_f, uint data_t, uint wrap_s, uint wrap_t, uint filter_min, uint filter_max, float width, float height)
{
    uint id = 0;
    Texture2D* newTex = new Texture2D();
    newTex->Setup(internal_f, img_f, data_t, wrap_s, wrap_t, filter_min, filter_max);
    newTex->Generate({ width,height }, NULL);
    allTexs.push_back(newTex);
    return newTex->ID;
}


void Texture2D::RefreshTexs(Point2D<float> size)
{
    for (int i = 0; i < allTexs.size(); i++)
    {
        auto tex = allTexs[i];
        glBindTexture(GL_TEXTURE_2D,tex->ID);
        tex->Size = size;
        glTexImage2D(GL_TEXTURE_2D, 0, tex->Internal_Format, (GLsizei)size.X, (GLsizei)size.Y, 0, tex->Image_Format, tex->Data_Type, NULL);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Resize(Point2D<float> size)
{
    if (Size == size)
        return;
    if (Stored)
        return;
    auto tex = this;
    glBindTexture(GL_TEXTURE_2D, tex->ID);
    tex->Size = size;
    glTexImage2D(GL_TEXTURE_2D, 0, tex->Internal_Format, (GLsizei)size.X, (GLsizei)size.Y, 0, tex->Image_Format, tex->Data_Type, Data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Refresh() {
    glBindTexture(GL_TEXTURE_2D, this->ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::ResizeRBODepth(Point2D<float> size, uint FBO, uint RBO) {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    if (RBO != 0) {
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)size.X, (GLsizei)size.Y);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}
