#pragma warning(disable : 4619 4616 26812)
// C4619/4616 #pragma warning warnings
// 26812: The enum type 'x' is unscoped. Prefer 'enum class' over 'enum' (Enum.3).
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <thread>

#include "all.h"
#include "text_renderer.h"
#include "Debug.h"
#include "scene.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;

#define DISPLAY_FBS_CNT 5.0f

struct DisplayCBParams {
    DrawObj2D* dObj;
    int tex_index;
};
struct ClickCBParams {
    uint v1;
    glm::vec4 color;
    Shader* shader;
    Framebuf* host;
};
struct btn1Params {
    Label* lb;
    int* count;
};


static void dpch_click_callback(sender_ptr sender, EventArgs* e, params_t params);
static void display_callback(sender_ptr sender, EventArgs* e, params_t params);
static void label_cords_update(sender_ptr sender, EventArgs* e, params_t params);
static void field_show_update(sender_ptr sender, EventArgs* e, params_t params);

static void scene_state_buttons(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    TabViewer* tv = (TabViewer*)params[0];
    if (tv->activeEntry == nullptr)
        return;
    Scene* scene = (Scene*)tv->activeEntry->Content;
    scene->SState = *((SceneState*)params[1]);
}
static void scene_camera_buttons(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    TabViewer* tv = (TabViewer*)params[0];
    if (tv->activeEntry == nullptr)
        return;
    Scene* scene = (Scene*)tv->activeEntry->Content;
    scene->mainCam->PMode = *((ProjectionMode*)params[1]);
}
static void scene_save_model_btn(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    TabViewer* tv = (TabViewer*)params[0];
    if (tv->activeEntry == nullptr)
        return;
    Scene* scene = (Scene*)tv->activeEntry->Content;
    scene->SaveDesignedModel("test");   //(*(std::string*)params[1]);
}

static void new_scene_callback(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    TabViewer* tv = (TabViewer*)params[0];
    Scene* scene2 = new Scene("scene2");
    scene2->Size.Set("base_rel", { 1, 1 }, MMode::Rel);
    scene2->Init();
    tv->AddTab(scene2);
}

static void new_grid_tab_callback(sender_ptr sender, MouseClickEventArgs* e, params_t params);

float ourLerp(float a, float b, float f) {
    return a + f * (b - a);
    this_thread::sleep_for(chrono::milliseconds(5000));
}


class Program : NamedObj {
public:
#pragma region vars
    int scene_num = 1;
    std::vector<void*> ToDelete;
    WindowBase* window = nullptr;
    std::vector<Framebuf*> dBufs;
    int dcnt = 0;
    Shader* shaderPass = nullptr;
    Shader* shaderPassCol = nullptr;
    TextRenderer* textRend = nullptr;
    Scene* scene1 = nullptr;
    StackPanel* Inspector = nullptr;
    StackPanel* Hierarchy = nullptr;
    Originbuf* guiFb = nullptr;
    Movable* leftFb = nullptr;
    Framebuf* topFb = nullptr;
    Movable* sceneFb = nullptr;
    Movable* rightFb = nullptr;
    Movable* downFb = nullptr;

    Event_E e_InvokeQ = Event_E(&Dispatcher::InvokeQ, nullptr);
#pragma endregion

    Program() : NamedObj("Program") {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = new MainWindow();
    }
    ~Program() {
        try {
            glfwTerminate();
        } catch (const std::exception& e) {
            cout << e.what();
        }
        for (void* item : ToDelete) { delete item; }
        delete textRend;
        delete window;
    }
    void InitAll() {
        InitResources();
        InitFields();
    }
    void InitResources() {
        rmtd::InitResources();
    }
    void InitFields() {
        shaderPass = rmtd::GetShader("pass_tex");
        shaderPassCol = rmtd::GetShader("pass_col");
        textRend = rmtd::s_TextRend;
        shaderPass->Use();
        shaderPass->SetVec3f("selCol", 0.f, 1.f, 0.7f);
    }
    void setup_base_objects() {
        guiFb = MakeFb<Originbuf>("guiFb");
        leftFb = MakeFb<Movable>("leftFb");
        topFb = MakeFb<Framebuf>("topFb");
        sceneFb = MakeFb<Movable>("sceneFb");
        rightFb = MakeFb<Movable>("rightFb");
        downFb = MakeFb<Movable>("downFb");
        window->GUI = guiFb;
        rmtd::PickFb->Resize(guiFb->ActualSize);
        sceneFb->Size.Clear();
        sceneFb->InitDone = true;

        static const float val = 0.1f;
        glm::vec4 background = { val,val,val,1 };
        guiFb->GetActiveTex().m_clear_color = { 1,0,0,1 };
        leftFb->GetActiveTex().m_clear_color = background;
        topFb->GetActiveTex().m_clear_color = background;
        sceneFb->GetActiveTex().m_clear_color = background;
        rightFb->GetActiveTex().m_clear_color = background;
        downFb->GetActiveTex().m_clear_color = background;

        Grid* grid = new Grid("grid");
        grid->Size.Set("base_rel", { 1, 1 }, MMode::Rel);
        grid->Init();
        grid->rows.Add(new Measure(20.0f, MMode::Abs));
        grid->rows.Add(new Measure(sp_size, MMode::Abs)); // Splitter
        grid->rows.Add(new Measure(0.6f, MMode::Rel));
        grid->rows.Add(new Measure(sp_size, MMode::Abs)); // Splitter
        grid->rows.Add(new Measure(0.4f, MMode::Star));
        grid->cols.Add(new Measure(0.2f, MMode::Star));
        grid->cols.Add(new Measure(sp_size, MMode::Abs)); // Splitter
        grid->cols.Add(new Measure(0.6f, MMode::Rel));
        grid->cols.Add(new Measure(sp_size, MMode::Abs)); // Splitter
        grid->cols.Add(new Measure(0.2f, MMode::Star));
        grid->AddChilds({ leftFb,topFb,sceneFb,rightFb,downFb });
        Splitter* sp1 = new Splitter(); sp1->Init();
        Splitter* sp2 = new Splitter(); sp2->Init();
        Splitter* sp3 = new Splitter(); sp3->ori = Orientation_S::Vertical; sp3->Init();
        Splitter* sp4 = new Splitter(); sp4->ori = Orientation_S::Vertical; sp4->Init();
        grid->AddChilds({ sp1, sp2, sp3, sp4 });

        grid->SetColumn(topFb, 0, 5);
        grid->SetRow(topFb, 0, 1);
        grid->SetColumn(leftFb, 0, 1);
        grid->SetRow(leftFb, 2, 1);
        grid->SetColumn(sceneFb, 2, 1);
        grid->SetRow(sceneFb, 2, 1);
        grid->SetColumn(rightFb, 4, 1);
        grid->SetRow(rightFb, 2, 3);
        grid->SetColumn(downFb, 0, 3);
        grid->SetRow(downFb, 4, 1);

        grid->SetColumn(sp1, 0, 5);//h
        grid->SetRow(sp1, 1, 1);
        grid->SetColumn(sp2, 0, 3);
        grid->SetRow(sp2, 3, 1);
        grid->SetColumn(sp3, 1, 1);//v
        grid->SetRow(sp3, 2, 1);
        grid->SetColumn(sp4, 3, 1);
        grid->SetRow(sp4, 2, 3);

        guiFb->AddChild(grid);

        Inspector = new StackPanel("Inspector");
        Inspector->Init();
        Hierarchy = new StackPanel("Hierarchy");
        Hierarchy->Init();
        rmtd::Inspector = Inspector;
        rmtd::Hierarchy = Hierarchy;
        StackPanel* spdown = new StackPanel("spdown");
        spdown->Init();
        spdown->AddChild(Inspector);
        Scrollable* rFb_sable = new Scrollable("rFb_sable", 0, 0, 0, 0);
        rFb_sable->bar_size = 10;
        rFb_sable->Size.Add({ 1,1 }, MMode::Rel);
        rFb_sable->Init();
        rFb_sable->SetProxy(spdown);
        rightFb->SetContent(rFb_sable);

        StackPanel* heir_sp = new StackPanel("heir_sp");
        heir_sp->Init();
        heir_sp->AddChild(Hierarchy);
        Scrollable* hier_sable = new Scrollable("hier_sable", 0, 0, 0, 0);
        hier_sable->bar_size = 10;
        hier_sable->Size.Add({ 1,1 }, MMode::Rel);
        hier_sable->Init();
        hier_sable->SetProxy(heir_sp);
        leftFb->SetContent(hier_sable);


        /*
        Movable* nothing = new Movable(100, 100, 100, 100);
        nothing->Init();
        guiFb->AddChild(nothing);
        nothing->SetTarget(leftFb);
        */
    }
    Button* MakeToolbarButton(std::string text) {
        Button* b1 = new Button(text, 0, 0, 30, 30);
        b1->Init();
        b1->Content->SetText(text);
        b1->Size.Add("toolbar_btn_rel", { 0,1 }, MMode::Rel);
        //b1->Aspect.apply = true;
        //b1->Margin = Margin_C(5);
        return b1;
    }
    void Run() {
        InitAll();
        setup_base_objects();

        scene1 = new Scene("scene1");
        scene1->Size.Set("base_rel", { 1, 1 }, MMode::Rel);
        scene1->Init();
        TabViewer* scene_tabv = new TabViewer();
        scene_tabv->Size.Set("scene_tabv_rel", { 1, 1 }, MMode::Rel);
        scene_tabv->Init();
        scene_tabv->AddTab(scene1);
        sceneFb->AddChild(scene_tabv);

        std::vector<params_t*> parsV;
        glm::vec2 origin = { 10,60 };
        glm::vec2 size = { 60, 30 };
        glm::vec2 move = { 70, 40 };
        float btn_size = 80;

        std::vector<Button*> envir_buttons;
        envir_buttons.push_back(MakeToolbarButton("N"));
        envir_buttons.push_back(MakeToolbarButton("G"));
        envir_buttons.push_back(MakeToolbarButton("S"));
        envir_buttons.push_back(MakeToolbarButton("I"));
        int envir_idx = 0;
        envir_buttons[envir_idx++]->e_MouseClick.Add(this, new_scene_callback, { scene_tabv });
        envir_buttons[envir_idx++]->e_MouseClick.Add(this, new_grid_tab_callback, { scene_tabv, this });
        envir_buttons[envir_idx++]->e_MouseClick.Add(this, scene_save_model_btn, { scene_tabv });

        std::vector<Button*> scene_buttons;
        scene_buttons.push_back(MakeToolbarButton("M"));
        scene_buttons.push_back(MakeToolbarButton("P"));
        scene_buttons.push_back(MakeToolbarButton("V"));
        scene_buttons.push_back(MakeToolbarButton("T"));
        scene_buttons.push_back(MakeToolbarButton("NN"));
        int scene_idx = 0;
        scene_buttons[scene_idx++]->e_MouseClick.Add(this, scene_state_buttons, { scene_tabv ,new int((int)SceneState::MOVING) });
        scene_buttons[scene_idx++]->e_MouseClick.Add(this, scene_state_buttons, { scene_tabv ,new int((int)SceneState::PICKING) });
        scene_buttons[scene_idx++]->e_MouseClick.Add(this, scene_state_buttons, { scene_tabv ,new int((int)SceneState::CREATE_CUBE) });
        scene_buttons[scene_idx++]->e_MouseClick.Add(this, scene_state_buttons, { scene_tabv ,new int((int)SceneState::CREATE_TRIANGLE) });
        scene_buttons[scene_idx++]->e_MouseClick.Add(this, scene_state_buttons, { scene_tabv ,new int((int)SceneState::NONE) });
        ToolBar* scene_toolbar = new ToolBar("scene_toolbar", 0, 0, 200, 0);
        scene_toolbar->Size.Set("toolbar_rel", { 0,1 }, MMode::Rel);
        scene_toolbar->Init();
        Button* b = new Button();
        scene_toolbar->AddChildsT(scene_buttons);
        ToolBar* environment_toolbar = new ToolBar("environment_toolbar", 0, 0, 200, 0);
        environment_toolbar->Size.Set("toolbar_rel", { 0,1 }, MMode::Rel);
        environment_toolbar->Init();
        environment_toolbar->AddChildsT(envir_buttons);
        ToolBar* main_toolbar = new ToolBar("main_toolbar");
        main_toolbar->Size.Set("toolbar_rel", { 1,1 }, MMode::Rel);
        main_toolbar->Init();
        main_toolbar->AddChild(environment_toolbar);
        main_toolbar->AddChild(scene_toolbar);
        topFb->AddChild(main_toolbar);


        StackPanel* dFB_spH = new StackPanel("dFB_spH");
        dFB_spH->Orientation = Orientation_S::Horizontal;
        dFB_spH->Init();
        downFb->SetContent(dFB_spH);
        StackPanel* sp1 = new StackPanel("sp1");
        sp1->Init();
        Scrollable* sa1 = new Scrollable("sa1", 0, 0, 180, 0);
        sa1->Init();
        sa1->SetProxy(sp1);
        downFb->Content->AddChild(sa1);

        //guiFb->ResizeTexs(guiFb->ActualSize);
        guiFb->SetLinked();

        e_InvokeQ.QInvoke();
        int framerate = 60;
        int frametime_ms = 1000 / framerate;
        while (!glfwWindowShouldClose(window->GLID)) {
            dbg::CountTillReady();
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            std::this_thread::sleep_for(chrono::milliseconds(frametime_ms - (int)(deltaTime * 1000)));
            lastFrame = static_cast<float>(glfwGetTime());
            deltaTime = 0.060f;

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Dispatcher::Perform();
            window->Perform(deltaTime);
            glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glfwSwapBuffers(window->GLID);
            glfwPollEvents();
        }
    }
    template<typename T = Framebuf>
    T* MakeFb(std::string name) {
        T* fb = new T(name);
        fb->m_shader = shaderPass;
        fb->Init();
        ToDelete.push_back(fb);
        return fb;
    }
    void DisplayAt(uint display_index, Framebuf* cd, int tex_index = 0) {
        auto dObj = rmtd::DFbs[display_index];
        DisplayCBParams* params = new DisplayCBParams{ dObj,tex_index };
        params_t pars;
        pars.push_back(params);
        cd->e_Display.Add(this, dpch_click_callback, { params });
    }
private:
};

int main() {
    Program p;
    p.Run();
}

static void ErrorCheck(std::string name) {
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("%s error, status: 0x%x\n", name.c_str(), Status); exit(1);
    }
}

static void display_callback(sender_ptr sender, EventArgs* e, params_t params) {
    DisplayCBParams dcbpars = *(DisplayCBParams*)params[0];
    Framebuf* fb_cd = (Framebuf*)dcbpars.dObj;
    params_t pars;
    pars.push_back(&dcbpars.tex_index);
    fb_cd->e_Display.Add(fb_cd, dpch_click_callback, pars);
}

static void dpch_click_callback(sender_ptr sender, EventArgs* e, params_t params) {
    DisplayCBParams* pars = (DisplayCBParams*)params[0];
    pars->dObj->CopyTexture((DrawObj2D*)sender, pars->tex_index);
}

static void label_cords_update(sender_ptr sender, EventArgs* e, params_t params) {
    Label* lb = (Label*)sender;
    Tuple1<WindowBase*>* tuple = (Tuple1<WindowBase*>*)params[0];
    auto& win = tuple->v1;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << "x:" << (int)win->cursor_pos.X << std::endl;
    ss << "y:" << (int)win->cursor_pos.Y;
    std::string s = ss.str();
    lb->SetText(s);
    lb->OnChange();
};

static void field_show_update(sender_ptr sender, EventArgs* e, params_t params) {
    Label* lb = (Label*)sender;
    auto tuple = (Tuple3<int, std::string, float*>*)params[0];
    std::stringstream ss;
    ss << std::fixed << std::setprecision(tuple->v1) << tuple->v2 << *tuple->v3 << std::endl;
    lb->SetText(ss.str());
    lb->OnChange();
};

static void cb_color_change(CheckBox* sender, CheckBoxEventArgs* e, params_t params) {
    Control* target = (Control*)params[0];
    if (e->Value)
        target->GetActiveTex().m_clear_color = { 0,0,0,1 };
    else
        target->GetActiveTex().m_clear_color = { 1,0,0,1 };

    target->OnChange();
};

static void new_grid_tab_callback(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    TabViewer* tv = (TabViewer*)params[0];
    /*
    Program* program = (Program*)params[1];

    Framebuf* leftFb = program->MakeFb<Movable>("leftFb");
    Framebuf* topFb = program->MakeFb<Framebuf>("topFb");
    Movable* sceneFb = program->MakeFb<Movable>("sceneFb");
    Framebuf* rightFb = program->MakeFb<Movable>("rightFb");
    Framebuf* downFb = program->MakeFb<Movable>("downFb");
    static const float val = 0.8f;
    glm::vec4 background = { val,val,val,1 };
    leftFb->GetActiveTex().m_clear_color = background;
    topFb->GetActiveTex().m_clear_color = background;
    sceneFb->GetActiveTex().m_clear_color = background;
    rightFb->GetActiveTex().m_clear_color = background;
    downFb->GetActiveTex().m_clear_color = background;

    Grid* grid = new Grid("grid");

    grid->Size.Set("base_rel", { 1, 1 }, MMode::Rel);
    grid->Init();
    grid->rows.Add(new Measure(20.0f, MMode::Abs));
    grid->cols.Add(new Measure(0.2f, MMode::Rel));
    grid->rows.Add(new Measure(0.6f, MMode::Rel));
    grid->rows.Add(new Measure(1.0f, MMode::Star));
    grid->cols.Add(new Measure(0.6f, MMode::Rel));
    grid->cols.Add(new Measure(0.2f, MMode::Rel));
    grid->AddChilds({ leftFb,topFb,sceneFb,rightFb,downFb });

    grid->SetColumn(topFb, 0, 3);
    grid->SetRow(topFb, 0, 1);
    grid->SetColumn(leftFb, 0, 1);
    grid->SetRow(leftFb, 1, 1);
    grid->SetColumn(sceneFb, 1, 1);
    grid->SetRow(sceneFb, 1, 1);
    grid->SetColumn(rightFb, 2, 1);
    grid->SetRow(rightFb, 1, 2);
    grid->SetColumn(downFb, 0, 2);
    grid->SetRow(downFb, 2, 1);

    sceneFb->AddChild(btn);
    tv->AddTab(grid);
    */

    /*
    Button* btn = new Button("btn");
    btn->m_clear_color_base = { 1,0,0,1 };
    btn->Init();
    btn->Content->SetText("r2g2");
    */

    Control* zone = new Control("zone");
    zone->Size.Set("base_rel", { 1, 1 }, MMode::Rel);
    float val = 0.9f;
    zone->m_clear_color_base = { val,val,val,1 };
    zone->Init();
    Dragger* btn = new Dragger(100, 100, 50, 50);
    float val2 = 0.f;
    btn->m_clear_color_base = { val2,val2,val2,1 };
    btn->Init();
    StackPanel* sp = new StackPanel(0, 0, 0, 50);
    sp->Orientation = Orientation_S::Horizontal;
    sp->Init();
    sp->HAlign = HorAlign::Right;
    CheckBox* c1 = new CheckBox(0, 0, 50, 50);
    c1->Init();
    c1->e_Toggle.Add(c1, cb_color_change, { zone });
    sp->AddChild(c1);
    zone->AddChild(btn);
    zone->AddChild(sp);
    tv->AddTab(zone);
}

