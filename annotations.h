#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include <string>
#include <vector>
#include "fsm.h"
#include "imgui.h"

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

enum class StatusStates
{
    CREATE,
    IDLE,
    EDIT,
};

enum class HoverStates
{
    INSIDE,
    OUTSIDE,
    HOVER,
};

class Rectangle
{
public:
    Rectangle();
    Rectangle(ImVec2 start, ImVec2 end);
    bool intersect(Rectangle rect);
    bool inside(ImVec2 point);
    ImVec2 center;
    ImVec2 span;

private:
};

class AnnotationInstance
{
public:
    // methods
    AnnotationInstance(void);          // init
    void set_fname(std::string fname); // set file name
    void set_color(float color[4]);    // set color
    void set_corner_start(ImVec2 pos); // set one corner coordinates
    void set_corner_end(ImVec2 pos);   // set the opposite end corder coordinates
    void draw(void);                   // draw itself on picture
    void update(void);                 // update fsm
    void update_bounding_box(void);    // update inner and outer hover box;

    // attributes
    ImVec2 coords[2];                                                     // coordinates : x_start, y_start, x_end, y_end
    FSM::Fsm<StatusStates, StatusStates::CREATE, std::string> status_fsm; // state machine to handle rendering
    FSM::Fsm<HoverStates, HoverStates::HOVER, std::string> hover_fsm;     // state machine to handle logic in edit mode
    uint8_t color_u8[4];                                                  // color
    std::string img_fname;                                                // image file containing the annotation instance
    bool selected;                                                        // is the instance being edited

private:
    int delta;            //
    Rectangle outer_rect; // bounding box to detect mouse hover
    Rectangle inner_rect; // bounding box to detect mouse hover
    ImVec2 start_vertex;
    ImVec2 end_vertex;
    ImVec2 mouse_on_image;
};

class Annotation
{
public:
    // methods
    Annotation(std::string label); // init
    void update_color(void);       // change the color to all instances to the current color

    // attributes
    std::string label;                    // name of the annotation
    char new_label[64];                   // new (during edition) name of the annotation
    annotation_type_t type;               // type of the annotation (single point coordinates, rectangle area...)
    float color[4];                       // color to display square / point on the image
    int shortcut;                         // key to select annotation
    bool selected;                        // is annotation selected / active?
    std::vector<AnnotationInstance> inst; // actual annotations on the current image
private:
};

#endif