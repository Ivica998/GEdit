#include "window.h"
#include "resource_manager.h"
#include "dispatcher.h"
#include "Debug.h"


void WindowBase::Render() {
    if (this->State == ComponentState::Enabled) {
        if (true) {
            dpch::s_TraceInfo.Width = Buffer.X;
            dpch::s_TraceInfo.Height = Buffer.Y;
            GUI->Prepare();
            Changed = false;
        }
        //Point2D<float> ActualSize = Size * m_size_scale;
        glViewport(0, 0, (int)ActualSize.X, (int)ActualSize.Y);
        GUI->Render();
    }
}
void WindowBase::Perform(float dt) {
    if (GUI != nullptr) {
        this->ProcessInput(dt);
        this->Update(dt);
        this->DoCollisions();
        this->Render();
    }
}
void WindowBase::Update(float dt) {
    static const float amount = 0.3f;
    if (update_cursor_pos) {
        double retX, retY;
        glfwGetCursorPos(GLID, &retX, &retY);
        cursor_pos_change = cursor_pos - last_cursor_pos;
        last_cursor_pos = cursor_pos;
        cursor_pos.X = (float)retX;
        cursor_pos.Y = (float)retY;
    } else update_cursor_pos = true;
    if (this->State == ComponentState::Enabled) {
        MouseRM mmode = rmtd::MouseRay.mode;
        m_lmb.t_from_press += dt;
        m_rmb.t_from_press += dt;
        switch (mmode) {
            case MouseRM::LMBPress: if (m_lmb.t_from_press > amount) rmtd::MouseRay.mode = MouseRM::LMBBeginHold; break;
            case MouseRM::LMBBeginHold: rmtd::MouseRay.mode = MouseRM::LMBHold; break;
            case MouseRM::RMBPress: if (m_rmb.t_from_press > amount) rmtd::MouseRay.mode = MouseRM::RMBBeginHold; break;
            case MouseRM::RMBBeginHold: rmtd::MouseRay.mode = MouseRM::RMBHold; break;
            case MouseRM::LMBRelease: if (m_lmb.t_from_press > amount) rmtd::MouseRay.mode = MouseRM::None;
            case MouseRM::RMBRelease: if (m_rmb.t_from_press > amount) rmtd::MouseRay.mode = MouseRM::None;
            default: break;
        }
        GUI->Update(dt);
    }
}
void WindowBase::DoCollisions() {
    if (this->State == ComponentState::Enabled) {
        //GUI->Value->DoCollisions();
    }
}
Point2D<float> WindowBase::GetNDCCursor() {
    return cursor_pos / ActualSize;
}
Point2D<float> WindowBase::GetNDCCursorChange() {
    return cursor_pos_change / ActualSize;
}
void WindowBase::ProcessInput(float dt) {

    for (auto& active : ActiveObjs) {
        active->ProcessInput(dt, InputArgs);
    }

    if (this->State == ComponentState::Enabled) {

    }

}
void WindowBase::OnMouseMove() {
    e_mouse_move.event_args->cursor_pos_value = cursor_pos;
    e_mouse_move.event_args->cursor_pos_change = cursor_pos_change;
    e_mouse_move.event_args->rel_pos_val = cursor_pos / ActualSize;
    e_mouse_move.event_args->rel_pos_cng = cursor_pos_change / ActualSize;
    e_mouse_move.event_args->handled = false;
    e_mouse_move.Invoke();
}

void WindowBase::PassiveMouseCB(float x, float y) {
    cursor_pos_change = cursor_pos - last_cursor_pos;
    last_cursor_pos = cursor_pos;
    cursor_pos.X = (float)x;
    cursor_pos.Y = (float)y;
    update_cursor_pos = false;
    OnMouseMove();

    rmtd::MouseRay.BeginTrace();
    GUI->PickAt(x / ActualSize.X, y / ActualSize.Y);
    rmtd::MouseRay.Hover = false;
}
void WindowBase::MouseCB(int button, int action, int x, int y) {
    bool press = false;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        m_lmb.WasPressed = m_lmb.IsPressed;
        m_lmb.IsPressed = (action == GLFW_PRESS);
        m_lmb.x = x;m_lmb.y = y;m_lmb.t_from_press = 0;
        if (m_lmb.IsPressed && !m_lmb.WasPressed) {
            rmtd::MouseRay.mode = MouseRM::LMBPress;
            press = true;}
        if (!m_lmb.IsPressed && m_lmb.WasPressed) {
            rmtd::MouseRay.mode = MouseRM::LMBRelease;
            dpch::InvokeMouseReleaseQ();}} 
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_rmb.WasPressed = m_rmb.IsPressed;
        m_rmb.IsPressed = (action == GLFW_PRESS);
        m_rmb.x = x; m_rmb.y = y; m_rmb.t_from_press = 0;
        if (m_rmb.IsPressed && !m_rmb.WasPressed) {
            rmtd::MouseRay.mode = MouseRM::RMBPress;
            press = true;}
        if (!m_rmb.IsPressed && m_rmb.WasPressed) {
            rmtd::MouseRay.mode = MouseRM::RMBRelease;
            dpch::InvokeMouseReleaseQ();}}
    if (press) {
        utils::ErraseFromSetIf<DrawObj2D*>(ActiveObjs, [](DrawObj2D* item) {return item->TryChangeActivity(false);});
        dpch::s_TraceInfo.Clear();
        GUI->PickAt((float)x / ActualSize.X, (float)y / ActualSize.Y);
        dpch::s_TraceInfo.Clear();}
}
static void mouse_button_callback(GLFWwindow* window, int Button, int Action, int Mode) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    rmtd::Windows[window]->MouseCB(Button, Action, (int)x, (int)y);
}
static void cursor_pos_callback(GLFWwindow* window, double x, double y) {
    WindowBase* win = rmtd::Windows[window];
    win->PassiveMouseCB((float)x, (float)y);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    //if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
       // glfwSetWindowShouldClose(window, true);
    if (key >= 0 && key < 1024) {
        WindowBase* win = rmtd::Windows[window];
        win->InputArgs.Mode = mode;
        if (action == GLFW_PRESS) {
            win->InputArgs.Keys[key] = true;
            win->InputArgs.Hold[key] = 0.5;
        } else if (action == GLFW_RELEASE) {
            win->InputArgs.Hold[key] = 0;
            win->InputArgs.Keys[key] = false;
            win->InputArgs.KeysProcessed[key] = false;
        }
    }
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    WindowBase* win = rmtd::Windows[window];
    win->m_size_scale.X = 1;
    win->m_size_scale.Y = 1;
    if (width != 0 && height != 0) {
        win->ActualSize = { (float)width,(float)height };
        win->GUI->Size.Set("base_abs", win->ActualSize, MMode::Abs);
        rmtd::PickFb->Resize(win->GUI->ActualSize);
        /*
        win->GUI->Unlink();
        win->GUI->SetLinked();
        */
        win->GUI->Refresh();
    }
    glViewport(0, 0, width, height);
}
int WindowBase::Init(bool skip, std::string name, GLFWwindow* parentID) {
    Buffer = Size;
    GLID = glfwCreateWindow((int)Size.X, (int)Size.Y, name.c_str(), nullptr, parentID);
    rmtd::Windows[GLID] = this; rmtd::CC(GLID);
    if (GLID == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate(); return -1;}
    glfwSetWindowPos(GLID, (int)this->Pos.X, (int)this->Pos.Y);
    glfwSetMouseButtonCallback(GLID, mouse_button_callback);
    glfwSetCursorPosCallback(GLID, cursor_pos_callback);
    glfwSetKeyCallback(GLID, key_callback);
    glfwSetFramebufferSizeCallback(GLID, framebuffer_size_callback);
    if (!skip) {glViewport(0, 0, (int)Size.X, (int)Size.Y);}
    return 0;
}
MainWindow::MainWindow(std::string name) : WindowBase(true, name) {
    Init();
}

int MainWindow::Init() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, (int)Size.X, (int)Size.Y);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return 0;
}
