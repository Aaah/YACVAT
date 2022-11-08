#pragma once

#define IM_VEC2_CLASS_EXTRA                             \
    constexpr ImVec2(const vec2<float> &f) : x(f.x), y(f.y) {} \
    operator vec2<float>() const { return vec2<float>(x, y); }
