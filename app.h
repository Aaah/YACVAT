#ifndef APP_H
#define APP_H

// #include <filesystem>
#include <string>
#include <vector>
#include <set>

class AnnotationApp
{
public:
    AnnotationApp();
    
    bool open_images_folder_flag;                // flag to open file dialog
    
    void update_images_folder(std::string path); // list image files
    void ui_images_folder(void);                 // draw the UI to displays files

private:
    std::string images_folder;            // path to valid folder containing images
    std::vector<std::string> image_files; // list of valid images in the folder
    std::vector<int> annotation_count;    // number of annotation on the current image
    std::set<std::string> ext_set;        // list of extensions accepted as images
    
    void read_image(std::string path); // load an image
};

#endif