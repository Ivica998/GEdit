#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>

#include "framebuf.h"
//#include "control.h"
#include "camera.h"
#include "utils.h"



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
static void mouse_button_callback(GLFWwindow* window, int Button, int Action, int Mode);
static void cursor_pos_callback(GLFWwindow* window, double x, double y);

extern float lastX;
extern float lastY;
extern bool firstMouse;
class Originbuf;
class WindowBase
{
public:
    GLFWwindow *GLID;
    GLFWwindow *ParentGLID;
    std::string Name;
    Originbuf* GUI;
    //Scene* Scene;
    //Framebuf* Inspector;
    ComponentState State;
    Point2D<float> Buffer;
    Point2D<float> Size;
    Point2D<float> Pos;
    Point2D<float> ActualSize;
    InputArgs_S InputArgs;
    bool Picking = false;
    bool Changed = true;
    std::unordered_set<DrawObj2D*> ActiveObjs;
    bool Active;

    MouseMoveEvent_E e_mouse_move = MouseMoveEvent_E(nullptr, this);


    //WindowBase(std::string name);
    WindowBase(bool skip, std::string name = "unnamed", GLFWwindow* parentID = nullptr, Originbuf* cd = nullptr,
        Point2D<float> pos = { 0.0f, 32.0f }, Point2D<float> size = { WIN_WIDTH, WIN_HEIGHT })
        : GUI(cd), Name(name), Pos(pos), Size(size), ActualSize(size), State(ComponentState::Enabled), m_lmb() {
        this->Init(skip, name, parentID);
    }
    WindowBase(std::string name = "unnamed", GLFWwindow* parentID = nullptr, Originbuf* cd = nullptr,
        Point2D<float> pos = { 300.0f, 200.0f }, Point2D<float> size = { WIN_WIDTH, WIN_HEIGHT })
        : WindowBase(false, name, parentID, cd, pos, size) {}




    int Init(bool skip, std::string name, GLFWwindow* parentID = nullptr);
    void Perform(float dt);
    void MouseCB(int button, int action, int x, int y);
    void ProcessInput(float dt);
    void PassiveMouseCB(float x, float y);
    void Update(float dt);
    void Render();
    void DoCollisions();
    void OnMouseMove();
    Point2D<float> GetNDCCursor();
    Point2D<float> GetNDCCursorChange();

    struct MousePress {
        bool IsPressed = false;
        bool WasPressed = false;
        int x = 0;
        int y = 0;
        float t_from_press = 0;
    } m_lmb;
    MousePress m_rmb;
    Point2D<float> cursor_pos = {};
    Point2D<float> last_cursor_pos = {};
    Point2D<float> cursor_pos_change = {};
    bool update_cursor_pos = true;
    Point2D<float> m_size_scale = { 1.f,1.f };

    bool operator == (const WindowBase& obj)
    {
        if (GLID == obj.GLID)
            return true;
        return false;
    }
};

class MainWindow : public WindowBase
{
public:
    MainWindow(std::string name = "main_window");
    int Init();
};
