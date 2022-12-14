#include <cmath>
#include "yacvat/rectangle.h"

// to enable basic instanciation inside another class
Rectangle::Rectangle(void)
{
}

Rectangle::Rectangle(vec2<float> start, vec2<float> end)
{
    set_topleft_vertex(start);
    set_bottomright_vertex(end);
}

Rectangle::Rectangle(const Rectangle &rhs)
{
    this->bottomright_vertex = rhs.bottomright_vertex;
    this->topleft_vertex = rhs.topleft_vertex;
    this->center = rhs.center;
    this->span = rhs.span;
}

bool Rectangle::intersect(Rectangle rect)
{
    if ((std::abs(center.x - rect.center.x) < 0.5 * (span.x + rect.span.x)) && (std::abs(center.y - rect.center.y) < 0.5 * (span.y + rect.span.y)))
    {
        return true;
    }
    return false;
}

bool Rectangle::inside(vec2<float> point)
{
    if ((2.0 * std::abs(point.x - center.x) < span.x) && (2.0 * std::abs(point.y - center.y) < span.y))
    {
        return true;
    }
    return false;
}

void Rectangle::set_center(vec2<float> point)
{
    // when setting the center, the span remains the same
    center = point;

    // update vertices
    topleft_vertex.x = center.x - span.x / 2.0;
    topleft_vertex.y = center.y - span.y / 2.0;
    bottomright_vertex.x = center.x + span.x / 2.0;
    bottomright_vertex.y = center.y + span.y / 2.0;
}

void Rectangle::set_span(vec2<float> point)
{
    // when setting the span, the center remains the same
    span = point;

    // update vertices
    topleft_vertex.x = center.x - span.x / 2.0;
    topleft_vertex.y = center.y - span.y / 2.0;
    bottomright_vertex.x = center.x + span.x / 2.0;
    bottomright_vertex.y = center.y + span.y / 2.0;
}

void Rectangle::set_topleft_vertex(vec2<float> point)
{
    // when setting one vertex, the second remains the same
    topleft_vertex = point;

    // update span and center
    span.x = std::abs(topleft_vertex.x - bottomright_vertex.x);
    span.y = std::abs(topleft_vertex.y - bottomright_vertex.y);
    center.x = (topleft_vertex.x + bottomright_vertex.x) / 2.0;
    center.y = (topleft_vertex.y + bottomright_vertex.y) / 2.0;
}

void Rectangle::set_bottomright_vertex(vec2<float> point)
{
    // when setting one vertex, the second remains the same
    if ((point.x < topleft_vertex.x) && (point.y < topleft_vertex.y))
    {
        bottomright_vertex = topleft_vertex;
        topleft_vertex = point;
    }
    else
    {
        bottomright_vertex = point;
    }

    // update span and center
    span.x = std::abs(topleft_vertex.x - bottomright_vertex.x);
    span.y = std::abs(topleft_vertex.y - bottomright_vertex.y);
    center.x = (topleft_vertex.x + bottomright_vertex.x) / 2.0;
    center.y = (topleft_vertex.y + bottomright_vertex.y) / 2.0;
}