#pragma once
#include "utils.h"
#include "framebuf.h"
#include "text_renderer.h"
#include "binding_manager.h"
#include "model.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include "Debug.h"

class IMouseEvents {
public:
	MouseClickEvent_E e_MouseClick;
	MouseClickEvent_E e_MousePress;
	MouseClickEvent_E e_MouseRelease;
	MouseHoverEvent_E e_MouseHover;
	IMouseEvents(DrawObj* owner) {
		e_MouseClick = MouseClickEvent_E(&Dispatcher::MouseClickQ, owner);
		e_MousePress = MouseClickEvent_E(&Dispatcher::MousePressQ, owner);
		e_MouseRelease = MouseClickEvent_E(&Dispatcher::MouseReleaseQ, owner);
		e_MouseHover = MouseHoverEvent_E(&Dispatcher::MouseReleaseQ, owner);
	}
};


typedef Control control_pb_t;
class Control : public Framebuf, public IMouseEvents {
public:
	Control(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Framebuf(name, xPos, yPos, xSize, ySize, GetPassShader()), IMouseEvents(this) {
	}
	Control(std::string name, glm::vec2 pos, glm::vec2 size)
		: Control(name, pos.x, pos.y, size.x, size.y) {
	}
	Control(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Control("control_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {
	}
	Control(glm::vec2 pos, glm::vec2 size)
		: Control("control_" + std::to_string(GUID), pos.x, pos.y, size.x, size.y) {
	}
	virtual bool PickAt(float xOld, float yOld) override;
	virtual PixelInfo PickFunc(float xNew, float yNew);
	virtual Point2D<float> SetPickFb() override;

	virtual void Prepare() override;
	virtual void Render() override;
	virtual uint RenderB(const Bounds& bn) override;
	void Delete();
	void InformChange(InformCB callback);
	virtual void Update(float dt = 0);
	virtual void ProcessInput(float dt, InputArgs_S& input) override;
	virtual DrawObj2D* MemClone() { return new Control(*this); }
	template<typename Tneed, typename Tgot>
	bool IsOfType(Tgot val) {
		return false;
	}
};

class Dummy2D : public Control {
public:
	using Control::Control;
	void Init() override;
	void Render() override;
};

struct TextData_S {
	int CurIdx = -1;
	Point2D<int> CurPos;
	std::vector<CharFramebuf*> Chars_v;
	std::vector<Bounds> Bounds_v;
	std::vector<Point2D<float>> Offs_v;
	std::vector<int> Char_num_v;
	Point2D<float> Offs_end;
	int Row_num;
	bool ToRender = true;
	TextData_S() { NextRow(); }
	void Add(Point2D<float> entry, CharFramebuf* chr) {
		Chars_v.push_back(chr);
		Offs_v.push_back(entry);
		Char_num_v.back()++;
	}
	void NextRow() {
		Char_num_v.push_back(0);
		Row_num++;
	}
	int Size() { return (int)Chars_v.size(); }
	int GetEnd() { return Size(); }
	void ToStart() { CurIdx = -1; }
	int ReadIndex = -1;
	Point2D<float>& Next() {
		ReadIndex++;
		CurPos.X++;
		if (Char_num_v.back() == CurPos.X)// num == pos
		{
			CurPos.X = 0;
			CurPos.Y++;
		}
		return Offs_v[CurIdx];
	}
	void Clear() {
		ReadIndex = -1; CurPos = { 0,0 }; Row_num = 0; Bounds_v.clear();
		Chars_v.clear(); Offs_v.clear(); Char_num_v.clear(); Offs_end = { 0,0 };
		NextRow();
	}
};

class TextBase : public Control {
public:
	TextChangeEvent_E e_TextChange = TextChangeEvent_E(&Dispatcher::InvokeQ, this);
	void OnTextChange();
	TextRenderer* TextRend;
	void SetTextRend(TextRenderer& textRend) { TextRend = &textRend; }
	HorAlign HTextAlign = HorAlign::None;
	VerAlign VTextAlign = VerAlign::None;
	void TextAlign(HorAlign h, VerAlign v) { HTextAlign = h; VTextAlign = v; }
	virtual void ResizeTexs(Point2D<float> size) override;
	std::string m_text = "";
	virtual void SetText(std::string text);
	std::map<uchar, CharFramebuf*> Characters;
	bool AutoNewLine = false;
	TextData_S TextData;
	float m_xPar = 5;
	float m_yPar = 5;
	float m_scalePar = 1;
	float m_lr_margin = 0;
	float m_ud_margin = 5;
	int m_Font_size = 24;
	glm::vec4 m_colorPar = glm::vec4(1, 1, 1, 1);
	TextBase(std::string name, float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: Control(name, xPos, yPos, xSize, ySize), TextRend(nullptr) {
		m_clear_color_base = { 0,0,0,1 };
		TextRend = rmtd::s_TextRend;
	}
	TextBase(float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: TextBase("textbase" + std::to_string(GUID), xPos, yPos, xSize, ySize) {
	}
	virtual void Init() override;

	virtual void AddChar(const int& index, const uchar& chr);
	virtual void RemoveChar(const int& index);
	virtual void Render() override;
	void RenderText();
	void SetParameters(float x = 5, float y = 5, float scale = 1, glm::vec4* color_ptr = nullptr);

	virtual DrawObj2D* MemClone() { return new TextBase(*this); }
};

class Label : public TextBase {
public:
	bool IsPressed = false;

	Label(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: TextBase(name, xPos, yPos, xSize, ySize) {
		m_clear_color_base = { 0,0,0,1 };
		m_scalePar = 0.5;
		m_text = "label";
	}
	Label(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Label("lb" + std::to_string(GUID), xPos, yPos, xSize, ySize) {
	}
	virtual void Init() override;;
	virtual PixelInfo PickFunc(float xNew, float yNew) override;
	virtual void Prepare() override;
	virtual void Render() override;
	virtual DrawObj2D* MemClone() { return new Label(*this); }
};

class TextBox : public TextBase {
public:
	bool IsPressed = false;
	int m_cursorPos = 0;
	CharMap_S charMap;
	virtual void SetText(std::string text) override { TextBase::SetText(text); OnTextChange(); }
	virtual std::vector<int>& filter();

	TextBox(std::string name, float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: TextBase(name, xPos, yPos, xSize, ySize) {
		m_clear_color_base = { 0.2f,0.2f,0.2f,1 };
		m_colorPar = { 0,0,0,1 };
		m_cursorPos = (int)Childs.size();
		tex_num = 2;
		m_text = "text";
	}
	TextBox(float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: TextBox("textbox" + std::to_string(GUID), xPos, yPos, xSize, ySize) {
	}
	virtual void Init() override;;
	virtual Point2D<float> SetPickFb() override;
	virtual PixelInfo PickFunc(float xNew, float yNew) override;
	virtual void Prepare() override;
	virtual void Render() override;
	virtual void Update(float dt = 0) override;
	virtual void ProcessInput(float dt, InputArgs_S& input) override;
	virtual bool TryChangeActivity(bool value) override;
	virtual DrawObj2D* MemClone() { return new TextBox(*this); }
	bool Check(float dt, InputArgs_S& input, int i);

};

template<typename Towner>
class Prop {
public:
	template<typename T>
	class Wrap : public DepObj<T> {
		virtual void OnSet(Towner* owner, T newValue) {
			DepObj<T>::OnSet(owner, newValue);
		}
	};
};

class NumberBox : public TextBox {
public: virtual std::vector<int>& filter() override;
	bool hasDot = false;
	NumberChangeEvent_E e_NumberChange = NumberChangeEvent_E(&Dispatcher::InvokeQ, this);
	void OnNumberChange();
	bool IsDecimal = true;
	class NB_Number : public DepObj<float> {
		typedef DepObj<float> base_t; using base_t::base_t;
		void OnSet(void* owner, float newValue) override {
			NumberBox* this_v = (NumberBox*)owner;
			this_v->old_number = Get(); *value = newValue; std::string str;
			if (this_v->IsDecimal) { str = utils::to_string(newValue);
				if ((int)newValue == newValue)this_v->hasDot = false;
				else this_v->hasDot = true;} 
			else {str = { std::to_string((int)newValue) };}
			this_v->TextBase::SetText(str);
			this_v->OnNumberChange();}};
	NB_Number Number = NB_Number(this); 
	float old_number = 0;
	virtual void SetText(std::string text) override { SetNumber(utl::safe_stof(text)); }
	virtual void SetNumber(float num) { Number.Set(num); }
	void ParseNumber(std::string text);
	template<typename T> void SetNumber(T val);
	NumberBox(std::string name, float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: TextBox(name, xPos, yPos, xSize, ySize) {
		m_clear_color_base = { 0.2f,0.2f,0.2f,1 };
		m_colorPar = { 0,0,0,1 };
		m_cursorPos = (int)Childs.size();
		tex_num = 2;
		m_text = "0";
	}
	NumberBox(float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: NumberBox("numberbox" + std::to_string(GUID), xPos, yPos, xSize, ySize) {
	}
	virtual void Init() override;;
	virtual void ProcessInput(float dt, InputArgs_S& input) override;
	virtual DrawObj2D* MemClone() { return new TextBox(*this); }
};
template<typename T>
inline void NumberBox::SetNumber(T val) {
	old_number = Number.value[0];
	Number.value[0] = (float)val;
	TextBase::SetText(utils::to_string(val));
	OnChange();
}



class IntNumberBox : public TextBox {
public:
	virtual std::vector<int>& filter() override;
	NumberChangeEvent_E e_NumberChange = NumberChangeEvent_E(&Dispatcher::InvokeQ, this);
	void OnNumberChange();
	class NB_Number : public DepObj<int> {
		typedef DepObj<int> base_t;
		using base_t::base_t;
		void OnSet(void* owner, int newValue) override {
			NumberBox* this_v = (NumberBox*)owner;
			*value = newValue;
			std::string str = utils::to_string(newValue);
			this_v->TextBase::SetText(str);
			this_v->OnNumberChange();
		}
	};
	NB_Number Number = NB_Number(this);
	int old_number = 0;
	virtual void SetText(std::string text) override { Number.Set(utl::safe_stoi(text)); }
	virtual void SetNumber(int num) { Number.Set(num); }
	IntNumberBox(std::string name, float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: TextBox(name, xPos, yPos, xSize, ySize) {
		m_clear_color_base = { 0.2f,0.2f,0.2f,1 };
		m_colorPar = { 0,0,0,1 };
		m_cursorPos = (int)Childs.size();
		tex_num = 2;
		m_text = "0";
	}
	IntNumberBox(float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: IntNumberBox("IntNumberBox" + std::to_string(GUID), xPos, yPos, xSize, ySize) {
	}

	virtual void Init() override;;
	virtual void ProcessInput(float dt, InputArgs_S& input) override;
	virtual DrawObj2D* MemClone() { return new TextBox(*this); }
};



class Clicker : public Control {
public:
	bool IsPressed = false;
	MouseClickData* btn_up_cb = nullptr;
	Clicker(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Control(name, xPos, yPos, xSize, ySize) {
		float val = 0.2f;
		//m_clear_color_base = { val,val,val,1 };
	}
	Clicker(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Clicker("Clicker_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	virtual void Init() override;;
	virtual PixelInfo PickFunc(float xNew, float yNew) override;
	virtual void Prepare() override;
	virtual void Render() override;
	virtual DrawObj2D* MemClone() { return new Clicker(*this); }
public:
	static void release_callback(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
		Clicker& b = *(Clicker*)sender;
		b.IsPressed = false;
		b.OnChange();
	}
};

class Button : public Clicker {
public:

	Label* Content = nullptr;
	void SetTextRend(TextRenderer& textRend) { Content->SetTextRend(textRend); }
	void SetText(std::string text) { Content->SetText(text); }
	Button(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Clicker(name, xPos, yPos, xSize, ySize) {
		m_clear_color_base = rmtd::Colors["button_bg_inactive"];
	}
	Button(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Button("Button_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	virtual void Init() override;;
	virtual void Prepare() override;
	virtual DrawObj2D* MemClone() { return new Button(*this); }
};

class CheckBox : public Clicker {
public:
	DepObj<bool> IsEnabled = new DepObj<bool>(this);
	Label* Content = nullptr;
	CheckBoxToggleEvent_E e_Toggle = CheckBoxToggleEvent_E(&dpch::InvokeQ, this);
	void SetTextRend(TextRenderer& textRend) { Content->SetTextRend(textRend); }
	void SetText(std::string text) { Content->SetText(text); }
	virtual void Toggle();
	void OnToggle();
	virtual void SetEnable(bool newVal);
	PixelInfo PickFunc(float xNew, float yNew) override;
	CheckBox(std::string name, float xPos = 0, float yPos = 0, float xSize = 25, float ySize = 25)
		: Clicker(name, xPos, yPos, xSize, ySize) {
		float val = 0.15f;
		m_clear_color_base = { val,val,val,1 };
	}
	CheckBox(float xPos = 0, float yPos = 0, float xSize = 25, float ySize = 25)
		: CheckBox("CheckBox_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	virtual void Init() override;
	virtual void Render() override;
	virtual void Prepare() override;

	virtual DrawObj2D* MemClone() { return new CheckBox(*this); }
};

class Collapser : public CheckBox {
public:
	using CheckBox::CheckBox;
	virtual void Init() override;;
	virtual void Render() override;
	virtual void Toggle() override;
	virtual void SetEnable(bool newVal);
};


class NumBox1 : public Control {
public:
	bool IsPressed = false;
	//int m_cursorPos = 0;
	float* m_count = 0;
	NumberBox* NB = nullptr;
	Button* PB = nullptr;
	Button* MB = nullptr;
	TextRenderer* TextRend = nullptr;
	float step = 1;
	NumBox1ValueChangeEvent_E e_ValueChange = NumBox1ValueChangeEvent_E(&dpch::InvokeQ, this);

	template<typename T>
	void SetNumber(T val);
	void ParseNumber(float val);
	void ParseNumberChange(float change) { ParseNumber(NB->Number.Get() + change); }
	void SetTextRend(TextRenderer& textRend) { TextRend = &textRend; }

	NumBox1(std::string name, float count, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Control(name, xPos, yPos, xSize, ySize) {
		if (m_count != nullptr)m_count[0] = count;
	}
	NumBox1(float count, float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: NumBox1("numberbox" + std::to_string(GUID), count, xPos, yPos, xSize, ySize) {}
	virtual void Init() override;
	virtual PixelInfo PickFunc(float xNew, float yNew) override;
	virtual void Prepare() override;
	virtual void Render() override;
	virtual DrawObj2D* MemClone() { return new NumBox1(*this); }
};
template<typename T>
inline void NumBox1::SetNumber(T val) {
	m_count[0] = (float)val;
	NB->SetNumber(val);
}


class Dragger;

class Placement_S {
public:
	Point2D<float> Val{};
	Point2D<float> Scale{};
	Point2D<float> Margin_LT{};
	Point2D<float> Margin_RB{};
	Control* m_owner;
	Placement_S(Control* owner) : m_owner(owner) {}
	Point2D<float> Get() {
		return Val * Scale;
	}
	void Place();
	void Compute();
};

class DragData {

};

class Dragger : public Clicker {
public:
	Control* Target = this;
	virtual void ComputeBounds() override;
	bool HDrag = true;
	bool VDrag = true;
	WindowBase* win = rmtd::CurWin;
	Point2D<float> DragOffset{};
	Point2D<float> StartPos{};
	Point2D<float> LastPos{};
	Placement_S Placement = Placement_S(this);
	void SetAtVal();
	void ComputePlacement();
	void Move(Point2D<float> change);
	void MoveRel(Point2D<float> change);
	void AdjustPosition(Point2D<float> rel_change);
	virtual void Measure() override;
	virtual void Arrange() override;
	void SetTarget(Control* target);
	void OnDrag();
	void OnDragEnd();
	void OnDragBegin();
	DragEvent_E e_drag = DragEvent_E(&dpch::InvokeQ, this);
	DragEvent_E e_drag_end = DragEvent_E(&dpch::InvokeQ, this);
	DragEvent_E e_drag_begin = DragEvent_E(&dpch::InvokeQ, this);

	//using Clicker::Clicker;
	Dragger(std::string name, float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: Clicker(name, xPos, yPos, xSize, ySize) {}
	Dragger(float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: Dragger("dragger" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	virtual void Init() override;
	void Position(Point2D<float> newPos);
	virtual void Prepare() override;
	virtual DrawObj2D* MemClone() { return new Dragger(*this); }
};
class DragManager {
	WindowBase* win = rmtd::CurWin;
	static std::unordered_map<Control*, DragData*> registered;

	void Register(Control* dObj) {
		if (registered.count(dObj) == 0) {
			registered.insert({ dObj,new DragData() });
			dObj->e_MouseClick.Add(dObj, begin_drag_callback, {});
			dObj->e_MouseRelease.Add(dObj, end_drag_callback, {});
			dObj->e_MousePress.Add(dObj, drag_callback, {});
		}



	}
	/*
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
	*/
	// ********************************************** DRAGGER *******************************************************
	static void begin_drag_callback(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
		Dragger* this_v = (Dragger*)sender;
		WindowBase* win = this_v->win;
		auto dd = DragManager::registered[this_v];
		Point2D<float> scaled = win->cursor_pos / win->m_size_scale;
		this_v->StartPos = this_v->ActualPos;
		this_v->DragOffset = scaled - this_v->ActualPos;
		this_v->OnDragBegin();
	}
	static void end_drag_callback(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
		Dragger* this_v = (Dragger*)sender;
		WindowBase* win = this_v->win;
		Point2D<float> scaled = win->cursor_pos / win->m_size_scale;
		this_v->Pos.MergeInto("base_abs", "drag_position_abs", 1);
		this_v->Pos.Set("drag_position_abs", { 0,0 });
		this_v->OnDragEnd();
	}
	static void drag_callback(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
		Dragger* this_v = (Dragger*)sender;
		WindowBase* win = this_v->win;
		Point2D<float> scaled = win->cursor_pos / win->m_size_scale;
		this_v->Position(scaled);
		this_v->OnDrag();
	}
};
enum class Orientation_S {
	NONE,
	Horizontal,
	Vertical,
};
class ScrollBar : public Control {
public:

	bool HScroll = true, VScroll = true;
	float bar_size = 10;
	Dragger* DRG = nullptr;
	Button* PB = nullptr, * MB = nullptr;
	Point2D<float> change{ 10.0f,10.0f };
	Point2D<float> scroll_pos{ 0.f,0.f };
	Point2D<float> OldVal{}, LValC{};
	Point2D<float> GetScrollVal() { return DRG->Placement.Val; }
	Point2D<float> GetLastValChange() { return LValC; }
	Orientation_S Orientation = Orientation_S::Horizontal;
	void SetOrientation(Orientation_S ori);
	void AdjustPosition(Point2D<float> rel_pos);
	ScrollEvent_E e_Scroll = ScrollEvent_E(&Dispatcher::ScrollQ, this);
	ScrollBar(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Control(name, xPos, yPos, xSize, ySize) {
		m_clear_color_base = { 0,0,0,1 };
	}
	ScrollBar(float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: ScrollBar("scrollbar" + std::to_string(GUID), xPos, yPos, xSize, ySize) {
	}
	virtual void Init() override;
	//virtual PixelInfo PickFunc(float xNew, float yNew) override;
	void OnScroll();// { e_Scroll.Invoke(); };
	virtual void Prepare() override;
	virtual void Render() override;
	virtual DrawObj2D* MemClone() { return new ScrollBar(*this); }
};
enum class ResizeMode : int {
	Fixed,
	Stretch
};
class Field : public Control {
public:
	Control* Header = nullptr;
	Label* Title = nullptr; //field label
	virtual void Init() override;
	using Control::Control;
};
class SharedSPStatic {
public:
	static std::unordered_set<uint> unadjustable;
};
template<typename T>
class StackPanelGen {
	static_assert(std::is_base_of<Control, T>::value, "T must inherit from Control");
public:
	// ********************************************** StackPanelT *******************************************************
	class StackPanelT : public T, public SharedSPStatic {
		static_assert(std::is_base_of<Control, T>::value, "T must inherit from Control");
	public:
		Orientation_S Orientation = Orientation_S::Vertical;
		void SetOrientation(Orientation_S newO) {
			Orientation = newO;
			bool isVert = Orientation == Orientation_S::Vertical;
			if (isVert) { T::Align(HorAlign::Stretch, VerAlign::None); } else { T::Align(HorAlign::None, VerAlign::Stretch); }
		}
		static Point2D<float> dummy;
		float MeasuredSize = 0;
		bool InformMeasureChange = true;
		StackPanelT(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
			: T(name, xPos, yPos, xSize, ySize) {
			static const float val = 0.f;
			T::m_clear_color_base = { val,val,val,0.5f };
		}
		StackPanelT(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
			: T("StackPanelT" + std::to_string(T::GUID), xPos, yPos, xSize, ySize) {
		}
		virtual DrawObj2D* MemClone() { return new StackPanelT(*this); }
		void ClearChilds() { Framebuf::ClearChilds(); if (T::Linked) {Measure();Arrange();}}
		void Refresh() { LinkMeasure(); Arrange();
			for (auto child : Framebuf::Childs) {child->Refresh();}}
		void UnlinkChild(DrawObj2D* child) { 
			child->e_MeasureChanged.Remove(this, child_measure_changed_callback); }
		static void child_measure_changed_callback(sender_ptr sender, EventArgs* e, params_t params) {
			StackPanelT* this_v = (StackPanelT*)params[0]; this_v->Measure(); this_v->Arrange();}
		void BeginLink() {
			T::BeginLink(); for (auto child : T::Childs) {child->Link(this);AdjustChild(child);
				child->e_MeasureChanged.Add(this, child_measure_changed_callback, { this });}}
		void SetLinked() {
			if (!T::Linked) {BeginLink();LinkMeasure();}
			Arrange(); for (auto child : T::Childs) {child->SetLinked();}}
		inline void AdjustChild(DrawObj2D* child) {
			if (unadjustable.count(child->GUID) != 0) return;
			bool isVert = Orientation == Orientation_S::Vertical; auto child_size = child->IntendedSize();
			child->Size.Clear(); child->Size.Add("child_computed_abs", child_size, MMode::Abs);
			if (isVert) { child->Align(HorAlign::Stretch, VerAlign::None); } 
			else { child->Align(HorAlign::None, VerAlign::Stretch); }}
		void LinkChild(DrawObj2D* child) {
			child->Link(this); AdjustChild(child); child->LinkMeasure();
			child->e_MeasureChanged.Add(this, child_measure_changed_callback, { this });
			Measure();Arrange();child->SetLinked();}
		virtual void LinkMeasure() override {
			InformMeasureChange = false; BeginLink();
			for (auto child : T::Childs) {child->LinkMeasure();}
			Measure();InformMeasureChange = true;}
		virtual void Measure() {
			Point2D<float> mod = { 0,1 }, pos{}, MeasuredSize{};
			bool isVert = Orientation == Orientation_S::Vertical;
			if (!isVert) mod.Swap(); auto childs = T::UCChilds();
			for (auto child : childs) { auto child_size = child->IntendedSize() * mod;
				MeasuredSize += child_size;child->Pos.Clear();
				child->Pos.Add("StackPanelT_computed_abs", pos, MMode::Abs); pos += child_size;}
			T::Size.Resize(mod.Swapped()); T::Size.Add("StackPanelT_abs", MeasuredSize, MMode::Abs);
			if (InformMeasureChange) T::OnMeasureChange();}
		void Arrange() {Framebuf::Arrange();
			for (auto child : Framebuf::Childs) {if (child->Linked) {child->OnChange();
				child->ComputeBounds();child->ResizeTexs(child->ActualSize);}}}
		void RemoveChild(DrawObj2D* dObj) {Framebuf::RemoveChild(dObj);Measure(); Arrange();}

		void Init() {
			T::Init();
			SetOrientation(Orientation);
		}
		void Prepare() { T::Prepare(); }
		void Render() { T::Render(); }
	};
};

class StackPanel : public StackPanelGen<Control>::StackPanelT {
public:
	typedef StackPanelGen<Control>::StackPanelT base_t;
	using base_t::base_t;
};

class FixedStackPanel : public StackPanel {
public:
	typedef StackPanel base_t;
	using base_t::base_t;
	int beginIndex = 0;
	virtual void Measure() override;
};

class ViewSpace : public Control {
public:
	ResizeMode VerRM = ResizeMode::Stretch;
	ResizeMode HorRM = ResizeMode::Fixed;
	static int indexer[8];
	Bounds m_texCords = { 0,0,1,1 };
	Bounds m_texCords_old = { 0,0,1,1 };
	inline void SetTexCords();
	Point2D<float> SpaceSize{ 0,0 };
	void SetSpaceSize(Point2D<float> newSize);
	Point2D<float> SPosRel{ 0, 0 }, SPosRel01{}, OldSPosRel01{};
	Point2D<float> ViewSizeRatio{ 1,1 };
	Control* Proxy = nullptr;
	uint VAO2 = 0, VBO = 0;
	int data_size = 0;
	float data[20];
	void BeginLink();
	void UnlinkChild(DrawObj2D* child);
	virtual void Measure() override;
	virtual void Arrange() override;
	void SetProxy(Control* proxy);
	void RemoveProxy();
	virtual Point2D<float> IntendedSize() override;
	virtual void ResizeTexs(Point2D<float> size) override;
	virtual void SetLinked() override;
	virtual void ComputeBounds();
	void UpdateTex();
	void UpdateCords();
	void UpdateBuffer(bool first, bool second);

	void MoveView(Point2D<float> nvc, Point2D<float> factor);

	ViewSpacePositionChangeEvent_E e_SpaceChanged = ViewSpacePositionChangeEvent_E(&dpch::InvokeQ, this);

	ViewSpace(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Control(name, xPos, yPos, xSize, ySize) {
		static const float val = 0.8f;
		m_clear_color_base = { 1.f,1.f,val,1.f };
	}
	ViewSpace(float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: ViewSpace("viewspace" + std::to_string(GUID), xPos, yPos, xSize, ySize) {
	}

	virtual void Init() override;
	virtual bool PickAt(float x, float y) override;
	//virtual PixelInfo PickFunc(float xNew, float yNew) override;
	//virtual void Prepare() override;
	virtual void Render() override;
	virtual DrawObj2D* MemClone() { return new ViewSpace(*this); }

};

class ToolBarBase : public Control {
public: using Control::Control;
	FixedStackPanel* TabZone = nullptr;
	Button* Left = nullptr, * Right = nullptr;
	virtual DrawObj2D* AddChild(DrawObj2D* dObj) override;
	virtual void AddChilds(std::vector<DrawObj2D*> dObjs) override;
	template<typename T> void AddChildsT(std::vector<T*> dObjs);
	virtual void InnerAddChild(DrawObj2D* dObj);
	virtual void Init() override;
	virtual DrawObj2D* MemClone() { return new ToolBarBase(*this); }
};
template<typename T>
inline void ToolBarBase::AddChildsT(std::vector<T*> dObjs) {
	for (auto dObj : dObjs) {
		TabZone->AddChild(dObj);
	}
}

class ToolBar : public ToolBarBase {
public:
	using ToolBarBase::ToolBarBase;
	virtual void Init() override;
};

class TabHolder : public ToolBarBase {
public:
	using ToolBarBase::ToolBarBase;
	virtual void Init() override;
};

struct TabViewerEntry {
	Button* TabCard;
	DrawObj2D* Content;
};
class TabViewer : public Control {
public:
	TabViewerEntry* activeEntry = nullptr;
	TabHolder* TabBar = nullptr;
	Control* Content = nullptr;
	std::vector<TabViewerEntry*> Entries;
	TabViewerEvent_E e_TabChanged = TabViewerEvent_E(&dpch::InvokeQ, this);
	TabViewerEvent_E e_TabAdded = TabViewerEvent_E(&dpch::InvokeQ, this);
	TabViewerEntry* AddTab(DrawObj2D* dObj);
	TabViewer(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Control(name, xPos, yPos, xSize, ySize) {
		//m_clear_color_base = { 0,0,0,1 };
		static const float val = 0.7f;
		m_clear_color_base = { val,val,val,1 };
	}
	TabViewer(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: TabViewer("TabViewer_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {
	}
	virtual void Init() override;
	virtual void Prepare() override;
	virtual DrawObj2D* MemClone() { return new TabViewer(*this); }
};


class Scrollable : public Control {
public:
	Bounds m_texCords;

	bool HScroll = true;
	bool VScroll = true;
	float bar_size = 10;
	ViewSpace* VS = nullptr;
	void SetViewSpace(ViewSpace* vs);
	void SetProxy(Control* proxy_p);
	Control* GetProxy() { return Proxy; }
	ScrollBar* RSB = nullptr;
	ScrollBar* BSB = nullptr;
	void MoveView();
	virtual void Link(Framebuf* parent) override;
	virtual void Unlink() override;

	Scrollable(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Control(name, xPos, yPos, xSize, ySize) {
		static const float val = 0.8f;
		m_clear_color_base = { val,val,val,1.f };
	}
	Scrollable(float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: Scrollable("scrollable" + std::to_string(GUID), xPos, yPos, xSize, ySize) {
	}

	virtual DrawObj2D* ProxyAddChild(DrawObj2D* dObj);
	virtual void ProxyAddChilds(std::vector<DrawObj2D*> dObjs);
	void ProxyClearChilds();
	virtual void Init() override;
	virtual DrawObj2D* MemClone() { return new Scrollable(*this); }
private:
	Control* Proxy = this;
};

class Movable;
class PlacementSnap {
public:
	Align_S m_align = Align_S();
	Point2D<float> m_size{};
	void Save(Control* obj);
};
class Movable : public Dragger {
public:
	PlacementSnap PSnap = PlacementSnap();
	bool docked = false;
	float bar_size = 20;
	Control* Content = nullptr;
	void SetContent(Control* cont);
	bool InitDone = false;
	void FreeDockLock();
	void RevertDockLock();
	Movable(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Dragger(name, xPos, yPos, xSize, ySize) {
		static const float val = 0.1f;
		m_clear_color_base = { val,val,0.2f,1.f };
	}
	Movable(float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: Movable("movable" + std::to_string(GUID), xPos, yPos, xSize, ySize) {
	}
	virtual void Link(Framebuf* parent) override;
	virtual DrawObj2D* AddChild(DrawObj2D* dObj) override;

	virtual void AddChilds(std::vector<DrawObj2D*> dObjs) override;
	virtual void Init() override;
	virtual void Render() override;
};

struct GridChildData {
	int row;
	int col;
	int rowspan = 1;
	int colspan = 1;
};

class GridMeasueCol : public MeasureCol {
public:
	void CalcStar();
	float Sum(int pos, int num);
	float Sum(std::vector<float> shift, int pos, int num);
	std::vector<float> Cumulative();
	std::vector<float> Cumulative(std::vector<float> shift);
};

class Grid : public Control {
public:
	Grid(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Control(name, xPos, yPos, xSize, ySize) {
		static const float val = 0.1f;
		m_clear_color_base = { val,val,0.2f,1.f };
	}
	Grid(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Grid("Grid" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	std::unordered_map<DrawObj*, GridChildData*> ChildData;
	void SetColumn(DrawObj* dObj, int idx, int span = 1);
	void SetRow(DrawObj* dObj, int idx, int span = 1);
	GridMeasueCol rows;
	GridMeasueCol cols;
	std::vector<float> ccols;
	std::vector<float> crows;
	std::vector<float> shift_cols;
	std::vector<float> shift_rows;
	virtual DrawObj2D* AddChild(DrawObj2D* dObj) override;
	virtual void AddChilds(std::vector<DrawObj2D*> dObjs) override;
	virtual void Measure();
	virtual void Arrange();
	virtual void AdjustSize();
	virtual void Init();
	virtual void SetLinked() override;
	virtual void LinkChild(DrawObj2D* child) override;
	virtual void UnlinkChild(DrawObj2D* child) override;
};


#define sp_size 5.0f
class Splitter : public Dragger {
public:
	Splitter(std::string name, float xSize = sp_size, float ySize = sp_size) : Dragger(name, 0, 0, xSize, ySize) {
		static const float val = 0.1f; m_clear_color_base = { val,val,0.1f,1.f };
	}
	Splitter(float xSize = sp_size, float ySize = sp_size) : Splitter("Splitter" + std::to_string(GUID), xSize, ySize) {}
	Orientation_S ori = Orientation_S::Horizontal;
	virtual void ResizeNeighbours();
	virtual void Init();
public:
	static void splitter_drag_end_callback(Dragger* sender, DragEventArgs* e, params_t params);
};
class Resizer : public Dragger {
public:
	using Dragger::Dragger;
	Orientation_S ori;
	//virtual void Init();
};
class Resizable : public Movable {
	Splitter* Top, * Bot, * Left, * Right;




};

class TexDisplay : public Control {
	TextRenderer* TextRend = nullptr;
	DrawObj2D* Target = nullptr;
	Framebuf* DP = nullptr;
	void SetTarget(uint guid);

	template<typename T>
	void SetIndex(T count);
	void SetTextRend(TextRenderer& textRend) { NB1->SetTextRend(textRend); }
	NumBox1* NB1 = nullptr;
	TexDisplay(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Control(name, xPos, yPos, xSize, ySize) {
		static const float val = 0.1f;
		m_clear_color_base = { val,val,0.2f,1.f };
		//m_clear_color_base = { 0,0,0,1 };
	}
	TexDisplay(float xPos = 0, float yPos = 0, float xSize = 40, float ySize = 20)
		: TexDisplay("TexDisplay_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	virtual void Init() override;
	virtual DrawObj2D* MemClone() { return new TexDisplay(*this); }
};
template<typename T>
inline void TexDisplay::SetIndex(T count) {
	NB1->SetNumber(count);
}

class PropertyFiled : public Field {
public:
	PropertyFiled(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Field(name, xPos, yPos, xSize, ySize) {
		m_clear_color_base = { 0,0,0,1 };
	}
	PropertyFiled(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: PropertyFiled("PropertyFiled_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	virtual void Init() override;
	virtual DrawObj2D* MemClone() { return new PropertyFiled(*this); }
};

class StackField : public StackPanelGen<Field>::StackPanelT {
public:
	typedef  StackPanelGen<Field>::StackPanelT base_t;
	using base_t::base_t;
	virtual void Init() override;
	virtual DrawObj2D* MemClone() { return new StackField(*this); }
};

class TextField : public PropertyFiled {
public:

	TextField(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: PropertyFiled(name, xPos, yPos, xSize, ySize) {
		m_clear_color_base = { 0,0,0,1 };
	}
	TextField(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: TextField("TextField_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	virtual void Init() override;
	virtual DrawObj2D* MemClone() { return new TextField(*this); }
};

template<typename T>
class NumberField : public PropertyFiled {
public:
	DepObj<T> Number = DepObj<T>(this);
	NumBox1* NB = nullptr;

	NumberField(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: PropertyFiled(name, xPos, yPos, xSize, ySize) {
		m_clear_color_base = { 0,0,0,1 };
	}
	NumberField(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: NumberField("NumberField_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	virtual void Init() {
		PropertyFiled::Init();
		NumBox1* nb = new NumBox1(Name + "_nb", 0, 0, 0, 0, 0);
		nb->Size.Set("prop_field_nb_rel", { 0.65f,0 }, MMode::Rel);
		nb->Size.Set("prop_field_nb_abs", { 0, FIELD_H }, MMode::Abs);
		nb->Pos.Set("prop_field_nb_rel", { 0.35f,0 }, MMode::Rel);
		nb->Init(); NB = nb; AddChild(nb);
		nb->NB->m_colorPar = { 1,1,1,1 };
		bmtd::SetBinding(&nb->NB->Number, &Number, BindingMode::Both);
	}
	virtual void Render() override {
		Control::Render();
	}
	virtual DrawObj2D* MemClone() { return new NumberField(*this); }
};

class ButtonField : public PropertyFiled {
public:
	Button* BTN = nullptr;
	using PropertyFiled::PropertyFiled;
	virtual void Init() override;
	virtual DrawObj2D* MemClone() { return new ButtonField(*this); }
};

class CheckBoxField : public PropertyFiled {
public:
	CheckBox* CB = nullptr;
	void SetEnable(bool newVal) { CB->SetEnable(newVal); }
	CheckBoxField(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: PropertyFiled(name, xPos, yPos, xSize, ySize) {
		m_clear_color_base = { 0,0,0,1 };
	}
	CheckBoxField(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: CheckBoxField("CheckBoxField_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	virtual void Init() override;
	virtual void Render() override;
	virtual DrawObj2D* MemClone() { return new CheckBoxField(*this); }
};

class CollapsableField : public StackField {
public: 
	CheckBox* Collapser = nullptr; 
	StackPanel* Content = nullptr;
	CollapsableField(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: StackField(name, xPos, yPos, xSize, ySize) {
		//m_clear_color_base = { 0,0,0,1 };
	}
	CollapsableField(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: CollapsableField("CollapsableField_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	void SetExpand(bool newVal);
	virtual void Init() override;
	virtual void Prepare() override;
	virtual DrawObj2D* MemClone() { return new CollapsableField(*this); }
};

class TextureField : public StackField {
public:
	class TextureID_DO : public DepObj<float> {
		typedef DepObj<float> base_t;
		using base_t::base_t;
		void OnSet(void* owner, float newVal) override {}
	};
	std::vector<Texture2D*>* texCol = nullptr;
	//TextureID_DO texID = TextureID_DO(this);
	void SetTexIndex(int val);
	void OnTexIndexChanged();
	void ParseTexIndex(int val);
	float* texID = nullptr;
	NumBox1* NB = nullptr;
	Texbuf* texBuf = nullptr;
	TextureIndexChangeEvent_E e_TexIndexChange = TextureIndexChangeEvent_E(&dpch::InvokeQ, this);

	TextureField(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: StackField(name, xPos, yPos, xSize, ySize) {
		m_clear_color_base = { 0,0,0,1 };
	}
	TextureField(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: TextureField("TextureField_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	virtual void Init() override;
	virtual DrawObj2D* MemClone() { return new TextureField(*this); }
};

class AdjustTextureField : public TextureField {
public:
	NumBox1* NB_Intens = nullptr;
	CheckBoxField* CBF_Enable = nullptr;
	float* intensity = nullptr;
	using TextureField::TextureField;
	virtual void Init() override;
	virtual DrawObj2D* MemClone() { return new AdjustTextureField(*this); }
};

class DrawObj3D : public DrawObj {
public:

	DrawObj3D(std::string name) : DrawObj(name) {};
	DrawObj3D() : DrawObj3D("GUID_" + std::to_string(GUID)) {}

	virtual void Init() = 0;
	virtual void Update(float dt) = 0;
	virtual void ProcessInput(float dt, InputArgs_S& input) = 0;
	virtual void Render() = 0;
	virtual void Render(Shader* shader) = 0;

	virtual Point2D<float> SetPickFb() = 0;
	virtual bool PickAt(float xOld, float yOld) = 0;
	//virtual PixelInfo BeginPickFunc(float xNew, float yNew);
	virtual PixelInfo PickFunc(float xNew, float yNew) = 0;

	/*
	Event_E e_PreRend = Event_E(&Dispatcher::InvokeQ, this);
	Event_E e_Display = Event_E(&Dispatcher::InvokeQ, this);
	Event_E e_Update = Event_E(&Dispatcher::UpdateQ, this);
	*/
};
class UI : public NamedObj {
public:
	CollapsableField* field;
	UI() : NamedObj("UI") {
		field = new CollapsableField();
		field->HAlign = HorAlign::Stretch;
		field->Init(); field->SetExpand(true);
	}
	virtual void GenerateFields() = 0;
	virtual void Rebind(Component* newHost) = 0;
	virtual void Notify() = 0;
	virtual void Free() = 0;
};
class Component;
class Camera;
template<typename T>
class UIManage {
	static_assert(std::is_base_of<UI, T>::value, "T must inherit from UI");
public:
	static std::unordered_set<T*> ActiveUIs;
	static std::unordered_set<T*> InactiveUIs;
	// cast je this klase naslednice
	static T* GetUI() {
		if (InactiveUIs.size() > 0) {
			T* UI = *InactiveUIs.begin();
			InactiveUIs.erase(UI);
			ActiveUIs.insert(UI);
			return UI;
		} else {
			auto newUI = new T();
			newUI->GenerateFields();
			ActiveUIs.insert(newUI);
			return newUI;
		}
	}
	static void FreeUI(T* cast) {
		if (ActiveUIs.count(cast) > 0) {
			InactiveUIs.insert(cast);
			ActiveUIs.erase(cast);
		}
	}
};

class TextureUI : public UI {
public:
	class Texture_Cord : public EDepObj<float, NumBox1, NumBox1ValueChangeEvent_E> {
	public:
		void ParseVal(NumBox1* nb) override;
		void SendBack() override {
			source->SetNumber(value[0]);
		}
	};
	NumberField<float>* nfu = nullptr;
	NumberField<float>* nfv = nullptr;
	Texture_Cord* u = new Texture_Cord();
	Texture_Cord* v = new Texture_Cord();
	void GenerateFields() override;
	void Rebind(Component* newHost) override;
	void Notify() override;
	void Free() override { UIManage<TextureUI>::FreeUI(this); }
};

class TexIndexUI : public UI {
public:
	class Index_DO : public EDepObj<int, NumBox1, NumBox1ValueChangeEvent_E> {
	public:
		void ParseVal(NumBox1* nb) override {
			auto val = nb->NB->Number.value[0];
			if (val < 0) {
				val = 0;
			}
			value[0] = (int)val;
		}
		void SendBack() override {
			source->SetNumber(value[0]);
		}
	};
	NumberField<int>* nfindex = nullptr;
	Index_DO* texindex = new Index_DO();
	void GenerateFields() override;
	void Rebind(Component* newHost) override;
	void Notify() override;
	void Free() override { UIManage<TexIndexUI>::FreeUI(this); }
};

class MultiTexture;
class MultiTextureUI : public UI {
public:
	MultiTexture* owner = nullptr;
	class TPsSize : public DepObj<int> {
		typedef DepObj<int> base_t;
		using base_t::base_t;
		void OnSet(void* owner, int newVal) override {
		}
	};
	TPsSize* tp_size = new TPsSize();
	std::vector<TextureUI*> texPoints;
	void GenerateFields() override;
	void Rebind(Component* newHost) override;
	void Notify() override;
	void Free() override;
};
class TransformUI : public UI {
public:
	class Transf_Cord : public EDepObj<float, NumBox1, NumBox1ValueChangeEvent_E> {
	public:
		void ParseVal(NumBox1* nb) override {
			auto val = nb->NB->Number.value[0];
			value[0] = val;
		}
		void SendBack() override {
			source->SetNumber(value[0]);
		}
	};
	NumberField<float>* nfx = nullptr;
	NumberField<float>* nfy = nullptr;
	NumberField<float>* nfz = nullptr;
	Transf_Cord* x = new Transf_Cord();
	Transf_Cord* y = new Transf_Cord();
	Transf_Cord* z = new Transf_Cord();
	void GenerateFields() override;
	void Rebind(Component* newHost) override;
	void Notify() override;
	void Free() override { UIManage<TransformUI>::FreeUI(this); }
};
class CameraUI : public UI {
public:
	class Transf_Cord : public EDepObj<float, NumBox1, NumBox1ValueChangeEvent_E> {
	public:
		void ParseVal(NumBox1* nb) override {
			auto val = nb->NB->Number.value[0];
			value[0] = val;
		}
		void SendBack() override {
			source->SetNumber(value[0]);
		}
	};
	NumberField<float>* nfx = nullptr;
	NumberField<float>* nfy = nullptr;
	NumberField<float>* nfz = nullptr;
	Transf_Cord* x = new Transf_Cord();
	Transf_Cord* y = new Transf_Cord();
	Transf_Cord* z = new Transf_Cord();
	void GenerateFields() override;
	void Rebind(Component* newHost) override;
	void Notify() override;
	void Free() { UIManage<CameraUI>::FreeUI(this); }
};

class Object;
class Component {
public:
	Object* owner = nullptr;
	Component(Object* owner_p = nullptr) {
		owner = owner_p;
	}
	virtual Control* RaiseUI() = 0;
	virtual void GetUI() = 0;
	virtual void FreeUI() = 0;
	virtual void Notify() = 0;
};

class TexIndex : public Component {
public:
	int index;
	TexIndexUI* UI = nullptr;
	Control* RaiseUI() { if (UI == nullptr) GetUI(); return UI->field; };
	void GetUI() { UI = UIManage<TexIndexUI>::GetUI(); UI->Rebind(this); }
	void FreeUI() { UI->Free(); UI = nullptr; }
	void Notify() { if (UI != nullptr) UI->Notify(); }
	using Component::Component;
};

class Transform : public Component {
public:
	glm::vec3 positions{};
	TransformUI* UI = nullptr;
	Control* RaiseUI() { if (UI == nullptr) GetUI(); return UI->field; };
	void GetUI() { UI = UIManage<TransformUI>::GetUI(); UI->Rebind(this); }
	void FreeUI() { if (UI != nullptr) { UI->Free(); UI = nullptr; } }
	void Notify() { if (UI != nullptr) UI->Notify(); }
	using Component::Component;
};

class Texture : public Component {
public:
	glm::vec2 positions{ 0, 0 };
	TextureUI* UI = nullptr;
	TextureUVChangeEvent_E e_textureUVChange = TextureUVChangeEvent_E(&dpch::InvokeQ, this);
	Texture(Object* owner_p, float x, float y) : Component(owner_p) {
		positions = { x,y };
	}
	Control* RaiseUI() { if (UI == nullptr) GetUI(); return UI->field; };
	void GetUI() { UI = UIManage<TextureUI>::GetUI(); UI->Rebind(this); }
	void FreeUI() { UI->Free(); UI = nullptr; }
	void Notify() { if (UI != nullptr) UI->Notify(); }
	using Component::Component;
};

class MultiTexture : public Component {
public:
	std::vector<Texture*> texPoints;
	MultiTextureUI* UI = nullptr;
	fields_t FIELDS;
	Control* RaiseUI() { if (UI == nullptr) GetUI(); return UI->field; };
	void GetUI() { UI = UIManage<MultiTextureUI>::GetUI(); UI->Rebind(this); }
	void FreeUI() { UI->Free(); UI = nullptr; }
	void Notify() { if (UI != nullptr) UI->Notify(); }
	using Component::Component;
};
class Scene;
class Object {
public:
	virtual void RaiseInspector(Scene* scene);
	std::vector<Component*> InspCompon;
	Transform* transform = new Transform(this);
	Object() {
		InspCompon.push_back(transform);
	}

};

class Control3D : public Object, public DrawObj3D, public IMouseEvents {
public:
	Control3D(std::string name) : DrawObj3D(name), IMouseEvents(this) {};
	Control3D() : Control3D("GUID_" + std::to_string(GUID)) {}
	virtual void Init() override;
	virtual void Update(float dt) override;
	virtual void ProcessInput(float dt, InputArgs_S& input) override;
	virtual void Render() override;
	virtual void Render(Shader* shader) override;

	virtual void PickNext(float xNew, float yNew);
	virtual Point2D<float> SetPickFb() override;
	virtual bool PickAt(float xOld, float yOld) override;
	virtual PixelInfo BeginPickFunc(float xNew, float yNew);
	virtual PixelInfo PickFunc(float xNew, float yNew);
	//virtual void OnChange() {};

	Event_E e_PreRend = Event_E(&Dispatcher::InvokeQ, this);
	Event_E e_Display = Event_E(&Dispatcher::InvokeQ, this);
	Event_E e_Update = Event_E(&Dispatcher::UpdateQ, this);
	SelectionChangeEvent_E e_SelectionChanged = SelectionChangeEvent_E(&dpch::InvokeQ, this);

	void SetSelected(bool val) {
		if (val != States.Selected) {
			States.Selected = val;
			e_SelectionChanged.event_args->OldValue = !val;
			e_SelectionChanged.event_args->Value = val;
			e_SelectionChanged.Invoke();
		}
	}
	inline bool GetSelected() const { return States.Selected; }

	void Translate(glm::vec3 value);
	virtual std::string GetType() { static std::string ret = "Control3D"; return ret; }
	virtual DrawObj3D* MemClone() { return new Control3D(*this); }
};
const float SPEED = 2.5f, SENSITIVITY = 0.1f, ZOOM = 45.0f;
class Camera : public Component, public Object {
public:
	CameraUI* UI = nullptr;
	Point2D<float*> LogicalSize;
	glm::vec3 Front, Up, Right, WorldUp;
	float Yaw, Pitch, MovementSpeed, MouseSensitivity, Zoom;
	bool firstMouse = true;
	ProjectionMode PMode = ProjectionMode::Perspective;
	Control* RaiseUI() { if (UI == nullptr) GetUI(); return UI->field; };
	void GetUI() { UI = UIManage<CameraUI>::GetUI(); UI->Rebind(this); }
	void FreeUI() { UI->Free(); UI = nullptr; }
	void Notify() { if (UI != nullptr) UI->Notify(); }
	Camera* Copy() {
		Camera* newCam = new Camera(*this);
		newCam->UI = nullptr;
		newCam->transform = new Transform(transform[0]);
		newCam->transform->UI = nullptr;
		return newCam;
	}
	Camera(Object* owner_p, glm::vec3 position = glm::vec3(0.0f, 3.0f, 5.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 front = { 0,0,-1 })
		: Component(owner_p), Front(front), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
		transform->positions = position;
		WorldUp = up;
		updateAngles();
		updateVectors();
	}
	glm::mat4 GetViewMatrix() {
		return glm::lookAt(transform->positions, transform->positions + Front, Up);
	}
	glm::mat4 GetProjectionMatrix();
	void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
		float velocity = MovementSpeed * deltaTime;
		if (direction == Camera_Movement::FORWARD)
			transform->positions += Front * velocity;
		if (direction == Camera_Movement::BACKWARD)
			transform->positions -= Front * velocity;
		if (direction == Camera_Movement::LEFT)
			transform->positions -= Right * velocity;
		if (direction == Camera_Movement::RIGHT)
			transform->positions += Right * velocity;
		Notify();
		transform->Notify();
		//transform->UI->Notify();
	}
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
		xoffset *= MouseSensitivity; yoffset *= MouseSensitivity;
		Yaw -= xoffset; Pitch -= yoffset;
		if (constrainPitch) {
			if (Pitch > 89.0f) Pitch = 89.0f;
			if (Pitch < -89.0f) Pitch = -89.0f;
		}
		updateVectors();
	}
	void Pivot(glm::vec3 center, Point2D<float> cords);
	void ProcessMouseScroll(float yoffset);
private:
	void updateVectors();
	void updateAngles();

};
class Vert : public Control3D {
public:
	std::vector<Component> components;
	uint VAO = 0, VBO = 0;
	Vert() { m_shader = rmtd::GetShader("allpoints"); }
	Vert(glm::vec3 pos) :Vert() { transform->positions = pos; }
	virtual void Init() override;
	virtual void Render() override;
	PixelInfo PickFunc(float xNew, float yNew);
	virtual std::string GetType() { static std::string ret = "Vert"; return ret; }
};

inline void texture_uv_changed_callback(Texture* sender, TextureUVChangeEventArgs* e, params_t params);
class Triangle : public Control3D {
public:
	Vert* v1, * v2, * v3;
	Texture* v1_tex = new Texture(this, 0, 0);
	Texture* v2_tex = new Texture(this, 0, 1);
	Texture* v3_tex = new Texture(this, 1, 1);
	glm::vec3 tangent, bitangent, normal;
	MultiTexture* texture = new MultiTexture(this);
	uint VAO = 0, VBO = 0;
	Triangle(Vert* p_v1, Vert* p_v2, Vert* p_v3) : v1(p_v1), v2(p_v2), v3(p_v3) {
		Setup();
	}
	typedef std::vector<Vert*>::const_iterator vert_iter;
	Triangle(vert_iter begin) {
		v1 = *begin; begin++;
		v2 = *begin; begin++;
		v3 = *begin;
		Setup();
	}
	void Setup() {
		ComputeTBN();
		InitInspector();
		v1_tex->e_textureUVChange.Add(this, texture_uv_changed_callback, { this });
		v2_tex->e_textureUVChange.Add(this, texture_uv_changed_callback, { this });
		v3_tex->e_textureUVChange.Add(this, texture_uv_changed_callback, { this });
	}
	virtual void InitInspector() {
		texture->texPoints.push_back(v1_tex);
		texture->texPoints.push_back(v2_tex);
		texture->texPoints.push_back(v3_tex);
		InspCompon.push_back(texture);
	}
	void ComputeTBN();
	virtual void Init() override;
	virtual void Render() override;
	PixelInfo PickFunc(float xNew, float yNew);
	virtual std::string GetType() { static std::string ret = "Triangle"; return ret; }
};
void texture_uv_changed_callback(Texture* sender, TextureUVChangeEventArgs* e, params_t params) {
	Triangle* this_v = (Triangle*)params[0];
	this_v->ComputeTBN();
}