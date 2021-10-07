#pragma once

#include "geo.h"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <deque>

namespace transport_catalogue {

    class TransportCatalogue {
    public:
        struct Bus {
            std::string bus_name;
            int all_stops = 0;
            int unique_stops = 0;
            int route_length = 0;
            double curvature = 1;
            std::deque<std::string_view> route;
        };

        struct Stop {
            std::string stop_name;
            geo::Coordinates coordinates;
        };

        void AddStop(Stop &stop);

        void AddBus(const std::string &bus, const std::deque<std::string> &route);

        void AddDistance(const std::string &stop1, const std::string &stop2, int dist);

        const Bus *GetBus(const std::string &bus);

        const Stop *GetStop(const std::string &stop);

        const std::set<std::string_view> &GetBusesForExistingStop(const std::string &stop);

        int GetDistance(const std::string &stop1, const std::string &stop2);

    private:

        struct PairStopHasher {
            size_t operator()(const std::pair<const Stop *, const Stop *> &pair) const {
                return static_cast<size_t>(pair.first->coordinates.lat + 37 * pair.first->coordinates.lng +
                                           37 * 37 * pair.second->coordinates.lat +
                                           37 * 37 * 37 * pair.second->coordinates.lng);
            }
        };

        std::deque<Bus> buses_;
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, const Stop *> stop_to_coords;
        std::unordered_map<std::string_view, const Bus *> bus_to_description;
        std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses;
        std::unordered_map<std::pair<const Stop *, const Stop *>, int, PairStopHasher> dist_between_stops;
    };

}