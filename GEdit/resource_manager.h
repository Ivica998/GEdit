#pragma once

#include <unordered_map>
#include <map>
#include <string>
#include <glad/glad.h>

//#include "framebuf.h"
#include "window.h"

#include "texture.h"
#include "shader.h"
#include "utils.h"

class Framebuf;
class DrawObj2D;
class Component;
class PickFramebuf;
class TextRenderer;
class StackPanel;
class Scrollable;
class Dummy2D;

struct ResHolder
{
    WindowBase* Windows;
    PixelInfo PickedObjs;
    std::map<uint, DrawObj2D*> DrawableObjs;
    std::map<uint, Framebuf*> Framebufs;
    std::map<std::string, Shader> Shaders;
    std::map<std::string, Texture2D> Textures;
};

class Object;
class Control3D;
struct InspectorData {
    Object* active;
    fields_t Content;
};
struct HierarchyData {
    Object* active;
    void SetHierarchy(Object* sender, Control* hierInfo);
};

class ResourceManager
{
public:
    friend class constructor;
    struct constructor {
        constructor() {
        
        }
        ~constructor()
        {
            for (void* item : ToDelete) { delete item; }
        }
    };
    static constructor cons;
    struct DragData_S {
        bool doing = true;
        DrawObj2D* dragged;
        Dummy2D* dummy;
    };

    static std::vector<void*> ToDelete;
    static std::vector<uint> UpdateChain;
    static PixelInfo PickedObjs;

    static std::map<GLFWwindow*, WindowBase*> Windows;

    static std::map<uint, TextRenderer*> TextRends;
    static std::map<uint, void*> Elements;


    static std::map<std::string, Shader>    Shaders;
    static std::map<std::string, Texture2D*> Textures;
    static std::unordered_map<Bounds, uint, BoundsHash> VAOs;

    static WindowBase* CurWin;
    static PickFramebuf* PickFb;
    static std::vector<Framebuf*> DFbs;
    static TextRenderer* s_TextRend;
    static MouseRay_S MouseRay;
    static StackPanel* Inspector;
    static StackPanel* SceneInfo;
    static StackPanel* Hierarchy;
    static InspectorData GlobalInspData;
    static HierarchyData HierData;
    static DragData_S DragData;
    static std::unordered_map<std::string,glm::vec4> Colors;
 
    static void InitResources();
    static Shader*    LoadShader(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile, std::string name);
    static Shader*    GetShader(std::string name);
    static Texture2D* LoadTexture(const char *file, bool alpha, std::string name);
    static Texture2D* GetTexture(std::string name);
    static void StoreElement(void* item, uint guid);
    static void* GetElement(uint guid);
    static void RemoveElement(uint guid);
    static void StoreTextRend(TextRenderer* tr);
    static TextRenderer* GetTextRend(uint ID);
    static void StoreVao(uint v, Bounds bn);
    static uint GetVAO(const Bounds& bn = Bounds::s_default);
    static void StorePicked(PixelInfo pi);
    static PixelInfo& GetPicked();
    static void CC(GLFWwindow* id);
    static void Clear();
    static uint NextGUID();

    template<typename T = Framebuf>
    static Framebuf* MakeFb(std::string name)
    {
        Framebuf* fb = new T(name);
        fb->Init();
        ToDelete.push_back(fb);
        return fb;
    }
private:
    static uint m_current_guid;
    // private constructor, that is we do not want any actual resource manager objects. Its members and functions should be publicly available (static).
    ResourceManager() { }
    static Shader    loadShaderFromFile(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile = nullptr);
    static Texture2D* loadTextureFromFile(const char* file, bool alpha);
    static void SaveTexture(std::string name, Texture2D* tex);
    static void SaveImage(std::string name, int width, int height, int nChannels, void* data);

        
};

typedef ResourceManager rmtd;
