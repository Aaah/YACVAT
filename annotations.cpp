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
    this->coords[0] = pos.x;
    this->coords[1] = pos.y;
}

void AnnotationInstance::set_corner_end(ImVec2 pos)
{
    this->coords[2] = pos.x;
    this->coords[3] = pos.y;
}

void AnnotationInstance::draw(void)
{
    // spdlog::debug("Drawing!");
}
