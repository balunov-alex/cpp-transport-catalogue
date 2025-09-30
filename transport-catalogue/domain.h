#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace transport {
    
struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Route {
    std::string name;
    std::vector<std::string> stops;
    bool is_roundtrip = false;
};
 
} // namespace transport
