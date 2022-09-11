#ifndef APP_H
#define APP_H

#include <filesystem>

class AnnotationApp
{
public:
    std::filesystem::path image_folder; // folder containing images to annotate
    void update_images_folder(void);    // list image files
    void ui_images_folder(void);        // draw the UI to displays files
};

#endif