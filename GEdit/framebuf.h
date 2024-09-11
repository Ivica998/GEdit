#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <map>
#include <unordered_set>
#include "shader.h"
#include "texture.h"
#include "utils.h"
#include "dispatcher.h"
#include "resource_manager.h"
/*
*/
#include <ft2build.h>
#include FT_FREETYPE_H

class WindowBase;
class DrawObj2D;

enum class HorAlign : int {
    None,
    Right,
    Left,
    Center,
    Stretch
};

enum class VerAlign : int {
    None,
    Bottom,
    Top,
    Center,
    Stretch
};

struct DrawObjStates {
    bool Changed = false;
    bool SizeChanged = false;
    bool DisplayReady = true;
    bool BoundsChanged = true;
    bool Selected = false;
    bool SelChanged = false;
    bool Enabled = true;
    bool Active = false;
    bool Hovered = false;
    bool RightMBDown = false;
    bool LeftMBDown = false;
    bool SetChanged(bool val) {
        if (Changed == val)
            return false;
        Changed = val;
        return true;
    }
};
struct CloneData {
    bool Cloning;
    DrawObj2D* Clone;
};

struct Margin_C {
    float left = 0;
    float top = 0;
    float right = 0;
    float bottom = 0;
    Margin_C(float l , float t , float r , float b ) : left(l), top(t), right(r), bottom(b) {}
    Margin_C(float val = 0) : Margin_C(val, val, val, val) {}
    Bounds GetRelativeChange(Point2D<float> size) {
        return { 
            -2 * left / size.X,
            -2 * top / size.Y,
            2 * right / size.X,
            2 * bottom / size.Y
        };
    }
};

class MeasureCol {
public:
    float star = 0, abs = 0, rel = 0, starVal = 0;
    float* total = dummy_total;
    float* p_total = dummy_p_total;
    std::vector<Measure*> measures;
    std::map<std::string, Measure*> saved;
    ~MeasureCol() { for (auto& item : saved) delete item.second; }
    void Link(float* p_t, float* c_t) { p_total = p_t; total = c_t; Compute(); }
    float ComputeSingle(Measure* item) const {
        static float retVal = 0;
        switch (item->mode) {
        case MMode::Abs: retVal = item->val; break;
        case MMode::Rel: retVal = item->val * *p_total; break;
        case MMode::Star: retVal = item->val * starVal; break;
        default: break;} return retVal;}
    float TotalOf(MMode mode) {
        float val = 0;
        for (auto item : measures) {
            if (item->mode == mode) val += item->val;}
        return val;}
    void Disable(MMode mode, std::string key) {
        float val = -TotalOf(mode);
        Add(key, new Measure(val, mode));}
    float Compute() const {
        float tmp = 0;
        for (Measure* item : measures){
            float ret = ComputeSingle(item); 
            tmp += ret;}
        return *total = tmp;}
    void Add(std::string key, Measure* item) { Add(item); saved[key] = item; }
    void Add(Measure* item) {
        measures.push_back(item);
        switch (item->mode) {
        case MMode::Abs: abs++; break;
        case MMode::Rel: rel++; break;
        case MMode::Star: star++; break;
        default: break;}
        *total += ComputeSingle(item);}
    void Resize(float val) {
        float tmp = 0;
        for (Measure* item : measures) {
            item->val *= val;
            float ret = ComputeSingle(item);
            tmp += ret;}
        *total = tmp;}
    void Set(std::string key, float val, MMode mode = MMode::Abs) { Change(0, key, val, mode); }
    void Add(std::string key, float val, MMode mode = MMode::Abs) { Change(1, key, val, mode); }
    void Mul(std::string key, float val, MMode mode = MMode::Abs) { Change(2, key, val, mode); }
    void Div(std::string key, float val, MMode mode = MMode::Abs) { Change(3, key, val, mode); }
    Measure* Change(int code, std::string& key, float val, MMode mode = MMode::Abs) {
        if (saved.count(key)) {
            auto item = saved[key];
            *total -= ComputeSingle(item);
            switch (code) {
            case 0: item->val = val;break;
            case 1: item->val += val;break;
            case 2: item->val *= val;break;
            case 3: item->val /= val;break;
            default: break;}
            *total += ComputeSingle(item);}
        else {
            auto item = new Measure{ val,mode };
            Add(key, item);}
        return saved[key];}
    void MyClear() { saved.clear(); measures.clear(); }//for (auto& item : saved) delete item.second; 
    bool IsRel() { return star + rel; }
private:
    static float* dummy_total;
    static float* dummy_p_total;
};
template<typename T>inline T& operator+=(T lhs, const MeasureCol& rhs) { return lhs += rhs.Compute(); }
template<typename T>inline T& operator-=(T lhs, const MeasureCol& rhs) { return lhs -= rhs.Compute(); }
template<typename T>inline T& operator*=(T lhs, const MeasureCol& rhs) { return lhs *= rhs.Compute(); }
template<typename T>inline T& operator/=(T lhs, const MeasureCol& rhs) { return lhs /= rhs.Compute(); }
template<typename T>inline float operator+(const T lhs, const MeasureCol& rhs) { return lhs + rhs.Compute(); }
template<typename T>inline float operator-(const T lhs, const MeasureCol& rhs) { return lhs - rhs.Compute(); }
template<typename T>inline float operator*(const T lhs, const MeasureCol& rhs) { return lhs * rhs.Compute(); }
template<typename T>inline float operator/(const T lhs, const MeasureCol& rhs) { return lhs / rhs.Compute(); }
template<typename T>inline float operator+(const MeasureCol& lhs, const T rhs) { return lhs.Compute() + rhs; }
template<typename T>inline float operator-(const MeasureCol& lhs, const T rhs) { return lhs.Compute() - rhs; }
template<typename T>inline float operator*(const MeasureCol& lhs, const T rhs) { return lhs.Compute() * rhs; }
template<typename T>inline float operator/(const MeasureCol& lhs, const T rhs) { return lhs.Compute() / rhs; }
inline float operator+(const MeasureCol& lhs, const MeasureCol& rhs) { return lhs.Compute() + rhs.Compute(); }
inline float operator-(const MeasureCol& lhs, const MeasureCol& rhs) { return lhs.Compute() - rhs.Compute(); }
inline float operator*(const MeasureCol& lhs, const MeasureCol& rhs) { return lhs.Compute() * rhs.Compute(); }
inline float operator/(const MeasureCol& lhs, const MeasureCol& rhs) { return lhs.Compute() / rhs.Compute(); }

class MeasureCol2D {
public:
    Point2D<MeasureCol> Meas;
    Point2D<float> Compute() { return { Meas.X.Compute(), Meas.Y.Compute() }; }
    bool Compute(Point2D<float>& target) { static auto old_target = target; return old_target != (target = Compute()); }
    void Link(Point2D<float*> p, Point2D<float>& c) { Meas.X.Link(p.X, &c.X); Meas.Y.Link(p.Y, &c.Y); }
    void Add(Point2D<float> val, MMode mode = MMode::Abs){ 
        Measure* Xpart = new Measure{ val.X, mode };
        Measure* Ypart = new Measure{ val.Y, mode };
       // saved[key] = { Xpart,Ypart };
        Meas.X.Add(Xpart); 
        Meas.Y.Add(Ypart); }
    void MergeInto(std::string dest, std::string src,int modif) {
        if (Meas.X.saved.count(dest) == 0 || Meas.X.saved.count(src) == 0) return;
        Meas.X.saved[dest]->val += Meas.X.saved[src]->val * modif;
        Meas.Y.saved[dest]->val += Meas.Y.saved[src]->val * modif;
        Meas.X.saved[src]->val = 0;
        Meas.Y.saved[src]->val = 0;}
    void Disable(MMode mode, std::string key) {
        Meas.X.Disable(mode, key);
        Meas.Y.Disable(mode, key);}
    void Save(std::string key, MMode mode){
        auto Xpart = new Measure{ {0},mode };
        auto Ypart = new Measure{ {0},mode };
        saved[key] = { Xpart,Ypart };
        Meas.X.Add(Xpart);
        Meas.Y.Add(Ypart);}
    void Resize(Point2D<float> val) {
        Meas.X.Resize(val.X);
        Meas.Y.Resize(val.Y);}
    void Set(std::string key, Point2D<float> val, MMode mode = MMode::Abs) { Change(0, key, val, mode); }
    void Add(std::string key, Point2D<float> val, MMode mode = MMode::Abs) { Change(1, key, val, mode); }
    void Mul(std::string key, Point2D<float> val, MMode mode = MMode::Abs) { Change(2, key, val, mode); }
    void Div(std::string key, Point2D<float> val, MMode mode = MMode::Abs) { Change(3, key, val, mode); }
    void Clear() { Meas.X.MyClear(); Meas.Y.MyClear(); saved.clear(); Compute(); }
    Point2D<Measure> Get(std::string key) { return { *(Meas.X.saved[key]), *(Meas.Y.saved[key]) }; }
    Point2D<Measure> operator[](const std::string& key) {
            if (saved.count(key)) return {*(Meas.X.saved[key]), * (Meas.Y.saved[key]) };
            else return {};}
private:
    void Change(int code, std::string& key, Point2D<float>& val, MMode mode = MMode::Abs) {
        if (!saved.count(key)) Save(key, mode);
        auto x = Meas.X.Change(code, key, val.X, mode);
        auto y = Meas.Y.Change(code, key, val.Y, mode);}
    std::map<std::string, Point2D<Measure*>> saved;
};
inline Point2D<float> operator/(const Point2D<float*>& lhs, const Point2D<float*>& rhs) {
    return { lhs.X[0] / rhs.X[0], lhs.Y[0] / rhs.Y[0] };
}

typedef float unit_t;

class NamedObj {
public:
    std::string Name = "";
    NamedObj(std::string name) : Name(name) {}
};

class Framebuf;
class DrawObj : public NamedObj{
public:
    Framebuf* Parent = nullptr;
    uint GUID = utils::StoreElement(this);
    Shader* m_shader = nullptr;
    DrawObjStates States{};
    DrawObjStates& GetStates() { return States; }

    Event_E e_BoundsChanged = Event_E(&Dispatcher::InvokeQ, this);
    Event_E e_MeasureChanged = Event_E(&Dispatcher::InvokeQ, this);
    Event_E e_Hover = Event_E(&Dispatcher::InvokeQ, this);

    virtual void OnBoundsChange();
    virtual void OnMeasureChange();
    virtual void OnHover();

    DrawObj(std::string name) : NamedObj(name) {}
    DrawObj() : DrawObj("GUID_" + std::to_string(GUID)) {}

    virtual void Init() = 0;
    virtual void Update(float dt) = 0;
    virtual void ProcessInput(float dt, InputArgs_S& input) = 0;
    virtual void Render() = 0;

    PickingData Pick;
    virtual void PickNext(float xNew, float yNew);
    virtual Point2D<float> SetPickFb() = 0;
    virtual bool PickAt(float xOld, float yOld) = 0;
    virtual PixelInfo BeginPickFunc(float xNew, float yNew);
    virtual PixelInfo PickFunc(float xNew, float yNew) = 0;
    virtual void StrongOnChange();
    virtual void OnChange();

    template<typename T>
    inline T* Cast() { return (T*)this; }
    //static uchar* pixels;
};

struct Aspect_C {
    bool apply = false;
    bool FromHeight = true;
    float shiftPositive = true;
    float value = 1;
};

struct Align_S {
    HorAlign HAlign = HorAlign::None;
    VerAlign VAlign = VerAlign::None;
    Margin_C Margin{};
    Aspect_C Aspect{}, LateAspect{};
    void Align(HorAlign h, VerAlign v) { HAlign = h; VAlign = v; }
    void Align(Align_S* src) { HAlign = src->HAlign; VAlign = src->VAlign; }
};

class DrawObj2D : public DrawObj, public Align_S {
public:
    bool Linked = false;
    int tex_num = 1, activeTex = 0;
    void SetWriteTex(int texIndex);
    unsigned int FBO, VAO, RBO_Depth;
    //virtual void OnChange() override;
    bool TextureResizeDisabled = false;

    //Event_E e_PreRend = Event_E(&Dispatcher::InvokeQ, this);
    Event_E e_Display = Event_E(&Dispatcher::InvokeQ, this);
    Event_E e_Update = Event_E(&Dispatcher::UpdateQ, this);

    VisibilityE Visibility = VisibilityE::Visible;
    void SetVisibility(VisibilityE vis);
    inline bool IsCollapsed() { return Visibility == VisibilityE::Collapsed; }
    std::vector<DrawObj2D*> UCChilds();
    Point2D<float> TexSize{-1, -1};
    std::vector<Texture2D*> Texs;
    Texture2D& GetActiveTex() { return *Texs[activeTex]; }
    void Refresh();
    virtual void ResizeTexs(Point2D<float> size);
    virtual void SetLinked() = 0;
    virtual void LinkChild(DrawObj2D* child) = 0;
    virtual void Link(Framebuf* parent) = 0;
    virtual void UnlinkChild(DrawObj2D* child) = 0;
    virtual void Unlink() = 0;
    virtual void Link2(Framebuf* parent) = 0;
    virtual void SetLinked2() = 0;
    virtual void SetLinked2(Framebuf* parent) = 0;
    Bounds m_Bounds, m_LastBound{ 0,0,0,0 };
    virtual void FixLayout();
    virtual void LinkMeasure() { Measure(); }
    virtual void Measure(); 
    virtual void Arrange();
    virtual void LastArrange(); 
    virtual void ComputeMeasures();
    virtual void ComputeBounds(); 
    virtual void FixBounds();
    MeasureCol2D Pos, Size;
    Point2D<float> ActualPos, ActualSize;
    Point2D<float*> LogicalSize{};
    virtual Point2D<float> IntendedSize();
    virtual void ComputeSize(); virtual void AdjustSizeWith(std::string key);
    virtual void ComputePos(); virtual void AdjustPosWith(std::string key);

    std::vector<DrawObj2D*> Childs;
    glm::vec4 m_clear_color_base = { 0,0,0,1 };
    DrawObj2D(std::string name, Shader* shader, float xPos = 0, float yPos = 0, float xSize = FB_WIDTH, float ySize = FB_HEIGHT)
        : DrawObj(name), ActualSize({ xSize, ySize }), ActualPos({ xPos, yPos }),
        FBO(0), VAO(0), Texs() {
        LogicalSize = { &ActualSize.X,&ActualSize.Y };
        m_shader = shader;
    }
    DrawObj2D(std::string name, float xPos = 0, float yPos = 0, float xSize = FB_WIDTH, float ySize = FB_HEIGHT)
        : DrawObj2D(name, GetPassShader(), xPos, yPos, xSize, ySize) {}
    DrawObj2D(float xPos = 0, float yPos = 0, float xSize = FB_WIDTH, float ySize = FB_HEIGHT)
        : DrawObj2D("dObj", GetPassShader(), xPos, yPos, xSize, ySize) {}
    virtual DrawObj2D* AddChild(DrawObj2D* dObj) = 0;
    virtual void AddChilds(std::vector<DrawObj2D*> dObjs) = 0;
    virtual void RemoveChild(DrawObj2D* dObj) = 0;
    void RemoveSelf();
    virtual void BeginPrepare() = 0;
    virtual void Prepare() = 0;
    virtual uint RenderB(const Bounds& bn) = 0;
    virtual void InformChange(InformCB callback);
    
    virtual bool TryChangeActivity(bool value) = 0;
    virtual DrawObj2D* MemClone() = 0;
    virtual void CopyTexture(DrawObj2D* obj, int index = 0) = 0;
    static Shader* GetPassShader();
    static uint next;
};

class Framebuf : public DrawObj2D {
public:
    explicit Framebuf(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0, Shader* shader = GetPassShader())
        : DrawObj2D(name, shader, xPos, yPos, xSize, ySize) {}
    Framebuf(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0, Shader* shader = GetPassShader())
        : Framebuf("fb" + std::to_string(GUID), xPos, yPos, xSize, ySize, shader) {}
    ~Framebuf();

    virtual void Init();
    virtual void Update(float dt);
    virtual void ProcessInput(float dt, InputArgs_S& input) override;
    virtual void BeginRender(bool clear = true);
    virtual void EndRender();
    virtual DrawObj2D* AddChild(DrawObj2D* dObj) override;
    virtual void AddChilds(std::vector<DrawObj2D*> dObjs) override;
    virtual void InsertChildAt(int index, DrawObj2D* dObj);
    virtual void RemoveChild(DrawObj2D* dObj) override;
    void ClearChilds();
    virtual void LinkChild(DrawObj2D* child) override;
    virtual void UnlinkChild(DrawObj2D* child) override;
    virtual void SetLinked() override;
    virtual void SetLinked2() override;
    virtual void SetLinked2(Framebuf* parent) override;
    virtual void BeginLink();
    virtual void Link(Framebuf* parent) override;
    virtual void Link2(Framebuf* parent) override;
    virtual void Unlink() override;
    virtual void BeginPrepare() override;
    virtual void Prepare() override;
    virtual void Render() override;
    virtual uint RenderB(const Bounds& bn) override;
    virtual Point2D<float> SetPickFb() override;
    virtual bool PickAt(float xOld, float yOld) override;
    virtual PixelInfo PickFunc(float xNew, float yNew) override;
    virtual bool TryChangeActivity(bool value) override;
    virtual DrawObj2D* MemClone() { return new Framebuf(*this); }
    void CopyTexture(DrawObj2D* obj, int index = 0) override;
    void ErrorCheck(std::string name);
    bool Close();
    void ClearColor(int index = 0);
    static uint TexRender(DrawObj2D& dobj);
private:
};

class Originbuf : public Framebuf {
public:
    Originbuf(std::string name, float xPos = 0, float yPos = 0, float xSize = FB_WIDTH, float ySize = FB_HEIGHT)
        : Framebuf(name, xPos, yPos, xSize, ySize, GetPassShader()) {}
    Originbuf(float xPos = 0, float yPos = 0, float xSize = FB_WIDTH, float ySize = FB_HEIGHT)
        : Originbuf("fb" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
    ~Originbuf() {};
    void Init() override;
    void Arrange() override { ResizeTexs(ActualSize); }
    void ComputeBounds() override {}
};

class Texbuf : public Framebuf {
public:
    Texbuf(std::string name, float xPos = 0, float yPos = 0, float xSize = FB_WIDTH, float ySize = FB_HEIGHT)
        : Framebuf(name, xPos, yPos, xSize, ySize, GetPassShader()) {}
    Texbuf(float xPos = 0, float yPos = 0, float xSize = FB_WIDTH, float ySize = FB_HEIGHT)
        : Texbuf("fb" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
    ~Texbuf() {};

    virtual void Update(float dt) override {}
    virtual void ProcessInput(float dt, InputArgs_S& input) override {}
    virtual void BeginRender(bool clear = true) override {}
    virtual void EndRender() override {}
    virtual DrawObj2D* AddChild(DrawObj2D* dObj) override { return nullptr; }
    void ClearChilds() {}
    virtual void BeginPrepare() override {}
    virtual void Prepare() override {}
    virtual Point2D<float> SetPickFb() override { return {}; }
    virtual bool PickAt(float xOld, float yOld) override { return false; }
    virtual PixelInfo PickFunc(float xNew, float yNew) override { return {}; }
    virtual bool TryChangeActivity(bool value) override { return true; }
    virtual DrawObj2D* MemClone() { return new Texbuf(*this); }
};

class PickFramebuf : public Framebuf {
public:
    PickFramebuf(Shader* shader = PickFramebuf::GetPickShader());
    void Init() override;

    void Resize(Point2D<float> size);
    virtual void Render() override;
    uint RenderB(const Bounds& bn) override;
    void BeginRender(bool clear = true) override;
    virtual void EndRender() override;
    static Shader* GetPickShader();
private:
};

class PickFramebuf3D : public PickFramebuf {
public:
    PickFramebuf3D(Shader* shader = PickFramebuf3D::GetPickShader());
    void Init() override;
    static Shader* GetPickShader();
private:
};

class CharFramebuf : public Framebuf {
public:
    uchar chr = 0;
    Point2D<int> Bearing = { 0,0 };
    Point2D<int> Advance = { 0,0 };
    uchar* buffer = nullptr;
    glm::vec4 color = { 1,1,1,1 };
    CharFramebuf(std::string name, FT_Face face, uchar* buf) : Framebuf(name) {
        buffer = buf;
        Setup(face);
        Init();
    }
    CharFramebuf(FT_Face face, uchar* buf) : CharFramebuf("char_fb" + std::to_string(GUID), face, buf) {}
    CharFramebuf(std::string name) : Framebuf(name) {}
    CharFramebuf() : CharFramebuf("char_fb" + std::to_string(GUID)) {}
    void Setup(FT_Face face);
    void Setup(float w, float h, int bX, int bY, int aX, int aY);
    void Init();

    virtual void Render() override;
    uint RenderB(const Bounds& bn) override;
    void BeginRender(bool clear = true) override;
    virtual void EndRender() override;

    virtual DrawObj2D* MemClone() { return new CharFramebuf(*this); }

};
