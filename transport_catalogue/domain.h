#pragma once

#include "geo.h"

#include <string>
#include <deque>

namespace domain {

    struct Bus {
        std::string bus_name;
        std::deque <std::string_view> route;
    };

    struct BusStatistics {
        int all_stops = 0;
        int unique_stops = 0;
        int route_length = 0;
        double curvature = 1;
    };

    struct Stop {
        std::string stop_name;
        geo::Coordinates coordinates;
    };
}