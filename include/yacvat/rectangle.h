#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

#include "imgui.h"

class Rectangle
{

public:
    Rectangle();
    Rectangle(ImVec2 start, ImVec2 end);

    // setters
    void set_center(ImVec2 point);
    void set_span(ImVec2 point);
    void set_topleft_vertex(ImVec2 point);
    void set_bottomright_vertex(ImVec2 point);

    // getters
    ImVec2 get_center() { return center; }
    ImVec2 get_span() { return span; }
    ImVec2 get_topleft_vertex() { return topleft_vertex; }
    ImVec2 get_bottomright_vertex() { return bottomright_vertex; }

    // processing
    bool intersect(Rectangle rect);
    bool inside(ImVec2 point);

private:
    ImVec2 center;
    ImVec2 span;
    ImVec2 topleft_vertex;
    ImVec2 bottomright_vertex;
};

#endif