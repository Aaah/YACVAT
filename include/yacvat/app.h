#ifndef APP_H
#define APP_H

// #include <filesystem>
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <map>

#include <SDL_opengl.h>
#include "annotations.h"
#include "nlohmann/json.hpp"

class AnnotationApp
{
public:
    AnnotationApp();           // default init
    void ui_initialize(void);  // init ui
    void ui_main_window(void); // main window

private:
    bool open_images_folder_flag;             // flag to open file dialog
    bool annotations_file_exists;             // is there an annotation file in the folder
    std::string images_folder;                // path to valid folder containing images
    std::vector<std::string> image_files;     // list of valid images in the folder
    std::vector<int> annotation_count;        // number of annotation on the current image
    std::set<std::string> ext_set;            // list of extensions accepted as images
    GLuint current_image_texture;             // opengl texture for the loaded image
    int current_image_width;                  // width of the picture
    int current_image_height;                 // height of the picture
    float scale;                              // scaling factor on the displayed image
    std::vector<Annotation> annotations;      // list of annotations available
    std::fstream fs;                          // file pointer to the annotation file
    std::string annotation_fname;             // full path
    std::string image_fname;                  // currently opened image file name
    nlohmann::json json;                      // json data structure
    bool compute_scale_flag;                  // compute scale factor to resize image
    std::map<std::string, int> ninstperimage; // dict to count the number of instances per image

    bool read_image(const char *filename, GLuint *out_texture, int *out_width, int *out_height);
    void check_annotations_file(void);             // look for the presence of an annotations file
    void activate_annotation(long unsigned int n); // activate annotation n and deactivate all others
    void parse_images_folder(std::string path);    // list image files
    void ui_images_folder(void);                   // draw the UI to displays files
    void ui_image_current(void);                   // display current image
    void ui_annotations_panel(void);               // create/edit annotations type
    void json_read(void);                          // read/write info to the annotation file
    void json_write(void);                         // read/write info to the annotation file
    void update_annotation_fsm(void);              // update the logic to handle annotation instances
    void clear_annotations(void);                  // clear all annotations
    void import_annotations_from_prev(void);       // import annotations from the previous image in the list
};

#endif