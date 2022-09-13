#ifndef APP_H
#define APP_H

// #include <filesystem>
#include <string>
#include <vector>
#include <set>

class AnnotationApp
{
public:
    // std::filesystem::path image_folder; // folder containing images to annotate
    AnnotationApp();
    bool open_images_folder_flag;                // flag to open file dialog
    void update_images_folder(std::string path); // list image files
    void ui_images_folder(void);                 // draw the UI to displays files

private:
    std::vector<char *> image_files;      // list of valid images in the folder
    std::set<std::string> extensions_set; // list of extensions accepted as images
};

#endif