#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

#include "vec2.h"

class Rectangle
{

public:
    Rectangle();
    Rectangle(vec2<float> start, vec2<float> end);
    Rectangle(const Rectangle &rhs); // copy constructor
    Rectangle &operator=(const Rectangle &rhs)
    {
        this->bottomright_vertex = rhs.bottomright_vertex;
        this->topleft_vertex = rhs.topleft_vertex;
        this->center = rhs.center;
        this->span = rhs.span;
        return *this;
    };

    // setters
    void set_center(vec2<float> point);
    void set_span(vec2<float> point);
    void set_topleft_vertex(vec2<float> point);
    void set_bottomright_vertex(vec2<float> point);

    // getters
    vec2<float> get_center() { return center; }
    vec2<float> get_span() { return span; }
    vec2<float> get_topleft_vertex() { return topleft_vertex; }
    vec2<float> get_bottomright_vertex() { return bottomright_vertex; }

    // processing
    bool intersect(Rectangle rect);
    bool inside(vec2<float> point);

private:
    vec2<float> center;
    vec2<float> span;
    vec2<float> topleft_vertex;
    vec2<float> bottomright_vertex;
};

#endif