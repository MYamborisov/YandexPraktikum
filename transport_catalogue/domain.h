#pragma once

#include "geo.h"

#include <string>
#include <deque>
#include <set>
#include <unordered_map>

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
        bool is_roundtrip = false;
    };

    struct Stop {
        std::string stop_name;
        geo::Coordinates coordinates;
    };

    struct DataPack {
        std::unordered_map<std::string_view, const domain::Stop *> stop_to_coords;
        std::unordered_map<std::string_view, const domain::Bus *> bus_to_description;
        std::unordered_map<std::string_view, const domain::BusStatistics *> bus_to_statistics;
        std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses;
    };
}