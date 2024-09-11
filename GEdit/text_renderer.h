#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <map>

#include <glad/glad.h>
#include <glm/glm.hpp>

//#include "utils.h"
#include "texture.h"
#include "shader.h"
#include "framebuf.h"


struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // size of glyph
    glm::ivec2   Bearing;   // offset from baseline to left/top of glyph
    unsigned int Advance;   // horizontal offset to advance to next glyph
};

class CharFramebuf;

class TextRenderer
{
public:
    //uint GUID = utils::StoreElement(this);
    uint ID = count++;
    Point2D<float> Size;
    std::map<uchar, CharFramebuf*> Characters;
    Shader* TextShader;
    Framebuf Fb;
    TextRenderer(Point2D<float> size);
    void Load(std::string font, unsigned int fontSize);
    void RenderToFb(std::string text, float x, float y, float scale, glm::vec4 color = glm::vec4(1.0f));
    void Render(std::string text, float x, float y, float scale, glm::vec4 color = glm::vec4(1.0f));
    static int count;
private:
    unsigned int VAO, VBO;
};

#endif 