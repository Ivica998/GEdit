

#include <algorithm>
#include <sstream>
#include <iostream>

#include "filesystem.h"

#include <irrklang/irrKlang.h>
using namespace irrklang;

#include "editor.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "base_object.h"
#include "particle_generator.h"
#include "post_processor.h"
#include "text_renderer.h"
#include "ball_object.h"
#include "../resource_manager.h"

SpriteRenderer    *Renderer;
ParticleGenerator *Particles;
PostProcessor     *Effects;
ISoundEngine      *SoundEngine = createIrrKlangDevice();
TextRenderer      *Text;

float ShakeTime = 0.0f;


Editor::Editor(unsigned int width, unsigned int height)
    : State(Editor_MENU), Keys(), KeysProcessed(), Width(width), Height(height), Level(0), Lives(3)
{ 
}

Editor::~Editor()
{
    delete Renderer;
    delete Particles;
    delete Effects;
    delete Text;
    SoundEngine->drop();
}

void Editor::Init()
{
    // load shaders
    ResourceManager::LoadShader("sprite.vs", "sprite.fs", nullptr, "sprite");
    ResourceManager::LoadShader("particle.vs", "particle.fs", nullptr, "particle");
    ResourceManager::LoadShader("post_processing.vs", "post_processing.fs", nullptr, "postprocessing");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").SetMatrix4("projection", projection);
    // load textures
    ResourceManager::LoadTexture("resources/textures/background.jpg", false, "background");
    ResourceManager::LoadTexture("resources/textures/awesomeface.png", true, "face");
    ResourceManager::LoadTexture("resources/textures/block.png", false, "block");
    ResourceManager::LoadTexture("resources/textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("resources/textures/paddle.png", true, "paddle");
    ResourceManager::LoadTexture("resources/textures/particle.png", true, "particle");
    ResourceManager::LoadTexture("resources/textures/powerup_speed.png", true, "powerup_speed");
    ResourceManager::LoadTexture("resources/textures/powerup_sticky.png", true, "powerup_sticky");
    ResourceManager::LoadTexture("resources/textures/powerup_increase.png", true, "powerup_increase");
    ResourceManager::LoadTexture("resources/textures/powerup_confuse.png", true, "powerup_confuse");
    ResourceManager::LoadTexture("resources/textures/powerup_chaos.png", true, "powerup_chaos");
    ResourceManager::LoadTexture("resources/textures/powerup_passthrough.png", true, "powerup_passthrough");
    // set render-specific controls
    Shader temp = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(temp);
    Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);
    Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width, this->Height);
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("resources/fonts/OCRAEXT.TTF", 24);
    // audio
    SoundEngine->play2D("resources/audio/breakout.mp3", true);
}

void Editor::Update(float dt)
{
    // update objects
}

void Editor::ProcessInput(float dt)
{
    if (this->State == Editor_MENU)
    {
        if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
        {
            this->State = Editor_ACTIVE;
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
        }
    }
}

void Editor::Render()
{
    if (this->State == Editor_ACTIVE || this->State == Editor_MENU || this->State == Editor_WIN)
    {
        // begin rendering to postprocessing framebuffer
        Effects->BeginRender();
            // draw background
            Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
            // draw particles	
            Particles->Draw();   
        // end rendering to postprocessing framebuffer
        Effects->EndRender();
        // render postprocessing quad
        Effects->Render(glfwGetTime());
        // render text (don't include in postprocessing)
        std::stringstream ss; ss << this->Lives;
        Text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);
    }
}

bool ShouldSpawn(unsigned int chance)
{
    unsigned int random = rand() % chance;
    return random == 0;
}

// collision detection
bool CheckCollision(BaseObject &one, BaseObject &two);
Collision CheckCollision(BallObject &one, BaseObject &two);
Direction VectorDirection(glm::vec2 closest);

void Editor::DoCollisions()
{

}

bool CheckCollision(BaseObject &one, BaseObject &two) // AABB - AABB collision
{
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

Collision CheckCollision(BallObject &one, BaseObject &two) // AABB - Circle collision
{
    // get center point circle first 
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // now that we know the clamped values, add this to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // now retrieve vector between center circle and closest point AABB and check if length < radius
    difference = closest - center;
    
    if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

// calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}

