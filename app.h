#ifndef APP_H
#define APP_H

// #include <filesystem>
#include <string>
#include <vector>
#include <set>
#include <fstream>

#include <SDL_opengl.h>
#include "annotations.h"
#include "nlohmann/json.hpp"

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
    void json_read(void);                        // read/write info to the annotation file
    void json_write(void);                       // read/write info to the annotation file
    void update_annotation_fsm(void);            //

private:
    std::string images_folder;            // path to valid folder containing images
    std::vector<std::string> image_files; // list of valid images in the folder
    std::vector<int> annotation_count;    // number of annotation on the current image
    std::set<std::string> ext_set;        // list of extensions accepted as images
    GLuint current_image_texture;         // opengl texture for the loaded image
    int current_image_width;              // width of the picture
    int current_image_height;             // height of the picture
    float scale;                          // scaling factor on the displayed image
    std::vector<Annotation> annotations;  // list of annotations available
    std::fstream fs;                      // file pointer to the annotation file
    std::string annotation_fname;         // full path
    nlohmann::json json;                  // json data structure

    bool read_image(const char *filename, GLuint *out_texture, int *out_width, int *out_height);
    void ui_annotations_continue(void); // ui panel if an annotations file exists
    void ui_annotations_setup(void);    // ui panel if no annotations file exists
    void check_annotations_file(void);  // look for the presence of an annotations file
    void json_update_annoations(void);  // create annotations file (with labels and types)
    void json_update_header(void);      // create annotations file (with labels and types)
    void json_parse_header(void);       // create annotations file (with labels and types)
};

#endif