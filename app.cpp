#include "app.h"
#include "spdlog/spdlog.h"
#include "imgui.h"

#include <iostream>
#include <vector>
#include <dirent.h>
#include <set>

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
        }
        n++;
    }
}

void AnnotationApp::ui_image_current()
{
    // todo : load texture only once on file selection
    // load image in ram
    if (current_image_texture != 0)
    {
        ImGui::Text("pointer = %p", current_image_texture);
        ImGui::Text("size = %d x %d", current_image_width, current_image_height);

        // todo : find out how the image needs to be scaled
        ImGui::Image((void *)(intptr_t)current_image_texture, ImVec2(current_image_width * 0.5f, current_image_height * 0.5f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
    }
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