#include "annotations.h"
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"
#include "fsm.h"
#include <fstream>
#include "imgui.h"

Annotation::Annotation(std::string label)
{
    // set attributes
    this->label = label;
    this->type = ANNOTATION_TYPE_POINT;

    color[3] = 1.0;
    for (auto n = 0; n < 3; n++)
    {
        this->color[n] = (float)std::rand() / RAND_MAX;
    }

    strcpy(this->new_label, this->label.c_str());
}

void Annotation::update_color(void)
{
    for (long unsigned int n = 0; n < this->inst.size(); n++)
    {
        this->inst[n].set_color(this->color);
    }
}

// -- INSTANCES
AnnotationInstance::AnnotationInstance(void)
{
    // finite state machine : status
    this->status_fsm.add_transitions({
        {StatusStates::CREATE, StatusStates::IDLE, "from_create_to_idle", nullptr, nullptr},
        {StatusStates::IDLE, StatusStates::EDIT, "from_idle_to_edit", nullptr, nullptr},
        {StatusStates::EDIT, StatusStates::IDLE, "from_edit_to_idle", nullptr, nullptr},
        {StatusStates::EDIT, StatusStates::CANCEL, "from_edit_to_cancel", nullptr, nullptr},
        {StatusStates::CANCEL, StatusStates::IDLE, "from_cancel_to_idle", nullptr, nullptr},
    });

    // finite state machine : mouse position to the box
    this->hover_fsm.add_transitions({
        {HoverStates::HOVER, HoverStates::INSIDE, "from_hover_to_inside", nullptr, nullptr},
        {HoverStates::HOVER, HoverStates::OUTSIDE, "from_hover_to_outside", nullptr, nullptr},
        {HoverStates::INSIDE, HoverStates::HOVER, "from_inside_to_hover", nullptr, nullptr},
        {HoverStates::OUTSIDE, HoverStates::HOVER, "from_outside_to_hover", nullptr, nullptr},
    });

    this->outer_rect = Rectangle(ImVec2(0, 0), ImVec2(0, 0));
    this->inner_rect = Rectangle(ImVec2(0, 0), ImVec2(0, 0));
    this->delta = 10.0;
    this->selected = false;
}

void AnnotationInstance::update_bounding_box(void)
{
    this->outer_rect.center.x = std::abs(this->coords[1].x + this->coords[0].x) / 2.0;
    this->outer_rect.center.y = std::abs(this->coords[1].y + this->coords[0].y) / 2.0;
    this->outer_rect.span.x = std::abs(this->coords[1].x - this->coords[0].x) + this->delta;
    this->outer_rect.span.y = std::abs(this->coords[1].y - this->coords[0].y) + this->delta;

    this->inner_rect.center = this->outer_rect.center;
    this->inner_rect.span.x = std::max(1, (int)std::abs(this->coords[1].x - this->coords[0].x) - this->delta);
    this->inner_rect.span.y = std::max(1, (int)std::abs(this->coords[1].y - this->coords[0].y) - this->delta);
}

void AnnotationInstance::set_fname(std::string fname)
{
    this->img_fname = fname;
}

void AnnotationInstance::set_color(float color[4])
{
    for (int k = 0; k < 4; k++)
        this->color_u8[k] = color[k] * 255;
}

void AnnotationInstance::set_corner_start(ImVec2 pos)
{
    this->coords[0] = pos;
}

void AnnotationInstance::set_corner_end(ImVec2 pos)
{
    coords[1].x = std::max(coords[0].x, pos.x);
    coords[1].y = std::max(coords[0].y, pos.y);
    coords[0].x = std::min(coords[0].x, pos.x);
    coords[0].y = std::min(coords[0].y, pos.y);
}

void AnnotationInstance::update(void)
{
    // goal : change states
    // define start & end vec2 to draw

    ImVec2 _w = ImGui::GetWindowPos();
    ImVec2 _m = ImGui::GetMousePos();

    // mouse position on image
    this->mouse_on_image = ImVec2(_m.x - _w.x, _m.y - _w.y);

    // compute absolute coodinates of the start vertex
    this->start_vertex = ImVec2(_w.x + this->coords[0].x, _w.y + this->coords[0].y);

    // update HOVER fsm
    if ((this->hover_fsm.state() == HoverStates::HOVER) && !outer_rect.inside(this->mouse_on_image))
    {
        this->hover_fsm.execute("from_hover_to_outside");
    }
    else if ((this->hover_fsm.state() == HoverStates::HOVER) && inner_rect.inside(this->mouse_on_image))
    {
        this->hover_fsm.execute("from_hover_to_inside");
    }
    else if ((this->hover_fsm.state() == HoverStates::OUTSIDE) && outer_rect.inside(this->mouse_on_image))
    {
        this->hover_fsm.execute("from_outside_to_hover");
    }
    else if ((this->hover_fsm.state() == HoverStates::INSIDE) && !inner_rect.inside(this->mouse_on_image))
    {
        this->hover_fsm.execute("from_inside_to_hover");
    }

    // update status fsm
    if (this->status_fsm.state() == StatusStates::CREATE)
    {
        // the end vertex is the mouse position on screen
        this->end_vertex = _m;
    }
    else
    {
        // position on screen of the end vertex
        this->end_vertex = ImVec2(this->coords[1].x + _w.x, this->coords[1].y + _w.y);

        if (this->status_fsm.state() == StatusStates::IDLE)
        {
            // switch to edit mode
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && (this->hover_fsm.state() == HoverStates::HOVER))
            {
                this->status_fsm.execute("from_idle_to_edit");
                spdlog::debug("IDLE : switching to EDIT");
            }
        }
        else if (this->status_fsm.state() == StatusStates::EDIT)
        {
            // switch to idle mode
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && (this->hover_fsm.state() == HoverStates::OUTSIDE))
            {
                this->status_fsm.execute("from_edit_to_cancel");
                spdlog::debug("EDIT : cancelling current action");
            }

            // todo : edition logic
        }
        else if (this->status_fsm.state() == StatusStates::CANCEL)
        {
            this->status_fsm.execute("from_cancel_to_idle");
            spdlog::debug("CANCEL : switching back to IDLE");
        }
    }
}

void AnnotationInstance::draw(void)
{
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    float _thickness = 1.0;
    if (this->status_fsm.state() == StatusStates::CREATE)
    {
        // draw the rectangle all the way to the mouse cursor
        draw_list->AddRect(this->start_vertex, ImGui::GetMousePos(), IM_COL32(this->color_u8[0], this->color_u8[1], this->color_u8[2], this->color_u8[3]), 0.0, 0, _thickness);
    }
    else
    {
        // update FSM
        if (this->status_fsm.state() == StatusStates::IDLE)
        {
            // change thickness if hovered
            if (this->hover_fsm.state() == HoverStates::HOVER)
            {
                _thickness = 2.0;
            }
        }
        else if (this->status_fsm.state() == StatusStates::EDIT)
        {
            // increase thickness in this mode
            _thickness = 2.0;
        }

        draw_list->AddRect(this->start_vertex, this->end_vertex, IM_COL32(this->color_u8[0], this->color_u8[1], this->color_u8[2], this->color_u8[3]), 0.0, 0, _thickness);
    }
}

Rectangle::Rectangle(void)
{
}

Rectangle::Rectangle(ImVec2 start, ImVec2 end)
{
    this->span.x = std::abs(end.x - start.x);
    this->span.y = std::abs(end.y - start.y);

    if (start.x < end.x)
        this->center.x = start.x + 0.5 * this->span.x;
    else
        this->center.x = end.x + 0.5 * this->span.x;

    if (start.y < end.y)
        this->center.y = start.y + 0.5 * this->span.y;
    else
        this->center.y = end.y + 0.5 * this->span.y;
}

bool Rectangle::intersect(Rectangle rect)
{
    if ((std::abs(center.x - rect.center.x) < 0.5 * (span.x + rect.span.x)) && (std::abs(center.y - rect.center.y) < 0.5 * (span.y + rect.span.y)))
    {
        return true;
    }
    return false;
}

bool Rectangle::inside(ImVec2 point)
{
    if ((2.0 * std::abs(point.x - center.x) < span.x) && (2.0 * std::abs(point.y - center.y) < span.y))
    {
        return true;
    }
    return false;
}
