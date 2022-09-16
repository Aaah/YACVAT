#ifndef APP_H
#define APP_H

// #include <filesystem>
#include <string>
#include <vector>
#include <set>
#include <SDL_opengl.h>

class AnnotationApp
{
public:
    AnnotationApp();

    bool open_images_folder_flag; // flag to open file dialog

    void update_images_folder(std::string path); // list image files
    void ui_images_folder(void);                 // draw the UI to displays files
    void ui_image_current(void);                 // display current image

private:
    std::string images_folder;            // path to valid folder containing images
    std::vector<std::string> image_files; // list of valid images in the folder
    std::vector<int> annotation_count;    // number of annotation on the current image
    std::set<std::string> ext_set;        // list of extensions accepted as images
    GLuint current_image_texture;              // opengl texture for the loaded image
    int current_image_width;                   // width of the picture
    int current_image_height;                  // height of the picture

    bool read_image(const char *filename, GLuint *out_texture, int *out_width, int *out_height);
};

#endif