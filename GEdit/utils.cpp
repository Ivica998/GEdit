#include "utils.h"
#include "resource_manager.h"
#include "dispatcher.h"
#include "framebuf.h"

uint utl::cube6VAO = 0;
uint utl::cube8VAO = 0;
uint utl::cubeVBO = 0;
uint utl::quadVAO = 0;
uint utl::quadVBO = 0;
const Bounds Bounds::s_default = Bounds();
const Bounds Bounds::s_flipY = Bounds{ 1.f,1.f,-1.f,-1.f };

int CharMap_S::Code[47] =    {  49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 81, 87, 69, 82, 84, 89, 85, 73, 79, 80, 91, 93, 92 , 65, 83, 68, 70, 71, 72, 74, 75, 76, 59, 39 , 90, 88, 67, 86, 66, 78, 77, 44, 46, 47, 32 };
int CharMap_S::noShift[47] = { '1','2','3','4','5','6','7','8','9','0','-','=','q','w','e','r','t','y','u','i','o','p','[',']','\\','a','s','d','f','g','h','j','k','l',';','\'','z','x','c','v','b','n','m',',','.','/',' ' };
int CharMap_S::Shift[47] =   { '!','@','#','$','%','^','&','*','(',')','_','+','Q','W','E','R','T','Y','U','I','O','P','{','}','|' ,'A','S','D','F','G','H','J','K','L',':','"' ,'Z','X','C','V','B','N','M','<','>','?',' ' };
CharMapEntry_S CharMap_S::Entries[47] = {
    {49, '1', '!'}, {50, '2', '@'}, {51, '3', '#'}, {52, '4', '$'}, {53, '5', '%'}, {54, '6', '^'}, {55, '7', '&'}, {56, '8', '*'}, {57, '9', '('},
    {48, '0', ')'}, {45, '-', '_'}, {61, '=', '+'}, {81, 'q', 'Q'}, {87, 'w', 'W'}, {69, 'e', 'E'}, {82, 'r', 'R'}, {84, 't', 'T'}, {89, 'y', 'Y'},
    {85, 'u', 'U'}, {73, 'i', 'I'}, {79, 'o', 'O'}, {80, 'p', 'P'}, {91, '[', '{'}, {93, ']', '}'}, {92, '\\','|'}, {65, 'a', 'A'}, {83, 's', 'S'},
    {68, 'd', 'D'}, {70, 'f', 'F'}, {71, 'g', 'G'}, {72, 'h', 'H'}, {74, 'j', 'J'}, {75, 'k', 'K'}, {76, 'l', 'L'}, {59, ';', ':'}, {39, '\'','"'},
    {90, 'z', 'Z'}, {88, 'x', 'X'}, {67, 'c', 'C'}, {86, 'v', 'V'}, {66, 'b', 'B'}, {78, 'n', 'N'}, {77, 'm', 'M'}, {44, ',', '<'}, {46, '.', '>'},
    {47, '/', '?'}, {32, ' ', ' '}
};
void CharMap_S::InitStatic()
{
   
}

uint utils::GetGUID()
{
    return rmtd::NextGUID();
}

uint utils::StoreElement(void* element)
{
    uint guid = rmtd::NextGUID();
    rmtd::StoreElement(element, guid);
    return guid;
}

void utils::FlipTexture(Framebuf* fb)
{

}

const int step = 1'000;
size_t UtilsHolder::HashF(float f, int times)
{
    size_t x = (size_t)round(f * HASHF_STEP);
    x *= myPow<size_t>(HASHF_STEP, times - 1);
    return x;
}

void UtilsHolder::Limit(float& val, float min, float max) {
    val = std::min(std::max(val, min), max);
}

void UtilsHolder::m11To01(float& x)
{
    x = (x + 1) / 2;
}

void UtilsHolder::m01To11(float& x)
{
    x = x * 2 - 1;
}

void UtilsHolder::m11To01(Bounds& bnd)
{
    m11To01(bnd.x1);m11To01(bnd.y1);
    m11To01(bnd.y2);m11To01(bnd.x2);
}

void UtilsHolder::m01To11(Bounds& bnd)
{
    m01To11(bnd.x1);m01To11(bnd.y1);
    m01To11(bnd.y2);m01To11(bnd.x2);
}

void UtilsHolder::m11To01(Point2D<float>& var) {
    m11To01(var.X);m11To01(var.Y);
}

void UtilsHolder::m01To11(Point2D<float>& var) {
    m01To11(var.X);m01To11(var.Y);
}

void UtilsHolder::ApplyAspect(Point2D<float>& pos, Point2D<float>& size, Align_S* obj, Point2D<float>& parantSize) {
    if (!obj->Aspect.apply)
        return;
    float val = 0;
    if (obj->Aspect.FromHeight) {
        if (obj->VAlign == VerAlign::Stretch)
            val = parantSize.Y;
        else
            val = size.Y * obj->Aspect.value;
        if (obj->Aspect.shiftPositive) {
            size.X -= val;
        } else {
            pos.X -= val;
            size.X -= val;
        }
    } else {
        if (obj->HAlign == HorAlign::Stretch)
            val = parantSize.X;
        else
            val = size.X * obj->Aspect.value;
        if (obj->Aspect.shiftPositive) {
            size.Y += val;
        } else {
            pos.Y -= val;
            size.Y -= val;
        }
    }
}

void UtilsHolder::ApplyAspect(Bounds& bn, Align_S* obj) {
    if (!obj->Aspect.apply)
        return;
    if (obj->Aspect.FromHeight) {
        auto size = bn.SpreadY();
        if (obj->Aspect.shiftPositive) {
            bn.x2 = bn.x1 + size;
            Limit(bn.x2, 0, 1);
        }
        else {
            bn.x1 = bn.x2 - size;
            Limit(bn.x1, 0, 1);
        }
    }
    else {
        auto size = bn.SpreadX();
        if (obj->Aspect.shiftPositive) {
            bn.y2 = bn.y1 + size;
            Limit(bn.y2, 0, 1);
        }
        else {
            bn.y1 = bn.y2 - size;
            Limit(bn.y1, 0, 1);
        }
    }
}

void UtilsHolder::Align(Bounds& bn, Align_S* obj) {
    switch (obj->HAlign) {
        case HorAlign::Left:
            bn.x2 = bn.x2 - bn.x1;
            bn.x1 = 0;
            break;
        case HorAlign::Right:
            bn.x1 = bn.x1 + 1 - bn.x2;
            bn.x2 = 1;
            break;
        case HorAlign::Center: {
            float spread = bn.x2 - bn.x1;
            bn.x1 = (1 - spread) / 2;
            bn.x2 = bn.x1 + spread;
        } break;
        case HorAlign::Stretch:
            bn.x1 = 0;
            bn.x2 = 1;
            break;
        default: break;
    }
    switch (obj->VAlign) {
        case VerAlign::Top:
            bn.y2 = bn.y2 - bn.y1;
            bn.y1 = 0;
            break;
        case VerAlign::Bottom:
            bn.y1 = bn.y1 + 1 - bn.y2;
            bn.y2 = 1;
            break;
        case VerAlign::Center: {
            float spread = bn.y2 - bn.y1;
            bn.y1 = (1 - spread) / 2;
            bn.y2 = bn.y1 + spread;
        } break;
        case VerAlign::Stretch:
            bn.y1 = 0;
            bn.y2 = 1;
            break;
        default: break;
    }
}
void UtilsHolder::Align(Bounds& bn, HorAlign HAlign, VerAlign VAlign)
{
    switch (HAlign) {
    case HorAlign::Left:
        bn.x2 = bn.x2 - bn.x1;
        bn.x1 = 0;
        break;
    case HorAlign::Right:
        bn.x1 = bn.x1 + 1 - bn.x2;
        bn.x2 = 1;
        break;
    case HorAlign::Center: {
        float spread = bn.x2 - bn.x1;
        bn.x1 = (1 - spread) / 2;
        bn.x2 = bn.x1 + spread;
    } break;
    case HorAlign::Stretch:
        bn.x1 = 0;
        bn.x2 = 1;
        break;
    default: break;
    }
    switch (VAlign) {
    case VerAlign::Top:
        bn.y2 = bn.y2 - bn.y1;
        bn.y1 = 0;
        break;
    case VerAlign::Bottom:
        bn.y1 = bn.y1 + 1 - bn.y2;
        bn.y2 = 1;
        break;
    case VerAlign::Center: {
        float spread = bn.y2 - bn.y1;
        bn.y1 = (1 - spread) / 2;
        bn.y2 = bn.y1 + spread;
    } break;
    case VerAlign::Stretch:
        bn.y1 = 0;
        bn.y2 = 1;
        break;
    default: break;
    }
}

std::pair<float, float> UtilsHolder::RemapPoint(float x, float y, Bounds b)
{
    float& x1 = b.x1;
    float& x2 = b.x2;
    float& y1 = b.y1;
    float& y2 = b.y2;
    //y = 1 - y;
    m11To01(x1); m11To01(y1); m11To01(x2); m11To01(y2);
    float xNew = (x - x1) / (x2 - x1);
    float yNew = (y - y1) / (y2 - y1);
    //yNew = 1 - yNew;
    return std::make_pair(xNew, yNew);
}
Point2D<float> UtilsHolder::RemapPoint(Point2D<float> cords, Bounds b) {
    float& x1 = b.x1;
    float& x2 = b.x2;
    float& y1 = b.y1;
    float& y2 = b.y2;
    //y = 1 - y;
    m11To01(x1); m11To01(y1); m11To01(x2); m11To01(y2);
    float xNew = (cords.X - x1) / (x2 - x1);
    float yNew = (cords.Y - y1) / (y2 - y1);
    //yNew = 1 - yNew;
    return { xNew,yNew };
}

std::pair<float, float> UtilsHolder::RemapPoint(float x, float y, float x1, float x2, float y1, float y2)
{
    y = 1 - y;
    m11To01(x1); m11To01(y1); m11To01(x2); m11To01(y2);
    float xNew = (x - x1) / (x2 - x1);
    float yNew = (y - y1) / (y2 - y1);
    yNew = 1 - yNew;
    return std::make_pair(xNew, yNew);
}

void UtilsHolder::Draw(uint v)
{
    glBindVertexArray(v);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void UtilsHolder::DrawStrips(uint v)
{
    glBindVertexArray(v);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

uint UtilsHolder::makeCube6VAO() {
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
    // first, configure the cube's VAO (and VBO)
    unsigned int VBO;
    glGenVertexArrays(1, &cube6VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cube6VAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    return cube6VAO;
}

void UtilsHolder::renderCube6() {
    if (cube6VAO == 0) {
        makeCube6VAO();
    }
    glBindVertexArray(cube6VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

uint UtilsHolder::makeCube8VAO()     {
    float vertices[] = {
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        // right face
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
        // bottom face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
         1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
    };
    glGenVertexArrays(1, &cube8VAO);
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(cube8VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return cube8VAO;
}

void UtilsHolder::renderCube8()
{
    if (cube8VAO == 0)
    {
        makeCube8VAO();
    }
    glBindVertexArray(cube8VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void UtilsHolder::renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

PixelInfo UtilsHolder::ReadPixel(PixelInfo& pInfo, uint FBO, float x, float y)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glFlush(); glFinish();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    uchar data[4];
    glReadPixels((GLint)x, (GLint)y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
    int pickedObjID =
        data[0] +
        data[1] * 256;
    int pickedPrimitiveID =
        data[2];
    if (pickedObjID > 5000)
        pickedObjID = 0;
    
    pInfo = { Identity(0, (uint)pickedObjID, 0, 0, pickedPrimitiveID) };
    //std::cout << "pickedObjID :" << pickedObjID << std::endl;
    //std::cout << "pickedPrimitiveID:" << pickedPrimitiveID << std::endl;
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    return pInfo;
}

int UtilsHolder::DigCnt(int number)
{
    int count = 1;
    while (number != (number /= 10))
    {
        count++;
    }
    return count;
}

int UtilsHolder::safe_stoi(std::string str) {
    if (str == "" || str == "-") return 0;
    return std::stoi(str);}
float UtilsHolder::safe_stof(std::string str) {
    if (str == "-." || str == "-") return -0;
    if (str == "" || str == ".") return 0;
    return std::stof(str);}
bool UtilsHolder::has_dot(std::string str) {
    if (str.find('.') != std::string::npos) return true;
    return false;}

Point2D<float> UtilsHolder::CordsOf(Framebuf* obj, Point2D<float> cords) {
   
    std::vector<Framebuf*> trace;
    Framebuf* next = obj;
    while (next) {
        trace.push_back(next);
        next = next->Parent;
    }
    for ( auto iter = trace.rbegin(); iter != trace.rend(); iter++) {
        cords = utl::RemapPoint(cords, iter[0]->m_Bounds);
    }
  
    return cords;
}

float UtilsHolder::trim(float value, float min, float max) {
    return std::max(std::min(value, max), min);
}


static uint NextGUID()
{
    return rmtd::NextGUID();
}

void MouseRay_S::TraceHover(DrawObj* obj) {
    if (cur_depth >= trace.size()) {
        obj->States.Hovered = true;
        trace.push_back(obj);
    }
    else {
        if (trace[cur_depth] != obj) {
            for (auto iter = trace.begin() + cur_depth; iter != trace.end(); iter++) {
                (*iter)->States.Hovered = false;
                (*iter)->States.Changed = true;
            }
            trace.erase(trace.begin() + cur_depth, trace.end());
            obj->States.Hovered = true;
            trace.push_back(obj);
        }
    }
    cur_depth++;
}
void MouseRay_S::CutTrace() {
    for (auto iter = trace.begin() + cur_depth; iter != trace.end(); iter++) {
        (*iter)->States.Hovered = false;
        (*iter)->States.Changed = true;
    }
    trace.erase(trace.begin() + cur_depth, trace.end());
}

    