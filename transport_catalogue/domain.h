#pragma once

#include "geo.h"

#include <string>
#include <deque>
#include <set>
#include <unordered_map>
#include <variant>
#include <vector>

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

    struct WaitItem {
        std::string stop_name;
        int time;
    };

    struct BusItem {
        std::string bus;
        int span_count;
        double time;
    };

    struct EfficientRoute {
        double total_time = 0;
        std::vector<std::pair<WaitItem, BusItem>> items;
    };

    struct PairStopHasher {
        size_t operator()(const std::pair<const domain::Stop *, const domain::Stop *> &pair) const {
            return static_cast<size_t>(pair.first->coordinates.lat
                                       + 37 * pair.first->coordinates.lng
                                       + 37 * 37 * pair.second->coordinates.lat
                                       + 37 * 37 * 37 * pair.second->coordinates.lng);
        }
    };

    struct DataPack {
        std::unordered_map<std::string_view, const domain::Stop *> stop_to_coords;
        std::unordered_map<std::string_view, const domain::Bus *> bus_to_description;
        std::unordered_map<std::string_view, const domain::BusStatistics *> bus_to_statistics;
        std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses;
        std::unordered_map<std::pair<const domain::Stop *, const domain::Stop *>, int, PairStopHasher> dist_between_stops;
    };
}