#include "app.h"
#include "spdlog/spdlog.h"

#include <iostream>
#include <vector>
#include <dirent.h>
#include <set>

AnnotationApp::AnnotationApp(void)
{
    open_images_folder_flag = false;

    extensions_set.clear();
    extensions_set.insert("png");
    extensions_set.insert("jpeg");
    extensions_set.insert("jpg");
}

void AnnotationApp::ui_images_folder(void)
{
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

            if (this->extensions_set.find(extension) != this->extensions_set.end())
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
}