#include "annotations.h"
#include "nlohmann/json.hpp"
#include <fstream>

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

bool Annotation::json_read(std::string fname)
{
    std::ifstream f(fname.c_str());
    nlohmann::json data = nlohmann::json::parse(f);

    // header

    // ? the app has to read and parse the json

    // instances
    return true;
}

bool Annotation::json_write(std::string fname)
{
    // header

    // ? if the file does not exist, create it, return false, else return true
    // ? if the label does not exist : dump it in the header section
    // ? if the label exists already : update attributes accordingly to comments in the header

    // instances

    // ? replace all instances
    return true;
}