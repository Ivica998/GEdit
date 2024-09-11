#ifndef BALLOBJECT_H
#define BALLOBJECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "base_object.h"
#include "texture.h"

class BallObject : public BaseObject
{
public:
    float   Radius;
    bool    Stuck;
    bool    Sticky, PassThrough;
    BallObject();
    BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite);
    // moves the ball, keeping it constrained within the window bounds (except bottom edge); returns new position
    glm::vec2 Move(float dt, unsigned int window_width);
    void      Reset(glm::vec2 position, glm::vec2 velocity);
};

#endif