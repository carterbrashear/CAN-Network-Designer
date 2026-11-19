#pragma once
#include "../Project/NetworkProject.hpp"
#include <utility>
#include <cstdint>
class Canvas2D{
    float zoom_level;
    float pan_x;
    float pan_y;
    int64_t selected_device_id;
    int64_t selected_bus_id;
    
    public:
        //uncomment when CommandStack is implemented
        //void render(NetworkProject& project, CommandStack& commands);
        std::pair<float, float> screenToWorld(float sx, float sy);
        std::pair<float,float> worldToScreen(float wx, float wy);
};