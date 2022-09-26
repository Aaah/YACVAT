#include "annotations.h"
#include "nlohmann/json.hpp"

// #include <stdlib> /* srand, rand */
// #include <time>   /* time */

// todo : change
AnnotationConfig::AnnotationConfig(std::string label)
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

// todo : json_read & json_write