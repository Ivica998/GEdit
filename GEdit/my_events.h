#pragma once
#include <vector>
#include "utils.h"
#include <type_traits>

class Tuple {};
template<typename T1>
struct Tuple1 : Tuple {
    Tuple1(T1 p1) : v1(p1) {}
    T1 v1;
};
template<typename T1, typename T2>
struct Tuple2 : Tuple {
    Tuple2(T1 p1, T2 p2) : v1(p1), v2(p2) {}
    T1 v1;T2 v2;
};
template<typename T1, typename T2, typename T3>
struct Tuple3 : Tuple {
    Tuple3(T1 p1, T2 p2, T3 p3) : v1(p1), v2(p2), v3(p3) {}
    T1 v1;T2 v2;T3 v3;
};
template<typename T1, typename T2, typename T3, typename T4>
struct Tuple4 : Tuple {
    Tuple4(T1 p1, T2 p2, T3 p3, T4 p4) : v1(p1), v2(p2), v3(p3), v4(p4) {}
    T1 v1;T2 v2;T3 v3;T4 v4;
};
template<typename T1, typename T2, typename T3, typename T4, typename T5>
struct Tuple5 : Tuple {
    Tuple5(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) : v1(p1), v2(p2), v3(p3), v4(p4), v5(p5) {}
    T1 v1;T2 v2;T3 v3;T4 v4;T5 v5;
};

class NamedObj;
class ChildData;
class Control;
class DrawObj;
class Shader;
class Scene;
class CheckBox;
class TabViewer;
class ViewSpace;
class Object;
class Control3D;
class Texture;
class NumBox1;
class TextureField;
class TexIndexUI;
class TransformUI;
class Dragger;
class WindowBase;

typedef std::vector<void*> v_vptr_t;
typedef DrawObj* sender_ptr;
typedef v_vptr_t params_t;

template<typename T1>
class SmartPtr {
    T1* ptr;
public:
    explicit SmartPtr(T1* p = NULL) { ptr = p; }
    ~SmartPtr() { delete (ptr); }

    T1& operator*() { return *ptr; }
    T1* operator->() { return ptr; }
};

struct Invokable {
    virtual void Invoke() = 0;
};

template<typename T>
void destroy_vector(std::vector<T*>& v) {
    while (!v.empty()) {
        delete v.back();
        v.pop_back();
    }
}

template<typename TSender>
class EventMaker {
public:
    template<typename TArgs>
    class Event {
    public:
        typedef void(*GenHandler)(TSender* sender, TArgs* e, params_t params);
        struct GenData : public Invokable {
            GenHandler callback = nullptr;
            TSender* sender = nullptr;
            TArgs* e = nullptr;
            params_t params;
            void Invoke() override { callback(sender, e, params); }
        };
        typedef GenHandler gt_handler;
        typedef TSender* gt_sender;
        typedef TArgs gt_args;
    private:
        std::unordered_map<NamedObj*, std::unordered_map<GenHandler, GenData*>> m_actions;
        std::vector<Invokable*>* m_dispQ = nullptr;
        std::vector<GenData*> m_regCB;
        TSender* m_owner = nullptr;
    public:
        TArgs* event_args = new TArgs();
        Event() {}
        Event(void* dispQ, TSender* owner) : m_dispQ((std::vector<Invokable*>*)dispQ), m_owner(owner) {}
        void Add(NamedObj* subscriber, void* callback, params_t pars) {
            auto& list = m_actions[subscriber];
            std::unordered_map<GenHandler*, GenData*> cb_map;
            GenData* item = new GenData();
            item->callback = (GenHandler)callback;
            item->params = pars;
            item->sender = m_owner;
            m_regCB.push_back(item);
            if (m_regCB.size() > 4)
                int x = 5;
            list.insert({ item->callback,item });
        }
        void Add(NamedObj* subscriber, void* callback, params_t pars, TSender* sender) {
            auto& list = m_actions[subscriber];
            std::unordered_map<GenHandler*, GenData*> cb_map;
            GenData* item = new GenData();
            item->callback = (GenHandler)callback;
            item->params = pars;
            item->sender = sender;
            m_regCB.push_back(item);
            list.insert({ item->callback,item });
        }
        void Remove(NamedObj* subscriber, GenHandler callback) {
            auto& list = m_actions[subscriber];
            if (!list.empty() && list.count(callback) != 0) {
                utils::ErraseFromVec(m_regCB, list[callback]);
                list.erase(callback);
            }
        }
        void QInvoke(TArgs* e) {
            for (int i = 0; i < m_regCB.size(); i++) {
                m_regCB[i]->e = e;
                m_dispQ->push_back(m_regCB[i]);
            }
        }
        void QInvoke() {
            for (int i = 0; i < m_regCB.size(); i++) {
                m_regCB[i]->e = event_args;
                m_dispQ->push_back(m_regCB[i]);
            }
        }
        void Invoke(TArgs* e) {
            for (int i = 0; i < m_regCB.size(); i++) {
                m_regCB[i]->e = e;
                m_regCB[i]->Invoke();
            }
        }
        void Invoke() {
            for (int i = 0; i < m_regCB.size(); i++) {
                m_regCB[i]->e = event_args;
                m_regCB[i]->Invoke();
            }
        }
        void InvokeRemove() {
            for (int i = 0; i < m_regCB.size(); i++) {
                m_regCB[i]->e = event_args;
                m_regCB[i]->Invoke();
            }
            RemoveAll();
        }
        void RemoveAll() {
            m_actions.clear();
            destroy_vector(m_regCB);
            m_regCB.clear();
        }
    };
};
typedef EventMaker<DrawObj> DrawObjEM;
typedef EventMaker<Shader> ShaderEM;
typedef EventMaker<Scene> SceneEM;
typedef EventMaker<CheckBox> CheckBoxEM;
typedef EventMaker<ViewSpace> ViewSpaceEM;
typedef EventMaker<TabViewer> TabViewerEM;
typedef EventMaker<Object> ObjectEM;
typedef EventMaker<Control3D> Control3DEM;
typedef EventMaker<Texture> TextureEM;
typedef EventMaker<NumBox1> NumBox1EM;
typedef EventMaker<TextureField> TextureFieldEM;
typedef EventMaker<TexIndexUI> TexIndexUIEM;
typedef EventMaker<TransformUI> TransformUIEM;
typedef EventMaker<Dragger> DraggerEM;
typedef EventMaker<WindowBase> WindowEM;

template<typename T1, typename T2, typename T3>
struct DataGen : public Invokable {
    T1 callback = nullptr;
    T2 sender = nullptr;
    T3* e = nullptr;
    params_t params;
    void Invoke() override { callback(sender, e, params); }
};


struct EventArgs {
    bool handled = false;
};
typedef DrawObjEM::Event<EventArgs> Event_E;

struct MouseEventArgs : public EventArgs {
    Point2D<float> rel_pos_val;
    Point2D<float> rel_pos_cng;
    Point2D<float> cursor_pos_value;
    Point2D<float> cursor_pos_change;
};
struct MouseClickEventArgs : public  MouseEventArgs {
};
typedef DrawObjEM::Event<MouseClickEventArgs> MouseClickEvent_E;
typedef MouseClickEvent_E::GenData MouseClickData;

class MouseHoverEventArgs : public  MouseEventArgs {
}; typedef DrawObjEM::Event<MouseHoverEventArgs> MouseHoverEvent_E;

struct ScrollEventArgs : public  EventArgs {
    Point2D<float> scroll_value;
    Point2D<float> scroll_change;
};
typedef DrawObjEM::Event<ScrollEventArgs> ScrollEvent_E;
typedef ScrollEvent_E::GenData ScrollData;

struct TextChangeEventArgs : public  EventArgs {
    std::string text_value;
    std::string text_change;
};typedef DrawObjEM::Event<TextChangeEventArgs> TextChangeEvent_E;

struct NumberChangeEventArgs : public  EventArgs {
    float number_value;
    float number_change;
};typedef DrawObjEM::Event<NumberChangeEventArgs> NumberChangeEvent_E;

struct ShaderEventArgs : public  EventArgs {
    glm::mat4 model;
};typedef ShaderEM::Event<ShaderEventArgs> ShaderEvent_E;

struct SceneEventArgs : public  EventArgs {

};typedef SceneEM::Event<SceneEventArgs> SceneEvent_E;

struct CheckBoxEventArgs : public  EventArgs {
    bool Value;
};typedef CheckBoxEM::Event<CheckBoxEventArgs> CheckBoxToggleEvent_E;

struct ViewSpaceEventArgs : public  EventArgs {
    Point2D<float> space_pos_rel;
    Point2D<float> space_pos_rel_change;
};typedef ViewSpaceEM::Event<ViewSpaceEventArgs> ViewSpacePositionChangeEvent_E;


struct TabViewerEventArgs : public  EventArgs {
    bool Value;
};typedef TabViewerEM::Event<TabViewerEventArgs> TabViewerEvent_E;

struct SelectionChangeEventArgs : public  EventArgs {
    bool OldValue;
    bool Value;
};typedef Control3DEM::Event<SelectionChangeEventArgs> SelectionChangeEvent_E;

struct TextureUVChangeEventArgs : public  EventArgs {
    glm::vec2 UV;
};typedef TextureEM::Event<TextureUVChangeEventArgs> TextureUVChangeEvent_E;

struct NumBox1ValueChangeEventArgs : public  EventArgs {
    float number_value;
    float number_change;
};typedef NumBox1EM::Event<NumBox1ValueChangeEventArgs> NumBox1ValueChangeEvent_E;


struct TextureIndexChangeEventArgs : public  EventArgs {
    int index;
};typedef TextureFieldEM::Event<TextureIndexChangeEventArgs> TextureIndexChangeEvent_E;


struct TexIndexUIIndexChangeEventArgs : public  EventArgs {
    int index;
};typedef TexIndexUIEM::Event<TexIndexUIIndexChangeEventArgs> TexIndexUIIndexChangeEvent_E;


struct TransformUICordChangeEventArgs : public  EventArgs {
    int index;
};typedef TransformUIEM::Event<TransformUICordChangeEventArgs> TransformUICordChangeEvent_E;

struct DragEventArgs : public  EventArgs {
    Point2D<float> pos;
    Point2D<float> pos_cng;
};typedef DraggerEM::Event<DragEventArgs> DragEvent_E;

class MouseMoveEventArgs : public  MouseEventArgs {
}; typedef WindowEM::Event<MouseMoveEventArgs> MouseMoveEvent_E;
/*
*/



class EDepObjB { public: virtual void OnValChange() = 0;};
template<typename TDst, typename TSrc, typename TSrcE>
class EDepObj : public EDepObjB {
public:
    typedef EventMaker<void> GenericEM;
    struct GenericEventArgs : public  EventArgs {
        void* Value = nullptr;
        void* owner = nullptr;
    };typedef  GenericEM::Event<GenericEventArgs> GenericEvent_E;

    static void full_parse_callback(typename TSrcE::gt_sender sender, typename TSrcE::gt_args* e, params_t params) {
        EDepObj* this_v = (EDepObj*)params[0];
        this_v->ParseFull(sender);
        e->handled = true;
    }
    void* owner = nullptr; 
    TDst* value = nullptr; 
    TSrc* source = nullptr;
    GenericEvent_E e_Action = GenericEvent_E(nullptr, this);
    EDepObj() {}
    EDepObj(void* owner_p) {
        value = new TDst(); owner = owner_p;
        e_Action.event_args->owner = owner_p;
    }
    EDepObj(void* owner_p, TDst* value_p) {
        value = value_p; owner = owner_p;
        e_Action.event_args->owner = owner_p;
    }
    inline TDst& Get() { return *value; }
    inline void Set(TDst val) {
        value[0] = val;
        Condition(val);
        OnValChange();
        HandleIfNot();
    }
    void inline ParseFull(TSrc* val) {
        ParseVal(val);
        Condition(value[0]);
        OnValChange();
        HandleIfNot();
    }
    virtual void ParseVal(TSrc* val) {}
    virtual void Condition(TDst val) {}
    virtual void OnSet(void* owner, TSrc newValue) {}
    virtual void SendBack() {}
    inline void HandleIfNot() {
        if (!e_Action.event_args->handled) {
            SendBack();
        }
    }
    void OnValChange() {
        e_Action.event_args->Value = (void*)value;
        e_Action.event_args->handled = false;
        e_Action.Invoke();
    }
    void Remap(void* owner_p, TDst* value_p) {
        e_Action.event_args->owner = owner_p;
        owner = owner_p;
        value = value_p;
        SendBack();
    }
};
