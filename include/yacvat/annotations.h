#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include <string>
#include <vector>
#include "vec2.h"
#define IMGUI_USER_CONFIG "yacvat/yacvat_imgui_config.h"

#include "fsm.h"
#include "imgui.h"
#include "rectangle.h"

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
    CANCEL,
};

enum class HoverStates
{
    INSIDE,
    OUTSIDE,
    HOVER,
};

enum class Direction
{
    NONE,
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

class AnnotationInstance
{
public:
    // methods
    AnnotationInstance(void);          // default constructor
    void set_fname(std::string fname); // set file name
    void set_color(float color[4]);    // set color
    void draw_area(void);              // draw itself on picture
    void draw_point(void);             // draw itself on picture
    void update_point(void);           // update fsm
    void update_area(void);            // update fsm
    void update_bounding_box(void);    // update inner and outer hover box;

    // attributes
    Rectangle rect_on_image;                                              // coordinates on image : x_start, y_start, x_end, y_end
    FSM::Fsm<StatusStates, StatusStates::CREATE, std::string> status_fsm; // state machine to handle rendering
    FSM::Fsm<HoverStates, HoverStates::HOVER, std::string> hover_fsm;     // state machine to handle logic in edit mode
    uint8_t color_u8[4];                                                  // color
    std::string img_fname;                                                // image file containing the annotation instance
    bool selected;                                                        // is the instance being edited
    bool request_json_write;                                              // has the isntance been update in a way that requires a json dump

private:
    vec2f offset;           // mouse to box center off when starting to drag
    int delta;              // offset to compute bounding boxes from the actual annotation box
    vec2f window_pos;       // window position on screen
    Rectangle rect;         // actual annotation box on screen
    Rectangle outer_rect;   // bounding box to detect mouse hover
    Rectangle inner_rect;   // bounding box to detect mouse hover
    bool dragging_flag;     // is the instance being dragged
    Direction resizing_dir; // is the instance being resized (!=0)
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