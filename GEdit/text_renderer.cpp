#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/freetype.h>

#include "text_renderer.h"
#include "resource_manager.h"

int TextRenderer::count = 0;

TextRenderer::TextRenderer(Point2D<float> size) : Fb("textRend_fb") {
    Fb.m_shader = rmtd::GetShader("pass_tex");
    rmtd::StoreTextRend(this);
    Size = size;
    // load and configure shader
    this->TextShader = rmtd::GetShader("text2d");
    this->TextShader->Use();
    this->TextShader->SetMat4("projection", glm::ortho(0.0f, Size.X, 0.0f, Size.Y));
    this->TextShader->SetInt("text", 0);
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextRenderer::Load(std::string font, unsigned int fontSize) {
    this->Characters.clear();
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) // all functions return a value different than 0 whenever an error occurred
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    FT_Face face;
    if (FT_New_Face(ft, font.c_str(), 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (!FT_Load_Char(face, '|', FT_LOAD_RENDER)) {
        int max_w = 6;
        int max_h = 21;
        CharFramebuf* ch = new CharFramebuf();
        auto& gly = face->glyph;
        auto& bm = face->glyph->bitmap;
        ch->Setup((float)max_w, (float)max_h, 0, 0, gly->advance.x, gly->advance.y);
        int size = std::max(max_h * max_w * 3, 3);
        char* buffer = new char[size]();
        int index = 0;
        for (int i = 0; i < max_h; i++) {
            for (int j = 0; j < max_w; j++) {
                if (j < 2) {
                    index = 3 * (i * max_w + j);
                    buffer[index + 0] = 1;
                    buffer[index + 1] = 1;
                    buffer[index + 2] = 1;
                } else break;
            }
        }
        ch->buffer = bm.buffer;
        ch->Init();
        ch->chr = 0;
        Characters.insert(std::pair<uchar, CharFramebuf*>(0, ch));
        utils::FlipTexture(ch);
    }
    for (GLubyte c = 1; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        CharFramebuf* ch = new CharFramebuf(face, face->glyph->bitmap.buffer);
        ch->chr = (uchar)c;
        Characters.insert(std::pair<uchar, CharFramebuf*>((uchar)c, ch));
        utils::FlipTexture(ch);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void TextRenderer::RenderToFb(std::string text, float x, float y, float scale, glm::vec4 color) {
    this->Fb.BeginRender();
    Render(text, x, y, scale, color);
    this->Fb.EndRender();
}

void TextRenderer::Render(std::string text, float x, float y, float scale, glm::vec4 color) {
    auto win = rmtd::CurWin;
    glViewport(0, 0, (int)win->Size.X, (int)win->Size.Y);
    //OldRend(text, x, y, scale, color);
    if (true) {
        this->TextShader->Use();
        this->TextShader->SetVec4f("textColor", color);
        this->TextShader->SetMat4("projection", glm::ortho(-1.0f, 1.0f, 1.0f, -1.0f));
        this->TextShader->SetInt("text", 0);

        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++) {
            CharFramebuf* ch = Characters[*c];

            float xpos = x + ch->Bearing.X * scale;
            float ypos = y + (Characters['H']->Bearing.Y - ch->Bearing.Y) * scale;
            float w = ch->ActualSize.X * scale;
            float h = ch->ActualSize.Y * scale;

            float x1, x2, y1, y2;
            x1 = xpos / Size.X;
            x2 = (xpos + w) / Size.X;
            y1 = ypos / Size.Y;
            y2 = (ypos + h) / Size.Y;
            utl::m01To11(x1);utl::m01To11(x2);utl::m01To11(y1);utl::m01To11(y2);
            ch->color = color;
            ch->RenderB({ x1, y1, x2, y2 });

            x += (ch->Advance.X >> 6) * scale;
        }
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

