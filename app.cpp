#include "app.h"
#include "spdlog/spdlog.h"
#include "imgui.h"

#include <iostream>
#include <vector>
#include <dirent.h>
#include <set>

#include "nlohmann/json.hpp"
#include <fstream>

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

    for (auto e : ext_set)
        spdlog::debug("set of extension allowed : {}", e);
}

void AnnotationApp::ui_annotations_panel(void)
{
    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;

    bool update_json_flag = false;

    if (ImGui::BeginTable("table_annotations", 5, flags))
    {
        ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        static char _unused_ids[64] = "";

        for (long unsigned int n = 0; n < this->annotations.size(); n++)
        {
            ImGui::TableNextRow();

            // shortcut to select the annotation
            ImGui::TableSetColumnIndex(0);
            this->annotations[n].shortcut = n;
            sprintf(_unused_ids, "%d##shortcutext", this->annotations[n].shortcut);
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
            if (ImGui::InputText(_unused_ids, this->annotations[n].new_label, 64, ImGuiInputTextFlags_EnterReturnsTrue))
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
            sprintf(_unused_ids, "-##delbuttont%ld", n);
            if (ImGui::Button(_unused_ids))
            {
                this->annotations.erase(this->annotations.begin() + n);
                update_json_flag = true;
            }
        }
        ImGui::EndTable();
    }

    // add new annotation
    if (ImGui::Button("+"))
    {
        this->annotations.push_back(Annotation("new label"));
        update_json_flag = true;
    }

    if (update_json_flag == true)
    {
        this->json_write();
    }

    // freeze current configuration and start labeling
    ImGui::Separator();

    // todo list annotations in the current image
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

    // create header
    nlohmann::json new_header;

    for (long unsigned n = 0; n < this->annotations.size(); n++)
    {
        new_header["header"][this->annotations[n].label.c_str()] = {{"type", this->annotations[n].type},
                                                                    {"color", this->annotations[n].color}};
    }

    // flush file
    std::ofstream f(this->annotation_fname.c_str());
    f << std::setw(4) << new_header << std::endl; // pretty json using setw(4)
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

    auto header = this->json["header"];
    if (header != NULL)
    {
        // empty list of annotations
        this->annotations.clear();

        for (nlohmann::json::iterator i = header.begin(); i != header.end(); ++i)
        {
            // create new annotation
            Annotation _ann = Annotation(i.key());

            // get attributes
            auto val = i.value();

            for (nlohmann::json::iterator it = val.begin(); it != val.end(); ++it)
            {
                std::string _key = it.key();

                if (_key == "type")
                {
                    _ann.type = val["type"].get<annotation_type_t>();
                }

                if (_key == "color")
                {
                    auto _vec = val["color"].get<std::vector<float>>();
                    for (int n = 0; n < 4; n++)
                    {
                        _ann.color[n] = _vec[n];
                    }
                }
            }

            // add annotation to the list of valid annotations
            this->annotations.push_back(_ann);
        }
    }

    // compose annotations from the header
    // get current image info
}

void AnnotationApp::ui_images_folder(void)
{
    int n = 0;
    static int selected = -1;
    for (auto e : this->image_files)
    {
        // single selectable to display filenames
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
        }
        n++;
    }
}

void AnnotationApp::ui_image_current()
{
    // load image in ram
    if (this->current_image_texture != 0)
    {
        // ImGui::Text("pointer = %p", current_image_texture);
        // ImGui::Text("size = %d x %d", current_image_width, current_image_height);
        // ImGui::Text("window size = %.0f x %.0f", ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

        // compute scaling factor to fir to the window (pane1)
        if (this->scale == 0.0)
        {
            this->scale = std::min(ImGui::GetWindowWidth() / current_image_width, ImGui::GetWindowHeight() / current_image_height);
            spdlog::debug("Resizing factor : {}", this->scale);
        }

        // draw image
        ImGui::Image(
            (void *)(intptr_t)this->current_image_texture,                     // image texture
            ImVec2(current_image_width * scale, current_image_height * scale), // x and y dimensions (scaled)
            ImVec2(0.0f, 0.0f),                                                // (x,y) coordinates start in [0.0, 1.0]
            ImVec2(1.0f, 1.0f)                                                 // (x,y) coordinates end in [0.0, 1.0]
        );

        // draw all annotations instances on the image
        for (long unsigned n = 0; n < this->annotations.size(); n++)
        {
            for (long unsigned m = 0; m < this->annotations[n].inst.size(); m++)
            {
                this->annotations[n].inst[m].draw();
            }
        }

        // fsm to handle drawing annotations
        // - todo : draw all existing annotations
        // - todo : add new annotation process
        // - todo : edit existing annotation (select + change attributes)
        if (ImGui::IsItemHovered())
            this->update_annotation_fsm();
    }
}

void AnnotationApp::update_annotation_fsm(void)
{
    ImVec2 _w = ImGui::GetWindowPos();
    ImVec2 _m = ImGui::GetMousePos();
    ImVec2 cursor_pos = ImVec2(_m.x - _w.x, _m.y - _w.y);

    bool create_new_instance_flag = true; // if true, will create a new instance of the active annotation
    bool create_state_flag = false;       // if true, fsm is creating and rendering the annotation instance
    int active_annotation = -1;           // track the id of the active annotation
    int active_instance = -1;             // track the id of the active instance

    // parse all states and instances to define the next FSM action
    for (long unsigned n = 0; n < this->annotations.size(); n++)
    {
        for (long unsigned m = 0; m < this->annotations[n].inst.size(); m++)
        {
            // spdlog::debug("annotation {} : instance {} state = {})", this->annotations[n].label, m, this->annotations[n].inst[m].fsm.state());
            if (this->annotations[n].inst[m].fsm.state() != States::IDLE)
            {
                create_new_instance_flag = false;
                create_state_flag = false;
            }

            if (this->annotations[n].inst[m].fsm.state() == States::CREATE)
            {
                create_new_instance_flag = false;
                create_state_flag = true;
                active_annotation = n;
                active_instance = m;
                continue;
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
                this->annotations[n].inst.push_back(AnnotationInstance(cursor_pos, this->annotations[n].color));
                spdlog::info("New Annotation Instance <{}, type {}> : at position ({},{})",
                             this->annotations[n].label,
                             this->annotations[n].type,
                             this->annotations[n].inst.back().coords[0].x,
                             this->annotations[n].inst.back().coords[0].y);
            }
        }
    }

    // create state
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && (create_state_flag == true))
    {
        // set the second corner coordinates
        this->annotations[active_annotation].inst[active_instance].set_corner_end(cursor_pos);

        // fsm : switch to idle state
        this->annotations[active_annotation].inst[active_instance].fsm.execute("from_create_to_idle");
    }

    // todo : render all annotation instances
}

void AnnotationApp::check_annotations_file(void)
{
    std::ifstream f(this->annotation_fname.c_str());
    this->annotations_file_exists = f.good();
    spdlog::debug("checking existence of annotation file : {}", this->annotations_file_exists);
}

void AnnotationApp::update_images_folder(std::string path)
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
                this->image_files.push_back(fn);
                spdlog::info("File added : {}", diread->d_name);
            }
        }
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
