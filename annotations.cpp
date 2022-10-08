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
        {STATE_CREATE, STATE_IDLE, 'a', nullptr, nullptr},
        {STATE_IDLE, STATE_EDIT, 'b', nullptr, nullptr},
        {STATE_EDIT, STATE_IDLE, 'c', nullptr, nullptr},
    });

    // position of the instance on the window
    this->coords[0] = pos.x;
    this->coords[1] = pos.y;
}

void AnnotationInstance::draw(void)
{
    spdlog::debug("Drawing!");
}
