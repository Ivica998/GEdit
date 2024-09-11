#pragma once
#include "control.h"
#include "camera.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils.h" //for WIN_WIDTH
#include <vector>

struct Material {
	static int count;
	std::string name;
	glm::vec3 Ka{ 1,1,1 };
	glm::vec3 Kd{ 1,1,1 };
	glm::vec3 Ks{ 0,0,0 };
	float Ns = 10.0f;
	float Ni = 1.0f; // optical desnsity; index of refraciton; 1.0 = light does not bend
	float d = 1.0f;
	//float illum = 1.0f;
	std::string map_Ka;
	std::string map_Kd;
	std::string map_Ks;
	std::string map_Ns;
	std::string map_d;
	std::string bump;
	std::string disp;
	std::string decal;
	std::string mtl_content() {
		stringstream str(stringstream::out);
		//str << "newmtl " << "mat" << std::to_string(count++) << "\n";
		str << "newmtl " << name << "\n";
		str << "Ka " << Ka.x << " " << Ka.y << " " << Ka.z << "\n";
		str << "Kd " << Kd.x << " " << Kd.y << " " << Kd.z << "\n";
		str << "Ks " << Ks.x << " " << Ks.y << " " << Ks.z << "\n";
		str << "Ns " << Ns << "\n";
		str << "Ni " << Ni << "\n";
		str << "d "  << Ns << "\n";
		if (map_Ka != "") str << "map_Ka " << map_Ka << "\n";
		if (map_Kd != "") str << "map_Kd " << map_Kd << "\n";
		if (map_Ks != "") str << "map_Ks " << map_Ks << "\n";
		if (map_Ns != "") str << "map_Ns " << map_Ns << "\n";
		if (map_d  != "") str << "map_d "  << map_d  << "\n";
		if (bump   != "") str << "bump "   << bump   << "\n";
		if (disp   != "") str << "disp "   << disp   << "\n";
		if (decal  != "") str << "decal "  << decal  << "\n";
		return str.str();
	}
};
class Scene;
struct TexPickFD;
struct CamsFD;
class SceneUI : public UI {
public:
	SceneUI(Scene* owner_p) : owner(owner_p) {
		GenerateFields();}
	Scene* owner = nullptr;
	CollapsableField* CLPS_SCENE = nullptr;
	CollapsableField* CLPS_CAMS = nullptr;
	CheckBoxField* CUBE_WF_FIELD = nullptr;
	CheckBoxField* TRIANGLE_WF_FIELD = nullptr;
	CheckBoxField* CKBX_GRID_SNAP = nullptr;
	CollapsableField* CLPS_TEXTURES = nullptr;
	void GenerateFields() override;
	void Rebind(Component* newHost) override {}
	void Notify() override {}
	void Free() override { }
private:
	CollapsableField* MakeTexPickField(TexPickFD& texFD, std::vector<Texture2D*>& textures);
	CollapsableField* MakeCamsField(CamsFD& camFD);
};

enum class PhaseE {
	NONE,
	PREP,
	DOING,
};
enum class SceneNavMode {
	NONE,
	FLYTHROUGH,
	PAN,
	ORBIT,
	DRAGZOOM,
};
enum class SceneState : int {
	NONE,
	DRAGGING,
	PICKING,
	MOVING,
	CREATE_CUBE,
	CREATE_TRIANGLE
};
struct DragData_S {
	Control3D* obj;
	std::unordered_set<Control3D*> dragged;
	glm::vec3 drag_origin;
	glm::vec2 oldscreenPos;
	bool drag_began = false;
	glm::vec3 center;
	void Clear() {
		obj = nullptr;
		drag_began = false;
		drag_origin = { 0,0,0 };
		oldscreenPos = { 0,0 };
		for (auto iter = dragged.begin();iter != dragged.end();) {
			(*iter++)->SetSelected(false);}
		for (auto obj : dragged) {
			obj->SetSelected(false);}
		center = { 0,0,0 };
		dragged.clear();
	}
	glm::vec3 Center() {
		center = { 0,0,0 };
		for (auto v : dragged) {
			center += v->transform->positions;}
		center /= (int)dragged.size();
		return center;
	}
};
struct CreateData_S {
	Control3D* obj;
	void Clear() {
		obj = nullptr;}
};
struct TriangleDefData_S {
	std::unordered_set<Vert*> verts;
	std::vector<Vert*> vq;
	void Clear() {
		auto iter = vq.begin();
		while (iter != vq.end()) { (*iter)->SetSelected(false); iter++; }
		verts.clear();
		vq.clear();}
	Triangle* DefineT() {
		auto iter = vq.begin();
		Triangle* tri = new Triangle(iter);
		vq[0]->SetSelected(false);
		verts.erase(vq[0]);
		vq.erase(vq.begin());
		tri->Init();
		return tri;}
	void Add(Vert* v) {
		v->SetSelected(true);
		verts.insert(v);
		vq.push_back(v);}
};
struct ModifierKeys_S {
	bool alt = false;
	bool ctrl = false;
	bool shift = false;
};
struct TriangleVAOManageData {
	static const uint extra_max = 10;
	int extra_cur = 0;
	bool changed;
	void New() {
		extra_cur++;
		changed = true;
	}
	void Del() {
		extra_cur--;
		changed = true;
	}
	void Clear() { extra_cur = 0; changed = false; }
	bool Ready() { return extra_cur >= extra_max; }
};
struct TriangleVertData_S {
	glm::vec3 v;
	glm::vec2 vt;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};
struct TriangleData_S {
	TriangleVertData_S p1;
	TriangleVertData_S p2;
	TriangleVertData_S p3;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct BufObj {
	uint VAO, VBO, FBO;
	std::vector<Texture2D*> Texs;
};

class Scene;
class WireFrame_DO : public DepObj<bool> {
	typedef DepObj<bool> base_t;
	using base_t::base_t;
	void OnSet(void* owner, bool newVal) override {
	}
};
struct TexPickFD {
	class TextureID_DO : public DepObj<float> {
		typedef DepObj<float> base_t;
		using base_t::base_t;
		void OnSet(void* owner, float newVal) override;
	};
	class TextureEnabled_DO : public DepObj<bool> {
		typedef DepObj<bool> base_t;
		using base_t::base_t;
		void OnSet(void* owner, bool newVal) override;
	};
	//Material mat;
	float* tex_index = nullptr;
	TextureEnabled_DO tex_enable{};
	float* intensity = nullptr;
	Texture2D* picked_tex = nullptr;
	Scene* owner;
	std::string enable_uniform_name;
	TexPickFD(Scene* owner_p, std::string eun);
};
struct CamsFD {
	class CamIndex_EDO : public EDepObj<float, NumBox1, NumBox1ValueChangeEvent_E> {
	public:
		void ParseVal(NumBox1* nb) override {
			auto val = nb->NB->Number.value[0];
			value[0] = val;
		}
		void Condition(float val){
			CamsFD* this_v = (CamsFD*)owner;
			this_v->ChangeActiveCam((int)val);
		}
		void SendBack() override {
			source->SetNumber(value[0]);
		}
	};
	CamIndex_EDO cam_index_edo;
	Camera* picked_cam = nullptr;
	Scene* owner;
	StackPanel* Content;
	CamsFD(Scene* owner_p) {
		cam_index_edo.owner = this;
		cam_index_edo.value = new float();
		cam_index_edo.value[0] = -1;
		owner = owner_p;
		Content = new StackPanel("activeCamZone");
		Content->Init();
	}
	void ChangeActiveCam(int newVal);
};

class Scene : public Control,public Object, public Component {
public:
	SceneUI* UI = nullptr;	

	InspectorData InspData;
	CollapsableField* HierInfo;
	CollapsableField* Triangle_HI;
	CollapsableField* Vert_HI;

	WireFrame_DO TriangleWF = WireFrame_DO(this);
	WireFrame_DO CubeWF = WireFrame_DO(this);
	TexPickFD diffuse_tp{ this, "diffuse_enabled" };
	TexPickFD normals_tp{ this, "normals_enabled" };
	TexPickFD displace_tp{ this, "displace_enabled" };
	std::vector<Texture2D*> textures;
	Texture2D* PickTex = nullptr;
	SceneNavMode SNavM = SceneNavMode::NONE;
	DepObj<bool> SnapToGrid = DepObj<bool>(this);;

	SceneState SState = SceneState::NONE;
	SceneState SStateOld = SceneState::NONE;
	PhaseE Phase = PhaseE::NONE;
	DragData_S DragData{};

	CreateData_S CreateData{};
	TriangleDefData_S TDData{};
	TriangleVAOManageData TVAOMData{};
	void RegisterForDrag(Control3D* obj);
	void DoDrag();
	ModifierKeys_S ModK{};
	uint VBOT, VAOT, FBOT;
	BufObj InfGrid{};
	BufObj DrawGrid{};

	SceneEvent_E e_SceneStateChanged = SceneEvent_E(&dpch::InvokeQ, this);
	SceneEvent_E e_SceneSelectionLost = SceneEvent_E(&dpch::InvokeQ, this);

	std::vector<Camera*> cams;
	Camera* mainCam = nullptr;
	CamsFD camFD{ this };

	vector<Control3D*> Objects;
	std::vector<Vert*> vertices;
	std::vector< Triangle*> triangles;
	Shader* active_shader = nullptr;
	Shader* vert_shader = nullptr;
	Shader* triangle_shader = nullptr;
	Shader* triangle_shader_tex = nullptr;
	Shader* pick_shader = nullptr;
	Shader* triangle_pick_shader = nullptr;
	Shader* grid_shader = nullptr;
	Shader* draw_grid_shader = nullptr;
	uint cubeVAO = 0;

	void RaiseHierarchy();
	void RaiseInspector();
	void ClearInspector();
	void SetInspector(Object* sender);
	virtual void Link(Framebuf* parent) override;
	void GridInit();
	void RenderGrid();
	void DrawGridInit();
	void RenderDrawGrid();
	void TriangleVAOInit(uint objNum);
	void TriangleVAOSetup(uint objNum);
	void TriangleVAOFill(std::vector<TriangleVertData_S>& data);
	virtual void RenderTriangles();
	void RenderVerts();
	Vert* AddVert(glm::vec3 vert);
	Vert* AddVert(Vert* vert);
	void AddVerts(std::vector<glm::vec3> verts);
	void RemoveVert(Vert* vert);
	Triangle* AddTriangle(uint v1_id, uint v2_id, uint v3_id);
	Triangle* AddTriangle(Triangle* tri);
	void AddTriangles(std::vector<Triangle*> tris);
	void RemoveTriangle(Triangle* tri);

	Point2D<float> SetPickFb() override;
	void EnableInfGridShader();
	void EnableDrawGridShader();
	void EnableVertShader();
	void EnablePickShader();
	void EnableTriangleShader();
	void EnableTrianglePickShader();
	PixelInfo BeginPickFunc(float xNew, float yNew) override;
	virtual PixelInfo PickFunc(float xNew, float yNew) override;
	virtual bool TryChangeActivity(bool value) override;
	virtual void ProcessInput(float dt, InputArgs_S& input) override;

	void SaveDesignedModel(std::string filename);

	Scene(std::string name, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Control(name, xPos, yPos, xSize, ySize) {
		m_clear_color_base = { 0,0,0,1 };}
	Scene(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0)
		: Scene("Scene_" + std::to_string(GUID), xPos, yPos, xSize, ySize) {}
	void InitSceneData();
	virtual void Init() override;
	virtual void BeginRender(bool clear = true) override;
	virtual void Render() override;
	virtual void Prepare() override;

	Control* RaiseUI() { return UI->field; };
	void GetUI() {}
	void FreeUI() {}
	void Notify() { UI->Notify(); }

	virtual DrawObj2D* MemClone() { return new Scene(*this); }
};