#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <glad/glad.h>

#include "resource_manager.h"

#include <iostream>
#include <sstream>
#include <fstream>
//#include <stdlib.h>

#include "stb_image.h"
#include "stb_image_write.h"
#include "framebuf.h"
#include "text_renderer.h"
#include "control.h"


// Instantiate static variables

ResourceManager::constructor ResourceManager::cons;// = ResourceManager::constructor();
std::vector<void*> ResourceManager::ToDelete;
std::vector<uint> ResourceManager::UpdateChain;

WindowBase* ResourceManager::CurWin;
std::map<GLFWwindow*, WindowBase*> ResourceManager::Windows;
std::map<std::string, Shader>   ResourceManager::Shaders;
std::map<std::string, Texture2D*> ResourceManager::Textures;
std::map<uint, void*> ResourceManager::Elements;
std::map<uint, TextRenderer*> ResourceManager::TextRends;
std::unordered_map<Bounds, uint, BoundsHash> ResourceManager::VAOs;
PixelInfo ResourceManager::PickedObjs;
PickFramebuf* ResourceManager::PickFb;
std::vector<Framebuf*> ResourceManager::DFbs;
TextRenderer* ResourceManager::s_TextRend;
MouseRay_S ResourceManager::MouseRay;
StackPanel* ResourceManager::Inspector;
StackPanel* ResourceManager::SceneInfo;
StackPanel* ResourceManager::Hierarchy;
InspectorData ResourceManager::GlobalInspData;
HierarchyData ResourceManager::HierData;
ResourceManager::DragData_S ResourceManager::DragData;

std::unordered_map<std::string,glm::vec4> ResourceManager::Colors;

uint ResourceManager::m_current_guid = 0;

void ResourceManager::InitResources() {
    LoadShader("resources/shaders/picking.vect", "resources/shaders/picking.frag", nullptr, "picking");
    LoadShader("resources/shaders/picking_3d.vect", "resources/shaders/picking_3d.frag", nullptr, "picking_3d");
    LoadShader("resources/shaders/pass_tex.vect", "resources/shaders/pass_tex.frag", nullptr, "pass_tex");
    LoadShader("resources/shaders/pass_col.vect", "resources/shaders/pass_col.frag", nullptr, "pass_col");
    LoadShader("resources/shaders/text_2d.vect", "resources/shaders/text_2d.frag", nullptr, "text2d");
    LoadShader("resources/shaders/text_3d.vect", "resources/shaders/text_3d.frag", nullptr, "text3d");
    LoadShader("resources/shaders/flip_tex.vect", "resources/shaders/flip_tex.frag", nullptr, "flip_tex");
    LoadShader("resources/shaders/point.vect", "resources/shaders/point.frag", nullptr, "point");
    LoadShader("resources/shaders/allpoints.vect", "resources/shaders/allpoints.frag", "resources/shaders/allpoints.geom", "allpoints");
    LoadShader("resources/shaders/model_sh.vect", "resources/shaders/model_sh.frag", nullptr, "model_sh");
    LoadShader("resources/shaders/pass_tex_3d.vect", "resources/shaders/pass_tex_3d.frag", nullptr, "pass_tex_3d");
    LoadShader("resources/shaders/pass_col_3d.vect", "resources/shaders/pass_col_3d.frag", nullptr, "pass_col_3d");
    LoadShader("resources/shaders/lshader_col.vect", "resources/shaders/lshader_col.frag", nullptr, "lshader_col");
    LoadShader("resources/shaders/vert_col.vect", "resources/shaders/vert_col.frag", nullptr, "vert_col");
    LoadShader("resources/shaders/triangle_col.vect", "resources/shaders/triangle_col.frag", nullptr, "triangle_col");
    LoadShader("resources/shaders/triangle_tex.vect", "resources/shaders/triangle_tex.frag", nullptr, "triangle_tex");
    LoadShader("resources/shaders/picking_3d_triang.vect", "resources/shaders/picking_3d_triang.frag", nullptr, "picking_3d_triang");
    LoadShader("resources/shaders/infinite_grid.vect", "resources/shaders/infinite_grid.frag", nullptr, "infinite_grid");
    LoadShader("resources/shaders/draw_grid.vect", "resources/shaders/draw_grid.frag", nullptr, "draw_grid");
    /*
LoadShader("resources/shaders/gbuf/light_pass.vect", "resources/shaders/gbuf/light_pass.frag", nullptr, "light_pass");
LoadShader("resources/shaders/gbuf/geometry_pass.vect", "resources/shaders/gbuf/geometry_pass.frag", nullptr, "geometry_pass");
LoadShader("resources/shaders/gbuf/light_pass_box.vect", "resources/shaders/gbuf/light_pass_box.frag", nullptr, "light_pass_box");
*/
    PickFb = new PickFramebuf();
    s_TextRend = new TextRenderer({ TR_HEIGHT, TR_WIDTH });
    s_TextRend->Load("resources/fonts/OCRAEXT.TTF", 24);
    //s_TextRend->Load("resources/fonts/Antonio-Bold.TTF", 24);

    rmtd::Colors.insert({ "button_fg_inactive", glm::vec4{ 0.8f, 0.8f, 0.8f, 1.0f } });
    rmtd::Colors.insert({ "button_bg_inactive", glm::vec4{ 0.1f, 0.1f, 0.1f, 1.0f } });
    rmtd::Colors.insert({ "button_fg_active", glm::vec4{ 0.0f, 1.0f, 1.0f, 1.0f } });
    rmtd::Colors.insert({ "button_bg_active", glm::vec4{ 0.05f, 0.05f, 0.1f, 1.0f } });
    rmtd::Colors.insert({ "drag_drop_dummy", glm::vec4{ 0.0f, 0.8f, 1.0f, 0.5f } });



    LoadTexture("resources/textures/wood.png", true, "wood.png");
    LoadTexture("resources/textures/container2.png", true, "container2.png");
    LoadTexture("resources/textures/container2_specular.png", true, "container2_specular.png");
    LoadTexture("resources/textures/toy_box_disp.png", true, "toy_box_disp.png");
    LoadTexture("resources/textures/toy_box_normal.png", true, "toy_box_normal.png");
    LoadTexture("resources/textures/window.png", true, "window.png");
    LoadTexture("resources/textures/face.png", true, "face.png");
    LoadTexture("resources/textures/wall.jpg", true, "wall.jpg");
    LoadTexture("resources/textures/bricks2.jpg", true, "bricks2.jpg");
    LoadTexture("resources/textures/bricks2_disp.jpg", true, "bricks2_disp.jpg");
    LoadTexture("resources/textures/bricks2_normal.jpg", true, "bricks2_normal.jpg");
    LoadTexture("resources/textures/brickwall.jpg", true, "brickwall.jpg");
    LoadTexture("resources/textures/brickwall_normal.jpg", true, "brickwall_normal.jpg");
    LoadTexture("resources/textures/container.jpg", true, "container.png");
    LoadTexture("resources/textures/grid_1.png", true, "grid_1.png");
    LoadTexture("resources/textures/grid_2.png", true, "grid_2.png");
    LoadTexture("resources/textures/grid_3.png", true, "grid_3.png");
    LoadTexture("resources/textures/grid_4.png", true, "grid_4.png");
    LoadTexture("resources/textures/grid_5.png", true, "grid_5.png");
    LoadTexture("resources/textures/grid_6.png", true, "grid_6.png");
    LoadTexture("resources/textures/grid_7.png", true, "grid_7.png");
    LoadTexture("resources/textures/grid_8.png", true, "grid_8.png");
    LoadTexture("resources/textures/grid_9.png", true, "grid_9.png");
    LoadTexture("resources/textures/grid_10.png", true, "grid_10.png");

    if (false) {
        int width = 800, height = 800;
        int nChannels = 4;
        //uint8_t* data = new uint8_t[width * height * nChannels];
        //char* data = (char*)malloc((size_t)(width * height * 4));
        uint8_t* data = (uint8_t*)calloc((size_t)width * height * nChannels, sizeof(uint8_t));

        int stride = width * nChannels;
        int lineSpace = width / 10;
        int index;
        struct tmp_color_S {
            uchar r;
            uchar g;
            uchar b;
            uchar a;
        };
        tmp_color_S border = { 150,0,0,255 };
        tmp_color_S line = { 150,150,150,255 };
        for (int ii = 1; ii <= 10; ii++) {
            int tk = 2 * ii + 1;
            int tku = ii;
            int tkl = -ii;
            for (int i = lineSpace; i < width; i += lineSpace) { // i dole , j desno
                for (int j = 0; j < width; j++) {
                    index = i * stride + j * nChannels;
                    for (int k = tkl; k < tku; k++) { // na desno
                        ((tmp_color_S*)(&data[index + k * stride]))[0] = line;
                    }

                    index = j * stride + i * nChannels;
                    for (int k = tkl; k < tku; k++) { // na dole
                        ((tmp_color_S*)(&data[index + k * nChannels]))[0] = line;
                    }
                }
            }
            for (int i = 0; i < width; i++) {
                index = i * nChannels;
                for (int k = 0; k < tk; k++) { // gornje 
                    ((tmp_color_S*)(&data[index + k * stride]))[0] = border;
                }

                index = (width - tk) * stride + i * nChannels;
                for (int k = 0; k < tk; k++) { // donje
                    ((tmp_color_S*)(&data[index + k * stride]))[0] = border;
                }
                index = i * stride;
                for (int k = 0; k < tk; k++) { // levo
                    ((tmp_color_S*)(&data[index + k * nChannels]))[0] = border;
                }

                index = (width - tk) * nChannels + i * stride;
                for (int k = 0; k < tk; k++) { // desno
                    ((tmp_color_S*)(&data[index + k * nChannels]))[0] = border;
                }
            }
            SaveImage("grid_" + std::to_string(ii), width, height, nChannels, data);
        }
        //delete[]data;
        free(data);
        SaveTexture("test1", Textures["container.png"]);
    }
    DragData.dummy = new Dummy2D("dd_target_dummy");
    DragData.dummy->Init();
}

Shader* ResourceManager::LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, std::string name) {
    Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return &Shaders[name];
}
Shader* ResourceManager::GetShader(std::string name) {
    return &Shaders[name];
}
Texture2D* ResourceManager::LoadTexture(const char* file, bool alpha, std::string name) {
    Textures[name] = loadTextureFromFile(file, alpha);
    Textures[name]->Name = name;
    return Textures[name];
}
Texture2D* ResourceManager::GetTexture(std::string name) {
    return Textures[name];
}
void ResourceManager::StoreElement(void* item, uint guid) {
    Elements[guid] = item;
}
void* ResourceManager::GetElement(uint guid) {
    void* temp = Elements[guid];
    DrawObj* test2 = (DrawObj*)temp;
    return temp;
}
void ResourceManager::RemoveElement(uint guid) {
    Elements.erase(guid);
}
void ResourceManager::StoreTextRend(TextRenderer* tr) {
    TextRends[tr->ID] = tr;
}
TextRenderer* ResourceManager::GetTextRend(uint ID) {
    TextRenderer* temp = TextRends[ID];
    return temp;
}
void ResourceManager::StoreVao(uint v, Bounds bn) {
    VAOs[bn] = v;
}
uint ResourceManager::GetVAO(const Bounds& bn_cr) {
    if (VAOs.count(bn_cr))
        return VAOs[bn_cr];
    unsigned int VBO, VAO;

    Bounds dc = {
        bn_cr.x1,
        bn_cr.y2 * -1,
        bn_cr.x2,
        bn_cr.y1 * -1,
        /*
        bn_cr.y1,
        bn_cr.y2,
        */
    };
    float am = 1.0f;
    Bounds bn_tx = {
        0.0f,
        0.0f,
        am,
        am,
    };
    Bounds dt = {
        bn_tx.u1,
        bn_tx.v1,
        bn_tx.u2,
        bn_tx.v2,
    };
    float quadVertices[] = {
        // positions        // texture Coords
        dc.x1, dc.y2, 0.0f, dt.u1, dt.v2,
        dc.x1, dc.y1, 0.0f, dt.u1, dt.v1,
        dc.x2, dc.y2, 0.0f, dt.u2, dt.v2,
        dc.x2, dc.y1, 0.0f, dt.u2, dt.v1,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    VAOs[bn_cr] = VAO;
    return VAO;
}
void ResourceManager::StorePicked(PixelInfo pi) {
    PickedObjs = pi;
}
PixelInfo& ResourceManager::GetPicked() {
    return PickedObjs;
}
void ResourceManager::CC(GLFWwindow* id) {
    glfwMakeContextCurrent(id);
    CurWin = Windows[id];
}
void ResourceManager::Clear() {
    for (auto& iter : Shaders)
        glDeleteProgram(iter.second.ID);
    for (auto iter : Textures)
        glDeleteTextures(1, &iter.second->ID);
}

std::wstring ExePath() {
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos);
}
Shader ResourceManager::loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile) {
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try {
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        vertexShaderFile.close();
        fragmentShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        if (gShaderFile != nullptr) {
            std::ifstream geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    } catch (std::exception e) {
        std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    const char* gShaderCode = geometryCode.c_str();
    Shader shader;
    shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
    return shader;
}
Texture2D* ResourceManager::loadTextureFromFile(const char* path, bool alpha) {
    Texture2D* texture = new Texture2D();
    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = 0;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        texture->Internal_Format = format;
        texture->Image_Format = format;
        texture->Generate({ (float)width, (float)height }, data);
        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    texture->Stored = true;
    return texture;
}
void ResourceManager::SaveTexture(std::string name, Texture2D* tex) {

    int width = (int)tex->Size.X;
    int height = (int)tex->Size.Y;
    int nChannels = 4;
    uint8_t* data = new uint8_t[width * height * nChannels];
    glBindTexture(GL_TEXTURE_2D, tex->ID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    SaveImage(name, width, height, nChannels, data);

    delete[] data;
}
void ResourceManager::SaveImage(std::string name, int width, int height, int nChannels, void* data) {

    auto ret = ExePath();
    char* fixed = new char[100];
    wcstombs(fixed, ret.c_str(), 100);
    string path(fixed);
    path = path.substr(0, path.find_last_of("\\/"));
    path = path.substr(0, path.find_last_of("\\/"));
    path = path + "\\GEdit\\resources\\generated\\textures\\" + name + ".png";
    stbi_write_png(path.c_str(), width, height, nChannels, data, 0);
    /*
    #include <filesystem>
    #include <iostream>

    std::filesystem::path cwd = std::filesystem::current_path() / "filename.txt";
    std::ofstream file(cwd.string());
    file.close();
    */
}
uint ResourceManager::NextGUID() {
    return ++m_current_guid;
}
void HierarchyData::SetHierarchy(Object* sender, Control* hierInfo) {
    auto Hierarchy = rmtd::Hierarchy;
    Hierarchy->ClearChilds();
    Hierarchy->AddChild(hierInfo);
    active = sender;
}

