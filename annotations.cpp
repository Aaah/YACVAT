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
    // outer rect on screen
    this->outer_rect.set_center(this->rect.get_center());
    ImVec2 span = this->rect.get_span();
    span.x += this->delta;
    span.y += this->delta;
    this->outer_rect.set_span(span);

    // inner rect on screen
    this->inner_rect.set_center(this->rect.get_center());
    span = this->rect.get_span();
    span.x = std::fmax(1, span.x - this->delta);
    span.y = std::fmax(1, span.y - this->delta);
    this->inner_rect.set_span(span);

    spdlog::debug("Bounding box updated");
    // spdlog::debug("Box (on screen) : box [{}, {}, {}, {}]", this->rect.get_center().x, this->rect.get_center().y, this->rect.get_span().x, this->rect.get_span().y);
    // spdlog::debug("Box (on screen) : outer [{}, {}, {}, {}]", this->outer_rect.get_center().x, this->outer_rect.get_center().y, this->outer_rect.get_span().x, this->outer_rect.get_span().y);
    // spdlog::debug("Box (on screen) : inner [{}, {}, {}, {}]", this->inner_rect.get_center().x, this->inner_rect.get_center().y, this->inner_rect.get_span().x, this->inner_rect.get_span().y);
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

void AnnotationInstance::update(void)
{
    ImVec2 _w = ImGui::GetWindowPos();
    bool update_flag = false;
    if ((_w.x != this->window_pos.x) || (_w.y != this->window_pos.y))
    {
        update_flag = true;
        this->window_pos = _w;
    }

    ImVec2 _m = ImGui::GetMousePos();

    // compute absolute coodinates of the start vertex
    ImVec2 _rect_on_image_topleft = this->rect_on_image.get_topleft_vertex();
    ImVec2 _rect_on_image_bottomright = this->rect_on_image.get_bottomright_vertex();

    if (update_flag == true)
        this->rect.set_topleft_vertex(ImVec2(window_pos.x + _rect_on_image_topleft.x, window_pos.y + _rect_on_image_topleft.y));

    // update HOVER fsm
    if ((this->hover_fsm.state() == HoverStates::HOVER) && !outer_rect.inside(_m))
    {
        this->hover_fsm.execute("from_hover_to_outside");
    }
    else if ((this->hover_fsm.state() == HoverStates::HOVER) && inner_rect.inside(_m))
    {
        this->hover_fsm.execute("from_hover_to_inside");
    }
    else if ((this->hover_fsm.state() == HoverStates::OUTSIDE) && outer_rect.inside(_m))
    {
        this->hover_fsm.execute("from_outside_to_hover");
    }
    else if ((this->hover_fsm.state() == HoverStates::INSIDE) && !inner_rect.inside(_m))
    {
        this->hover_fsm.execute("from_inside_to_hover");
    }

    // update status fsm
    if (this->status_fsm.state() == StatusStates::CREATE)
    {
        // the end vertex is the mouse position on screen
        this->rect.set_bottomright_vertex(_m);
    }
    else
    {
        // the instance is unselected unless it is in edit mode
        this->selected = false;

        // position on screen of the end vertex
        if (update_flag == true)
            this->rect.set_bottomright_vertex(ImVec2(_rect_on_image_bottomright.x + window_pos.x, _rect_on_image_bottomright.y + window_pos.y));

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
            // the instance is selected by default in edit mode
            this->selected = true;

            // switch to idle mode
            if (ImGui::IsKeyPressed(526) || (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && (this->hover_fsm.state() == HoverStates::OUTSIDE)))
            {
                this->status_fsm.execute("from_edit_to_cancel");
                spdlog::debug("EDIT : cancelling current action");
            }

            // drag instance around in the image
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && (this->hover_fsm.state() == HoverStates::INSIDE))
            {
                // update flag are set to trigger processing when the drag stops
                this->dragging_flag = true;
            }

            if (this->dragging_flag == true)
            {
                // update the center of the rectangle on the screen to follow the mouse cursor
                this->rect.set_center(_m);

                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                {
                    update_flag = true;                      // request bounding box update
                    this->dragging_flag = false;             // reset the processing flag
                    this->request_json_write = true;         // request a json dump
                    ImVec2 center = this->rect.get_center(); // update the rect on image
                    center.x -= this->window_pos.x;
                    center.y -= this->window_pos.y;
                    this->rect_on_image.set_center(center);
                }
            }
            // todo : edit single side on hover + drag + edge detection
        }
        else if (this->status_fsm.state() == StatusStates::CANCEL)
        {
            this->status_fsm.execute("from_cancel_to_idle");
            spdlog::debug("CANCEL : switching back to IDLE");
        }
    }

    if (update_flag == true)
        this->update_bounding_box();
}

void AnnotationInstance::draw(void)
{
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    float _thickness = 1.0;
    if (this->status_fsm.state() == StatusStates::CREATE)
    {
        // draw the rectangle all the way to the mouse cursor
        draw_list->AddRect(this->rect.get_topleft_vertex(), ImGui::GetMousePos(), IM_COL32(this->color_u8[0], this->color_u8[1], this->color_u8[2], this->color_u8[3]), 0.0, 0, _thickness);
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

            if ((this->hover_fsm.state() == HoverStates::INSIDE) || (this->dragging_flag == true))
            {
                draw_list->AddRectFilled(this->rect.get_topleft_vertex(), this->rect.get_bottomright_vertex(), IM_COL32(this->color_u8[0], this->color_u8[1], this->color_u8[2], 25));
            }
        }

        draw_list->AddRect(this->rect.get_topleft_vertex(), this->rect.get_bottomright_vertex(), IM_COL32(this->color_u8[0], this->color_u8[1], this->color_u8[2], this->color_u8[3]), 0.0, 0, _thickness);
    }
}
