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

AnnotationInstance::AnnotationInstance(ImVec2 pos)
{
    // finite state machine instanciation
    this->fsm.add_transitions({
        {States::CREATE, States::IDLE, "from_create_to_idle", nullptr, nullptr},
        {States::IDLE, States::EDIT, "from_idle_to_edit", nullptr, nullptr},
        {States::EDIT, States::IDLE, "from_edit_to_idle", nullptr, nullptr},
    });

    // position of the instance on the window
    this->set_corner_start(pos);
}

void AnnotationInstance::set_corner_start(ImVec2 pos)
{
    this->coords[0] = pos;
}

void AnnotationInstance::set_corner_end(ImVec2 pos)
{
    this->coords[1] = pos;
}

void AnnotationInstance::draw(void)
{
    // todo : color memed as IM_COL32 as well as floats
    // todo : store those absolute coordinates for optimisation?

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 _w = ImGui::GetWindowPos();
    ImVec2 _m = ImGui::GetMousePos();

    if (this->fsm.state() == States::CREATE)
    {
        // compute absolute coodinates
        ImVec2 start = _w;
        start.x += this->coords[0].x;
        start.y += this->coords[0].y;

        draw_list->AddRect(start, _m, IM_COL32(0, 255, 0, 255), 0.0, 0, 2.0);
    }
    else if (this->fsm.state() == States::IDLE)
    {
        // compute absolute coodinates
        ImVec2 start = _w;
        start.x += this->coords[0].x;
        start.y += this->coords[0].y;

        ImVec2 end = _w;
        end.x += this->coords[1].x;
        end.y += this->coords[1].y;

        draw_list->AddRect(start, end, IM_COL32(0, 255, 0, 255), 0.0, 0, 1.0);
    }
}
