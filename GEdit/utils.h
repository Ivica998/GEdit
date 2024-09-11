#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>
#include <string>
#include <map>
//#include <functional>
#include <unordered_map>
#include <unordered_set>
//#include "dispatcher.h"

#define All_WIDTH 800
#define All_HEIGHT 600
#define FB_WIDTH All_WIDTH
#define FB_HEIGHT All_HEIGHT
#define TR_WIDTH All_WIDTH
#define TR_HEIGHT All_HEIGHT
#define WIN_WIDTH All_WIDTH
#define WIN_HEIGHT All_HEIGHT
#define HASHF_STEP 1000
#define FIELD_H 30
#define COMPONENT_TITLE_H 20

class WindowBase;
class DrawObj;
class DrawObj2D;
class Framebuf;
class PickFramebuf;

typedef std::vector<DrawObj2D*> fields_t;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef long long ll;
typedef bool(*InformCB)(DrawObj2D* sender);

struct print_visitor {
    template<class FieldData>
    void operator()(FieldData f) {
        std::cout << f.name() << "=" << f.get() << std::endl;
    }
};

template<class T>
void print_fields(T& x) {
    visit_each(x, print_visitor());
    //std::copy()
}
/*
template <typename... Ts>
void foo_print(Ts... args) {
    ((std::cout << args << ' '), ...);
}
template <class ... Ts>
void Foo(Ts && ... inputs) {
    int i = 0;
    ([&] {
        ++i;
        std::cout << "input " << i << " = " << inputs << std::endl;
        } (), ...);
}
*/

namespace utils {
    uint GetGUID();
    uint StoreElement(void* element);
    template<typename T>
    int IndexOf(std::vector<T> v, T obj);
    template<typename T>
    void AppendVec(std::vector<T>& v1, std::vector<T>& v2);
    template<typename T, typename T2>
    void AppendVec(std::vector<T>& v1, std::vector<T2>& v2);
    template<typename T, typename T2>
    void ErraseFromVec(std::vector<T>& v1, T2 val);
    void FlipTexture(Framebuf* fb);
    inline std::string to_string(const float& t) {
        std::string str{ std::to_string(t) };
        int offset{ 1 };
        if (str.find_last_not_of('0') == str.find('.')) { offset = 0; }
        str.erase(str.find_last_not_of('0') + offset, std::string::npos);
        return str;
    }
    inline std::string to_string(const int& t) {
        std::string str{ std::to_string(t) };
        return str;
    }
    template<typename T>
    void ErraseFromVecIf(std::vector<T>& v, bool(*callback)(T)) {
        v.erase(std::remove_if(v.begin(), v.end(), callback), v.end());
    }
    template<typename T>
    void ErraseFromSetIf(std::unordered_set<T>& v, bool(*callback)(T)) {
        //typename std::unordered_set::iterator set_iter;
        for (auto it = v.begin(); it != v.end(); /* blank */) {
            if (callback(*it)) {
                v.erase(it++);
            } else {
                ++it;
            }
        }
    }
    template<typename T>
    void InsertInVec(std::vector<T>& vec, int index, T val) {
        vec.insert(vec.begin() + index, val);
    }
    template<typename T>
    T VecSum(std::vector<T>& vec, int index, int count) {
        T retVal = 0; int n = index + count;
        if (n > vec.size()) n = vec.size();
        for (int i = index; i < n; i++) {
            retVal += vec[i];
        }
        return retVal;
    }

}
template<typename T>
int utils::IndexOf(std::vector<T> v, T obj)
{
    auto tmp = std::find(v.begin(), v.end(), obj) - v.begin();
    return (int)tmp;
}
template<typename T>
void utils::AppendVec(std::vector<T>& v1, std::vector<T>& v2) {
    v1.insert(v1.end(), v2.begin(), v2.end());
}
template<typename T, typename T2>
void utils::AppendVec(std::vector<T>& dest, std::vector<T2>& src) {
    typedef typename std::vector<T2>::iterator iter;
    for (iter i = src.begin(); i < src.end(); ++i) {
        dest.push_back((T)(*i));
    }
}
template<typename T, typename T2>
void utils::ErraseFromVec(std::vector<T>& vec, T2 val) {
    vec.erase(std::remove(vec.begin(), vec.end(), val), vec.end());
}

// ********************************************* STRUCTS
enum class MMode : int {
    None,
    Abs,
    Rel,
    Star,
};
class DrawObj2D;
struct Measure {
    float val = 0.f;
    MMode mode = MMode::Abs;
    Measure(float val_p = 0) : val(val_p) {}
    Measure(float val_p, MMode p_mode) : val(val_p), mode(p_mode) {}

};
/*
template<typename T>inline T& operator+=(T lhs, const Measure& rhs) { return lhs += rhs.val; }
template<typename T>inline T& operator-=(T lhs, const Measure& rhs) { return lhs -= rhs.val; }
template<typename T>inline T& operator*=(T lhs, const Measure& rhs) { return lhs *= rhs.val; }
template<typename T>inline T& operator/=(T lhs, const Measure& rhs) { return lhs /= rhs.val; }
template<typename T>inline float operator+(const T lhs, const Measure& rhs) { return lhs + rhs.val; }
template<typename T>inline float operator-(const T lhs, const Measure& rhs) { return lhs - rhs.val; }
template<typename T>inline float operator*(const T lhs, const Measure& rhs) { return lhs * rhs.val; }
template<typename T>inline float operator/(const T lhs, const Measure& rhs) { return lhs / rhs.val; }
template<typename T>inline Measure& operator+=(Measure& lhs, const T rhs) { lhs.val += rhs; return lhs; }
template<typename T>inline Measure& operator-=(Measure& lhs, const T rhs) { lhs.val -= rhs; return lhs; }
template<typename T>inline Measure& operator*=(Measure& lhs, const T rhs) { lhs.val *= rhs; return lhs; }
template<typename T>inline Measure& operator/=(Measure& lhs, const T rhs) { lhs.val /= rhs; return lhs; }
template<typename T>inline float operator+(const Measure& lhs, const T rhs) { return lhs.val + rhs; }
template<typename T>inline float operator-(const Measure& lhs, const T rhs) { return lhs.val - rhs; }
template<typename T>inline float operator*(const Measure& lhs, const T rhs) { return lhs.val * rhs; }
template<typename T>inline float operator/(const Measure& lhs, const T rhs) { return lhs.val / rhs; }
*/
inline Measure& operator+=(Measure& lhs, const Measure& rhs) { lhs.val += rhs.val; return lhs; }
inline Measure& operator-=(Measure& lhs, const Measure& rhs) { lhs.val -= rhs.val; return lhs; }
inline Measure& operator*=(Measure& lhs, const Measure& rhs) { lhs.val *= rhs.val; return lhs; }
inline Measure& operator/=(Measure& lhs, const Measure& rhs) { lhs.val /= rhs.val; return lhs; }
inline float operator+(const Measure& lhs, const Measure& rhs) { return lhs.val + rhs.val; }
inline float operator-(const Measure& lhs, const Measure& rhs) { return lhs.val - rhs.val; }
inline float operator*(const Measure& lhs, const Measure& rhs) { return lhs.val * rhs.val; }
inline float operator/(const Measure& lhs, const Measure& rhs) { return lhs.val / rhs.val; }


struct NatFloat { float f; };

template<typename T>
struct Point2D
{
    T X;
    T Y;
    void Swap() {
        tmp = X;
        X = Y;
        Y = tmp;
    }
    Point2D<T> Swapped() { return { Y, X }; }
    bool Trim(T down, T up) {
        T oldX = X; T oldY = Y;
        X = std::max(down, std::min(up, X));
        Y = std::max(down, std::min(up, Y));
        return oldX != X || oldY != Y;
    }
    bool Within(T min, T max) {
        return X > min && X < max && Y > min && Y < max;
    }
    template<typename T1>
    void BoundUp(Point2D<T1> up) {
        X = std::min(up.X, X);
        Y = std::min(up.Y, Y);
    }
    template<typename T1>
    void BoundDown(Point2D<T1> down) {
        X = std::max(down.X, X);
        Y = std::max(down.Y, Y);
    }
    template<typename T1>
    void Max(Point2D<T1> val) {
        X = std::max(val.X, X);
        Y = std::max(val.Y, Y);
    }
    template<typename T1>
    void Min(Point2D<T1> val) {
        X = std::min(val.X, X);
        Y = std::min(val.Y, Y);
    }
    bool Around(T base, T epsi) { return Within(base - epsi, base + epsi); }
    void FixZero() {
        if (X == 0) X = 1;
        if (Y == 0) Y = 1;
    }
    template<typename T1>
    Point2D<T> SafeZeroDivide(const Point2D<T1>& rhs) const {
        auto ret = Point2D<T>{ X / rhs.X, Y / rhs.Y };
        if (rhs.X == 0) ret.X = 0;
        if (rhs.Y == 0) ret.Y = 0;
        return ret;
    }
    bool HasZero() { return X == 0 || Y == 0; }
    bool Has(T x, T y) { return X == x && Y == y; }
    template<typename T1>
    Point2D<T>& operator=(const Point2D<T1>& rhs) {
        this->X = rhs.X;
        this->Y = rhs.Y;
        return *this;
    };
    Point2D<int> Round() {
        return Point2D<int>{ (int)std::round((float)X), (int)std::round((float)Y) };
    }
    template<typename T1>
    inline Point2D<T1> derefC() { return Point2D<T1>{ *(X), *(Y) }; };
    inline Point2D<float> derefC() { return Point2D<float>{ *(X), * (Y) }; };
    inline Point2D<T> operator-(){return { -X,-Y };}
    template<typename T1>
    inline Point2D<T1> Cast() { return { (T1)X, (T1)Y }; }
    inline bool Within(Point2D<T> range) {
        return (X > -range.X && X < range.X&& Y > -range.Y && Y < range.Y);
    }
    /*
    Point2D<T>& operator=(const glm::vec2 p) {
        X = p.x;
        Y = p.y;
        return *this;
    }
    */
    operator glm::vec2() {
        return glm::vec2{ X,Y };
    }
private:
    static T tmp;
};

template <typename T>
T Point2D<T>::tmp;

template<typename T1, typename T2>
auto operator*(const Point2D<T1>& lhs, const Point2D<T2>& rhs)
-> Point2D<decltype(std::declval<T1>()* std::declval<T2>())>
{
    return { lhs.X * rhs.X, lhs.Y * rhs.Y };
}

template<typename T1, typename T2>
auto operator/(const Point2D<T1>& lhs, const Point2D<T2>& rhs)
-> Point2D<decltype(std::declval<T1>()* std::declval<T2>())>
{
    auto tmp = 
        lhs.SafeZeroDivide<T2>(rhs);
    return tmp;
    return { lhs.X / rhs.X, lhs.Y / rhs.Y };
}



template<typename T1, typename T2>
bool operator==(const Point2D<T1>& lhs, const Point2D<T2>& rhs)
{
    return lhs.X == rhs.X && lhs.Y == rhs.Y;
}
template<typename T1, typename T2>
bool operator!=(const Point2D<T1>& lhs, const Point2D<T2>& rhs)
{
    return !(lhs == rhs);
}
template<typename T1, typename T2>
Point2D<T1> operator*(const Point2D<T1>& p, const T2& v)
{
    return { p.X * v, p.Y * v };
}

template<typename T1, typename T2>
Point2D<T1> operator*(const T2& v, const Point2D<T1>& p)
{
    return p * v;
}
template<typename T1, typename T2>
auto operator+(const Point2D<T1>& lhs, const Point2D<T2>& rhs)
-> Point2D<decltype(std::declval<T1>()* std::declval<T2>())>
{
    return { lhs.X + rhs.X, lhs.Y + rhs.Y };
}
template<typename T1, typename T2>
auto operator-(const Point2D<T1>& lhs, const Point2D<T2>& rhs)
-> Point2D<decltype(std::declval<T1>()* std::declval<T2>())>
{
    return { lhs.X - rhs.X, lhs.Y - rhs.Y };
}
template<typename T1, typename T2>
Point2D<T1>& operator+=(Point2D<T1>& lhs, const Point2D<T2>& rhs)
{
    lhs.X += rhs.X;
    lhs.Y += rhs.Y;
    return lhs;
};
template<typename T1, typename T2>
Point2D<T1>& operator-=(Point2D<T1>& lhs, const Point2D<T2>& rhs)
{
    lhs.X -= rhs.X;
    lhs.Y -= rhs.Y;
    return lhs;
};
template<typename T1, typename T2>
Point2D<T1>& operator*=(Point2D<T1>& lhs, const Point2D<T2>& rhs)
{
    lhs.X *= rhs.X;
    lhs.Y *= rhs.Y;
    return lhs;
};
template<typename T1, typename T2>
Point2D<T1>& operator/=(Point2D<T1>& lhs, const Point2D<T2>& rhs)
{
    lhs.X /= rhs.X;
    lhs.Y /= rhs.Y;
    return lhs;
};
/*
*/

enum class ComponentState : int {
    Enabled = 0,
    Disabled
};
struct LightInfo
{
    glm::vec3 Position;
    glm::vec3 Color;
};

template<typename T1 = int>
static T1 myPow(T1 x, unsigned int p)
{
    if (p == 0) return 1;
    if (p == 1) return x;

    T1 tmp = myPow<T1>(x, p / 2);
    if (p % 2 == 0) return tmp * tmp;
    else return x * tmp * tmp;
}

static size_t HashF(float f, int times)
{
    size_t x = (size_t)round(f * HASHF_STEP);
    x *= myPow<size_t>(HASHF_STEP, times - 1);
    return x;
}

struct InputArgs_S
{
    float Hold[1024];
    bool Keys[1024];
    bool KeysProcessed[1024];
    int Mode;

};

enum class MouseRM : int {
    None,
    LMBPress,
    LMBRelease,
    LMBBeginHold,
    LMBHold,
    LMBReleaseHold,
    RMBPress,
    RMBRelease,
    RMBBeginHold,
    RMBHold,
    RMBReleaseHold,
};

enum class VisibilityE : int {
    Visible,
    Hidden,
    Collapsed
};

inline const char* ToString(MouseRM v) {
    switch (v) {
    case MouseRM::None: return "None";
    case MouseRM::LMBPress: return "LMBPress";
    case MouseRM::LMBRelease:return "LMBRelease";
    case MouseRM::LMBHold:return "LMBHold";
    case MouseRM::RMBPress:return "RMBPress";
    case MouseRM::RMBRelease:return "RMBRelease";
    case MouseRM::RMBHold:return "RMBHold";
    default: return "[WTF]";
    }
}
/*
inline const char* ToString(SceneState v) {
    switch (v) {
    case SceneState::NONE: return "NONE";
    case SceneState::CREATE_CUBE: return "CREATE_CUBE";
    case SceneState::DRAGGING:return "DRAGGING";
    case SceneState::MOVING:return "MOVING";
    case SceneState::PICKING:return "PICKING";
    default: return "[WTF]";
    }
}
*/
struct MouseRay_S {
    MouseRM mode;
    bool Hover = true;
    std::vector<DrawObj*> trace;
    int cur_depth;
    void BeginTrace() {
        cur_depth = 0;
        Hover = true;
    }
    void TraceHover(DrawObj* obj);
    void CutTrace();
};
struct CharMapEntry_S {
    int code;
    int noShift;
    int shift;
};
struct CharMap_S {
    static const int Count = 47;
    static int Code[47];
    static int noShift[47];
    static int Shift[47];
    static CharMapEntry_S Entries[47];
    friend class constructor;
private:
    struct constructor {
        constructor() {
            
        }
        ~constructor()
        {
        }
    };
    static constructor cons;
    void InitStatic();
};
struct myVec2 {
    union { float x, r; };
    union { float y, g; };
    myVec2(float xp, float yp) : x(xp), y(yp) {}
    float Mid() { return (x + y) / 2; }
    bool const operator==(const myVec2& vec) const {
        return x == vec.x && y == vec.y;
    }
    bool const operator!=(const myVec2& vec) const {
        return !(*this == vec);
    }
    float const Size() const {
        return std::sqrtf(x * x + y * y);
    }
    bool const operator<(const myVec2& vec) const {
        return Size() < vec.Size();
    }
};
template<typename T1>
myVec2 operator*(const myVec2& p, const T1& v) {
    return { p.x * v, p.y * v };
}
template<typename T1>
myVec2 operator*(const T1& v, const myVec2& p) {
    return p * v;
}

struct myVec2Hash {
    size_t operator()(const myVec2& vec) const {
        size_t hash = std::hash<float>()(vec.x) + std::hash<float>()(vec.y);
        return hash;
    }
};

struct myVec3 {
    union { float x, r; };
    union { float y, g; };
    union { float z, b; };
    myVec3(float xp, float yp, float zp) : x(xp), y(yp), z(zp) {}
    float Mid() { return (x + y + z) / 3; }
    bool const operator==(const myVec3& vec) const {
        return x == vec.x && y == vec.y && z == vec.z;
    }
    bool const operator!=(const myVec3& vec) const {
        return !(*this == vec);
    }
    float const Size() const {
        return std::sqrtf(x * x + y * y + z * z);
    }
    bool const operator<(const myVec3& vec) const {
        return Size() < vec.Size();
    }
};
template<typename T1>
myVec3 operator*(const myVec3& p, const T1& v) {
    return { p.x * v, p.y * v, p.z * v };
}
template<typename T1>
myVec3 operator*(const T1& v, const myVec3& p) {
    return p * v;
}

struct myVec3Hash {
    size_t operator()(const myVec3& vec) const {
        size_t hash = std::hash<float>()(vec.x) + std::hash<float>()(vec.y) + std::hash<float>()(vec.z);
        return hash;
    }
};

struct glmVec2Hash {
    size_t operator()(const glm::vec2& vec) const {
        size_t hash = std::hash<float>()(vec.x) + std::hash<float>()(vec.y);
        return hash;
    }
};

struct glmVec3Hash {
    size_t operator()(const glm::vec3& vec) const {
        size_t hash = std::hash<float>()(vec.x) + std::hash<float>()(vec.y) + std::hash<float>()(vec.z);
        return hash;
    }
};

struct Bounds
{
    union { float x1, u1, t1; };
    union { float y1, v1, s1; };
    union { float x2, u2, t2; };
    union { float y2, v2, s2; };
    Bounds(float xpoz1 = -1.0, float ypoz1 = -1.0, float xpoz2 = 1.0, float ypoz2 = 1.0)
        : x1(xpoz1), y1(ypoz1), x2(xpoz2), y2(ypoz2) {}
    float Area() const { return (x2 - x1) * (y2 - y1); }
    float SpreadX() const { return (x2 - x1) / 2; }
    float SpreadY() const { return (y2 - y1) / 2; }
    Point2D<float> Size() const { return { SpreadX(),SpreadY() }; }
    Point2D<float> Pos() const { return { (x1 + 1) / 2,(y1 + 1) / 2 }; }
    Bounds Directed() { return { x1, -y2, x2, -y1 }; }
    Bounds DirectedTex() { return { x1, 1-y2, x2, 1-y1 }; }
    bool const operator==(const Bounds& bn) const {
        return x1 == bn.x1 && y1 == bn.y1 && x2 == bn.x2 && y2 == bn.y2;
    }
    bool const operator!=(const Bounds& bn) const {
        return !(*this == bn);
    }
    bool const operator<(const Bounds& bn) const {
        return Area() < bn.Area();
    }
    Point2D<float> Middle() { return { (x1 + x2) / 2 , (y1 + y2) / 2 }; }
    const static Bounds s_default;
    const static Bounds s_flipY;
    Bounds& operator+=(const Bounds& lhs) {
        x1 += lhs.x1;
        x2 += lhs.x2;
        y1 += lhs.y1;
        y2 += lhs.y2;
        return *this;
    }
    Bounds& operator-=(const Bounds& lhs) {
        x1 -= lhs.x1;
        x2 -= lhs.x2;
        y1 -= lhs.y1;
        y2 -= lhs.y2;
        return *this;
    }
};
template<typename T1>
Bounds operator*(const Bounds& p, const T1& v)
{
    float cX = p.SpreadX() / 2 * ((float)v - 1.0f);
    float cY = p.SpreadY() / 2 * ((float)v - 1.0f);
    return { p.x1 - cX, p.y1 - cY, p.x2 + cX, p.y2 + cY };
}
template<typename T1>
Bounds operator*(const T1& v, const Bounds& p)
{
    return p * v;
}



struct BoundsHash
{
    size_t operator()(const Bounds& bn) const
    {
        size_t hash = std::hash<float>()(bn.x1) + std::hash<float>()(bn.y1) + std::hash<float>()(bn.x2) + std::hash<float>()(bn.y2);
        return hash;
    }
};

struct PickFuncArgs
{
    float X = 0;
    float Y = 0;
    DrawObj2D* cd;
};

struct Identity
{
    uint GUID;
    uint ObjID;
    uint DrawID;
    uint GLID; //UINT32_MAX;
    uint PrimID;
    Identity(uint guID = 0, uint objID = 0, uint drawID = 0, uint glID = 0, uint primID = 0)
        : GUID(guID), ObjID(objID), DrawID(drawID), GLID(glID), PrimID(primID) { }
    void Print() {
        printf("GUID %u, ObjID %u, DrawID %u,GLID %u,PrimID %u\n", GUID, ObjID, DrawID, GLID, PrimID);
    }
};

struct PixelInfo {
    Identity Ident;
    std::string Name = "";
    void Print2(bool justName = false) {
        printf("Name \"%s\"; ", Name.c_str());
        if (!justName)
            Ident.Print();
    }
    void Print() {
        printf("PickedId: \"%u\", Name: \"%s\", GUID: \"%u\"\n", Ident.ObjID, Name.c_str(), Ident.GUID);
    }
    bool IsMatch(uint guid) {
        return guid == Ident.GUID;
    }
    bool IsMatch(uint objID, uint drawID) {
        return objID == Ident.ObjID && drawID == Ident.DrawID;
    }
    bool IsMatch(uint objID, uint drawID, uint primID) { return objID == Ident.ObjID && drawID == Ident.DrawID && primID == Ident.PrimID; }
    bool const operator==(const PixelInfo& obj) const {
        return Ident.ObjID == obj.Ident.ObjID && Ident.DrawID == obj.Ident.DrawID;
    }
};

struct PixelInfoHash
{
    size_t operator()(const PixelInfo& obj) const
    {
        size_t hash = std::hash<uint>()(obj.Ident.GUID) + std::hash<uint>()(obj.Ident.DrawID);
        return hash;
    }
};
class DrawObj;
struct PickingData
{
    bool Pickable = true;
    PickFramebuf* FB = nullptr;

    std::vector<uint> Map;
    std::vector<std::vector<uint>> MultiMap;
    PixelInfo pInfo;
    bool PickChanged = true;
    DrawObj* PickedObj = nullptr;
};
enum class Camera_Movement {
    NONE,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};
enum class ProjectionMode {
    Perspective,
    Orthogonal,
};

struct Align_S;
enum class HorAlign;
enum class VerAlign;
class UtilsHolder
{
public:
    static unsigned int cube8VAO, cube6VAO;
    static unsigned int cubeVBO;
    static unsigned int quadVAO;
    static unsigned int quadVBO;

    static size_t HashF(float f, int times);
    static void Limit(float& val, float min, float max);
    static void m11To01(float& x);
    static void m01To11(float& x);
    static void m11To01(Bounds& bnd);
    static void m01To11(Bounds& bnd);
    static void m11To01(Point2D<float>& var);
    static void m01To11(Point2D<float>& var);
    static void ApplyAspect(Bounds& bn, Align_S* obj);
    static void ApplyAspect(Point2D<float>& pos, Point2D<float>& size, Align_S* obj, Point2D<float>& parantSize);
    static void Align(Bounds& bn, Align_S* obj);
    static void Align(Bounds& bn, HorAlign HAlign, VerAlign VAlign);
    static std::pair<float, float> RemapPoint(float x, float y, Bounds b);
    static Point2D<float> RemapPoint(Point2D<float> cords, Bounds b);
    std::pair<float, float> RemapPoint(float x, float y, float x1, float x2, float y1, float y2);
    static void Draw(uint v);
    static void DrawStrips(uint v);
    template<typename T = float>
    static uint makeS(float x1, float y1, float x2, float y2);
    template<typename T = float>
    static uint make(float x1, float y1, float x2, float y2);
    static uint makeCube6VAO();
    static void renderCube6();
    static uint makeCube8VAO();
    static void renderCube8();
    static void renderQuad();
    static PixelInfo ReadPixel(PixelInfo& pInfo, uint FBO, float x, float y);
    static int DigCnt(int number);
    template<typename T>
    static void fillBuffer(T* src, T* dest, int size, int offset, int stride, int amount, int* indexer);
    template<typename T>
    static void copy_arr(T* src, T* dest, int amount);
    template<typename T>
    static std::string struct_to_str(void* strct, int size, int nlat);
    static int safe_stoi(std::string str);
    static float safe_stof(std::string str);
    static bool has_dot(std::string str);
    static Point2D<float> CordsOf(Framebuf* screenCord, Point2D<float> cords);
    static float trim(float value, float min, float max);
private:
    UtilsHolder() { }
};

typedef UtilsHolder utl;

template<typename T>
inline uint UtilsHolder::makeS(float x1, float y1, float x2, float y2)
{
    unsigned int VBO, VAO = 0;
    float quadVertices[] = {
        // positions
        x1, y2, 0.0f, 0.0f, 1.0f,
        x1, y1, 0.0f, 0.0f, 0.0f,
        x2, y2, 0.0f, 1.0f, 1.0f,
        x2, y1, 0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(T), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(T), (void*)(3 * sizeof(T)));
    glBindVertexArray(0);
    return VAO;
}

template<typename T>
inline uint UtilsHolder::make(float x1, float y1, float x2, float y2)
{
    float vertices[] = {
        x2,  y2, 0.0f,  // top right
        x2,  y1, 0.0f,  // bottom right
        x1,  y1, 0.0f,  // bottom left
        x1,  y2, 0.0f   // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(T), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return VAO;
}

template<typename T>
inline void UtilsHolder::fillBuffer(T* src, T* dest, int size, int offset, int stride, int amount, int* indexer)
{
    int index = 0;
    dest += offset;
    for (int i = 0; i + amount < size; i += stride)
    {
        for (int j = 0; j < amount; j++)
        {
            dest[i + j] = src[indexer[index++]];
        }
    }
}

template<typename T>
inline void UtilsHolder::copy_arr(T* src, T* dest, int amount)
{
    for (size_t i = 0; i < amount; i++)
    {
        dest[i] = src[i];
    }
}

template<typename T>
inline std::string UtilsHolder::struct_to_str(void* strct, int size, int nlat) {
    std::string ret = "";
    T* cast = (T*)strct;
    nlat++;
    for (int i = 0; i < size; i++) {
        ret += std::to_string(*cast) + ", ";
        if (i % nlat == 0)
            ret += "\n";
    }
    return ret;
}

