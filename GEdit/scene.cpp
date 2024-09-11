#include "scene.h"

int Material::count = 0;
// ********************************************** SceneUI *******************************************************
void text_field_tex_index_changed_callback(TextureField* sender, TextureIndexChangeEventArgs* e, params_t params) {
    TexPickFD* this_v = (TexPickFD*)params[0];
    int size = (int)this_v->owner->textures.size();
    int intVal = e->index;
    if (intVal >= 0 && intVal < size) {
        this_v->picked_tex = this_v->owner->textures[intVal];} 
    else {
        intVal = std::max(std::min(intVal, size), -1);
        sender->SetTexIndex(intVal);}
}
CollapsableField* SceneUI::MakeTexPickField(TexPickFD& texFD, std::vector<Texture2D*>& textures) {
    CollapsableField* clps_image = new CollapsableField("clps_img");
    clps_image->HAlign = HorAlign::Stretch;
    clps_image->Init();
    AdjustTextureField* tftid = new AdjustTextureField();
    tftid->Init();
    tftid->texCol = &textures;
    tftid->Title->SetText("Index:");
    tftid->Title->m_scalePar = 0.4f;
    clps_image->Content->AddChild(tftid);
    texFD.tex_index = tftid->texID;
    texFD.tex_index[0] = -1;
    tftid->e_TexIndexChange.Add(this, text_field_tex_index_changed_callback, { &texFD });
    bmtd::SetBinding(&tftid->CBF_Enable->CB->IsEnabled, &texFD.tex_enable);
    texFD.intensity = tftid->intensity;
    return clps_image;
}
void add_camera_callback(sender_ptr sender, MouseEventArgs* e, params_t params) {
    Scene* scene = (Scene*)params[0];
    Camera* newCam = scene->camFD.picked_cam[0].Copy();
    scene->cams.push_back(newCam);
    scene->camFD.ChangeActiveCam((int)scene->cams.size());
}
CollapsableField* SceneUI::MakeCamsField(CamsFD& camsFD) {
    CollapsableField* clps_cams = new CollapsableField("clps_img");
    clps_cams->HAlign = HorAlign::Stretch;
    clps_cams->Init();
    NumBox1* camIndex = new NumBox1(clps_cams->Name + "_camIndex", 0, 0, 0, 0, 0);
    camIndex->Size.Set("prop_field_camIndex_abs", { 0, FIELD_H }, MMode::Abs);
    camIndex->Init();
    camIndex->NB->m_colorPar = { 1,1,1,1 };
    camIndex->NB->IsDecimal = false;
    camIndex->e_ValueChange.Add(this, CamsFD::CamIndex_EDO::full_parse_callback, { &camsFD.cam_index_edo });
    camsFD.cam_index_edo.source = camIndex;
    camsFD.cam_index_edo.Set(0);
    Button* btn = new Button();
    btn->Init();
    btn->e_MouseRelease.Add(this, add_camera_callback, { owner });
    btn->Content->SetText("Add Camera");
    btn->Size.Set("prop_field_camAdd_abs", { 0, FIELD_H }, MMode::Abs);
    clps_cams->Content->AddChilds({ camIndex, camsFD.Content, btn });
    return clps_cams;
}
void SceneUI::GenerateFields() {

    CollapsableField* CLPS_SCENE = new CollapsableField("CLPS_SCENE");
    CLPS_SCENE->HAlign = HorAlign::Stretch;
    CLPS_SCENE->Init();
    CLPS_SCENE->Title->SetText("--Scene--");

    CollapsableField* CLPS_CAMS = MakeCamsField(owner->camFD);
    CLPS_CAMS->Title->SetText("Cams:");
    CheckBoxField* CUBE_WF_FIELD = new CheckBoxField("CUBE_WF_FIELD");
    CUBE_WF_FIELD->Init();
    CUBE_WF_FIELD->Title->SetText("CubeWF");
    CheckBoxField* TRIANGLE_WF_FIELD = new CheckBoxField("TRIANGLE_WF_FIELD");
    TRIANGLE_WF_FIELD->Init();
    TRIANGLE_WF_FIELD->Title->SetText("TriangleWF");
    TRIANGLE_WF_FIELD->Title->m_scalePar -= 0.1f;
    CheckBoxField* CKBX_GRID_SNAP = new CheckBoxField("CKBX_GRID_SNAP");
    CKBX_GRID_SNAP->Init();
    CKBX_GRID_SNAP->Title->SetText("SnapToGrid");
    CKBX_GRID_SNAP->Title->m_scalePar -= 0.1f;

    CollapsableField* clps_diffuse = MakeTexPickField(owner->diffuse_tp, owner->textures);
    clps_diffuse->Title->SetText("Diffuse");
    CollapsableField* clps_normals = MakeTexPickField(owner->normals_tp, owner->textures);
    clps_normals->Title->SetText("Normals");
    CollapsableField* clps_displace = MakeTexPickField(owner->displace_tp, owner->textures);
    clps_displace->Title->SetText("Displace");
    CollapsableField* CLPS_TEXTURES = new CollapsableField("CLPS_TEXTURES");
    CLPS_TEXTURES->HAlign = HorAlign::Stretch;
    CLPS_TEXTURES->Init();
    CLPS_TEXTURES->Title->SetText("Textures");
    // SceneUI::Init(){...
    CLPS_CAMS->Content->AddChilds({ CUBE_WF_FIELD,TRIANGLE_WF_FIELD });
    CLPS_TEXTURES->Content->AddChilds({ clps_diffuse,clps_normals,clps_displace });
    CLPS_SCENE->Content->AddChilds({ CLPS_CAMS, CKBX_GRID_SNAP, CLPS_TEXTURES });
    // ...}
    CLPS_CAMS->SetExpand(true);
    bmtd::SetBinding(&CUBE_WF_FIELD->CB->IsEnabled, &owner->CubeWF);
    bmtd::SetBinding(&TRIANGLE_WF_FIELD->CB->IsEnabled, &owner->TriangleWF);
    bmtd::SetBinding(&CKBX_GRID_SNAP->CB->IsEnabled, &owner->SnapToGrid);
    CKBX_GRID_SNAP->SetEnable(true);
    CLPS_SCENE->SetExpand(true);
    field = CLPS_SCENE;
}
// ********************************************** TexPickFD *******************************************************
void TexPickFD::TextureID_DO::OnSet(void* owner, float newVal) {

}
void TexPickFD::TextureEnabled_DO::OnSet(void* owner, bool newVal) {
    TexPickFD* this_v = (TexPickFD*)owner;
    auto& shader = this_v->owner->triangle_shader_tex;
    shader->Use();
    shader->SetBool(this_v->enable_uniform_name.c_str(), newVal);
    this_v->owner->active_shader->Use();
}
// ********************************************** CamsFD *******************************************************
void CamsFD::ChangeActiveCam(int newVal) {
    int size = (int)owner->cams.size();
    int index = (int)newVal;
    if (index >= 0 && index < size) {
        if (picked_cam != nullptr)
            picked_cam->FreeUI();
        picked_cam = owner->cams[index];
        owner->mainCam = picked_cam;
        Control* fields = picked_cam->RaiseUI();
        Content->ClearChilds();
        Content->AddChild(fields);

    } else {
        index = std::max(std::min(index, size - 1), 0);
        cam_index_edo.value[0] = ((float)index);
    }
}
// ********************************************** Scene *******************************************************
void refresh_trigger_cb(sender_ptr sender, EventArgs* e, params_t params) {
    ((DrawObj2D*)params[0])->OnChange();
}
void scene_mb_release(sender_ptr sender, MouseClickEventArgs* e, params_t params) {
    glfwSetInputMode(rmtd::CurWin->GLID, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    Scene* this_v = (Scene*)sender;
    this_v->SNavM = SceneNavMode::NONE;

}
int fba_tex_bind() {
    GLint encoding = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &encoding);
    return encoding;
}
int r_tex_bind() {
    GLint whichID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID);
    return whichID;
}
void Scene::Init() {
    Control::Init();
    cubeVAO = utl::makeCube6VAO();
    TriangleVAOInit(TVAOMData.extra_max);
    if (mainCam == nullptr) {
        mainCam = new Camera(this, glm::vec3(0.0f, 3.0f, 5.0f));
        cams.push_back(mainCam);
    }
    mainCam->LogicalSize = { &ActualSize.X,&ActualSize.Y };
    vert_shader = rmtd::GetShader("vert_col");
    vert_shader->Use();
    vert_shader->SetInt("FragColor", 0);//write
    vert_shader->SetInt("PickInfo", 1);
    vert_shader->SetBool("selected", false);
    triangle_shader = rmtd::GetShader("triangle_col");
    triangle_shader->Use();
    triangle_shader->SetBool("selected", false);
    triangle_shader->SetVec4f("color", { 0.0f,1.0f,0.6f,0.8f });
    triangle_shader->SetBool("wireframe", false);
    triangle_shader_tex = rmtd::GetShader("triangle_tex");
    triangle_shader_tex->Use();
    triangle_shader_tex->SetInt("diffuseMap", 0);
    triangle_shader_tex->SetInt("normalMap", 1);
    triangle_shader_tex->SetInt("depthMap", 2);
    triangle_shader_tex->SetBool("normals_enabled", false);
    triangle_shader_tex->SetBool("displace_enabled", false);
    triangle_shader_tex->SetBool("diffuse_enabled", false);
    grid_shader = rmtd::GetShader("infinite_grid");
    draw_grid_shader = rmtd::GetShader("draw_grid");
    active_shader = vert_shader;

    pick_shader = rmtd::GetShader("picking_3d");
    triangle_pick_shader = rmtd::GetShader("picking_3d_triang");
    dpch::e_Update.Add(this, refresh_trigger_cb, { this });
    e_MouseRelease.Add(this, scene_mb_release, {});

    auto texMap = rmtd::Textures;
    for (auto& texx : texMap)
        textures.push_back(texx.second);
    diffuse_tp.picked_tex = textures[0];
    normals_tp.picked_tex = textures[0];
    displace_tp.picked_tex = textures[0];

    GridInit();
    DrawGridInit();
    InitSceneData();
}
void hier_scene_selected_callback(sender_ptr sender, MouseEventArgs* e, params_t params) {
    Scene* scene = (Scene*)params[0];
    scene->RaiseInspector();
}
void Scene::InitSceneData() {
    UI = new SceneUI(this);
    InspCompon.clear();
    InspCompon.push_back(this);
    HierInfo = new CollapsableField("HierInfo");
    HierInfo->Init();
    Button* scene_btn = new Button("hier_scene_btn", 0, 0, 0, FIELD_H);
    scene_btn->Size.Set("prop_field_btn_rel", { 1.0f,0 }, MMode::Rel);
    scene_btn->Clicker::Init();
    scene_btn->e_MouseClick.Add(this, hier_scene_selected_callback, { this });
    HierInfo->Header->RemoveChild(HierInfo->Title);
    HierInfo->Header->InsertChildAt(0, scene_btn);
    scene_btn->Content = HierInfo->Title;
    scene_btn->AddChild(HierInfo->Title);
    HierInfo->Title->SetText("Scene");
    Vert_HI = new CollapsableField("Vert_HI");
    Vert_HI->Init();
    Vert_HI->Title->SetText("vertices");
    Triangle_HI = new CollapsableField("Triangle_HI");
    Triangle_HI->Init();
    Triangle_HI->Title->SetText("triangles");
    HierInfo->Content->AddChild(Vert_HI);
    HierInfo->Content->AddChild(Triangle_HI);
}
bool Check(float dt, InputArgs_S& input, int i, float holdneed = 0) {
    if (input.Keys[i] && !input.KeysProcessed[i]) {
        float hold = input.Hold[i];
        if (hold > holdneed) {
            return true;
        }
    }
    return false;
}
void triangle_create_state_end_callback(Scene* sender, SceneEventArgs* e, params_t params) {
    sender->TDData.Clear();
};
void cube_create_state_end_callback(Scene* sender, SceneEventArgs* e, params_t params) {
    sender->RemoveVert((Vert*)sender->CreateData.obj);
    sender->TVAOMData.Del();
    sender->CreateData.Clear();
    sender->DragData.Clear();
};
void Scene::ProcessInput(float dt, InputArgs_S& input) {

    if (Check(dt, input, GLFW_KEY_ESCAPE)) {
        SState = SceneState::NONE;
        Phase = PhaseE::NONE;
    }
    if (SState != SStateOld) {
        e_SceneStateChanged.InvokeRemove();
        Phase = PhaseE::NONE;
    }
    SStateOld = SState;
    MouseRM mmode = rmtd::MouseRay.mode;
    ModK.shift = input.Mode & GLFW_MOD_SHIFT;
    ModK.alt = input.Mode & GLFW_MOD_ALT;
    ModK.ctrl = input.Mode & GLFW_MOD_CONTROL;
    if (ModK.alt)
        int c = 5;
    bool Hover = rmtd::MouseRay.Hover;
    dbg::Log(ToString(mmode));
    switch (SState) {
        case SceneState::NONE:break;
        case SceneState::DRAGGING:
        case SceneState::PICKING: {
            switch (Phase) {
                case PhaseE::NONE:
                    break;
                case PhaseE::PREP:
                    break;
                case PhaseE::DOING:
                    switch (mmode) {
                        case MouseRM::None:
                        case MouseRM::LMBRelease:
                        case MouseRM::LMBReleaseHold:
                        case MouseRM::RMBRelease:
                        case MouseRM::RMBReleaseHold:
                            Phase = PhaseE::PREP;
                            DragData.Clear(); break;
                        default: {
                            if (ModK.alt) {
                                if (DragData.dragged.size() > 0) {
                                    auto center = DragData.Center();
                                    auto cposc = rmtd::CurWin->GetNDCCursorChange();
                                    //cposc = utl::CordsOf(this, cposc);
                                    mainCam->Pivot(center, cposc);
                                }
                            } else DoDrag();
                        } break;
                    } break;
                default: break;
            }
        } break;
        case SceneState::MOVING: {
            if (true) {
                switch (mmode) {
                    case MouseRM::None: SNavM = SceneNavMode::NONE; break;
                    case MouseRM::LMBPress: {
                        if (ModK.alt && !ModK.ctrl) { SNavM = SceneNavMode::ORBIT; }
                        if (ModK.ctrl && ModK.alt) { SNavM = SceneNavMode::PAN; }
                    } break;
                    case MouseRM::LMBRelease: break;
                    case MouseRM::LMBBeginHold: break;
                    case MouseRM::LMBHold: break;
                    case MouseRM::LMBReleaseHold: break;
                    case MouseRM::RMBPress: {
                        if (ModK.alt && !ModK.ctrl) { SNavM = SceneNavMode::DRAGZOOM; }
                        if (ModK.ctrl && ModK.alt) {}
                    } break;
                    case MouseRM::RMBRelease: break;
                    case MouseRM::RMBBeginHold: {
                        glfwSetInputMode(rmtd::CurWin->GLID, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                        e_MouseRelease.QInvoke();
                        SNavM = SceneNavMode::FLYTHROUGH;
                    } break;
                    case MouseRM::RMBHold: break;
                    case MouseRM::RMBReleaseHold: break;
                    default:  break;
                }
            }
            switch (SNavM) {
                case SceneNavMode::NONE:
                    break;
                case SceneNavMode::FLYTHROUGH: {
                    SNavM = SceneNavMode::FLYTHROUGH;
                    Camera_Movement cm = Camera_Movement::NONE;
                    if (Check(dt, input, GLFW_KEY_LEFT)) { cm = Camera_Movement::LEFT; }
                    if (Check(dt, input, GLFW_KEY_RIGHT)) { cm = Camera_Movement::RIGHT; }
                    if (Check(dt, input, GLFW_KEY_UP)) { cm = Camera_Movement::FORWARD; }
                    if (Check(dt, input, GLFW_KEY_DOWN)) { cm = Camera_Movement::BACKWARD; }
                    if (cm != Camera_Movement::NONE) mainCam->ProcessKeyboard(cm, dt);
                    auto change = rmtd::CurWin->cursor_pos_change;
                    //change = utl::CordsOf(this, change);
                    mainCam->ProcessMouseMovement(change.X, change.Y, true);
                } break;
                case SceneNavMode::PAN: //mainCam->Pan();
                    break;
                case SceneNavMode::ORBIT: //mainCam->Orbit();
                    break;
                case SceneNavMode::DRAGZOOM: //mainCam->DragZoom();
                    break;
                default: break;
            }
        } break;
        case SceneState::CREATE_CUBE: {
            switch (Phase) {
                case PhaseE::NONE:
                    e_SceneStateChanged.Add(this, cube_create_state_end_callback, {});
                case PhaseE::PREP:
                    Phase = PhaseE::DOING;
                    glm::vec3 pos = mainCam->Front;
                    pos *= 5;
                    pos += mainCam->transform->positions;
                    {
                        Vert* vert = new Vert(pos); //AddVert(pos);
                        CreateData.obj = vert;
                        SetInspector(vert);
                        glm::mat4 view = mainCam->GetViewMatrix();
                        DragData.drag_origin = view * glm::vec4(vert->transform->positions, 1.0f);
                        DragData.obj = vert;
                        DragData.dragged.insert(vert);
                    }
                case PhaseE::DOING:
                    DoDrag();
                default: break;
            }
        } break;
        case SceneState::CREATE_TRIANGLE: {
            switch (Phase) {
                case PhaseE::NONE:
                    e_SceneStateChanged.Add(this, triangle_create_state_end_callback, {});
                case PhaseE::PREP:
                    Phase = PhaseE::DOING;
                case PhaseE::DOING:
                default:break;
            }
        }
        default: break;
    }
    if (States.Active) {}
    if (Check(dt, input, GLFW_KEY_ENTER)) {}
    if (Check(dt, input, GLFW_KEY_BACKSPACE)) {}
}
Point2D<float> Scene::SetPickFb() {
    glEnable(GL_DEPTH_TEST);
    Pick.FB->BeginRender();
    Point2D<float> vp_size = { std::min(Pick.FB->ActualSize.X, ActualSize.X), std::min(Pick.FB->ActualSize.Y,ActualSize.Y) };
    glViewport(0, 0, (GLsizei)vp_size.X, (GLsizei)vp_size.Y);
    EnablePickShader();
    Pick.Map.clear();
    Pick.MultiMap.clear();
    int id = 0;
    int r, g;
    active_shader->SetInt("multirend", 0);
    for (auto& v : vertices) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, v->transform->positions);
        model = glm::scale(model, glm::vec3(0.25));
        active_shader->SetMat4("model", model);
        Pick.Map.push_back(v->GUID);
        id++;
        r = (id & 0x000000FF) >> 0;
        g = (id & 0x0000FF00) >> 8;
        active_shader->SetVec4f("PickingColor", r / 255.0f, g / 255.0f, 0, 1);
        utl::renderCube6();
    }
    std::vector<glm::vec3> data;
    Pick.Map.push_back((uint)Pick.MultiMap.size());
    Pick.MultiMap.push_back({});
    for (auto& t : triangles) {
        Pick.MultiMap.back().push_back(t->GUID);
    }
    int size = (int)triangles.size() * 3;
    glm::mat4 model = glm::mat4(1.0f);
    id++;
    r = (id & 0x000000FF) >> 0;
    g = (id & 0x0000FF00) >> 8;

    EnableTrianglePickShader();
    active_shader->SetMat4("model", model);
    //active_shader->SetInt("multirend", 1);
    active_shader->SetVec4f("PickingColor", r / 255.0f, g / 255.0f, 0, 1);
    if (size > 0) {
        glBindVertexArray(VAOT);
        glDrawArrays(GL_TRIANGLES, 0, size);
        glBindVertexArray(0);
    }
    glDisable(GL_DEPTH_TEST);

    Pick.FB->EndRender();
    return vp_size;
}
PixelInfo Scene::BeginPickFunc(float xNew, float yNew) {
    /*
    if (Pick.PickChanged)
        SetPickFb();
    utl::ReadPixel(Pick.pInfo, Pick.FB->FBO, Pick.FB->ActualSize.X * xNew, Pick.FB->ActualSize.Y * (1 - yNew));
    */
    Point2D<float> size = SetPickFb();
    utl::ReadPixel(Pick.pInfo, Pick.FB->FBO, size.X * xNew, size.Y * (1 - yNew));

    auto& pInfo = Pick.pInfo;
    if (pInfo.Ident.ObjID == 0)
        Pick.PickedObj = nullptr;
    else {
        if (pInfo.Ident.PrimID != 0) {
            //multimap rend
            int index = Pick.Map[pInfo.Ident.ObjID - 1];
            auto all = Pick.MultiMap[index];
            pInfo.Ident.GUID = all[pInfo.Ident.PrimID - 1];
        } else {
            pInfo.Ident.GUID = Pick.Map[pInfo.Ident.ObjID - 1];
        }
        DrawObj* child = (DrawObj*)rmtd::GetElement(pInfo.Ident.GUID);
        pInfo.Name = child->Name;
        pInfo.Print();
        Pick.PickedObj = child;
    }
    PickFunc(xNew, yNew);
    return Pick.pInfo;
}
bool Scene::TryChangeActivity(bool value) {
    return false;
}
PixelInfo Scene::PickFunc(float xNew, float yNew) {
    auto& pInfo = Pick.pInfo;
    auto& states = GetStates();
    Control3D* child = (Control3D*)(Pick.PickedObj);




    if (rmtd::MouseRay.Hover) {
        e_MouseHover.Invoke();
        rmtd::MouseRay.trace.push_back(this);
        if (child != nullptr)
            child->PickAt(xNew, yNew);
    } else {

        if (SState == SceneState::CREATE_CUBE) {
            switch (Phase) {
                case PhaseE::NONE:break;
                case PhaseE::PREP:break;
                case PhaseE::DOING:
                    AddVert((Vert*)CreateData.obj);
                    DragData.Clear();
                    CreateData.Clear();
                    SState = SceneState::CREATE_CUBE;
                    Phase = PhaseE::PREP;
                    break;
                default:break;
            }
        }

        if (child == nullptr) {
            auto args = new MouseClickEventArgs();
            e_MouseClick.QInvoke(args);
            e_MousePress.QInvoke();
            e_MouseRelease.QInvoke();
        } else {
            SetInspector(child);
            switch (SState) {
                case SceneState::NONE: break;
                case SceneState::PICKING: {
                    std::string type = child->GetType();
                    if (type == "Vert") {
                        RegisterForDrag(child);
                        if (!ModK.ctrl) {
                            Phase = PhaseE::DOING;
                            glm::mat4 view = mainCam->GetViewMatrix();
                            DragData.drag_origin = view * glm::vec4(child->transform->positions, 1.0f);
                            DragData.obj = child;

                            auto cpos = rmtd::CurWin->GetNDCCursor();
                            cpos = utl::CordsOf(this, cpos);
                            utl::m01To11(cpos);
                            DragData.oldscreenPos = { cpos.X * ActualSize.X,cpos.Y * ActualSize.Y };
                        }
                    } else if (type == "Triangle") {
                        if (!ModK.ctrl) {
                        }
                    } break;
                } break;
                case SceneState::CREATE_CUBE: {
                    switch (Phase) {
                        case PhaseE::NONE:break;
                        case PhaseE::PREP:break;
                        case PhaseE::DOING:
                            AddVert((Vert*)CreateData.obj);
                            DragData.Clear();
                            CreateData.Clear();
                            SState = SceneState::CREATE_CUBE;
                            Phase = PhaseE::PREP;
                            break;
                        default:break;
                    }
                } break;
                case SceneState::CREATE_TRIANGLE: {
                    if (!ModK.ctrl) {
                        if (child->GetType() == "Vert") {
                            TDData.Add((Vert*)child);
                            if (TDData.verts.size() == 3) {
                                TVAOMData.New();
                                Triangle* tri = TDData.DefineT();
                                AddTriangle(tri);
                            }
                            break;
                        }
                    }
                } break;
                default:
                    break;
            }
            if (child->Pick.Pickable)
                child->PickAt(xNew, yNew);
        }
    }
    return pInfo;
}
struct gridData {
    glm::vec3 v;
    glm::vec2 vt;
};
void Scene::GridInit() {
    Framebuf* grid = new Framebuf();
    grid->Init();
    grid->Texs[0] = textures[0];
    grid_shader->Use();
    grid_shader->SetInt("diffuseMap", 0);
    InfGrid.Texs.push_back(rmtd::Textures["grid_3.png"]);
    float val = 5;
    std::vector<gridData> gridPlane{
             {glm::vec3(-val, 0, -val),{0,0}},
             {glm::vec3(-val, 0, val), {0,1}},
             {glm::vec3(val, 0, val), {1,1}},
             {glm::vec3(-val, 0, -val), {0,0}},
             {glm::vec3(val, 0, val), {1,1}},
             {glm::vec3(val, 0, -val), {1,0}},
    };
    glGenFramebuffers(1, &InfGrid.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, InfGrid.FBO);

    auto tmp = new Texture2D();
    tmp->Generate(ActualSize, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmp->ID, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glActiveTexture(GL_TEXTURE0);
    tmp->Bind();
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (uint)ActualSize.X, (uint)ActualSize.Y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    glGenVertexArrays(1, &InfGrid.VAO);
    glGenBuffers(1, &InfGrid.VBO);
    int stride = sizeof(gridData);
    glBindBuffer(GL_ARRAY_BUFFER, InfGrid.VBO);
    glBufferData(GL_ARRAY_BUFFER, gridPlane.size() * stride, gridPlane.data(), GL_STATIC_DRAW);
    glBindVertexArray(InfGrid.VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Scene::DrawGridInit() {
    draw_grid_shader->Use();
    std::vector<glm::vec3> gridPlane{
             glm::vec3(1, 1, 0), glm::vec3(-1, -1, 0), glm::vec3(-1, 1, 0),
            glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(1, -1, 0)
    };
    glGenFramebuffers(1, &DrawGrid.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, DrawGrid.FBO);
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (uint)ActualSize.X, (uint)ActualSize.Y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    glGenVertexArrays(1, &DrawGrid.VAO);
    glGenBuffers(1, &DrawGrid.VBO);
    int stride = sizeof(glm::vec3);
    glBindBuffer(GL_ARRAY_BUFFER, DrawGrid.VBO);
    glBufferData(GL_ARRAY_BUFFER, gridPlane.size() * stride, gridPlane.data(), GL_STATIC_DRAW);
    glBindVertexArray(DrawGrid.VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Scene::Prepare() {

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    BeginRender();
    for (auto& ch : Objects) {
        ch->Render();
    }
    EnableVertShader();
    RenderVerts();

    EnableTriangleShader();
    RenderTriangles();
    EnableInfGridShader();
    RenderGrid();
    //EnableDrawGridShader();
    //RenderDrawGrid();
    EndRender();
    glDisable(GL_DEPTH_TEST);
}
void Scene::EnableInfGridShader() {
    active_shader = grid_shader;
    active_shader->Use();
    glm::mat4 projection = mainCam->GetProjectionMatrix();
    glm::mat4 view = mainCam->GetViewMatrix();
    active_shader->SetMat4("projection", projection);
    active_shader->SetMat4("view", view);
}
void Scene::EnableDrawGridShader() {
    active_shader = draw_grid_shader;
    active_shader->Use();
    glm::mat4 projection = mainCam->GetProjectionMatrix();
    glm::mat4 view = mainCam->GetViewMatrix();
    active_shader->SetMat4("projection", projection);
    active_shader->SetMat4("view", view);
}
void Scene::RenderGrid() {

    glBindVertexArray(InfGrid.VAO);
    glActiveTexture(GL_TEXTURE0);
    InfGrid.Texs[0]->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
void Scene::RenderDrawGrid() {

    glBindVertexArray(DrawGrid.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
void Scene::RenderVerts() {
    if (CubeWF.value[0]) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
    for (auto& v : vertices) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, v->transform->positions);
        model = glm::scale(model, glm::vec3(0.25));
        active_shader->SetMat4("model", model);
        active_shader->SetBool("selected", v->States.Selected);
        utl::renderCube6();
    }
    if (CreateData.obj != nullptr) {
        auto& v = CreateData.obj;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, v->transform->positions);
        model = glm::scale(model, glm::vec3(0.25));
        active_shader->SetMat4("model", model);
        active_shader->SetBool("selected", v->States.Selected);
        utl::renderCube6();
    }

    if (CubeWF.value[0]) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }
}
void Scene::RenderTriangles() {
    std::vector<TriangleVertData_S> data2;
    for (auto& t : triangles) {
        data2.push_back({
            t->v1->transform->positions,
            t->v1_tex->positions,
            t->normal,
            t->tangent,
            t->bitangent
            });
        data2.push_back({
            t->v2->transform->positions,
            t->v2_tex->positions,
            t->normal,
            t->tangent,
            t->bitangent
            });
        data2.push_back({
            t->v3->transform->positions,
            t->v3_tex->positions,
            t->normal,
            t->tangent,
            t->bitangent
            });
    }
    int size = (int)data2.size();
    if (TVAOMData.Ready()) {
        TriangleVAOSetup(size / 3 + TVAOMData.extra_max);
        TVAOMData.Clear();
    }
    TriangleVAOFill(data2);
    if (size > 0) {
        glBindVertexArray(VAOT);
        if (TriangleWF.value[0]) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLES, 0, size);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuse_tp.picked_tex->ID);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normals_tp.picked_tex->ID);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, displace_tp.picked_tex->ID);
            glDrawArrays(GL_TRIANGLES, 0, size);
        }
        glBindVertexArray(0);
    }
}
void Scene::Render() {
    Framebuf::Render();
}
void Scene::SaveDesignedModel(std::string filename) {

    std::string path = "resources\\generated\\models\\" + filename;

    Material mat;
    mat.name = "mat0";
    mat.map_Ka = diffuse_tp.picked_tex->Name;
    mat.map_Kd = diffuse_tp.picked_tex->Name;
    mat.bump = normals_tp.picked_tex->Name;
    mat.disp = displace_tp.picked_tex->Name;

    std::ofstream dot_mtl;
    dot_mtl.open(path + ".mtl", fstream::out);
    dot_mtl << mat.mtl_content();

    std::unordered_map<glm::vec3, int, glmVec3Hash> vmap;
    std::unordered_map<glm::vec2, int, glmVec2Hash> vtmap;
    std::vector<glm::vec3> bad_dif_v;
    std::vector<glm::vec3> dif_v;
    std::vector<glm::vec2> dif_vt;
    float maxX = -FLT_MAX;
    float maxY = -FLT_MAX;
    float maxZ = -FLT_MAX;
    float minX = FLT_MAX;
    float minY = FLT_MAX;
    float minZ = FLT_MAX;
    int count = 1;
    for (auto item : vertices) {
        auto& pos = item->transform->positions;
        if (vmap.count(pos) == 0) {
            vmap.insert({ pos, count++ });
            bad_dif_v.push_back(pos);
            maxX = std::max(pos.x, maxX);
            maxY = std::max(pos.y, maxY);
            maxZ = std::max(pos.z, maxZ);
            minX = std::min(pos.x, minX);
            minY = std::min(pos.y, minY);
            minZ = std::min(pos.z, minZ);
        }
    }

    glm::vec3 translate;
    translate.x = (maxX + minX) / 2;
    translate.y = (maxY + minY) / 2;
    translate.z = (maxZ + minZ) / 2;
    float spreadX = maxX - minX;
    float spreadY = maxY - minY;
    float spreadZ = maxZ - minZ;
    float max = std::max(std::max(spreadX, spreadY), spreadZ);
    float maxhalf = max / 2;
    for (auto& item : bad_dif_v) {
        dif_v.push_back(((item - translate) / maxhalf));
    }
    int count_vt = 1;
    for (auto item : triangles) {
        if (vtmap.count(item->v1_tex->positions) == 0) {
            vtmap.insert({ item->v1_tex->positions,count_vt++ });
            dif_vt.push_back(item->v1_tex->positions);
        }
        if (vtmap.count(item->v2_tex->positions) == 0) {
            vtmap.insert({ item->v2_tex->positions,count_vt++ });
            dif_vt.push_back(item->v2_tex->positions);
        }
        if (vtmap.count(item->v3_tex->positions) == 0) {
            vtmap.insert({ item->v3_tex->positions,count_vt++ });
            dif_vt.push_back(item->v3_tex->positions);
        }
    }

    stringstream str(stringstream::out);
    str << "mtllib " << filename << ".mtl" << "\n"; //mozda drugo
    str << "o " << filename << "\n";
    for (auto pos : dif_v) {
        str << "v " << pos.x << " " << pos.y << " " << pos.z << "\n";
    }
    for (auto pos : dif_vt) {
        str << "vt " << pos.x << " " << pos.y << "\n";
    }
    str << "usemtl " << mat.name << "\n";
    str << "s off\n";
    int v1, v2, v3, vt1, vt2, vt3;
    for (int i = 0; i < triangles.size(); i++) {
        v1 = vmap[triangles[i]->v1->transform->positions];
        v2 = vmap[triangles[i]->v2->transform->positions];
        v3 = vmap[triangles[i]->v3->transform->positions];
        vt1 = vtmap[triangles[i]->v1_tex->positions];
        vt2 = vtmap[triangles[i]->v2_tex->positions];
        vt3 = vtmap[triangles[i]->v3_tex->positions];
        str << "f " << v1 << "/" << vt1 << " " << v2 << "/" << vt2 << " " << v3 << "/" << vt3 << "\n";
    }

    std::ofstream dot_obj;
    dot_obj.open(path + ".obj", fstream::out);
    dot_obj << str.str();
}

void Scene::TriangleVAOInit(uint objNum) {
    glGenFramebuffers(1, &FBOT);
    glBindFramebuffer(GL_FRAMEBUFFER, FBOT);

    glGenVertexArrays(1, &VAOT);
    glGenBuffers(1, &VBOT);

    auto tmp = new Texture2D();
    tmp->Generate({ 1,1 }, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmp->ID, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glActiveTexture(GL_TEXTURE0);
    tmp->Bind();
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, FB_WIDTH, FB_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    TriangleVAOSetup(objNum);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Scene::TriangleVAOSetup(uint objNum) {
    int stride = sizeof(TriangleVertData_S);
    glBindBuffer(GL_ARRAY_BUFFER, VBOT);
    glBufferData(GL_ARRAY_BUFFER, objNum * 3 * stride, NULL, GL_DYNAMIC_DRAW);
    glBindVertexArray(VAOT);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(11 * sizeof(float)));
    glBindVertexArray(0);
}
void Scene::TriangleVAOFill(std::vector<TriangleVertData_S>& data) {
    if (data.size() > 0) {
        int stride = sizeof(TriangleVertData_S);
        glBindBuffer(GL_ARRAY_BUFFER, VBOT);
        glBufferSubData(GL_ARRAY_BUFFER, 0, data.size() * stride, data.data());
        glBindVertexArray(VAOT);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(11 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}
void Scene::EnableVertShader() {
    active_shader = vert_shader;
    active_shader->Use();
    glm::mat4 projection = mainCam->GetProjectionMatrix();
    glm::mat4 view = mainCam->GetViewMatrix();
    //dbg::Log(utl::struct_to_str<float>((void*)&view, 16, 4));
    active_shader->SetMat4("projection", projection);
    active_shader->SetMat4("view", view);
    glm::vec3 lightPos = mainCam->transform->positions;
    active_shader->SetVec3f("light.position", lightPos);
    active_shader->SetVec3f("viewPos", mainCam->transform->positions);
    glm::vec3 lightColor;
    lightColor.x = 0.0f;
    lightColor.y = 1.0f;
    lightColor.z = 0.2f;
    glm::vec3 diffuseColor = lightColor * glm::vec3(0.6f); // decrease the influence
    glm::vec3 ambientColor = diffuseColor * glm::vec3(0.4f); // low influence
    active_shader->SetVec3f("light.ambient", ambientColor);
    active_shader->SetVec3f("light.diffuse", diffuseColor);
    active_shader->SetVec3f("light.specular", 1.0f, 1.0f, 1.0f);
    active_shader->SetVec3f("material.ambient", 1.0f, 0.5f, 0.31f);
    active_shader->SetVec3f("material.diffuse", 1.0f, 0.5f, 0.31f);
    active_shader->SetVec3f("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
    active_shader->SetFloat("material.shininess", 32.0f);
}
void Scene::EnableTriangleShader() {
    if (diffuse_tp.tex_index[0] == -1 || diffuse_tp.tex_index[0] == textures.size()) {
        active_shader = triangle_shader;
    } else {
        active_shader = triangle_shader_tex;
        active_shader->Use();
        triangle_shader_tex->SetVec3f("viewPos", mainCam->transform->positions);
        triangle_shader_tex->SetVec3f("lightPos", mainCam->transform->positions);
        triangle_shader_tex->SetFloat("heightScale", displace_tp.intensity[0]);
        triangle_shader_tex->SetFloat("diffuse_mod", diffuse_tp.intensity[0]);
        triangle_shader_tex->SetFloat("normals_mod", normals_tp.intensity[0]);
    }
    active_shader->Use();
    glm::mat4 projection = mainCam->GetProjectionMatrix();
    glm::mat4 view = mainCam->GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    active_shader->SetMat4("model", model);
    active_shader->SetMat4("projection", projection);
    active_shader->SetMat4("view", view);

}
void Scene::BeginRender(bool clear) {
    glViewport(0, 0, (GLsizei)LogicalSize.X[0], (GLsizei)LogicalSize.Y[0]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->FBO);
    if (clear) {
        ClearColor();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

void control3d_selection_changed_callback(Control3D* sender, SelectionChangeEventArgs* e, params_t params) {
    Scene* scene = (Scene*)params[0];
    Button* btn = (Button*)params[1];
    if (e->Value) {
        scene->DragData.dragged.insert(sender);
        btn->Content->m_colorPar = rmtd::Colors["button_fg_active"];
        btn->Content->GetActiveTex().m_clear_color = rmtd::Colors["button_bg_active"];
    } else {
        scene->DragData.dragged.erase(sender);
        btn->Content->m_colorPar = rmtd::Colors["button_fg_inactive"];
        btn->Content->GetActiveTex().m_clear_color = rmtd::Colors["button_bg_inactive"];
    }
    scene->SetInspector(sender);
    btn->Content->TextData.ToRender = true;
    btn->Content->OnChange();
}

void hier_object_selected_callback(sender_ptr sender, MouseEventArgs* e, params_t params) {
    Control3D* c3d = (Control3D*)params[0];
    c3d->SetSelected(!c3d->GetSelected());
}
Vert* Scene::AddVert(Vert* vert) {
    Button* btn = new Button("vertex", 0, 0, 0, FIELD_H);
    btn->Init();
    Vert_HI->Content->AddChild(btn);
    vert->Parent = this;
    vertices.push_back(vert);
    btn->Content->SetText("vert[" + std::to_string(vertices.size() - 1) + "]");
    btn->e_MouseClick.Add(this, hier_object_selected_callback, { vert });
    vert->e_SelectionChanged.Add(this, control3d_selection_changed_callback, { this,btn });
    return vert;
}
Vert* Scene::AddVert(glm::vec3 v) {
    Vert* vert = new Vert{ v };
    AddVert(vert);
    return vert;
}
void Scene::AddVerts(std::vector<glm::vec3> verts) {
    for (glm::vec3& v : verts) { AddVert(v); }
}
void Scene::RemoveVert(Vert* vert) {
    utils::ErraseFromVec(vertices, vert);
}
Triangle* Scene::AddTriangle(uint v1_id, uint v2_id, uint v3_id) {
    Triangle* tri = new Triangle(vertices[v1_id], vertices[v2_id], vertices[v3_id]);
    tri->Init();
    return AddTriangle(tri);
}
Triangle* Scene::AddTriangle(Triangle* tri) {
    Button* btn = new Button("triangle", 0, 0, 0, FIELD_H);
    btn->Init();
    Triangle_HI->Content->AddChild(btn);
    tri->Parent = this;
    triangles.push_back(tri);
    btn->Content->SetText("tri[" + std::to_string(triangles.size() - 1) + "]");
    btn->e_MouseClick.Add(this, hier_object_selected_callback, { tri });
    tri->e_SelectionChanged.Add(this, control3d_selection_changed_callback, { this,btn });
    return tri;
}
void Scene::AddTriangles(std::vector<Triangle*> tris) {
    for (auto tri : tris) {
        AddTriangle(tri);
    }
}
void Scene::RemoveTriangle(Triangle* tri) {
    utils::ErraseFromVec(triangles, tri);
}
void Scene::EnablePickShader() {
    active_shader = pick_shader;
    active_shader->Use();
    glm::mat4 projection = mainCam->GetProjectionMatrix();
    glm::mat4 view = mainCam->GetViewMatrix();
    active_shader->SetMat4("projection", projection);
    active_shader->SetMat4("view", view);
}
void Scene::EnableTrianglePickShader() {
    active_shader = triangle_pick_shader;
    active_shader->Use();
    glm::mat4 projection = mainCam->GetProjectionMatrix();
    glm::mat4 view = mainCam->GetViewMatrix();
    active_shader->SetMat4("projection", projection);
    active_shader->SetMat4("view", view);
}
void Scene::RegisterForDrag(Control3D* obj) {
    DragData.dragged.insert(obj);
    obj->SetSelected(true);
}
void Scene::DoDrag() {
    if (DragData.obj == nullptr)
        return;
    glm::mat4 projection = mainCam->GetProjectionMatrix();
    glm::mat4 projInv = glm::inverse(projection);
    glm::mat4 view = mainCam->GetViewMatrix();
    glm::mat4 viewInv = glm::inverse(view);
    auto cpos = rmtd::CurWin->GetNDCCursor();
    cpos = utl::CordsOf(this, cpos);
    utl::m01To11(cpos);
    glm::vec4 ray_ndc(cpos.X, -cpos.Y, 1.0f, 1.0f);
    glm::vec4 ray_view = projInv * ray_ndc;
    glm::vec3 intersect = glm::vec3(ray_view * -DragData.drag_origin.z);
    glm::vec4 change_view = glm::vec4(intersect - DragData.drag_origin, 1.0f);
    viewInv[3] = glm::vec4(0, 0, 0, viewInv[3].w);
    glm::vec3 change_world = glm::vec3(viewInv * change_view);

    for (auto obj : DragData.dragged) {
        if (SnapToGrid.value[0]) {
            auto val = obj->transform->positions;
            val += change_world;
            val = glm::round(val);
            obj->transform->positions = val;
        } else
            obj->transform->positions += change_world;
        obj->transform->Notify();
    }
    DragData.drag_origin = view * glm::vec4(DragData.obj->transform->positions, 1.0f);
}
void Scene::RaiseHierarchy() {
    if (rmtd::HierData.active == this) return;
    rmtd::HierData.SetHierarchy(this, HierInfo);
    ClearInspector();
    SetInspector(InspData.active);
}
void Scene::ClearInspector() {
    if (rmtd::GlobalInspData.active != nullptr)
        for (auto comp : rmtd::GlobalInspData.active->InspCompon) {
            comp->FreeUI();}
    rmtd::Inspector->ClearChilds();
    rmtd::Inspector->OnChange();
    rmtd::GlobalInspData.active = nullptr;
}
void Scene::SetInspector(Object* sender) {
    if(rmtd::GlobalInspData.active == sender || sender == nullptr) return;
    ClearInspector(); InspData.Content.clear();
    for (auto comp : sender->InspCompon) {
        Control* field = comp->RaiseUI();
        InspData.Content.push_back(field);}
    rmtd::Inspector->AddChilds(InspData.Content);
    rmtd::GlobalInspData.active = sender;
    InspData.active = sender;
}
void Scene::RaiseInspector() {
    SetInspector(this);
}
void Scene::Link(Framebuf* parent) {
    States.Active = true;
    rmtd::CurWin->ActiveObjs.insert(this);
    Control::Link(parent);
    RaiseHierarchy();
}

TexPickFD::TexPickFD(Scene* owner_p, std::string eun) {
    enable_uniform_name = eun;
    owner = owner_p;
    /*
    tex_index.owner = this;
    tex_index.value = new float();
    tex_index.value[0] = -1;
    */
    tex_enable.owner = this;
    tex_enable.value = new bool();
}
