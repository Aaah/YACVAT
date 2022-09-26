#ifndef APP_H
#define APP_H

// #include <filesystem>
#include <string>
#include <vector>
#include <set>
#include <SDL_opengl.h>
#include "annotations.h"

class AnnotationApp
{
public:
    AnnotationApp();

    bool open_images_folder_flag; // flag to open file dialog
    bool annotations_file_exists;

    void update_images_folder(std::string path); // list image files
    void ui_images_folder(void);                 // draw the UI to displays files
    void ui_image_current(void);                 // display current image
    void ui_annotations_panel(void);             // create/edit annotations type

private:
    std::string images_folder;                 // path to valid folder containing images
    std::vector<std::string> image_files;      // list of valid images in the folder
    std::vector<int> annotation_count;         // number of annotation on the current image
    std::set<std::string> ext_set;             // list of extensions accepted as images
    GLuint current_image_texture;              // opengl texture for the loaded image
    int current_image_width;                   // width of the picture
    int current_image_height;                  // height of the picture
    float scale;                               // scaling factor on the displayed image
    std::vector<AnnotationConfig> annotations; // list of annotations available

    bool read_image(const char *filename, GLuint *out_texture, int *out_width, int *out_height);
    void ui_annotations_continue(void); // ui panel if an annotations file exists
    void ui_annotations_setup(void);    // ui panel if no annotations file exists
    void check_annotations_file(void);  // look for the presence of an annotations file
    void json_update_annoations(void);  // create annotations file (with labels and types)
    void json_update_header(void);      // create annotations file (with labels and types)
};

#endif