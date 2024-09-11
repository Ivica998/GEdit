
#include "control.h"
#include "scene.h"
template<typename T>
Point2D<float> StackPanelGen<T>::StackPanelT::dummy;
std::unordered_set<uint> SharedSPStatic::unadjustable;
template<typename T>
std::unordered_set<T*> UIManage<T>::ActiveUIs;
template<typename T>
std::unordered_set<T*> UIManage<T>::InactiveUIs;

// ********************************************** CONTROL *******************************************************
bool Control::PickAt(float x, float y) {
    return Framebuf::PickAt(x, y);
}
PixelInfo Control::PickFunc(float xNew, float yNew) {
    return Framebuf::PickFunc(xNew, yNew);
}
void Control::Delete() {
    //GetParent??->RemoveChild(ThisGUID);
    rmtd::RemoveElement(GUID);
}
void Control::Update(float dt) {
}
void Control::InformChange(InformCB callback) {
    DrawObj2D* next = (DrawObj2D*)Parent;
    if (next && callback(next)) {
        next->InformChange(callback);
    }
}
void Control::ProcessInput(float dt, InputArgs_S& input) {
    Framebuf::ProcessInput(dt, input);
}
Point2D<float> Control::SetPickFb() {
    return Framebuf::SetPickFb();
}
void Control::Prepare() {
    Framebuf::Prepare();
    //e_PreRend.QInvoke();
}
void Control::Render() {
    return Framebuf::Render();
}
uint Control::RenderB(const Bounds& bn) {
    return Framebuf::RenderB(bn);
}
// ********************************************** TEXTBASE *******************************************************
void TextBase::OnTextChange() {
    e_TextChange.event_args->text_change = m_text;
    e_TextChange.event_args->text_value = m_text;
    e_TextChange.Invoke();
}
void TextBase::AddChar(const int& index, const uchar& chr) {
    m_text.insert(m_text.begin() + index, chr);
    TextData.ToRender = true;
    OnTextChange();
}
void TextBase::RemoveChar(const int& index) {
    m_text.erase(index, 1);
    TextData.ToRender = true;
    OnTextChange();
}
void TextBase::RenderText() {
    TextData.ToRender = false;
    if (Characters.size() == 0) return;

    float scale = m_scalePar;
    auto& dbCol = Characters;
    auto& sourceCol = m_text;
    float ud_margin = m_ud_margin * scale;
    float lr_margin = m_lr_margin * scale;
    float x = lr_margin, y = ud_margin;
    float rowHeight = dbCol['H']->Bearing.Y * scale + ud_margin;
    ClearChilds(); TextData.Clear();
    float Start = lr_margin;
    float End = AutoNewLine ? (ActualSize.X - 6 * scale - lr_margin) : FLT_MAX;
    for (auto c = sourceCol.begin(); c != sourceCol.end(); c++) {
        CharFramebuf* ch = dbCol[(uchar)*c];
        TextData.Add({ x,y }, ch);
        x += (ch->Advance.X >> 6) * scale;
        if (x > End || ch->chr == '\n') {
            x = Start;
            y += rowHeight;
            TextData.NextRow();
        }
    }
    TextData.Offs_v.push_back({ x,y }); // !care mismatch;
    TextData.Offs_end = { x,y };
    auto& col = TextData.Chars_v;
    auto& tex = Texs[activeTex];
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    BeginRender();
    for (int i = 0; i < col.size(); ++i) {
        auto& pt = TextData.Offs_v[i];
        x = pt.X;
        y = pt.Y;
        auto ch = col[i];
        float xpos = x + ch->Bearing.X * scale;
        float ypos = y + (dbCol['H']->Bearing.Y - ch->Bearing.Y) * scale;
        float w = ch->ActualSize.X * scale;
        float h = ch->ActualSize.Y * scale;
        float x1, x2, y1, y2;
        x1 = xpos / tex->Size.X;
        x2 = (xpos + w) / tex->Size.X;
        y1 = ypos / tex->Size.Y;
        y2 = (ypos + h) / tex->Size.Y;
        utl::m01To11(x1);utl::m01To11(x2);utl::m01To11(y1);utl::m01To11(y2);
        ch->color = m_colorPar;
        Bounds bnd{ x1, y1, x2, y2 };
        TextData.Bounds_v.push_back(bnd);
        if (ch->chr != '\n')
            ch->RenderB(bnd);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    EndRender();
}
void TextBase::SetParameters(float x, float y, float scale, glm::vec4* color_ptr) {
    m_xPar = x;
    m_yPar = y;
    m_scalePar = scale;
    if (color_ptr)
        m_colorPar = *color_ptr;
}
void TextBase::ResizeTexs(Point2D<float> size) {
    Framebuf::ResizeTexs(size);
    TextData.ToRender = true;
}
void TextBase::SetText(std::string text) {
    m_text = text;
    if (TextData.CurIdx > text.size())
        TextData.CurIdx = (int)text.size();
    //OnChange();
    TextData.ToRender = true;
}
void TextBase::Init() {
    Framebuf::Init();
    for (auto tex : Texs) {
        tex->Filter_Max = GL_LINEAR;
        tex->Filter_Min = GL_LINEAR;
        tex->Refresh();
        /*
        */
    }
    Pick.FB = rmtd::PickFb;
    Pick.Pickable = false;

    Characters = TextRend->Characters;
    OnChange();
    //GetStates().Changed = true;
}
void TextBase::Render() {
    Control::Render();
}
// ********************************************** LABEL *******************************************************
void Label::Init() {
    TextBase::Init();
}
PixelInfo Label::PickFunc(float xNew, float yNew) {
    return PixelInfo();
}
void Label::Prepare() {
    RenderText();
}
void Label::Render() {
    Control::Render();
}
std::vector<int>& TextBox::filter() {
    static std::vector<int> chars = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46 };
    return chars;
}
// ********************************************** TEXTBOX *******************************************************
void TextBox::Init() {
    TextBase::Init();
    Pick.Pickable = true;
    Texs[1]->m_clear_color = { 0,0,0,0 };
}
PixelInfo TextBox::PickFunc(float xNew, float yNew) {
    auto& pInfo = Pick.pInfo;
    auto& states = GetStates();
    DrawObj* child = Pick.PickedObj;
    if (rmtd::MouseRay.Hover) {
        e_MouseHover.Invoke();
        rmtd::MouseRay.trace.push_back(this);
    } else {
        IsPressed = true;
        states.Active = true;
        rmtd::CurWin->ActiveObjs.insert(this);
        if (child == nullptr)
            TextData.CurIdx = (int)TextData.Chars_v.size();
        else {
            TextData.CurIdx = pInfo.Ident.ObjID - 1;
        }
        OnChange();
    }
    return pInfo;
}
Point2D<float> TextBox::SetPickFb() {
    auto& col = TextData.Chars_v;
    Pick.FB->BeginRender();
    Point2D<float> size = { std::min(Pick.FB->ActualSize.X, ActualSize.X), std::min(Pick.FB->ActualSize.Y,ActualSize.Y) };
    glViewport(0, 0, (GLsizei)size.X, (GLsizei)size.Y);
    int id = 0;
    int r, g, b;
    for (int i = 0; i < col.size(); ++i) {
        auto ch = col[i];
        Pick.Map.push_back(ch->GUID);
        id++;
        r = (id & 0x000000FF) >> 0;
        g = (id & 0x0000FF00) >> 8;
        b = (id & 0x00FF0000) >> 16;
        Pick.FB->m_shader->SetVec4f("PickingColor", r / 255.0f, g / 255.0f, b / 255.0f, 1);
        Pick.FB->RenderB(TextData.Bounds_v[i]);
    }
    Pick.FB->EndRender();
    return size;
}
void TextBox::Prepare() {
    if (TextData.ToRender)
        RenderText();

    int& index = TextData.CurIdx;
    if (States.Active && index >= 0 && index <= TextData.Size()) {
        BeginRender(false);

        auto& tex = Texs[activeTex];
        auto& pt = TextData.Offs_v[index];
        float x = pt.X;
        float y = pt.Y;

        uchar c = '_';
        CharFramebuf* ch = Characters[c];
        float xpos = x + ch->Bearing.X;
        float ypos = y + (Characters['H']->Bearing.Y - ch->Bearing.Y);
        float w = ch->ActualSize.X;
        float h = ch->ActualSize.Y;

        float x1, x2, y1, y2;
        x1 = xpos / tex->Size.X;
        x2 = (xpos + w) / tex->Size.X;
        y1 = ypos / tex->Size.Y;
        y2 = (ypos + h) / tex->Size.Y;
        utl::m01To11(x1);utl::m01To11(x2);utl::m01To11(y1);utl::m01To11(y2);
        ch->m_shader->Use();
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        //ClearColor(0);
        ch->color = { 0,1,1,1 };
        ch->RenderB({ x1, y1, x2, y2 });
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        EndRender();
    }
}
void TextBox::Render() {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    TextBase::Render();
    activeTex = 1;
    TextBase::Render();
    activeTex = 0;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void TextBox::Update(float dt) {

}
bool TextBox::Check(float dt, InputArgs_S& input, int i) {
    if (input.Keys[i] && !input.KeysProcessed[i]) {
        float& hold = input.Hold[i];
        bool flag = hold == 0.5;
        hold -= dt;
        //printf("hold = %f\n", hold);
        if (flag || hold < 0) {
            if (hold < 0)
                hold = 0.04f;
            //input.KeysProcessed[i] = true;
            auto& states = GetStates();
            OnChange();
            return true;
        }
    }
    return false;
}
void TextBox::ProcessInput(float dt, InputArgs_S& input) {

    int& pos = TextData.CurIdx;
    if (pos < 0) return;
    bool shift = false;
    auto avChars = filter();
    if (input.Mode && GLFW_MOD_SHIFT) { shift = true; }
    for (int i = 0; i < avChars.size(); i++) {
        auto& item = charMap.Entries[avChars[i]];
        if (Check(dt, input, item.code)) {
            if (shift) { AddChar(pos, item.shift); pos++; } else { AddChar(pos, item.noShift); pos++; }
        }
    }
    if (Check(dt, input, GLFW_KEY_ENTER)) { AddChar(pos, '\n'); pos++; }
    if (Check(dt, input, GLFW_KEY_BACKSPACE) && pos > 0) { input.KeysProcessed[259] = false; RemoveChar(pos - 1); if (pos > 0) pos--; }
    if (Check(dt, input, GLFW_KEY_LEFT) && pos > 0) { if (pos > 0) { pos--; } }
    if (Check(dt, input, GLFW_KEY_RIGHT)) { if (pos < TextData.GetEnd()) { pos++; } }
}

bool TextBox::TryChangeActivity(bool value) {
    States.Active = value;
    if (value == false) {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        ClearColor(1);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        OnChange();
    }
    return true;
}


// ********************************************** NUMBERBOX *******************************************************
std::vector<int>& NumberBox::filter() {
    static std::vector<int> chars = { 0,1,2,3,4,5,6,7,8,9 };
    return chars;
}
static void numberbox_text_changed(sender_ptr sender, TextChangeEventArgs* e, params_t params) {
    NumberBox* this_v = (NumberBox*)sender;
    this_v->ParseNumber(e->text_value);
    e->handled = true;
}
void NumberBox::ParseNumber(std::string text) {
    float val = utl::safe_stof(text);
    old_number = Number.Get();
    Number.value[0] = val;
    std::string str;
    if (IsDecimal) {
        str = utils::to_string(val);
        if ((int)val == val)hasDot = false;
        else hasDot = true;
    } else { str = { std::to_string((int)val) }; }
    OnNumberChange();
    if (!e_NumberChange.event_args->handled) {
        SetText(str);
    } else OnChange();
}
void NumberBox::OnNumberChange() {
    e_NumberChange.event_args->number_change = Number.Get() - old_number;
    e_NumberChange.event_args->number_value = Number.Get();
    e_NumberChange.event_args->handled = false;
    e_NumberChange.Invoke();
}
void NumberBox::Init() {
    TextBox::Init();
    params_t pars;
    e_TextChange.Add(this, numberbox_text_changed, pars);
    float val = 0.7f;
    m_colorPar = { val,val,val,1 };
}

void NumberBox::ProcessInput(float dt, InputArgs_S& input) {
    int& pos = TextData.CurIdx;
    if (pos < 0) return;
    bool shift = false;
    auto& avChars = filter();
    for (int i = 0; i < avChars.size(); i++) {
        auto& item = charMap.Entries[avChars[i]];
        if (Check(dt, input, item.code)) {
            AddChar(pos++, item.noShift);
        }
    }
    auto item = charMap.Entries[44];// dot
    if (!utl::has_dot(m_text) && Check(dt, input, item.code)) { AddChar(pos, item.noShift); pos++; }
    item = charMap.Entries[10];// minus
    if (pos == 0 && Check(dt, input, item.code)) { AddChar(pos, item.noShift); pos++; if (m_text[pos] == '.') { AddChar(pos, '0'); pos++; } }
    if (Check(dt, input, GLFW_KEY_BACKSPACE) && pos > 0) { input.KeysProcessed[259] = false; pos--; RemoveChar(pos); if (pos < 0) pos = 0; }
    if (Check(dt, input, GLFW_KEY_LEFT) && pos > 0) { if (pos > 0) { pos--; } }
    if (Check(dt, input, GLFW_KEY_RIGHT)) { if (pos < TextData.GetEnd()) { pos++; } }
}

// ********************************************** IntNumberBox *******************************************************
std::vector<int>& IntNumberBox::filter() {
    static std::vector<int> chars = { 0,1,2,3,4,5,6,7,8,9 };
    return chars;
}
void IntNumberBox::OnNumberChange() {}
static void IntNumberBox_text_changed(sender_ptr sender, TextChangeEventArgs* e, params_t params) {
    IntNumberBox* this_v = (IntNumberBox*)sender;
    this_v->SetText(e->text_value);
}
void IntNumberBox::Init() {
    TextBox::Init();
    params_t pars;
    e_TextChange.Add(this, IntNumberBox_text_changed, pars);
    float val = 0.7f; m_colorPar = { val,val,val,1 };
}

void IntNumberBox::ProcessInput(float dt, InputArgs_S& input) {
    int& pos = TextData.CurIdx;
    if (pos < 0) return;
    bool shift = false;
    auto& avChars = filter();
    for (int i = 0; i < avChars.size(); i++) {
        auto& item = charMap.Entries[avChars[i]];
        if (Check(dt, input, item.code)) {
            AddChar(pos++, item.noShift);
        }
    }
    if (Check(dt, input, GLFW_KEY_BACKSPACE) && pos > 0) { input.KeysProcessed[259] = false; pos--; RemoveChar(pos); if (pos < 0) pos = 0; }
    if (Check(dt, input, GLFW_KEY_LEFT) && pos > 0) { if (pos > 0) { pos--; } }
    if (Check(dt, input, GLFW_KEY_RIGHT)) { if (pos < TextData.GetEnd()) { pos++; } }
}

// ********************************************** CLICKER *******************************************************
void Clicker::Init() {
    Framebuf::Init();
    //Pick.FB = new PickFramebuf();
    btn_up_cb = new MouseClickData();
    btn_up_cb->callback = release_callback;
    btn_up_cb->sender = this;
}
PixelInfo Clicker::PickFunc(float xNew, float yNew) {
    auto& pInfo = Pick.pInfo;
    auto& states = GetStates();
    DrawObj* child = Pick.PickedObj;
    if (rmtd::MouseRay.Hover) {
        if (child != nullptr)
            child->PickAt(xNew, yNew);
    } else {
        if (child == nullptr) {
            WindowBase* win = rmtd::CurWin;
            Point2D<float> s_pos = win->cursor_pos / win->m_size_scale;
            Point2D<float> s_change = win->cursor_pos_change / win->m_size_scale;
            e_MousePress.event_args->cursor_pos_value = s_pos;
            e_MousePress.event_args->cursor_pos_change = s_change;
            e_MouseClick.QInvoke();
            e_MousePress.QInvoke();
            e_MouseRelease.QInvoke();
            Dispatcher::MouseReleaseQ.push_back(btn_up_cb);
            IsPressed = true;
            states.Active = true;
            OnChange();
            rmtd::CurWin->ActiveObjs.insert(this);

        } else if (child->Pick.Pickable)
            child->PickAt(xNew, yNew);
    }
    return pInfo;
}
void Clicker::Prepare() {
    Framebuf::Prepare();
}
void Clicker::Render() {
    m_shader->Use();
    if (IsPressed) m_shader->SetBool("isSelected", true);
    if (States.Hovered) m_shader->SetBool("isHovered", true);
    Control::Render();
    if (IsPressed) m_shader->SetBool("isSelected", false);
    if (States.Hovered) m_shader->SetBool("isHovered", false);
}
// ********************************************** BUTTON *******************************************************
void Button::Init() {
    Clicker::Init();
    Content = new Label(Name + "_label", 0.f, 0.f, 0.f, 0.f);
    Content->m_scalePar = 0.7f;
    Content->Size.Set("button_content_rel", { 1, 1 }, MMode::Rel);
    Content->Size.Set("button_content_abs", { 0, 0 }, MMode::Abs);
    Content->VAlign = VerAlign::Stretch;
    Content->HAlign = HorAlign::Stretch;
    Content->m_clear_color_base = m_clear_color_base;
    Content->m_colorPar = rmtd::Colors["button_fg_inactive"];
    Content->Init();
    AddChild(Content);
}
void Button::Prepare() {
    Framebuf::Prepare();
    Content->BeginPrepare();
    BeginRender(false);
    Content->Render();
    EndRender();
}
// ********************************************** CheckBox *******************************************************

PixelInfo CheckBox::PickFunc(float xNew, float yNew) {
    auto& pInfo = Pick.pInfo; auto& states = GetStates();
    if (rmtd::MouseRay.Hover) { e_MouseHover.Invoke();}
    else {auto args = new MouseClickEventArgs();
        e_MouseClick.Invoke(args);
        Toggle();
        OnChange();}
    return pInfo;}
void CheckBox::Toggle() {
    IsPressed = !IsPressed; IsEnabled.Set(IsPressed);
    Content->m_colorPar = IsPressed ? glm::vec4{ 0, 1, 1, 1 } : glm::vec4{ 1, 1, 1, 1 };
    OnToggle();}
void CheckBox::OnToggle() {
    auto e = new CheckBoxEventArgs();
    e->Value = *IsEnabled.value; e_Toggle.Invoke(e);}
void CheckBox::Prepare() {
    Framebuf::Prepare(); Content->BeginPrepare();
    BeginRender(false); Content->Render(); EndRender();}

void CheckBox::SetEnable(bool newVal) {
    IsPressed = newVal;
    IsEnabled.Set(IsPressed);
    Content->m_colorPar = IsPressed ? glm::vec4{ 0, 1, 1, 1 } : glm::vec4{ 1, 1, 1, 1 };
}

void CheckBox::Init() {
    Clicker::Init();
    Content = new Label(Name + "CheckBox", 0.f, 0.f, 0.f, 0.f);
    Content->Size.Set("CheckBox_content_rel", { 1, 1 }, MMode::Rel);
    Content->VAlign = VerAlign::Stretch;
    Content->HAlign = HorAlign::Stretch;
    Content->m_colorPar = { 1,1,1,1 };
    Content->m_scalePar = 1;
    Content->m_clear_color_base = m_clear_color_base;
    Content->Init();
    Content->SetText("");
    AddChild(Content);
}

void CheckBox::Render() {
    m_shader->Use();
    if (States.Hovered) m_shader->SetBool("isHovered", true);
    Control::Render();
    if (States.Hovered) m_shader->SetBool("isHovered", false);
}
// ********************************************** Expander *******************************************************
void Collapser::Init() {
    CheckBox::Init();
    HAlign = HorAlign::Right;
    Content->SetText("v");
    Content->m_colorPar = { 1,1,1,1 };
    Content->m_scalePar = 1;
}
void Collapser::Render() {
    CheckBox::Render();
}
void Collapser::Toggle() {
    CheckBox::Toggle();
    Content->SetText(IsPressed ? "^" : "v");
}
void Collapser::SetEnable(bool newVal) {
    CheckBox::SetEnable(newVal);
    Content->SetText(IsPressed ? "^" : "v");
}
// ********************************************** NumBox1 *******************************************************
struct NBCPars {
    int mod;
    float* step;
};
static void num_btn_click(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    NumBox1* nb = (NumBox1*)params[0];
    NBCPars* pars = (NBCPars*)params[1];
    float change = *(pars->step) * pars->mod;
    nb->ParseNumberChange(change);
    e->handled = true;
}
static void nb1_numberbox_changed_callback(sender_ptr sender, NumberChangeEventArgs* e, params_t params) {
    NumBox1* nb = (NumBox1*)params[0];
    nb->ParseNumber(e->number_value);
    e->handled = true;
}
void NumBox1::ParseNumber(float val) {
    e_ValueChange.event_args->number_change = val - m_count[0];
    e_ValueChange.event_args->number_value = val;
    e_ValueChange.event_args->handled = false;
    m_count[0] = val;
    e_ValueChange.Invoke();
    if (!e_ValueChange.event_args->handled) {
        SetNumber(val);
    }
}
void NumBox1::Init() {
    Control::Init();
    float btn_size_x = 20;

    NB = new NumberBox(Name + "_nb", 0.f, 0.f, 0.f, 0.f);
    NB->Size.Set("numbox1_nb_rel", { 1,1 }, MMode::Rel);
    NB->Size.Set("numbox1_nb_abs", { -btn_size_x,0 }, MMode::Abs);
    NB->m_clear_color_base = { 0.2,0.2,0.2,1 };
    NB->VAlign = VerAlign::Stretch;
    NB->Init();
    AddChild(NB);

    Button* plus_btn = new Button(Name + "_plus_btn", 0.f, 0.f, 0.f, 0.f);
    plus_btn->Size.Set("numbox1_btn_rel", { 0,0.5 }, MMode::Rel);
    plus_btn->Size.Set("numbox1_btn_abs", { btn_size_x,0 }, MMode::Abs);
    plus_btn->Align(HorAlign::Right, VerAlign::Top);
    plus_btn->Init();
    plus_btn->Content->Name = plus_btn->Name + "_content";
    plus_btn->Content->SetText("+");
    PB = plus_btn;
    AddChild(plus_btn);

    Button* mins_btn = new Button(Name + "_mins_btn", 0.f, 0.f, 0.f, 0.f);
    mins_btn->Size.Set("numbox1_btn_rel", { 0,0.5 }, MMode::Rel);
    mins_btn->Size.Set("numbox1_btn_abs", { btn_size_x,0 }, MMode::Abs);
    mins_btn->Align(HorAlign::Right, VerAlign::Bottom);
    mins_btn->Init();
    mins_btn->Content->Name = mins_btn->Name + "_content";
    mins_btn->Content->SetText("-");
    AddChild(mins_btn);
    if (TextRend) {
        NB->SetTextRend(*TextRend);
        plus_btn->Content->SetTextRend(*TextRend);
        mins_btn->Content->SetTextRend(*TextRend);
    }

    params_t pars;
    pars.push_back(this);
    pars.push_back(new NBCPars{ 1, &step });
    plus_btn->e_MouseClick.Add(this, num_btn_click, pars);
    params_t pars2;
    pars2.push_back(this);
    pars2.push_back(new NBCPars{ -1, &step });
    mins_btn->e_MouseClick.Add(this, num_btn_click, pars2);

    m_count = NB->Number.value;
    NB->e_NumberChange.Add(this, nb1_numberbox_changed_callback, { this });
}
PixelInfo NumBox1::PickFunc(float xNew, float yNew) {
    auto& pInfo = Pick.pInfo;
    auto& states = GetStates();
    DrawObj* child = Pick.PickedObj;

    if (rmtd::MouseRay.Hover) {
        e_MouseHover.Invoke();
        rmtd::MouseRay.trace.push_back(this);
        if (child != nullptr)
            child->PickAt(xNew, yNew);
    } else {
        IsPressed = true;
        states.Active = true;
        rmtd::CurWin->ActiveObjs.insert(this);
        if (child == nullptr) {
            return pInfo;
        }
        if (child->Pick.Pickable)
            child->PickAt(xNew, yNew);

        OnChange();
    }
    return pInfo;
}

void NumBox1::Prepare() {
    Framebuf::Prepare();
}

void NumBox1::Render() {
    Framebuf::Render();
}
// ********************************************** Placement *******************************************************
void Placement_S::Place() {
    auto unfixed = m_owner->ActualPos - Margin_LT; // od M_LT do kraja abs
    Val = (unfixed) / Scale; // nameti na rel
    if (Val.Trim(0, 1)) {  // ogranici na priznato rel
        // - unfixed namesti na total = M_LT..pa se onda dopuni priznati abs sa Get()
        //m_owner->Pos.Add("base_abs", Get() - unfixed);
        m_owner->Pos.Add("drag_position_abs", Get() - unfixed);
    }
}
void Placement_S::Compute() {
    Scale = m_owner->Parent->LogicalSize.derefC() - m_owner->ActualSize - Margin_LT - Margin_RB;
}
// ********************************************** DRAGGER *******************************************************
void begin_drag_callback(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    Dragger* this_v = (Dragger*)sender;
    WindowBase* win = this_v->win;
    Point2D<float> scaled = win->cursor_pos / win->m_size_scale;
    this_v->StartPos = this_v->Target->ActualPos;
    this_v->DragOffset = scaled - this_v->Target->ActualPos;
    this_v->OnDragBegin();
}
void end_drag_callback(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    Dragger* this_v = (Dragger*)sender;
    WindowBase* win = this_v->win;
    Point2D<float> scaled = win->cursor_pos / win->m_size_scale;
    this_v->Target->Pos.MergeInto("base_abs", "drag_position_abs", 1);
    this_v->Target->Pos.Set("drag_position_abs", { 0,0 });
    this_v->OnDragEnd();
}
void drag_callback(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    Dragger* this_v = (Dragger*)sender;
    WindowBase* win = this_v->win;
    Point2D<float> scaled = win->cursor_pos / win->m_size_scale;
    this_v->Position(scaled);
    this_v->OnDrag();
}
void Dragger::Position(Point2D<float> newOffsetPos) {
    LastPos = Target->ActualPos;
    Point2D<float> totalExtra{ 0,0 };
    if (HDrag)
        totalExtra.X = newOffsetPos.X - StartPos.X - DragOffset.X;
    if (VDrag)
        totalExtra.Y = newOffsetPos.Y - StartPos.Y - DragOffset.Y;
    cout << newOffsetPos.X << "," << newOffsetPos.Y << "\n";
    Target->Pos.Set("drag_position_abs", totalExtra);
    Target->FixLayout();
    Target->OnChange();
    /*
    auto win = rmtd::CurWin;
    auto cur = win->cursor_pos;
    auto size = Parent->ActualSize;
    printf("curX = %f\n", cur.X);
    printf("curY = %f\n", cur.Y);
    printf("X = %f\n", ActualPos.X);
    printf("Y = %f\n", ActualPos.Y);
    printf("sizeX = %f\n", size.X);
    printf("sizeY = %f\n", size.Y);
    */
}
void Dragger::AdjustPosition(Point2D<float> rel_pos) {
    Placement.Compute();
    auto new_pos = rel_pos * Placement.Scale;
    Point2D<float> mod{ 1,1 };
    if (!HDrag)
        mod.X = 0;
    if (!VDrag)
        mod.Y = 0;
    auto drag_measure = Target->Pos["drag_position_abs"];
    Point2D<float> drag = { drag_measure.X.val, drag_measure.Y.val };
    new_pos = new_pos + Placement.Margin_LT;
    Target->Pos.Add("base_abs", (new_pos - Target->ActualPos) * mod);
    cout << "boudn to: " << Target->ActualPos.X << "," << Target->ActualPos.Y << "\n";
    //Target->FixLayout();
    ComputePlacement();
    OnDrag();
    Target->OnChange();
}
void Dragger::Measure() {
    ComputePlacement();
    ComputeMeasures();
    OnMeasureChange();
}
void Dragger::Arrange() {
    Framebuf::Arrange();
}
void Dragger::SetTarget(Control* target) {
    Target = target;
    Placement.m_owner = target;
}
void Dragger::OnDrag() {
    e_drag.event_args->pos = ActualPos;
    e_drag.event_args->pos_cng = ActualPos - LastPos;
    e_drag.event_args->handled = false;
    e_drag.Invoke();
}
void Dragger::OnDragEnd() {
    e_drag_end.event_args->pos = ActualPos;
    e_drag_end.event_args->pos_cng = ActualPos - StartPos;
    e_drag_end.event_args->handled = false;
    e_drag_end.Invoke();
}
void Dragger::OnDragBegin() {
    e_drag_begin.event_args->pos = ActualPos;
    e_drag_begin.event_args->pos_cng = { 0,0 };
    e_drag_begin.event_args->handled = false;
    e_drag_begin.Invoke();
}
void Dragger::ComputeBounds() {
    Framebuf::ComputeBounds();
}
void Dragger::SetAtVal() {
    Pos.Set("drag_position_abs", Placement.Get() - Placement.Margin_LT);
    FixLayout();
    OnDrag();
    OnChange();
}
void Dragger::Move(Point2D<float> change) {
    Pos.Add("base_abs", change);
    FixLayout();
    OnDrag();
    OnChange();
}
void Dragger::MoveRel(Point2D<float> change) {
    if (!HDrag)
        change.X = 0;
    if (!VDrag)
        change.Y = 0;
    Placement.Compute();
    auto old = Placement.Get();
    Placement.Val += change;
    change = Placement.Get();
    Pos.Add("base_abs", -old + change);
    ComputeBounds();
    OnDrag();
    OnChange();
}
void Dragger::ComputePlacement() {
    Placement.Compute();
    Placement.Place();
    Target->ComputeBounds();
    Target->OnMeasureChange();
}
void Dragger::Init() {
    Clicker::Init();
    e_MouseClick.Add(this, begin_drag_callback, {});
    e_MouseRelease.Add(this, end_drag_callback, {});
    e_MousePress.Add(this, drag_callback, {});
}
void Dragger::Prepare() {
    Clicker::Prepare();
}

// ********************************************** SLIDER *******************************************************
struct SBCPars {
    int direction = 1;
    ScrollBar* target = nullptr;
};
void ScrollBar::SetOrientation(Orientation_S ori) {
    if (ori != Orientation && ori != Orientation_S::NONE)
        change.Swap();
    Orientation = ori;
    switch (ori) {
        case Orientation_S::NONE:
            break;
        case Orientation_S::Horizontal:
            PB->Align(HorAlign::Right, VerAlign::Stretch);
            MB->Align(HorAlign::Left, VerAlign::Stretch);
            DRG->Align(HorAlign::None, VerAlign::Stretch);
            PB->Content->SetText(">");
            MB->Content->SetText("<");
            DRG->HDrag = true;
            DRG->VDrag = false;
            DRG->Size.Set("base_abs", { 4 * bar_size, 0 });
            DRG->Pos.Set("base_abs", { 0, bar_size }, MMode::Abs);
            DRG->Placement.Margin_RB = { bar_size, 0 };
            DRG->Placement.Margin_LT = { bar_size, 0 };
            Align(HorAlign::None, VerAlign::Bottom);
            Size.Set("scroll_stretch", { 1, 0 });
            Size.Set("scroll_abs", { 0, bar_size }, MMode::Abs);

            break;
        case Orientation_S::Vertical:
            PB->Align(HorAlign::Stretch, VerAlign::Bottom);
            MB->Align(HorAlign::Stretch, VerAlign::Top);
            DRG->Align(HorAlign::Stretch, VerAlign::None);
            PB->Content->SetText("v");
            MB->Content->SetText("^");
            DRG->HDrag = false;
            DRG->VDrag = true;
            DRG->Size.Set("base_abs", { 0, 4 * bar_size });
            DRG->Pos.Set("base_abs", { bar_size,0 }, MMode::Abs);
            DRG->Placement.Margin_RB = { 0, bar_size };
            DRG->Placement.Margin_LT = { 0, bar_size };
            Align(HorAlign::Right, VerAlign::None);
            Size.Set("scroll_stretch", { 0, 1 });
            Size.Set("scroll_abs", { bar_size, 0 }, MMode::Abs);
            break;
        default:
            break;
    }
    PB->States.Changed = true;
    MB->States.Changed = true;
    DRG->States.Changed = true;
    OnChange();
    std::string key = "sa_sb_abs";
    PB->Size.Set(key, { bar_size, bar_size });
    MB->Size.Set(key, { bar_size, bar_size });
}
void ScrollBar::AdjustPosition(Point2D<float> rel_pos) {
    DRG->AdjustPosition(rel_pos);
    LValC = DRG->Placement.Val - OldVal;
    OldVal = DRG->Placement.Val;
}
static void scrollbar_btn_click(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    SBCPars* pars = (SBCPars*)params[0];
    ScrollBar* this_v = (ScrollBar*)pars->target;
    this_v->DRG->Move(this_v->change * pars->direction);
    this_v->OnScroll();
}
static void scrollbar_drag(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    SBCPars* pars = (SBCPars*)params[0];
    ScrollBar* this_v = (ScrollBar*)pars->target;
    this_v->OnScroll();
}
void ScrollBar::Init() {
    Control::Init();
    Size.Set("scroll_stretch", { 0, 0 }, MMode::Rel);
    Size.Set("scroll_abs", { 0, 0 }, MMode::Abs);
    float test = 0;
    Button* plus_btn = new Button(Name + "_plus_btn", 0.f, 0.f, test, test);
    plus_btn->Init();
    plus_btn->Content->Name = plus_btn->Name + "_content";
    PB = plus_btn;
    AddChild(plus_btn);

    Button* mins_btn = new Button(Name + "_mins_btn", 0.f, 0.f, test, test);
    mins_btn->Init();
    mins_btn->Content->Name = mins_btn->Name + "_content";
    MB = mins_btn;
    AddChild(mins_btn);

    Dragger* drg = new Dragger(Name + "_scroll_drag", 0, 0, bar_size, bar_size);
    drg->Init();
    DRG = drg;
    AddChild(drg);

    params_t pars;
    pars.push_back(new SBCPars{ 1, this });
    plus_btn->e_MouseClick.Add(this, scrollbar_btn_click, pars);
    params_t pars2;
    pars2.push_back(new SBCPars{ -1, this });
    mins_btn->e_MouseClick.Add(this, scrollbar_btn_click, pars2);
    params_t pars3;
    pars3.push_back(new SBCPars{ -1, this });
    DRG->e_MousePress.Add(this, scrollbar_drag, pars3);

    static const float val = 0.f;
    GetActiveTex().m_clear_color = { val,val,val,0.5f };
    SetOrientation(Orientation);
}
void ScrollBar::OnScroll() {
    auto args = new ScrollEventArgs();
    args->scroll_change = LValC = DRG->Placement.Val - OldVal;
    args->scroll_value = OldVal = DRG->Placement.Val;
    e_Scroll.Invoke(args);
}
void ScrollBar::Prepare() {
    Framebuf::Prepare();
}
void ScrollBar::Render() {
    Control::Render();
}
// ********************************************** FixedStackPanel *******************************************************
void FixedStackPanel::Measure() {
    Point2D<float> mod = { 0,1 }, pos{}, MeasuredSize{};
    bool isVert = Orientation == Orientation_S::Vertical;
    if (!isVert) mod.Swap();
    for (auto child : Childs)
        child->Visibility = VisibilityE::Collapsed;
    for (int i = beginIndex; i < Childs.size(); i++) {
        auto child = Childs[i];
        auto child_size = child->ActualSize * mod;
        MeasuredSize += child_size;
        if (isVert) { if (MeasuredSize.Y > ActualSize.Y) { break; } } else { if (MeasuredSize.X > ActualSize.X) { break; } }
        child->Pos.Clear();
        child->Pos.Add("StackPanelT_computed_abs", pos, MMode::Abs);
        child->Visibility = VisibilityE::Visible;
        pos += child_size;
    }
}

// ********************************************** ViewSpace *******************************************************
static void vs_proxy_measure_changed_callback(sender_ptr sender, EventArgs* e, params_t params) {
    ViewSpace* this_v = (ViewSpace*)params[0];
    if (this_v->Proxy != nullptr)//&& SpaceSize != Proxy->ActualSize)
        this_v->SetSpaceSize(this_v->Proxy->ActualSize);
    this_v->OnChange();
}
void ViewSpace::SetLinked() {
    Measure(); Arrange();
    SetSpaceSize(Proxy->ActualSize);
    UpdateCords();
    Framebuf::SetLinked();
    MoveView({}, {});
}
void ViewSpace::BeginLink() {
    Framebuf::BeginLink();
    Proxy->e_MeasureChanged.Add(this, vs_proxy_measure_changed_callback, { this });
    Proxy->Link(this); Proxy->BeginLink();
}
void ViewSpace::Measure() {
    if (!Linked) BeginLink();
    Framebuf::Measure();
    //Proxy->Measure();
}
void ViewSpace::Arrange() {
    States.Changed = true;
    Framebuf::ComputeBounds();
    Framebuf::ResizeTexs(ActualSize);
}
void ViewSpace::UnlinkChild(DrawObj2D* child) {
    child->e_MeasureChanged.Remove(this, vs_proxy_measure_changed_callback);
}
void ViewSpace::SetProxy(Control* proxy) {
    RemoveProxy();
    Proxy = proxy;
    AddChild(proxy);
    float w = 0, h = 0;
    if (HorRM == ResizeMode::Fixed)w = 1;
    if (VerRM == ResizeMode::Fixed)h = 1;
    Proxy->Size.Set("viewspace_proxy_size_rel", { w, h }, MMode::Rel);
    //StackPanel::unadjustable.insert(Proxy->GUID);
}
void ViewSpace::RemoveProxy() {
    if (Proxy == nullptr) return;
    RemoveChild(Proxy);
    Proxy->Size.Set("viewspace_proxy_size_rel", { 0, 0 }, MMode::Rel);
}
Point2D<float> ViewSpace::IntendedSize() { return LogicalSize.derefC(); }
void ViewSpace::ResizeTexs(Point2D<float> size) {}
void ViewSpace::ComputeBounds() {}

struct dataaanow {
    Point2D<float> ActualSize;
    Point2D<float> SpaceSize;
    Point2D<float> ViewSizeRatio;
    Point2D<float> SPosRel;
    Point2D<float> SPosRel01;
    Point2D<float> oldSPosRel01;
    Bounds m_texCords;
};
std::vector<dataaanow> datanow;
void ViewSpace::SetTexCords() {
    m_texCords_old = m_texCords;
    m_texCords.x1 = SPosRel.X;
    m_texCords.y1 = SPosRel.Y;
    m_texCords.x2 = SPosRel.X + ViewSizeRatio.X;
    m_texCords.y2 = SPosRel.Y + ViewSizeRatio.Y;
}
void ViewSpace::SetSpaceSize(Point2D<float> newSize) {
    newSize.X = std::max(ActualSize.X, newSize.X);
    newSize.Y = std::max(ActualSize.Y, newSize.Y);
    if (HorRM == ResizeMode::Fixed) newSize.X = ActualSize.X;
    if (VerRM == ResizeMode::Fixed) newSize.Y = ActualSize.Y;
    auto SPosAbs = SPosRel * SpaceSize;
    SpaceSize = newSize;
    if (!SpaceSize.HasZero()) {
        ViewSizeRatio = ActualSize / SpaceSize;
        SPosRel = SPosAbs / SpaceSize;
    } else ViewSizeRatio = { 1,1 };
    SPosRel.X = std::min(SPosRel.X, 1.0f - ViewSizeRatio.X);
    SPosRel.Y = std::min(SPosRel.Y, 1.0f - ViewSizeRatio.Y);
    OldSPosRel01 = SPosRel01;
    SPosRel01 = SPosRel / (Point2D<float>{ 1, 1 } - ViewSizeRatio);
    e_SpaceChanged.event_args->space_pos_rel = SPosRel01;
    e_SpaceChanged.event_args->space_pos_rel_change = SPosRel01 - OldSPosRel01;
    e_SpaceChanged.Invoke();
    LogicalSize.X = &(SpaceSize.X); LogicalSize.Y = &(SpaceSize.Y);
    Framebuf::ResizeTexs(SpaceSize); UpdateTex(); UpdateCords();

    datanow.push_back({
        ActualSize,
        SpaceSize,
        ViewSizeRatio,
        SPosRel,
        SPosRel01,
        OldSPosRel01,
        m_texCords
        }
    );
}
struct datamove123 {
    Point2D<float> ActualSize;
    Point2D<float> SpaceSize;
    Point2D<float> ViewSizeRatio;
    Point2D<float> SPosRel;
    Bounds m_texCords;
    Point2D<float> nvc;
    Point2D<float> factor;
};
std::vector<datamove123> datmoveNow;
int ViewSpace::indexer[] = { 0, 3, 0, 1, 2, 3, 2, 1 };
void ViewSpace::MoveView(Point2D<float> nvc, Point2D<float> mod) {
    if (!SpaceSize.HasZero())
        ViewSizeRatio = ActualSize / SpaceSize;
    nvc *= mod;
    auto map_factor = Point2D<float>{ 1,1 } - ViewSizeRatio;
    nvc = nvc * map_factor;
    SPosRel += nvc;
    UpdateTex();
    OnChange();
    datmoveNow.push_back(
        {
            ActualSize,
            SpaceSize,
            ViewSizeRatio,
            SPosRel,
            m_texCords,
            nvc,
            mod,
        }
    );
}
void ViewSpace::UpdateCords() {
    Bounds directed = m_Bounds.Directed();
    float* vert_cord = (float*)&directed;
    float last[20];
    utl::copy_arr<float>(data, last, 20);
    utl::fillBuffer<float>(vert_cord, data, 20, 0, 5, 2, indexer);
    std::cout << last << std::endl << data << std::endl;
    UpdateBuffer(true, false);
}
void ViewSpace::UpdateTex() {
    SetTexCords();
    auto directed = m_texCords.DirectedTex();
    float* tex_cord = (float*)&directed;
    utl::fillBuffer<float>(tex_cord, data, 20, 3, 5, 2, indexer);
    UpdateBuffer(false, true);
}
bool ViewSpace::PickAt(float x, float y) {
    dpch::TraceIn(this);
    float xNew, yNew;
    std::tie(xNew, yNew) = utl::RemapPoint(x, y, m_Bounds);
    Bounds bnd = { 0,0,0,0 };
    float sx = SpaceSize.X / ActualSize.X;
    float sy = SpaceSize.Y / ActualSize.Y;
    bnd.x2 = sx;bnd.y2 = sy;
    utl::m01To11(bnd);
    float xSNew, ySNew;
    std::tie(xSNew, ySNew) = utl::RemapPoint(xNew, yNew, bnd);
    xSNew += SPosRel.X; ySNew += SPosRel.Y;
    PixelInfo pInfo = BeginPickFunc(xSNew, ySNew);
    dpch::TraceOut();
    return true;
}
void ViewSpace::UpdateBuffer(bool vert, bool tex) {
    if (!(vert || tex)) return;
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), &data);
    glBindVertexArray(VAO2);
    if (vert) {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    }
    if (tex) {
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void ViewSpace::Init() {
    Framebuf::Init();
    SetSpaceSize(ActualSize);
    Bounds dc = {};
    Bounds bn_tx = m_texCords.DirectedTex();
    Bounds dt = bn_tx;
    float tmp_data[] = {
        dc.x1, dc.y2, 0.0f, dt.u1, dt.v2,
        dc.x1, dc.y1, 0.0f, dt.u1, dt.v1,
        dc.x2, dc.y2, 0.0f, dt.u2, dt.v2,
        dc.x2, dc.y1, 0.0f, dt.u2, dt.v1,
    }; for (int i = 0; i < 20; i++) data[i] = tmp_data[i];
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_DYNAMIC_DRAW);
    glBindVertexArray(VAO2);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void ViewSpace::Render() {
    this->m_shader->Use();
    glActiveTexture(GL_TEXTURE0);
    this->Texs[activeTex]->Bind();
    glBindVertexArray(VAO2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
// ********************************************** ToolBarBase *******************************************************
struct TBBCBPars {
    FixedStackPanel* target;
    int change;
};
static void toolbarbase_btn_click_callback(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    TBBCBPars* pars = (TBBCBPars*)params[0];
    auto& index = pars->target->beginIndex;
    index += pars->change;
    index = std::max(std::min(index, (int)(pars->target->Childs.size() - 1)), (int)0);
    pars->target->FixLayout();
    pars->target->OnChange();
}
#define TAB_H 20
DrawObj2D* ToolBarBase::AddChild(DrawObj2D* dObj) { return TabZone->AddChild(dObj); }
void ToolBarBase::AddChilds(std::vector<DrawObj2D*> dObjs) { TabZone->AddChilds(dObjs); }
void ToolBarBase::InnerAddChild(DrawObj2D* dObj) { Framebuf::AddChild(dObj); }
void ToolBarBase::Init() {
    Control::Init(); float btn_width = 10;
    Button* btn_left = new Button(Name + "_btn_left", 0, 0, btn_width, TAB_H);
    btn_left->Init(); Left = btn_left; InnerAddChild(btn_left);
    btn_left->Content->Name = btn_left->Name + "_content";
    btn_left->Content->SetText("<");
    Button* btn_right = new Button(Name + "_btn_right", 0, 0, btn_width, TAB_H);
    btn_right->Init(); Right = btn_right; InnerAddChild(btn_right);
    btn_right->Content->Name = btn_right->Name + "_content";
    btn_right->Content->SetText(">");
    btn_right->HAlign = HorAlign::Right;
    FixedStackPanel* content = new FixedStackPanel(Name + "_Content");
    content->Orientation = Orientation_S::Horizontal;
    content->Init(); TabZone = content; InnerAddChild(content);
    content->HAlign = HorAlign::None;
    content->Size.Add("tbb_content_size_rel", { 1,0 }, MMode::Rel);
    content->Size.Add("tbb_content_size_abs", { -2 * btn_width,TAB_H }, MMode::Abs);
    content->Pos.Add("tbb_content_pos_abs", { btn_width,0 }, MMode::Abs);
    btn_left->e_MouseClick.Add(this, toolbarbase_btn_click_callback, { new TBBCBPars{ content,-1 } });
    btn_right->e_MouseClick.Add(this, toolbarbase_btn_click_callback, { new TBBCBPars{ content,1 } });
}
// ********************************************** ToolBar *******************************************************
void ToolBar::Init() {
    ToolBarBase::Init();
}
// ********************************************** TabHeader *******************************************************
void TabHolder::Init() {
    ToolBarBase::Init();
}
// ********************************************** TabViewer *******************************************************
struct TVECBPars {
    TabViewer* target;
    TabViewerEntry* source;
};
void tabclick_callback(sender_ptr* sender, MouseClickEventArgs* e, params_t params) {
    TVECBPars* pars = (TVECBPars*)params[0];
    auto target = pars->target;
    auto source = pars->source;
    if (target->activeEntry != nullptr) {
        auto btn = target->activeEntry->TabCard;
        btn->Content->m_colorPar = rmtd::Colors["button_fg_inactive"];
        btn->Content->GetActiveTex().m_clear_color = rmtd::Colors["button_bg_inactive"];
        btn->Content->TextData.ToRender = true;
        btn->OnChange();
    }
    target->activeEntry = source;
    target->Content->ClearChilds();
    target->Content->AddChild(source->Content);
    {
        auto btn = target->activeEntry->TabCard;
        btn->Content->m_colorPar = rmtd::Colors["button_fg_active"];
        btn->Content->GetActiveTex().m_clear_color = rmtd::Colors["button_bg_active"];
        btn->Content->TextData.ToRender = true;
        btn->OnChange();}
    source->Content->OnChange();
}
TabViewerEntry* TabViewer::AddTab(DrawObj2D* dObj) {
    TabViewerEntry* newEntry = new TabViewerEntry();
    Button* tab_card = new Button(Name + "_tab_card", 0, 0, 50, TAB_H);
    tab_card->Init();
    tab_card->Content->Name = tab_card->Name + "_content";
    tab_card->Content->SetText(dObj->Name);
    newEntry->TabCard = tab_card;
    newEntry->Content = dObj;
    tab_card->e_MouseClick.Add(this, tabclick_callback, { new TVECBPars{ this, newEntry } });
    TabBar->TabZone->AddChild(tab_card);
    Entries.push_back(newEntry);
    return newEntry;
}
void TabViewer::Init() {
    Control::Init();
    TabHolder* tab_holder = new TabHolder(Name + "_tabholder", 0, 0, 0, TAB_H);
    tab_holder->Size.Add("tabviewer_tabholder_size_rel", { 1,0 }, MMode::Rel);
    tab_holder->Init(); TabBar = tab_holder; AddChild(tab_holder);
    Control* content = new Control(Name + "_Content");
    content->Init(); Content = content; AddChild(content);
    content->HAlign = HorAlign::Right;
    content->Size.Add("tabviewer_content_size_rel", { 1,1 }, MMode::Rel);
    content->Size.Add("tabviewer_content_size_abs", { 0,-TAB_H }, MMode::Abs);
    content->VAlign = VerAlign::Bottom;
}
void TabViewer::Prepare() {
    Control::Prepare();
}
// ********************************************** SCROLLABLE *******************************************************
struct SAbleCPars {
    bool onX = true;
    Scrollable* target = nullptr;
};
void vs_size_changed_callback(ViewSpace* sender, ViewSpaceEventArgs* e, params_t params) {
    Scrollable* this_v = (Scrollable*)params[0];
    this_v->BSB->AdjustPosition(e->space_pos_rel);
    this_v->RSB->AdjustPosition(e->space_pos_rel);
}
static void scrollbar_scroll(sender_ptr sender, ScrollEventArgs* e, params_t params) {
    SAbleCPars* pars = (SAbleCPars*)params[0];
    Scrollable* this_v = (Scrollable*)pars->target;
    if (pars->onX) { this_v->VS->MoveView(e->scroll_change, { 1,0 }); } else { this_v->VS->MoveView(e->scroll_change, { 0,1 }); }
}
void Scrollable::MoveView() {}
void Scrollable::Link(Framebuf* parent) {
    Control::Link(parent);
}
void Scrollable::Unlink() {
    Control::Unlink();
}
void Scrollable::SetViewSpace(ViewSpace* vs) {
    vs->Size.Clear(); vs->Pos.Clear();
    vs->Size.Set("sable_cont_rel", { 1,1 }, MMode::Rel);
    vs->Size.Set("sable_cont_abs", { -bar_size, -bar_size }, MMode::Abs);
    VS = vs; AddChild(vs);
}
void Scrollable::SetProxy(Control* proxy_p) {
    VS->SetProxy(proxy_p);
    Proxy = proxy_p;
    OnChange();
}
DrawObj2D* Scrollable::ProxyAddChild(DrawObj2D* dObj) {
    return Proxy->AddChild(dObj);
    OnChange();
}
void Scrollable::ProxyAddChilds(std::vector<DrawObj2D*> dObjs) {
    Proxy->AddChilds(dObjs);
    OnChange();
}
void Scrollable::ProxyClearChilds() {
    Proxy->ClearChilds();
    OnChange();
}
void Scrollable::Init() {
    Control::Init();

    ScrollBar* right_sb = new ScrollBar(Name + "_right_sb", 0.f, 0.f, 0.f, 0.f);
    right_sb->bar_size = bar_size;
    right_sb->Init();
    right_sb->Size.Add("sable_abs", { 0, -bar_size }, MMode::Abs);
    right_sb->SetOrientation(Orientation_S::Vertical);
    RSB = right_sb;
    AddChild(right_sb);

    ScrollBar* bottom_sb = new ScrollBar(Name + "_bottom_sb", 0.f, 0.f, 0.f, 0.f);
    bottom_sb->bar_size = bar_size;
    bottom_sb->Init();
    bottom_sb->Size.Add("sable_abs", { -bar_size, 0 }, MMode::Abs);
    bottom_sb->SetOrientation(Orientation_S::Horizontal);
    BSB = bottom_sb;
    AddChild(bottom_sb);

    ViewSpace* view_space = new ViewSpace(Name + "_view_space", 0.f, 0.f, 0.f, 0.f);
    view_space->Size.Set("sable_cont_rel", { 1,1 }, MMode::Rel);
    view_space->Size.Set("sable_cont_abs", { -bar_size, -bar_size }, MMode::Abs);
    view_space->Init();
    VS = view_space;
    Proxy = VS;
    AddChild(view_space);

    VS->e_SpaceChanged.Add(this, vs_size_changed_callback, { this });
    params_t pars;
    pars.push_back(new SAbleCPars{ false, this });
    right_sb->e_Scroll.Add(this, scrollbar_scroll, pars);
    params_t pars2;
    pars2.push_back(new SAbleCPars{ true, this });
    bottom_sb->e_Scroll.Add(this, scrollbar_scroll, pars2);
}
// ********************************************** MOVABLE *******************************************************
void Movable::SetContent(Control* cont) {
    ClearChilds();
    cont->Size.Clear();
    cont->Pos.Clear();
    cont->Size.Set("bar_rel", { 1,1 }, MMode::Rel);
    cont->Size.Add({ 0, -bar_size }, MMode::Abs);
    cont->Align(HorAlign::Stretch, VerAlign::Bottom);
    Content = cont;
    Framebuf::AddChild(cont);
}
void Movable::FreeDockLock() {
    PSnap.Save(Target);
    Target->Align(HorAlign::None, VerAlign::None);
    auto posCng = DragOffset;
    float factor = (Target->ActualSize - Target->Size["stretch"]).X / Target->ActualSize.X;
    Target->Pos.MergeInto("base_abs", "stretch", 1);
    posCng.X -= DragOffset.X * factor;//+ActualPos.X;
    posCng.Y = 0;//- Pos["stretch"].Y.val;
    Target->Pos.Add("base_abs", posCng);
    Target->Refresh();
}
void Movable::RevertDockLock() {
    Target->Align(&PSnap.m_align);
}
void Movable::Link(Framebuf* parent) {
    InitDone = true;
    Framebuf::Link(parent);
}
DrawObj2D* Movable::AddChild(DrawObj2D* dObj) {
    if (!InitDone) Framebuf::AddChild(dObj);
    else { SetContent((Control*)dObj); }
    return nullptr;
}
void Movable::AddChilds(std::vector<DrawObj2D*> dObjs) {
    if (!InitDone)Framebuf::AddChilds(dObjs);
    else { if (!dObjs.empty())SetContent((Control*)dObjs.back()); }
}
void movable_docking_end_new_dock_callback(Dragger* sender, DragEventArgs* e, params_t params) {
    auto& dd = rmtd::DragData;
    Movable* this_v = (Movable*)params[0];\
    Control* target = this_v->Target;
    WindowBase* win = (WindowBase*)params[1];
    this_v->e_drag_end.Remove(this_v, movable_docking_end_new_dock_callback);
    win->GUI->RemoveChild(dd.dummy);
    target->Align(dd.dummy);
    target->OnChange();
    target->RemoveSelf();
    win->GUI->AddChild(target);
    //target->Refresh();
    dd.dragged = nullptr;
    this_v->docked = true;
}
void movable_docking_callback(WindowBase* sender, MouseMoveEventArgs* e, params_t params) {
    static const float margin = 0.1f;
    auto& dd = rmtd::DragData;
    Movable* this_v = (Movable*)params[0];
    Control* target = this_v->Target;
    if (!e->rel_pos_val.Within(margin, 1-margin)) {
        Dummy2D*& dmy = dd.dummy;
        if (dd.dragged == nullptr) { // add
            dmy->Size.Clear(); dmy->Size.Set("base_abs", target->ActualSize, MMode::Abs);
            dmy->Pos.Clear(); dmy->Pos.Set("base_abs", target->ActualPos, MMode::Abs);
            sender->GUI->AddChild(dmy);
            dd.dragged = this_v;
            this_v->e_drag_end.Add(this_v, movable_docking_end_new_dock_callback, { this_v, sender });
        } else
            int x = 5;
        if (e->rel_pos_val.X < margin) {
            dmy->Align(HorAlign::Left, VerAlign::Stretch);
        } else if (e->rel_pos_val.X > 1 - margin) {
            dmy->Align(HorAlign::Right, VerAlign::Stretch);
        } else if (e->rel_pos_val.Y < margin) {
            dmy->Align(HorAlign::Stretch, VerAlign::Top);
        } else if (e->rel_pos_val.Y > 1 - margin) {
            dmy->Align(HorAlign::Stretch, VerAlign::Bottom);
        }
        dmy->Refresh();
        dmy->OnChange();
    } else if(dd.dragged != nullptr) { // remove
        this_v->e_drag_end.Remove(this_v, movable_docking_end_new_dock_callback);
        sender->GUI->RemoveChild(dd.dummy);
        dd.dragged = nullptr;
    }
}
void movable_docking_start_callback(Dragger* sender, DragEventArgs* e, params_t params) {
    Movable* this_v = (Movable*)params[0];
    rmtd::CurWin->e_mouse_move.Add(this_v, movable_docking_callback, { this_v });
    this_v->docked = false;
    this_v->FreeDockLock();
}
void movable_docking_end_callback(Dragger* sender, DragEventArgs* e, params_t params) {
    Movable* this_v = (Movable*)params[0];
    rmtd::CurWin->e_mouse_move.Remove(this_v, movable_docking_callback);
    //this_v->RevertDockLock();
}
void Movable::Init() {
    Dragger::Init();
    e_drag_begin.Add(this, movable_docking_start_callback, { this });
    e_drag_end.Add(this, movable_docking_end_callback, { this });
}
void Movable::Render() { Control::Render(); }
// ********************************************** TexDisplay *******************************************************
void TexDisplay::SetTarget(uint guid) {
    Target = (DrawObj2D*)rmtd::GetElement(guid);
    if (Target != nullptr && Target->States.DisplayReady)
        DP->CopyTexture(Target, 0);
}
static void texdisplay_update(sender_ptr sender, EventArgs* e, params_t params) {
    sender->Cast<DrawObj2D>()->OnChange();
};
void TexDisplay::Init() {
    Control::Init();
    Framebuf* dspl = new Framebuf(Name + "_texdisplay");
    dspl->Size.Set("texdspl_rel", { 1,1 }, MMode::Rel);
    dspl->Size.Set("base_abs", { 0,-FIELD_H });
    dspl->Init(); DP = dspl; AddChild(dspl);
    NumBox1* nb = new NumBox1(Name + "_nb", 0, 0, 0, 0, FIELD_H);
    nb->Align(HorAlign::Stretch, VerAlign::Bottom);
    nb->SetTextRend(*TextRend);
    nb->Init(); NB1 = nb; AddChild(nb);
    Dispatcher::e_Update.Add(this, texdisplay_update, { }, dspl);
}
// ********************************************** Field *******************************************************
void Field::Init() {
    Control::Init();
    Header = new Control("Header");
    Header->Init();
    Header->Size.Set("prop_field_header_rel", { 1, 0 }, MMode::Rel);
    Header->Size.Set("prop_field_header_abs", { 0, FIELD_H }, MMode::Abs);
    Label* title = new Label(Header->Name + "_Title", 0, 0, 0, 0);
    float val = 0.2f;
    title->m_clear_color_base = { val,val,val,1 };
    title->Size.Set("prop_field_lb_rel", { 1,0 }, MMode::Rel);
    title->Size.Set("prop_field_lb_abs", { 0, FIELD_H }, MMode::Abs);
    title->Init(); Title = title; Title->m_scalePar = 0.8f;
    Header->AddChild(title);
    AddChild(Header);
    Size.Set("prop_field_rel", { 1,0 }, MMode::Rel);
    Size.Set("prop_field_abs", { 0, FIELD_H }, MMode::Abs);
}
// ********************************************** PropertyFiled *******************************************************
void PropertyFiled::Init() { Field::Init(); }
// ********************************************** StackField *******************************************************
void StackField::Init() { base_t::Init(); }
// ********************************************** TextField *******************************************************
void TextField::Init() {
    PropertyFiled::Init();

    TextBox* tb = new TextBox(Name + "_tb", 0, 0, 0, 0);
    tb->m_clear_color_base = { 1,1,0,1 };
    tb->Size.Set("prop_field_tb_rel", { 0.65f,0 }, MMode::Rel);
    tb->Size.Set("prop_field_tb_abs", { 0, FIELD_H }, MMode::Abs);
    tb->Pos.Set("prop_field_tb_rel", { 0.35f,0 }, MMode::Rel);
    tb->Init();
    AddChild(tb);
}
// ********************************************** NumberField *******************************************************
// ********************************************** ButtonField *******************************************************
void ButtonField::Init() {
    PropertyFiled::Init();
    Button* btn = new Button(Name + "_btn", 0, 0, 0, 0);
    btn->Size.Set("prop_field_btn_rel", { 0.65f,0 }, MMode::Rel);
    btn->Size.Set("prop_field_btn_abs", { 0, FIELD_H }, MMode::Abs);
    btn->Pos.Set("prop_field_btn_rel", { 0.35f,0 }, MMode::Rel);
    btn->Content->m_colorPar = { 1,1,1,1 };
    btn->Init(); BTN = btn; AddChild(btn);
}
// ********************************************** CheckBoxField *******************************************************
void CheckBoxField::Init() {
    PropertyFiled::Init();

    CheckBox* cb = new CheckBox(Name + "_cb", 0, 0, FIELD_H, FIELD_H);
    cb->HAlign = HorAlign::Right;
    cb->Init();
    cb->Content->SetText("X");
    cb->Content->m_colorPar = { 1,1,1,1 };
    Title->Size.Set("prop_field_lb_rel", { 1,0 }, MMode::Rel);
    Title->Size.Set("prop_field_lb_abs", { 0, FIELD_H }, MMode::Abs);
    AddChild(cb); CB = cb;
}
void CheckBoxField::Render() {
    Control::Render();
}
// ********************************************** CollapsableField *******************************************************
void toggle_callback(CheckBox* sender, CheckBoxEventArgs* e, params_t params) {
    CollapsableField* this_v = (CollapsableField*)params[0];
    this_v->SetExpand(e->Value);
}
void CollapsableField::Init() {
    StackField::Init();
    ::Collapser* collapser = new ::Collapser(Header->Name + "_Collapser", 0, 0, 20, FIELD_H);
    collapser->m_clear_color_base = Title->m_clear_color_base;
    collapser->Init();
    Title->Size.Set("prop_field_lb_rel", { 1,0 }, MMode::Rel);
    Title->Size.Set("prop_field_lb_abs", { 0, FIELD_H }, MMode::Abs);
    Header->AddChild(collapser);
    Collapser = collapser;
    StackPanel* content = new StackPanel(Name + "_Content");
    content->Init();
    content->HAlign = HorAlign::Right;
    StackPanel::unadjustable.insert(content->GUID);
    content->Size.Add("clps_content_size_rel", { 1,0 }, MMode::Rel);
    content->Size.Add("clps_content_size_abs", { -10,0 }, MMode::Abs);
    AddChild(content);
    Content = content;
    //Content->Visibility = VisibilityE::Collapsed;
    params_t pars;
    pars.push_back(this);
    collapser->e_Toggle.Add(this, toggle_callback, pars);
    SetExpand(false);
}
void CollapsableField::Prepare() {
    Control::Prepare();
}
void CollapsableField::SetExpand(bool newVal) {
    Collapser->SetEnable(newVal);
    if (newVal)
        Content->SetVisibility(VisibilityE::Visible);
    else
        Content->SetVisibility(VisibilityE::Collapsed);
    if (Linked) {
        //Measure();
        //Arrange();
        Content->States.Changed = true;
    }
}
// ********************************************** TextureField *******************************************************
void tex_index_changed_callback(NumBox1* sender, NumBox1ValueChangeEventArgs* e, params_t params) {
    TextureField* this_v = (TextureField*)params[0];
    int intVal = (int)this_v->texID[0];
    this_v->ParseTexIndex(intVal);
    e->handled = true;
}
void TextureField::SetTexIndex(int val) {
    //texID[0] = val;
    NB->SetNumber(val);
}
void TextureField::ParseTexIndex(int val) {
    int intVal = (int)texID[0];
    OnTexIndexChanged();
    if (!e_TexIndexChange.event_args->handled) {
        SetTexIndex((int)texID[0]);
    }
    int size = (int)texCol->size();
    if (intVal >= 0 && intVal < size) {
        texBuf->Texs[0] = texCol[0][intVal];
    }
}
void TextureField::OnTexIndexChanged() {
    e_TexIndexChange.event_args->index = (int)texID[0];
    e_TexIndexChange.event_args->handled = false;
    e_TexIndexChange.Invoke();
}
void TextureField::Init() {
    StackField::Init();

    NumBox1* nb = new NumBox1(Header->Name + "_nb", 0, 0, 0, 0, 0);
    nb->Size.Set("prop_field_nb_rel", { 0.65f,0 }, MMode::Rel);
    nb->Size.Set("prop_field_nb_abs", { 0, FIELD_H }, MMode::Abs);
    nb->Pos.Set("prop_field_nb_rel", { 0.35f,0 }, MMode::Rel);
    nb->Init();
    nb->NB->m_colorPar = { 1,1,1,1 };
    nb->NB->IsDecimal = false;
    NB = nb;
    Header->AddChild(nb);
    texID = NB->NB->Number.value;
    //bmtd::SetBinding(&nb->NB->Number, &texID, BindingMode::Both);
    nb->e_ValueChange.Add(this, tex_index_changed_callback, { this });
    texBuf = new Texbuf(Name + "_texBuf", 0, 0, 0, 120);
    texBuf->Init();
    AddChild(texBuf);
}

// ********************************************** TextureWithIntenstiyField *******************************************************
void intensity_changed_callback(NumBox1* sender, NumBox1ValueChangeEventArgs* e, params_t params) {
    AdjustTextureField* this_v = (AdjustTextureField*)params[0];
    sender->NB->SetNumber((float)e->number_value);
    e->handled = true;
}
void AdjustTextureField::Init() {
    TextureField::Init();
    NB_Intens = new NumBox1(Header->Name + "_nb_intens", 0, 0, 0, 0, 0);
    NB_Intens->Size.Set("prop_field_nb_intens_abs", { 0, FIELD_H }, MMode::Abs);
    NB_Intens->Init();
    NB_Intens->NB->m_colorPar = { 1,1,1,1 };
    NB_Intens->step = 0.1f;
    intensity = NB_Intens->NB->Number.value;
    NB_Intens->SetNumber(1);
    AddChild(NB_Intens);
    NB_Intens->e_ValueChange.Add(this, intensity_changed_callback, { this });
    CBF_Enable = new CheckBoxField("CBF_Enable");
    CBF_Enable->Init();
    CBF_Enable->Title->SetText("Enable");
    AddChild(CBF_Enable);
}
// ********************************************** Control3D *******************************************************
void Object::RaiseInspector(Scene* scene) {
    scene->SetInspector(this);
}
void Control3D::Init() {}
void Control3D::Update(float dt) {}
void Control3D::ProcessInput(float dt, InputArgs_S& input) {}
void Control3D::Render() {
    Render(m_shader);
}
void Control3D::Render(Shader* shader) {}
void Control3D::PickNext(float xNew, float yNew) {}
Point2D<float> Control3D::SetPickFb() { return {}; }
bool Control3D::PickAt(float xOld, float yOld) {
    PickFunc(xOld, yOld);
    return true;
}
PixelInfo Control3D::BeginPickFunc(float xNew, float yNew) { return {}; }
PixelInfo Control3D::PickFunc(float xNew, float yNew) { return {}; }
void Control3D::Translate(glm::vec3 value) {
    //transform.position += value;
}
// ********************************************** Vert *******************************************************
void begin_3d_drag(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    Dragger* this_v = (Dragger*)sender;
    WindowBase* win = this_v->win;
    Point2D<float> scaled = win->cursor_pos / win->m_size_scale;
    this_v->DragOffset = scaled;
}
void end_3d_drag(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    Dragger* this_v = (Dragger*)sender;
    WindowBase* win = this_v->win;
    Point2D<float> scaled = win->cursor_pos / win->m_size_scale;
    this_v->Pos.Add("base_abs", this_v->Pos["draggin"] + this_v->Pos["position"]);
    this_v->Pos.Set("draggin", {});
    this_v->Pos.Set("position", {});
}
void do_3d_drag(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    Dragger* this_v = (Dragger*)sender;
    WindowBase* win = this_v->win;
    Point2D<float> scaled = win->cursor_pos / win->m_size_scale;

    this_v->Position(scaled);
}
void Vert::Init() {
    glm::vec3 data = transform->positions;
}
void Vert::Render() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDrawArrays(GL_POINTS, 0, 1);
}
PixelInfo Vert::PickFunc(float xNew, float yNew) {
    glm::vec3& vec = transform->positions;
    auto cpos = rmtd::CurWin->cursor_pos;
    auto scene = (Scene*)Parent;

    return {};
}


// ********************************************** Triangle *******************************************************
void Triangle::ComputeTBN() {
    glm::vec3 edge1 = v2->transform->positions - v1->transform->positions;
    glm::vec3 edge2 = v3->transform->positions - v1->transform->positions;
    glm::vec2 deltaUV1 = v2_tex->positions - v1_tex->positions;
    glm::vec2 deltaUV2 = v3_tex->positions - v1_tex->positions;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent = glm::normalize(tangent);
    bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent = glm::normalize(bitangent);
    normal = glm::normalize(glm::cross(tangent, bitangent));
}

void Triangle::Init() {
    Control3D::Init();
}
void Triangle::Render() {
    this->m_shader->Use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    glBindVertexArray(0);
}
PixelInfo Triangle::PickFunc(float xNew, float yNew) {
    return {};
}
void TransformUI::GenerateFields() {
    nfx = new NumberField<float>("nfx");
    nfx->Init();
    nfx->Title->SetText(" X:");
    nfy = new NumberField<float>("nfy");
    nfy->Init();
    nfy->Title->SetText(" Y:");
    nfz = new NumberField<float>("nfz");
    nfz->Init();
    nfz->Title->SetText(" Z:");
    field->Title->SetText("Transfrom:");
    field->Content->AddChilds({ nfx,nfy,nfz });
    nfx->NB->e_ValueChange.Add(this, Transf_Cord::full_parse_callback, { x });
    x->source = nfx->NB;
    nfy->NB->e_ValueChange.Add(this, Transf_Cord::full_parse_callback, { y });
    y->source = nfy->NB;
    nfz->NB->e_ValueChange.Add(this, Transf_Cord::full_parse_callback, { z });
    z->source = nfz->NB;
    /*
    bmtd::SetBinding(x, &nfx->Number, BindingMode::Both);
    bmtd::SetBinding(y, &nfy->Number, BindingMode::Both);
    bmtd::SetBinding(z, &nfz->Number, BindingMode::Both);
    */
}
void TransformUI::Rebind(Component* newHost) {
    Transform* cast = (Transform*)newHost;
    x->Remap(newHost, &cast->positions.x);
    y->Remap(newHost, &cast->positions.y);
    z->Remap(newHost, &cast->positions.z);
}
void TransformUI::Notify() {
    x->SendBack();
    y->SendBack();
    z->SendBack();
}
void TexIndexUI::GenerateFields() {
    field->Title->SetText("Tex Index:");
    field->Title->m_scalePar = 0.5f;
    nfindex = new NumberField<int>("nfindex");
    nfindex->Init();
    nfindex->Title->SetText("Index:");
    nfindex->Title->m_scalePar = 0.4f;
    field->Content->AddChild(nfindex);
    nfindex->NB->e_ValueChange.Add(this, Index_DO::full_parse_callback, { this });
    texindex->source = nfindex->NB;
}
void TexIndexUI::Rebind(Component* newHost) {
    TexIndex* cast = (TexIndex*)newHost;
    texindex->Remap(newHost, &cast->index);
}
void TexIndexUI::Notify() {
    texindex->SendBack();
}
void TextureUI::Texture_Cord::ParseVal(NumBox1* nb) {
    auto val = nb->NB->Number.value[0];
    val = std::max(std::min(val, 1.0f), 0.0f);
    *value = val;
    Texture* cast = (Texture*)owner;
    cast->e_textureUVChange.event_args->UV = { cast->UI->u->value[0],cast->UI->v->value[0] };
    cast->e_textureUVChange.Invoke();
    cast->Notify();
}
void TextureUI::GenerateFields() {
    field->Title->SetText("Texture:");
    nfu = new NumberField<float>("nfu");
    nfu->Init();
    nfu->Title->SetText(" U:");
    nfu->NB->step = 0.05f;
    nfv = new NumberField<float>("nfv");
    nfv->Init();
    nfv->Title->SetText(" V:");
    nfv->NB->step = 0.05f;
    field->Content->AddChilds({ nfu,nfv });
    nfu->NB->e_ValueChange.Add(this, Texture_Cord::full_parse_callback, { u });
    u->source = nfu->NB;
    nfv->NB->e_ValueChange.Add(this, Texture_Cord::full_parse_callback, { v });
    v->source = nfv->NB;
}
void TextureUI::Rebind(Component* newHost) {
    Texture* cast = (Texture*)newHost;
    u->Remap(newHost, &cast->positions.x);
    v->Remap(newHost, &cast->positions.y);
}
void TextureUI::Notify() {
    u->SendBack();
    v->SendBack();
}
void MultiTextureUI::GenerateFields() {
    field->Title->SetText("Texture:");
}
void MultiTextureUI::Rebind(Component* newHost) {
    MultiTexture* cast = (MultiTexture*)newHost;
    owner = cast;int count = 0;
    for (auto tex : cast->texPoints) {
        tex->GetUI();
        tex->UI->field->Title->SetText("Texture[" + std::to_string(count++) + "]:");
        texPoints.push_back(tex->UI);
        ((CollapsableField*)field)->Content->AddChild(tex->UI->field);
    }
}
void MultiTextureUI::Notify() {
    for (auto tp : texPoints) {
        tp->Notify();
    }
}
void MultiTextureUI::Free() {
    for (auto tp : owner->texPoints) {
        tp->FreeUI();
    }
    texPoints.clear();
    field->Content->ClearChilds();
    UIManage<MultiTextureUI>::FreeUI(this);
}

// ********************************************** Camera *******************************************************

void CameraUI::GenerateFields() {
    field->Title->SetText("Transfrom:");
    nfx = new NumberField<float>("nfx");
    nfx->Init();
    nfx->Title->SetText(" X:");
    nfy = new NumberField<float>("nfy");
    nfy->Init();
    nfy->Title->SetText(" Y:");
    nfz = new NumberField<float>("nfz");
    nfz->Init();
    nfz->Title->SetText(" Z:");
    field->Content->AddChilds({ nfx,nfy,nfz });
    nfx->NB->e_ValueChange.Add(this, Transf_Cord::full_parse_callback, { x });
    x->source = nfx->NB;
    nfy->NB->e_ValueChange.Add(this, Transf_Cord::full_parse_callback, { y });
    y->source = nfy->NB;
    nfz->NB->e_ValueChange.Add(this, Transf_Cord::full_parse_callback, { z });
    z->source = nfz->NB;
}
void CameraUI::Rebind(Component* newHost) {
    Camera* cast = (Camera*)newHost;
    x->Remap(newHost, &cast->transform->positions.x);
    y->Remap(newHost, &cast->transform->positions.y);
    z->Remap(newHost, &cast->transform->positions.z);
}
void CameraUI::Notify() {
    x->SendBack();
    y->SendBack();
    z->SendBack();
}
glm::mat4 Camera::GetProjectionMatrix() {
    glm::mat4 projection;
    switch (PMode) {
        case ProjectionMode::Perspective:
            projection = glm::perspective(glm::radians(Zoom), (float)*LogicalSize.X / (float)*LogicalSize.Y, 0.1f, 100.0f);
            break;
        case ProjectionMode::Orthogonal:
            projection = glm::ortho(glm::radians(Zoom), (float)*LogicalSize.X / (float)*LogicalSize.Y, 0.1f, 100.0f);
            break;
        default:
            projection = glm::identity<glm::mat4>();
            break;
    }
    return projection;
}
const float pi = 3.14159265358979323846f;
struct dataa {
    glm::mat4 model;
    glm::vec3 position;
    glm::vec3 center;
    glm::vec3 Front;
    glm::vec3 Right;
    glm::vec3 Up;
    Point2D<float> cords;
    float Pitch;
    float Yaw;
};

std::vector<dataa> dataaa;
void Camera::Pivot(glm::vec3 center, Point2D<float> cords) {
    auto angle = cords * -9;
    glm::mat4 model = glm::mat4(1);

    transform->positions -= center;
    model = glm::rotate(model, angle.X, Up);
    model = glm::rotate(model, angle.Y, Right);
    transform->positions = model * glm::vec4(transform->positions, 1.0f);
    transform->positions += center;
    //transform->UI->Notify();
    Front = glm::normalize(center - transform->positions);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));

    updateAngles();
    dataaa.push_back(
        {
            model,
            transform->positions,
            center,
            Front,
            Right,
            Up,
            cords,
            Pitch,
            Yaw
        }
    );
}
void Camera::ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}
void Camera::updateVectors() {
    glm::vec3 front;
    front.x = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));

    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
void Camera::updateAngles() {
    Pitch = glm::degrees(glm::asin(Front.y));
    //Yaw = glm::degrees(glm::atan(Front.z, Front.y));
    glm::vec3 projected;
    if (Front.y != 0) {
        projected = { Front.x, 0, Front.z };
        projected = glm::normalize(projected);
    } else {
        projected = Front;
    }
    Yaw = glm::degrees(glm::acos(projected.z));
    if (projected.x < 0)
        Yaw = -Yaw;
}

void Grid::SetColumn(DrawObj* dObj, int idx, int span) {
    auto cd = ChildData[dObj];
    cd->col = idx;
    cd->colspan = span;
}

void Grid::SetRow(DrawObj* dObj, int idx, int span) {
    auto cd = ChildData[dObj];
    cd->row = idx;
    cd->rowspan = span;
}

DrawObj2D* Grid::AddChild(DrawObj2D* dObj) {
    GridChildData* gcd = new GridChildData();
    ChildData.insert({ dObj,gcd });
    return Framebuf::AddChild(dObj);
}


void Grid::AddChilds(std::vector<DrawObj2D*> dObjs) {
    Framebuf::AddChilds(dObjs);
}

void Grid::Measure() {
    ComputeMeasures();
    cols.CalcStar();
    rows.CalcStar();
    ccols = cols.Cumulative(shift_cols);
    crows = rows.Cumulative(shift_rows);
    for (auto child : Childs) {
        auto cd = ChildData[child];
        Point2D<float> size = { cols.Sum(shift_cols, cd->col, cd->colspan), rows.Sum(shift_rows, cd->row, cd->rowspan) };
        Point2D<float> pos = { ccols[cd->col] , crows[cd->row] };
        child->Size.Set("grid_gen_size_abs", size, MMode::Abs);
        child->Pos.Set("grid_gen_pos_abs", pos, MMode::Abs);
    }
}
void Grid::Arrange() {
    Control::Arrange();
}
void Grid::AdjustSize() {}
void Grid::Init() {
    Control::Init();
    cols.Link(&ActualSize.X, new float);
    rows.Link(&ActualSize.Y, new float);
}
static void grid_child_measure_changed_callback(sender_ptr sender, EventArgs* e, params_t params) {
    Grid* this_v = (Grid*)params[0];
    //this_v->Measure();
    //this_v->Arrange();
}
void Grid::SetLinked() {
    for (auto child : Framebuf::Childs) {
        if (!child->Linked) {
            child->Size.Set("streach", { 0,0 });
            child->Pos.Set("streach", { 0,0 });
            child->Size.Disable(MMode::Abs, "grid_disable_size_abs");
            child->Size.Disable(MMode::Rel, "grid_disable_size_rel");
            child->Pos.Disable(MMode::Abs, "grid_disable_pos_abs");
            child->Pos.Disable(MMode::Rel, "grid_disable_pos_rel");
            child->e_MeasureChanged.Add(this, grid_child_measure_changed_callback, { this });
        }
    }
    shift_cols.clear();
    for (int i = 0; i < cols.measures.size(); i++) {
        shift_cols.push_back(1);
    }
    shift_rows.clear();
    for (int i = 0; i < rows.measures.size(); i++) {
        shift_rows.push_back(1);
    }
    Measure();
    Framebuf::SetLinked();
}
void Grid::LinkChild(DrawObj2D* child) {
    /*
    child->Size.Disable(MMode::Abs, "grid_disable_size_abs");
    child->Size.Disable(MMode::Rel, "grid_disable_size_rel");
    child->Pos.Disable(MMode::Abs, "grid_disable_pos_abs");
    child->Pos.Disable(MMode::Rel, "grid_disable_pos_rel");
    child->e_MeasureChanged.Add(this, grid_child_measure_changed_callback, { this });
    */
    Control::LinkChild(child);
}
void Grid::UnlinkChild(DrawObj2D* child) {
    child->Size.Set("grid_disable_size_abs", {});
    child->Size.Set("grid_disable_size_rel", {});
    child->Pos.Set("grid_disable_pos_abs", {});
    child->Pos.Set("grid_disable_pos_rel", {});
    child->Size.Set("grid_gen_size_abs", {});
    child->Pos.Set("grid_gen_pos_abs", {});
    child->e_MeasureChanged.Remove(this, grid_child_measure_changed_callback);
    //Control::Unlink();
}

void GridMeasueCol::CalcStar() {
    float val = 0;
    float sNum = 0;
    for (auto item : measures) {
        switch (item->mode) {
            case MMode::Abs: val += item->val; break;
            case MMode::Rel: val += item->val * *p_total; break;
            case MMode::Star: sNum += item->val; break;
            default: break;
        }
    }
    float taken = val;
    starVal = p_total[0] - taken;
    starVal /= sNum;
    if (sNum == 0) starVal = 0;
}

float GridMeasueCol::Sum(int pos, int num) {
    float val = 0;
    int maxnum = (int)measures.size();
    maxnum = std::min(num, maxnum - pos);
    for (int i = 0; i < maxnum; i++) {
        val += ComputeSingle(measures[pos + i]);
    }
    return val;
}

float GridMeasueCol::Sum(std::vector<float> shift, int pos, int num) {
    float val = 0;
    int maxnum = (int)measures.size();
    maxnum = std::min(num, maxnum - pos);
    for (int i = 0; i < maxnum; i++) {
        val += ComputeSingle(measures[pos + i]) * shift[pos + i];
    }
    return val;
}

std::vector<float> GridMeasueCol::Cumulative() {
    std::vector<float> vec;
    float val = 0;
    vec.push_back(val);
    for (auto item : measures) {
        switch (item->mode) {
            case MMode::Abs: val += item->val; break;
            case MMode::Rel: val += item->val * *p_total; break;
            case MMode::Star: val += item->val * starVal; break;
            default: break;
        }
        vec.push_back(val);
    }
    return vec;
}

std::vector<float> GridMeasueCol::Cumulative(std::vector<float> shift) {
    std::vector<float> vec;
    float val = 0;
    vec.push_back(val);
    for (int i = 0; i < measures.size(); i++) {
        auto& item = measures[i];
        auto& sft = shift[i];
        switch (item->mode) {
            case MMode::Abs: val += item->val * sft; break;
            case MMode::Rel: val += item->val * *p_total * sft; break;
            case MMode::Star: val += item->val * starVal * sft; break;
            default: break;
        }
        vec.push_back(val);
    }
    return vec;
}

void Splitter::ResizeNeighbours() {
}


void Splitter::Init() {
    Dragger::Init();
    if (ori == Orientation_S::Horizontal) {
        //Align(HorAlign::Stretch, VerAlign::None);
        VDrag = true;
        HDrag = false;
    } else {
        //Align(HorAlign::None, VerAlign::Stretch);
        VDrag = false;
        HDrag = true;
    }
    e_drag_end.Add(this, splitter_drag_end_callback, { this });
}

void Splitter::splitter_drag_end_callback(Dragger* sender, DragEventArgs* e, params_t params) {
    Splitter* this_v = (Splitter*)params[0];
    if (Grid* grid = dynamic_cast<Grid*>(this_v->Parent)) {
        sender->Pos.Add("base_abs", -e->pos_cng);
        auto cd = grid->ChildData[this_v];
        int pos; float rel = 0;
        if (this_v->ori == Orientation_S::Vertical) {
            pos = cd->col - 1;
            rel = e->pos_cng.X / grid->cols.ComputeSingle(grid->cols.measures[pos]);
            grid->shift_cols[pos] += rel;
            pos = cd->col + 1;
            rel = e->pos_cng.X / grid->cols.ComputeSingle(grid->cols.measures[pos]);
            grid->shift_cols[pos] -= rel;
        } else {
            pos = cd->row - 1;
            rel = e->pos_cng.Y / grid->rows.ComputeSingle(grid->rows.measures[pos]);
            grid->shift_rows[pos] += rel;
            pos = cd->row + 1;
            rel = e->pos_cng.Y / grid->rows.ComputeSingle(grid->rows.measures[pos]);
            grid->shift_rows[pos] -= rel;
        }
        grid->OnChange();
        grid->Refresh();
    }
}

void Dummy2D::Init() {
    m_clear_color_base = rmtd::Colors["drag_drop_dummy"];
    Control::Init();
}

void Dummy2D::Render() {
    Framebuf::Render();
}

void PlacementSnap::Save(Control* obj) {
    m_align.Align(obj);
    m_size = obj->ActualSize;
}


