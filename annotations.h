#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include <string>

/*

Two concepts are needed : the configuration of the annotation and the instances
- configuration :
    - is valid for the current folder
    - must create an annotation file if it does not exist
    - determines the type of instances
    - if the type attribute changes : erase all instances
    - if the label attribute changes : update all instances & json without erasing
    - same with the color attribute
- instances :
    - are valid for the current picture
    - can draw on the picture
    - parse the annotation file to populate
    - write to the annotation file when an annotation is added
    - has a config that is given by the label of the config linked to it
*/

typedef enum
{
    ANNOTATION_TYPE_POINT,
    ANNOTATION_TYPE_AREA,
} annotation_type_t;

class Annotation
{
public:
    Annotation(std::string label);      // init
    bool json_read(std::string fname);  // parse annotation file
    bool json_write(std::string fname); // dump annotations to file

    std::string label;      // name of the annotation
    char new_label[64];     // new (during edition) name of the annotation
    annotation_type_t type; // type of the annotation (single point coordinates, rectangle area...)
    float color[4];         // color to display square / point on the image
    int shortcut;           // key to select annotation

    // std::vector<AnnotationInstance> inst; // actual annotations on the current image

private:
};

// class AnnotationInstance
// {
// public:
//     AnnotationInstance(void); // init
//     void draw(void);          // draw itself on picture

// private:
// }

#endif