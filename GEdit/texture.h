#ifndef TEXTURE_H
#define TEXTURE_H

#define SKIP 0x0000

#include <glad/glad.h>

#include "utils.h"

class Texture2D
{
public:
    std::string Name;
    glm::vec4 m_clear_color = { 0,0,0,1 };
    bool Stored = false;
    unsigned int ID;
    uchar* Data = nullptr;
    Point2D<float> Size;
    unsigned int Internal_Format; // format of texture object
    unsigned int Image_Format; // format of loaded image
    unsigned int Data_Type;
    unsigned int Wrap_S; // wrapping mode on S axis
    unsigned int Wrap_T; // wrapping mode on T axis
    unsigned int Filter_Min; // filtering mode if texture pixels < screen pixels
    unsigned int Filter_Max; // filtering mode if texture pixels > screen pixels
    Texture2D();
    void Setup(uint internal_f,uint img_f,uint data_t,uint wrap_s,uint wrap_t,uint filter_min,uint filter_max);
    void Generate(Point2D<float> size, unsigned char* data);
    void Bind() const;
    void Resize(Point2D<float> size);
    void Refresh();

    static void ResizeRBODepth(Point2D<float> size, uint FBO, uint RBO);
    static uint GenDepthTex(Point2D<float> size);
    static uint Generate(uint internal_f, uint img_f, uint data_t, uint wrap_s, uint wrap_t, uint filter_min, uint filter_max, float width, float height);
    static void RefreshTexs(Point2D<float> size);
    static std::vector<Texture2D*> allTexs;
};

#endif