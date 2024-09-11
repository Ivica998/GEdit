#include <windows.h>
#include <algorithm>
#include "framebuf.h"
#include "resource_manager.h"
#include "dispatcher.h"
#include "utils.h"
#include <iostream>
#include <string.h>
#include <iostream>
#include <chrono>
#include <thread>

uint DrawObj2D::next = 1;
float* MeasureCol::dummy_total = new float;
float* MeasureCol::dummy_p_total = new float;
//uchar* DrawObj::pixels = new uchar[FB_WIDTH * FB_HEIGHT * 4];

void DrawObj::PickNext(float xNew, float yNew) {

}
PixelInfo DrawObj::BeginPickFunc(float xNew, float yNew) {
    //if (Pick.PickChanged)
    Point2D<float> size = SetPickFb();
    utl::ReadPixel(Pick.pInfo, Pick.FB->FBO, size.X * xNew, size.Y * (1 - yNew));

    auto& pInfo = Pick.pInfo;
    if (pInfo.Ident.ObjID == 0)
        Pick.PickedObj = nullptr;
    else {
        pInfo.Ident.GUID = Pick.Map[pInfo.Ident.ObjID - 1];
        DrawObj* child = (DrawObj*)rmtd::GetElement(pInfo.Ident.GUID);
        pInfo.Name = child->Name;
        Pick.PickedObj = child;
    }
    if (rmtd::MouseRay.Hover) {
        auto& MRInfo = rmtd::MouseRay;
        MRInfo.TraceHover(this);
        this->OnChange();
        if(Pick.PickedObj == nullptr)
            MRInfo.CutTrace();
        //e_MouseHover.Invoke();
    }

    PickFunc(xNew, yNew);
    return Pick.pInfo;
}
void DrawObj::StrongOnChange() {
    States.Changed = true;
    if (Parent != nullptr)
        Parent->StrongOnChange();
}
void DrawObj::OnChange() {
    if (States.Changed)
        return;
    States.Changed = true;
    if (Parent != nullptr)
        Parent->OnChange();
}
void DrawObj::OnBoundsChange() {
    e_BoundsChanged.event_args->handled = false;
    e_BoundsChanged.Invoke();
}
void DrawObj::OnMeasureChange() {
    e_MeasureChanged.event_args->handled = false;
    e_MeasureChanged.Invoke();
}
void DrawObj::OnHover() {
    e_Hover.event_args->handled = false;
    e_Hover.Invoke();
    if (!e_Hover.event_args->handled)
        Parent->OnHover();
}
void DrawObj2D::SetWriteTex(int texIndex) {
    m_shader->SetInt("outatch", texIndex);
}
bool change_callback(DrawObj2D* dObj) {
    if (dObj->States.Changed == true)
        return false;
    return dObj->States.Changed = true;
}
void DrawObj2D::SetVisibility(VisibilityE vis) {
        if (Visibility == vis)
            return;
        bool need = Visibility == VisibilityE::Collapsed || vis == VisibilityE::Collapsed;
        Visibility = vis;
        if (need) {
            OnMeasureChange();
            //OnBoundsChange();
        }
}
std::vector<DrawObj2D*> DrawObj2D::UCChilds() {
    std::vector<DrawObj2D*> ret;
    for (auto child : Childs) {
        if (!child->IsCollapsed())
            ret.push_back(child);
    }
    return ret;
}
void DrawObj2D::ResizeTexs(Point2D<float> size) {
    if (TextureResizeDisabled) return;
    int n = sizeof(Texs) / sizeof(Texs[0]);
    if (TexSize != size) { 
        TexSize = size;
        for (auto iter = Texs.begin(); iter != Texs.end(); ++iter) {
            (*iter)->Resize(size);}
        Texture2D::ResizeRBODepth(size, FBO, RBO_Depth);
    }
}
void DrawObj2D::ComputeMeasures() {
    Size.Compute();
    Pos.Compute();
}
void DrawObj2D::ComputeBounds() // AS,AP utd
{
    if (GUID == 223)
        int x = 5;
    Pos.Set("stretch", { 0,0 });
    Size.Set("stretch", { 0,0 });
    bool ChangeSize = false;
    Bounds bnd = { 0,0,0,0 };
    auto pIS =
        Parent->IntendedSize();
    //Parent->LogicalSize.derefC();
    auto ISize =
        //IntendedSize();
        ActualSize - Size["stretch"];
    auto IPos = ActualPos;
    utl::ApplyAspect(IPos,ISize,this, pIS);

    bnd.x1 = pIS.X != 0 ? IPos.X / pIS.X : 0;
    bnd.x2 = pIS.X != 0 ? bnd.x1 + ISize.X / pIS.X : 0;
    bnd.y1 = pIS.Y != 0 ? IPos.Y / pIS.Y : 0;
    bnd.y2 = pIS.Y != 0 ? bnd.y1 + ISize.Y / pIS.Y : 0;
    utl::Align(bnd, this);
    /*
    utl::Limit(bnd.x1, 0, 1);
    utl::Limit(bnd.x2, 0, 1);
    utl::Limit(bnd.y1, 0, 1);
    utl::Limit(bnd.y2, 0, 1);
    */
    utl::m01To11(bnd);


    if (GUID == 140) {
        if (
            bnd.x1 > 1 ||
            bnd.x2 > 1 ||
            bnd.y1 > 1 ||
            bnd.y2 > 1
            )                 {
            int x = 5;
        }
    }
    m_LastBound = m_Bounds;
    m_Bounds = bnd;

    AdjustSizeWith("stretch");
    AdjustPosWith("stretch");

    Bounds change = Margin.GetRelativeChange(ActualSize);
    m_Bounds += change;
    m_LastBound += change;
    if (m_Bounds != m_LastBound)
        OnBoundsChange();
        //States.BoundsChanged = true;

    VAO = rmtd::GetVAO(bnd);
}
void DrawObj2D::FixBounds() {
    if (!Linked)
        return;
    States.Changed = true;
    ComputeMeasures();
    //Arrange();
    for (auto child : Childs) {
        child->FixBounds();
    }
}
Point2D<float> DrawObj2D::IntendedSize() {
    auto ret = ActualSize;
    if (HAlign == HorAlign::Stretch || VAlign == VerAlign::Stretch) {
        auto pIS = Parent->IntendedSize();
        if (HAlign == HorAlign::Stretch)
            ret.X = pIS.X;
        if (VAlign == VerAlign::Stretch)
            ret.Y = pIS.Y;
    }
    return ret;
}
void DrawObj2D::ComputeSize() {
    ActualSize = Size.Compute() - Size["stretch"];
}
void DrawObj2D::AdjustSizeWith(std::string key) // when AS n bnd r utd
{
    auto ratio = m_Bounds.Size();
    Size.Set(key, Parent->LogicalSize.derefC<float>() * ratio - (ActualSize - Size[key]));
}
void DrawObj2D::ComputePos() {
    ActualPos = Pos.Compute() - Pos["stretch"];
}
void DrawObj2D::AdjustPosWith(std::string key) {
    auto ratio = m_Bounds.Pos();
    Pos.Set(key, Parent->LogicalSize.derefC<float>() * ratio - (ActualPos - Pos[key]));
}
Shader* DrawObj2D::GetPassShader() {
    return ResourceManager::GetShader("pass_tex");
}

Framebuf::~Framebuf() {
}
void DrawObj2D::InformChange(InformCB callback) {
    DrawObj2D* next = Parent;
    if (next && callback(next)) {
        next->InformChange(callback);
    }
}
void DrawObj2D::RemoveSelf() {
    if (Parent != nullptr) {
        Parent->RemoveChild(this);
    }
}
void Framebuf::Init() {
    Pick.FB = rmtd::PickFb;
    glGenFramebuffers(1, &this->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    Pos.Add("base_abs", this->ActualPos);
    Size.Add("base_abs", this->ActualSize);
    TexSize = { -1,-1 };
    unsigned int attachments[5];
    for (int i = 0; i < tex_num; i++) {
        auto tmp = new Texture2D();
        tmp->Generate({ 1,1 }, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tmp->ID, 0);
        attachments[i] = GL_COLOR_ATTACHMENT0 + i;
        this->ErrorCheck("FBO");
        tmp->m_clear_color = m_clear_color_base;
        glActiveTexture(GL_TEXTURE0 + i);
        tmp->Bind();
        Texs.push_back(tmp);
    }
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glGenRenderbuffers(1, &RBO_Depth);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO_Depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO_Depth);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    VAO = rmtd::GetVAO();
}
void Framebuf::Update(float dt) {}
void Framebuf::ProcessInput(float dt, InputArgs_S& input) {}
void Framebuf::ClearColor(int index) {
    auto& tmp = Texs[index]->m_clear_color;
    glClearColor(tmp.r, tmp.g, tmp.b, tmp.a);
}
void Framebuf::AddChilds(std::vector<DrawObj2D*> dObjs){
    for (auto item : dObjs)
        AddChild(item);
}
void Framebuf::InsertChildAt(int index, DrawObj2D* dObj) {
    utils::InsertInVec(Childs, index, dObj);
    DrawObj2D* newItem = dObj;
    if (Linked) {
        LinkChild(newItem);
        States.Changed = false;
        OnChange();
    }
}
DrawObj2D* Framebuf::AddChild(DrawObj2D* dObj){
    DrawObj2D* newItem = dObj;
    Childs.push_back(newItem);
    if (Linked) {
        LinkChild(newItem);
        States.Changed = false;
        OnChange();
        StrongOnChange();}
    return newItem;
}
void Framebuf::RemoveChild(DrawObj2D* dObj) {
    dObj->Unlink();
    utils::ErraseFromVec(Childs, dObj);
}
void Framebuf::ClearChilds() {
    for (auto iter = Childs.begin(); iter != Childs.end(); iter++) {
        DrawObj2D* cd = *iter;
        UnlinkChild(cd);
        cd->Unlink();}
    Childs.clear();
}
void Framebuf::BeginRender(bool clear) {
    glViewport(0, 0, (GLsizei)LogicalSize.X[0], (GLsizei)LogicalSize.Y[0]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->FBO);
    if (clear) {
        ClearColor();
        glClear(GL_COLOR_BUFFER_BIT);}
}
void Framebuf::EndRender() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
void DrawObj2D::Refresh() {
    FixLayout();
    for (auto& child : Childs) {
        child->Refresh();
    } States.Changed = true;
}
void DrawObj2D::FixLayout() {
    Measure();
    Arrange();
}
void DrawObj2D::Measure() {
    ComputeMeasures();
}
void DrawObj2D::Arrange() {
    OnChange();
    ComputeBounds();
    ResizeTexs(ActualSize);
}
void DrawObj2D::LastArrange() {}
void Framebuf::BeginLink() {
    Linked = true;
    States.Changed = true;
}
void Framebuf::SetLinked() {
    if (!Linked) BeginLink();
    Arrange();
    for (auto& child : Childs) {
        LinkChild(child);
    }
}
void Framebuf::LinkChild(DrawObj2D* child) {
    child->Link(this);
    child->SetLinked();
}
void Framebuf::Link(Framebuf* parent) {
    Parent = parent;
    Pos.Link(parent->LogicalSize, ActualPos);
    Size.Link(parent->LogicalSize, ActualSize);
}
void Framebuf::UnlinkChild(DrawObj2D* child) {}
void Framebuf::Unlink() {
    Parent = nullptr; Linked = false;
    for (auto child : Childs) {
        UnlinkChild(child);
        child->Unlink();
    }
}
void Framebuf::SetLinked2(Framebuf* parent) {
    if (!Linked) Link(parent);
    SetLinked2();
}
void Framebuf::SetLinked2() { // kraj procesa
    Arrange();
    for (auto& child : Childs) {
        if (!child->Linked) {
            child->Link(this);
        }
        child->SetLinked2();
    }
}
void Framebuf::Link2(Framebuf* parent) {
    if (Linked == true)
        int x = 5;
    Linked = true;
    States.Changed = true;
    Parent = parent;
    Pos.Link(parent->LogicalSize, ActualPos);
    Size.Link(parent->LogicalSize, ActualSize);
}
void Framebuf::BeginPrepare() {
    Prepare();
    e_Display.QInvoke();
}
void Framebuf::Prepare() {
    auto old = dpch::s_TraceInfo;
    dpch::TraceIn(this);
    DrawObj2D* next;
    for (auto& ch : Childs) {
        next = ch;
        if (next->States.Changed) {
            next->States.Changed = false;
            next->BeginPrepare();
        }
    }
    BeginRender();
    for (auto& ch : Childs) {
        next = ch;
        next->Render();
    }
    EndRender();
    dpch::TraceOut();

}
void Framebuf::Render() {
    if (Visibility == VisibilityE::Visible) {
        this->m_shader->Use();
        glActiveTexture(GL_TEXTURE0);
        this->Texs[activeTex]->Bind();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
}
uint Framebuf::TexRender(DrawObj2D& dobj) {
    dobj.m_shader->Use();
    glActiveTexture(GL_TEXTURE0);
    dobj.Texs[dobj.activeTex]->Bind();
    int vao = rmtd::GetVAO(Bounds::s_default);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    return 0;
}
uint Framebuf::RenderB(const Bounds& bn) {
    uint oldVAO = VAO;
    VAO = rmtd::GetVAO(bn);
    Render();
    VAO = oldVAO;
    return 0;
}
Point2D<float> Framebuf::SetPickFb() {
    Pick.FB->BeginRender();
    Point2D<float> size = { std::min(Pick.FB->ActualSize.X, ActualSize.X), std::min(Pick.FB->ActualSize.Y,ActualSize.Y) };
    glViewport(0, 0, (GLsizei)size.X, (GLsizei)size.Y);
    Pick.Map.clear();
    int id = 0;
    int r, g, b;
    for (auto& ch : UCChilds()) {
        if (!ch->Pick.Pickable)
            continue;
        Pick.Map.push_back(ch->GUID);
        id++;
        r = (id & 0x000000FF) >> 0;
        g = (id & 0x0000FF00) >> 8;
        b = (id & 0x00FF0000) >> 16;
        Pick.FB->m_shader->SetVec4f("PickingColor", r / 255.0f, g / 255.0f, b / 255.0f, 1);
        Pick.FB->RenderB(ch->m_Bounds);
    }
    Pick.FB->EndRender();
    return size;
}
bool Framebuf::PickAt(float x, float y) {
    dpch::TraceIn(this);
    float xNew, yNew;
    std::tie(xNew, yNew) = utl::RemapPoint(x, y, m_Bounds);
    PixelInfo pInfo = BeginPickFunc(xNew, yNew);
    dpch::TraceOut();
    return true;
}
PixelInfo Framebuf::PickFunc(float xNew, float yNew) {
    auto& pInfo = Pick.pInfo;
    auto& states = GetStates();
    DrawObj* child = Pick.PickedObj;
    if (child == nullptr) return pInfo;
    if (rmtd::MouseRay.Hover) {
        //e_MouseHover.Invoke();
        rmtd::MouseRay.trace.push_back(this);
        if (child != nullptr)
            child->PickAt(xNew, yNew);
    } else {
        if (child->Pick.Pickable)
            child->PickAt(xNew, yNew);
    }
    return pInfo;
}
bool Framebuf::TryChangeActivity(bool value) {
    States.Active = value;
    return true;
}
void Framebuf::CopyTexture(DrawObj2D* obj, int index) {
    GLint drawFboId = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
    BeginRender();
    int old = obj->activeTex;
    obj->activeTex = index;
    Framebuf::TexRender(*obj);
    obj->activeTex = old;
    OnChange();
    States.Changed = false;
    glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
}
void Framebuf::ErrorCheck(std::string name) {
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("%s error, status: 0x%x\n", name.c_str(), Status); exit(1);
    }
}

PickFramebuf::PickFramebuf(Shader* shader)
    : Framebuf("pickFB") {
    m_shader = shader;
    Init();
}
void PickFramebuf::Init() {
    ActualSize = { 800, 600 };

    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto tmp = new Texture2D();
    tmp->Setup(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, SKIP, SKIP, GL_LINEAR, GL_LINEAR);
    tmp->Generate(LogicalSize.derefC(), NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmp->ID, 0);
    Texs.push_back(tmp);
    unsigned int depthTexture = Texture2D::GenDepthTex(ActualSize);
    ErrorCheck("FBO");
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    VAO = rmtd::GetVAO();
}
void PickFramebuf::BeginRender(bool clear) {
    glViewport(0, 0, (GLsizei)LogicalSize.X[0], (GLsizei)LogicalSize.Y[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    ClearColor();
    if (clear)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_shader->Use();
}
void PickFramebuf::EndRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void PickFramebuf::Resize(Point2D<float> size) {
    ActualSize = size;
    ResizeTexs(size);
}
void PickFramebuf::Render() {
    this->Texs[activeTex]->Bind();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
uint PickFramebuf::RenderB(const Bounds& bn) {
    return Framebuf::RenderB(bn);
}
Shader* PickFramebuf::GetPickShader() {
    return rmtd::GetShader("picking");
}
Shader* PickFramebuf3D::GetPickShader() {
    return rmtd::GetShader("picking_3d");
}
void PickFramebuf3D::Init() {

    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto tmp = new Texture2D();
    tmp->Setup(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, SKIP, SKIP, GL_NEAREST, GL_NEAREST);
    tmp->Generate(LogicalSize.derefC(), NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmp->ID, 0);
    Texs.push_back(tmp);
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, FB_WIDTH, FB_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    ErrorCheck("FBO");
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    VAO = rmtd::GetVAO();
}

void CharFramebuf::Setup(FT_Face face) {
    ActualSize.X = (unit_t)std::max(face->glyph->bitmap.width, 1u);
    ActualSize.Y = (unit_t)std::max(face->glyph->bitmap.rows, 1u);
    Bearing.X = face->glyph->bitmap_left;
    Bearing.Y = face->glyph->bitmap_top;
    Advance.X = face->glyph->advance.x;
    Advance.Y = face->glyph->advance.y;
}
void CharFramebuf::Setup(float w, float h, int bX, int bY, int aX, int aY) {
    ActualSize.X = w;
    ActualSize.Y = h;
    Bearing.X = bX;
    Bearing.Y = bY;
    Advance.X = aX;
    Advance.Y = aY;
}
void CharFramebuf::Init() {
    glGenFramebuffers(1, &this->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    auto tmp = new Texture2D();
    Texs.push_back(tmp);
    tmp->Setup(GL_RED, GL_RED, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
        /*
        GL_NEAREST, GL_NEAREST);
        */
        GL_LINEAR, GL_LINEAR);
    tmp->Generate(ActualSize, buffer);
    tmp->Stored = true;
    this->m_shader = rmtd::GetShader("text3d");
    this->m_shader->Use();
    this->m_shader->SetInt("text", 0);

    //tmp.Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmp->ID, 0);
    this->ErrorCheck("FBO");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    VAO = rmtd::GetVAO();

    tmp->m_clear_color = { 0,0,0,1 };
    Pick.Pickable = false;
    m_shader->SetFloat("width", ActualSize.X);
}
void CharFramebuf::BeginRender(bool clear) {
    //glViewport(0, 0, Width, Height);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    ClearColor();
    if (clear)
        glClear(GL_COLOR_BUFFER_BIT);
}
void CharFramebuf::EndRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void CharFramebuf::Render() {
    this->m_shader->Use();
    this->m_shader->SetVec4f("textColor", color);
    this->m_shader->SetVec4f("bgColor", m_clear_color_base);
    //this->m_shader->SetMat4("projection", glm::ortho(0.0f, static_cast<float>(ActualSize.X), static_cast<float>(ActualSize.Y), 0.0f));
    this->m_shader->SetMat4("projection", glm::ortho(1.0f, -1.0f, -1.0f, 1.0f));
    //this->m_shader->SetInt("text", 0);
    glActiveTexture(GL_TEXTURE0);
    this->Texs[activeTex]->Bind();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
uint CharFramebuf::RenderB(const Bounds& bn) {
    return Framebuf::RenderB(bn);
}

void Originbuf::Init() {
    Framebuf::Init();
    Size.Meas.X.total = &ActualSize.X;
    Size.Meas.Y.total = &ActualSize.Y;
}
