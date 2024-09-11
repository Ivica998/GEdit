#pragma once
#include <vector>
#include <tuple>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "base_object.h"

enum EditorState {
    Editor_ACTIVE,
    Editor_MENU,
    Editor_WIN
};

enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};
typedef std::tuple<bool, Direction, glm::vec2> Collision; // <collision?, what direction?, difference vector center - closest point>

class Editor
{
public:
    EditorState             State;
    bool                    Keys[1024];
    bool                    KeysProcessed[1024];
    unsigned int            Width, Height;
    unsigned int            Level;
    unsigned int            Lives;
    Editor(unsigned int width, unsigned int height);
    ~Editor();
    void Init();
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();
    void DoCollisions();
};