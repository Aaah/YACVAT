#include "app.h"
#include "spdlog/spdlog.h"
#include "imgui.h"

#include <iostream>
#include <vector>
#include <dirent.h>
#include <set>

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
    // does not work
    for (std::string e : this->image_files)
        ImGui::Button(e.c_str());
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
                this->image_files.push_back(diread->d_name);
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