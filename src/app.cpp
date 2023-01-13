#include "yacvat/app.h"
#include "yacvat/IconsFontAwesome4.h"
#include "yacvat/notofont.h"
#include "yacvat/fontawesome.h"
#include "yacvat/IconsFontAwesome4.h"
#include "yacvat/vec2.h"

#include "spdlog/spdlog.h"
#include "imgui.h"
#include "nlohmann/json.hpp"
#include "ImGuiFileDialog.h" // add-on filedialogs

#include <iostream>
#include <vector>
#include <dirent.h>
#include <set>
#include <fstream>
#include <algorithm> // for reverse

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

AnnotationApp::AnnotationApp(void)
{
    spdlog::info("Instanciation of AnnotationApp object.");

    open_images_folder_flag = false;

    ext_set.clear();
    ext_set.insert("png");
    ext_set.insert("jpeg");
    ext_set.insert("jpg");

    current_image_texture = 0;

    for (auto e : ext_set)
        spdlog::debug("set of extension allowed : {}", e);
}

void AnnotationApp::ui_initialize(void)
{
    ImGuiIO &io = ImGui::GetIO();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // to enable dragging on the image without moving the window around
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("assets/NotoSans-Regular.ttf", 18.0f);

    // basic font
    io.Fonts->AddFontFromMemoryCompressedTTF(NotoFont_compressed_data, NotoFont_compressed_size, 18.0f);

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromMemoryCompressedTTF(fontawesome_webfont_compressed_data, fontawesome_webfont_compressed_size, 16.0f, &icons_config, icons_ranges);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != NULL);
}

void AnnotationApp::ui_main_window(void)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("Annotation Tool", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

    ImGui::BeginChild("Pane2", ImVec2(200, -1.f), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Images Menu"))
        {
            if (ImGui::MenuItem("Open folder"))
                this->open_images_folder_flag = true;

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (this->open_images_folder_flag == true)
    {
        ImGuiFileDialog::Instance()->OpenDialog("FolderChooser", "Choose a Directory", nullptr, ".", ImGuiFileDialogFlags_Modal);

        // display
        if (ImGuiFileDialog::Instance()->Display("FolderChooser"))
        {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                this->parse_images_folder(ImGuiFileDialog::Instance()->GetCurrentPath());
            }

            // close
            ImGuiFileDialog::Instance()->Close();
            this->open_images_folder_flag = false;
        }
    }

    // display images files
    this->ui_images_folder();

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("Pane3", ImVec2(300, -1.f), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);

    if (ImGui::IsKeyPressed(ImGuiKey_Insert))
    {
        this->import_annotations_from_prev();
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu(ICON_FA_PENCIL "  Annotations"))
        {
            if (ImGui::MenuItem("Clear all"))
            {
                this->clear_annotations();
            }
            if (ImGui::MenuItem("Import from previous image", "Insert"))
            {
                this->import_annotations_from_prev();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    this->ui_annotations_panel();

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("Pane1", ImVec2(-1.f, -1.f), false, ImGuiWindowFlags_AlwaysAutoResize);

    this->ui_image_current();

    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::End();
}

void AnnotationApp::ui_annotations_panel(void)
{
    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;

    bool update_json_flag = false;

    if (ImGui::BeginTable("table_annotations", 5, flags))
    {
        ImGui::TableSetupColumn(ICON_FA_KEYBOARD_O, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(ICON_FA_PAINT_BRUSH, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(ICON_FA_TRASH, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        static char _unused_ids[64] = "";

        for (long unsigned int n = 0; n < this->annotations.size(); n++)
        {
            ImGui::TableNextRow();

            // shortcut to select the annotation
            if ((n < 12) && ImGui::IsKeyPressed(ImGuiKey_F1 + n))
            {
                this->activate_annotation(n);
            }

            // display the name of the shortcut
            ImGui::TableSetColumnIndex(0);
            this->annotations[n].shortcut = n;
            sprintf(_unused_ids, "F%d##shortcutext", this->annotations[n].shortcut + 1);
            if (ImGui::Selectable(_unused_ids, &this->annotations[n].selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
            {
                spdlog::debug("selecting : {} -> {}", n, this->annotations[n].selected);

                // only 1 selection allowed
                if (this->annotations[n].selected)
                {
                    for (long unsigned int m = 0; m < this->annotations.size(); m++)
                    {
                        if (m != n)
                            this->annotations[m].selected = false;
                    }
                }
            }

            ImGui::TableSetColumnIndex(1);
            sprintf(_unused_ids, "##color%ld", n);
            if (ImGui::ColorEdit4(_unused_ids, this->annotations[n].color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
            {
                update_json_flag = true;
                this->annotations[n].update_color();
            }

            // label of the annotation
            ImGui::TableSetColumnIndex(2);
            sprintf(_unused_ids, "##inputtext%ld", n);
            ImGui::PushItemWidth(-1);
            // ImGuiInputTextFlags_EnterReturnsTrue
            if (ImGui::InputText(_unused_ids, this->annotations[n].new_label, 64))
            {
                this->annotations[n].label = this->annotations[n].new_label; // basic recopy for now
                spdlog::debug("[label {}] new label : {}", n, this->annotations[n].label);
                update_json_flag = true;
            }
            ImGui::PopItemWidth();

            // annotation type
            ImGui::TableSetColumnIndex(3);
            sprintf(_unused_ids, "##combotext%ld", n);
            ImGui::PushItemWidth(-1);
            if (ImGui::Combo(_unused_ids, (int *)&this->annotations[n].type, "POINT\0AREA"))
            {
                spdlog::debug("[label {}] new type : {}", n, this->annotations[n].type);
                update_json_flag = true;
            }
            ImGui::PopItemWidth();

            ImGui::TableSetColumnIndex(4);
            sprintf(_unused_ids, ICON_FA_MINUS_CIRCLE "##delbuttont%ld", n);
            if (ImGui::Button(_unused_ids))
            {
                this->annotations.erase(this->annotations.begin() + n);
                update_json_flag = true;
            }
        }
        ImGui::EndTable();
    }

    // add new annotation
    if (ImGui::Button(ICON_FA_PLUS_CIRCLE "  Create new label"))
    {
        this->annotations.push_back(Annotation("new label"));
        update_json_flag = true;
    }

    // freeze current configuration and start labeling
    ImGui::Separator();

    // list annotations in the current image
    if (ImGui::BeginTable("table_annotations_inst", 2, flags))
    {
        ImGui::TableSetupColumn("Instances", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(ICON_FA_TRASH, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        static char _unused_ids[64] = "";

        for (long unsigned int n = 0; n < this->annotations.size(); n++)
        {
            for (long unsigned int m = 0; m < this->annotations[n].inst.size(); m++)
            {
                if (this->annotations[n].inst[m].img_fname == this->image_fname)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    sprintf(_unused_ids, "%s-%ld##labelinst", this->annotations[n].label.c_str(), m);
                    if (ImGui::Selectable(_unused_ids, &this->annotations[n].inst[m].selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
                    {
                        spdlog::debug("selecting : {} -> {}", m, this->annotations[n].inst[m].selected);

                        // only 1 selection allowed
                        if (this->annotations[n].inst[m].selected)
                        {
                            for (long unsigned int kk = 0; kk < this->annotations[n].inst.size(); kk++)
                            {
                                if (kk != m)
                                {
                                    this->annotations[n].inst[kk].selected = false;
                                }
                            }
                        }

                        // toggle edit mode
                        if (this->annotations[n].inst[m].selected)
                        {
                            this->annotations[n].inst[m].status_fsm.execute("from_idle_to_edit");
                        }
                    }

                    ImGui::TableSetColumnIndex(1);
                    sprintf(_unused_ids, ICON_FA_MINUS_CIRCLE "##delbuttontinst%ldx%ld", n, m);
                    if (ImGui::Button(_unused_ids))
                    {
                        this->annotations[n].inst.erase(this->annotations[n].inst.begin() + m);
                        update_json_flag = true;
                    }
                }
            }
        }

        ImGui::EndTable();
    }

    if (update_json_flag == true)
    {
        this->json_write();
    }
}

void AnnotationApp::json_write(void)
{
    spdlog::debug("Writing json file : {}", this->annotation_fname.c_str());

    // check if the annotation file exists, create it if needed
    if (!this->annotations_file_exists)
    {
        spdlog::info("Creating json file : {}", this->annotation_fname.c_str());
        fs.open(this->annotation_fname, std::ios::out | std::ios::app);
        fs.close();
    }

    // reset annotation count per file to reparse the structure as it is exported
    for (auto it = this->ninstperimage.begin(); it != this->ninstperimage.end(); ++it)
    {
        it->second = 0;
    }

    nlohmann::json json_data;
    for (long unsigned n = 0; n < this->annotations.size(); n++)
    {
        // config
        json_data[this->annotations[n].label.c_str()]["config"] = {{"type", this->annotations[n].type},
                                                                   {"color", this->annotations[n].color}};

        // instances
        json_data[this->annotations[n].label.c_str()]["instances"] = nlohmann::json::array();
        for (long unsigned m = 0; m < this->annotations[n].inst.size(); m++)
        {
            // spdlog::debug("[{}, {}, {}, {}] / {}", this->annotations[n].inst[m].rect_on_image.get_topleft_vertex().x, this->annotations[n].inst[m].rect_on_image.get_topleft_vertex().y, this->annotations[n].inst[m].rect_on_image.get_bottomright_vertex().x, this->annotations[n].inst[m].rect_on_image.get_bottomright_vertex().y, this->scale);
            json_data[this->annotations[n].label.c_str()]["instances"].push_back(
                nlohmann::json::object({
                    {"file", this->annotations[n].inst[m].img_fname.c_str()},                                       // file
                    {"x_start", this->annotations[n].inst[m].rect_on_image.get_topleft_vertex().x / this->scale},   // x start coordinates
                    {"y_start", this->annotations[n].inst[m].rect_on_image.get_topleft_vertex().y / this->scale},   // y start coordinates
                    {"x_end", this->annotations[n].inst[m].rect_on_image.get_bottomright_vertex().x / this->scale}, // x end coordinates
                    {"y_end", this->annotations[n].inst[m].rect_on_image.get_bottomright_vertex().y / this->scale}  // y end coordinates
                }));

            this->ninstperimage[this->annotations[n].inst[m].img_fname] = this->ninstperimage[this->annotations[n].inst[m].img_fname] + 1;
        }
    }

    // flush file
    std::ofstream f(this->annotation_fname.c_str());
    f << std::setw(4) << json_data << std::endl; // pretty json using setw(4)
}

void AnnotationApp::json_read(void)
{
    spdlog::debug("Parsing json file : {}", this->annotation_fname.c_str());

    // check if the annotation file exists
    if (this->annotations_file_exists == false)
        return;

    // parse the annotation file
    std::ifstream f(this->annotation_fname.c_str());
    this->json = nlohmann::json::parse(f);

    // empty list of annotations
    this->annotations.clear();
    this->ninstperimage.clear();

    // extract annotations
    for (nlohmann::json::iterator i = json.begin(); i != json.end(); ++i)
    {
        // create new annotation
        Annotation _ann = Annotation(i.key());

        // config
        auto config = i.value()["config"];
        if (config != NULL)
        {
            for (nlohmann::json::iterator j = config.begin(); j != config.end(); ++j)
            {
                std::string _key = j.key();

                if (_key == "type")
                {
                    _ann.type = config["type"].get<annotation_type_t>();
                }

                if (_key == "color")
                {
                    auto _vec = config["color"].get<std::vector<float>>();
                    for (int n = 0; n < 4; n++)
                    {
                        _ann.color[n] = _vec[n];
                    }
                }
            }

            // add annotation to the list of valid annotations
            _ann.selected = false;
            this->annotations.push_back(_ann);
        }

        // instances
        auto insts = i.value()["instances"];
        if (insts != NULL)
        {
            for (nlohmann::json::iterator j = insts.begin(); j != insts.end(); ++j)
            {
                // create a new instance
                AnnotationInstance _inst;
                auto val = j.value();

                // retrieve file name
                _inst.img_fname = val["file"].get<std::string>();

                // update dictionary counting instances per image
                this->ninstperimage[_inst.img_fname] = this->ninstperimage[_inst.img_fname] + 1;

                // retrieve corner positions of the instance
                float x_start = val["x_start"].get<float>() * this->scale;
                float y_start = val["y_start"].get<float>() * this->scale;
                float x_end = val["x_end"].get<float>() * this->scale;
                float y_end = val["y_end"].get<float>() * this->scale;

                // spdlog::debug("[{}, {}, {}, {}] * {}", x_start, y_start, x_end, y_end, this->scale);

                _inst.rect_on_image.set_bottomright_vertex(vec2f(x_end, y_end));
                _inst.rect_on_image.set_topleft_vertex(vec2f(x_start, y_start));

                // retrieve color
                for (int k = 0; k < 4; k++)
                    _inst.color_u8[k] = this->annotations.back().color[k] * 255;

                // switch state to idle
                _inst.status_fsm.execute("from_create_to_idle");
                _inst.hover_fsm.execute("from_hover_to_outside");
                _inst.selected = false;

                // push annotation instance
                this->annotations.back().inst.push_back(_inst);
            }
        }
    }
}

void AnnotationApp::ui_images_folder(void)
{
    int n = 0;
    static int selected = -1;

    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;

    if (ImGui::Button(ICON_FA_FOLDER_OPEN " Load Folder", ImVec2(-1,50)))
    {
        this->open_images_folder_flag = true;
    }

    if (ImGui::BeginTable("table_images", 2, flags))
    {
        ImGui::TableSetupColumn(ICON_FA_STICKY_NOTE, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(ICON_FA_PICTURE_O "  Pictures", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (auto e : this->image_files)
        {

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", this->ninstperimage[e]);

            // single selectable to display filenames
            ImGui::TableSetColumnIndex(1);
            if (ImGui::Selectable(e.c_str(), selected == n))
            {
                selected = n;

                // create full filename
                // todo : use boost lib
                std::string fn = this->images_folder + "/" + e;
                spdlog::debug("Loading image in RAM : {}", fn);

                // current_image_texture = 0;
                current_image_width = 0;
                current_image_height = 0;
                current_image_texture = 0;
                bool ret = this->read_image(fn.c_str(), &current_image_texture, &current_image_width, &current_image_height);
                IM_ASSERT(ret);

                this->scale = 0.0;
                this->image_fname = e;
                this->compute_scale_flag = true;
            }
            n++;
        }

        ImGui::EndTable();
    }
}

void AnnotationApp::ui_image_current()
{
    if (this->current_image_texture != 0)
    {
        // ImGui::Text("pointer = %p", current_image_texture);
        // ImGui::Text("size = %d x %d", current_image_width, current_image_height);
        // ImGui::Text("window size = %.0f x %.0f", ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
        vec2f view = ImGui::GetContentRegionAvail();

        // compute scaling factor to fir to the window (pane1)
        if (this->compute_scale_flag == true)
        {
            // reset flag
            this->compute_scale_flag = false;

            // compute scale
            this->scale = std::min(ImGui::GetWindowWidth() / current_image_width, ImGui::GetWindowHeight() / current_image_height);
            spdlog::debug("Resizing factor : {}", this->scale);

            // save view size
            this->img_view.x = view.x;
            this->img_view.y = view.y;

            // parse json once the scale is obtained
            this->json_read();
        }

        if ((view.x != this->img_view.x) || (view.y != this->img_view.y))
        {
            this->compute_scale_flag = true;
        }

        // draw image
        ImGui::Image(
            (void *)(intptr_t)this->current_image_texture,                     // image texture
            ImVec2(current_image_width * scale, current_image_height * scale), // x and y dimensions (scaled)
            ImVec2(0.0f, 0.0f),                                                // (x,y) coordinates start in [0.0, 1.0]
            ImVec2(1.0f, 1.0f)                                                 // (x,y) coordinates end in [0.0, 1.0]
        );

        for (long unsigned n = 0; n < this->annotations.size(); n++)
        {
            // draw all annotations instances on the image
            for (long unsigned m = 0; m < this->annotations[n].inst.size(); m++)
            {
                if (this->annotations[n].inst[m].img_fname == this->image_fname)
                {
                    // update and draw on screen
                    if (this->annotations[n].type == ANNOTATION_TYPE_AREA)
                    {
                        this->annotations[n].inst[m].update_area();
                        this->annotations[n].inst[m].draw_area();
                    }
                    else
                    {
                        this->annotations[n].inst[m].update_point();
                        this->annotations[n].inst[m].draw_point();
                    }
                }
            }
        }

        // fsm to handle drawing annotations
        if (ImGui::IsItemHovered())
            this->update_annotation_fsm();
    }
}

void AnnotationApp::update_annotation_fsm(void)
{
    vec2f _w = ImGui::GetWindowPos();
    vec2f _m = ImGui::GetMousePos();
    vec2f cursor_pos = _m - _w;

    bool create_new_instance_flag = true; // if true, will create a new instance of the active annotation
    bool create_state_flag = false;       // if true, fsm is creating and rendering the annotation instance
    int active_annotation = -1;           // track the id of the active annotation
    int active_instance = -1;             // track the id of the active instance
    bool need_json_write = false;

    for (long unsigned n = 0; n < this->annotations.size(); n++)
    {
        // parse all states and instances to define the next FSM action
        for (long unsigned m = 0; m < this->annotations[n].inst.size(); m++)
        {
            if (this->annotations[n].inst[m].status_fsm.state() != StatusStates::IDLE)
            {
                create_new_instance_flag = false;
                create_state_flag = false;
            }

            if (this->annotations[n].inst[m].status_fsm.state() == StatusStates::CREATE)
            {
                create_new_instance_flag = false;
                create_state_flag = true;
                active_annotation = n;
                active_instance = m;
                continue;
            }

            // delete annotation instance on DELETE
            if ((this->annotations[n].inst[m].selected == true) && ImGui::IsKeyPressed(ImGuiKey_Delete))
            {
                this->annotations[n].inst.erase(this->annotations[n].inst.begin() + m);
                this->annotations[n].inst[m].request_json_write = true;
            }

            // order a json write
            if (this->annotations[n].inst[m].request_json_write == true)
            {
                need_json_write = true;
                this->annotations[n].inst[m].request_json_write = false;
            }
        }
    }

    // creating a new annotation instance
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && (create_new_instance_flag == true))
    {
        for (long unsigned n = 0; n < this->annotations.size(); n++)
        {
            if (this->annotations[n].selected)
            {
                AnnotationInstance _ann;
                _ann.set_fname(this->image_fname);
                _ann.rect_on_image.set_bottomright_vertex(cursor_pos);
                _ann.rect_on_image.set_topleft_vertex(cursor_pos);
                _ann.set_color(this->annotations[n].color);

                this->annotations[n].inst.push_back(_ann);

                spdlog::info("New Annotation Instance <{}, type {}> on file {}: at position ({},{})",
                             this->annotations[n].label,
                             this->annotations[n].type,
                             this->annotations[n].inst.back().img_fname,
                             this->annotations[n].inst.back().rect_on_image.get_topleft_vertex().x,
                             this->annotations[n].inst.back().rect_on_image.get_topleft_vertex().y);
            }
        }
    }

    // creating a new instance...
    if (create_state_flag == true)
    {
        // abort creation on escape
        if (ImGui::IsKeyPressed(526))
        {
            spdlog::debug("CANCEL : destroying instance");
            this->annotations[active_annotation].inst.erase(this->annotations[active_annotation].inst.begin() + active_instance);
        }

        // complete creation
        if (this->annotations[active_annotation].type == ANNOTATION_TYPE_POINT)
        {
            // POINT : set center at mouse cursor and switch to idle
            this->annotations[active_annotation].inst[active_instance].rect_on_image.set_center(cursor_pos);
            this->annotations[active_annotation].inst[active_instance].rect_on_image.set_span(vec2f(10, 10));
            this->annotations[active_annotation].inst[active_instance].status_fsm.execute("from_create_to_idle");
            this->annotations[active_annotation].inst[active_instance].update_point();
            this->annotations[active_annotation].inst[active_instance].update_bounding_box();

            need_json_write = true;
        }
        else
        {
            // AREA : complete on mouse click
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                // set the second corner coordinates
                this->annotations[active_annotation].inst[active_instance].rect_on_image.set_bottomright_vertex(cursor_pos);

                // fsm : switch to idle state
                this->annotations[active_annotation].inst[active_instance].status_fsm.execute("from_create_to_idle");

                // update state (bounding box)
                this->annotations[active_annotation].inst[active_instance].update_area();
                this->annotations[active_annotation].inst[active_instance].update_bounding_box();

                // request json update
                need_json_write = true;
            }
        }
    }

    // update json
    if (need_json_write == true)
        this->json_write();
}

void AnnotationApp::check_annotations_file(void)
{
    std::ifstream f(this->annotation_fname.c_str());
    this->annotations_file_exists = f.good();
    spdlog::debug("checking existence of annotation file : {}", this->annotations_file_exists);
}

void AnnotationApp::parse_images_folder(std::string path)
{
    DIR *dir;
    struct dirent *diread;

    // empty the list and do the search from scratch
    this->image_files.clear();

    if ((dir = opendir(path.c_str())) != nullptr)
    {
        spdlog::info("Browsing folder : {}", path.c_str());
        while ((diread = readdir(dir)) != nullptr)
        {
            spdlog::debug("testing : {}", diread->d_name);

            // filter images
            if (!strcmp(diread->d_name, "."))
                continue;
            if (!strcmp(diread->d_name, ".."))
                continue;
            if (diread->d_name[0] == '.')
                continue;

            // add file to the list if the extension is allowed
            std::string fn = diread->d_name;
            std::string extension = fn.substr(fn.find_last_of(".") + 1);

            spdlog::debug("extension : {}", extension);

            if (this->ext_set.find(extension) != this->ext_set.end())
            {
                // add image to the set
                this->image_files.push_back(fn);

                // create entry in dictionary counting instances
                if (this->ninstperimage.find(fn) == this->ninstperimage.end())
                {
                    this->ninstperimage[fn] = 0;
                }

                // -- log
                spdlog::info("File added : {}", diread->d_name);
            }
        }

        // sort filenames alphabetically
        std::sort(this->image_files.begin(), this->image_files.end(),
                  [](const std::string &a, const std::string &b) -> bool
                  { return a < b; });

        closedir(dir);
    }
    else
    {
        spdlog::error("Folder {} cannot be open", path.c_str());
    }

    // memorizing the folder for later use
    this->images_folder = path;

    // create a priori the annotation file name
    this->annotation_fname = path + "/annotations.json";
    spdlog::debug("Expected annotation file : {}", this->annotation_fname.c_str());

    // read and parse json file if it exists
    this->check_annotations_file();
    this->json_read();
}

// Simple helper function to load an image into a OpenGL texture with common settings
bool AnnotationApp::read_image(const char *filename, GLuint *out_texture, int *out_width, int *out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char *image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

void AnnotationApp::activate_annotation(long unsigned int k)
{
    for (long unsigned int n = 0; n < this->annotations.size(); n++)
    {
        if (n == k)
        {
            this->annotations[n].selected = true;
        }
        else
        {

            this->annotations[n].selected = false;
        }
    }
}

void AnnotationApp::clear_annotations(void)
{
    this->annotations.clear();
    this->ninstperimage.clear();
}

void AnnotationApp::import_annotations_from_prev(void)
{
    // find previous image than the current one
    std::string prev_fname;
    for (long unsigned int n = 1; n < this->image_files.size(); n++)
    {
        if (this->image_files[n].compare(this->image_fname) == 0)
        {
            prev_fname = this->image_files[n - 1];
            break;
        }
    }

    // import all annotations from this previous image
    if (!prev_fname.empty())
    {
        for (auto &annotation : this->annotations)
        {
            for (auto &instance : annotation.inst)
            {
                if (instance.img_fname == prev_fname)
                {
                    // ? using copy constructor created by compiler by default
                    AnnotationInstance _inst = AnnotationInstance(instance);
                    _inst.set_fname(this->image_fname);
                    annotation.inst.push_back(_inst);
                }
            }
        }

        // dump new data to file
        this->json_write();
    }
}