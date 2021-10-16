#pragma once

#include "domain.h"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <deque>

namespace transport_catalogue {

    class TransportCatalogue {
    public:
        void AddStop(domain::Stop &stop);

        void AddBus(const std::string &bus, const std::deque<std::string> &route);

        void SetDistance(const std::string &from, const std::string &to, int dist);

        const domain::Bus *GetBus(const std::string &bus) const;

        const domain::BusStatistics *GetBusStatistics(const std::string &bus) const;

        const domain::Stop *GetStop(const std::string &stop) const;

        const std::set<std::string_view> &GetBusesForExistingStop(const std::string &stop) const;

        int GetDistance(const std::string &from, const std::string &to) const;

    private:

        struct PairStopHasher {
            size_t operator()(const std::pair<const domain::Stop *, const domain::Stop *> &pair) const {
                return static_cast<size_t>(pair.first->coordinates.lat
                                           + 37 * pair.first->coordinates.lng
                                           + 37 * 37 * pair.second->coordinates.lat
                                           + 37 * 37 * 37 * pair.second->coordinates.lng);
            }
        };

        std::deque<domain::Bus> buses_;
        std::deque<domain::Stop> stops_;
        std::deque<domain::BusStatistics> statistics_;
        std::unordered_map<std::string_view, const domain::Stop *> stop_to_coords;
        std::unordered_map<std::string_view, const domain::Bus *> bus_to_description;
        std::unordered_map<std::string_view, const domain::BusStatistics *> bus_to_statistics;
        std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses;
        std::unordered_map<std::pair<const domain::Stop *, const domain::Stop *>, int, PairStopHasher> dist_between_stops;
    };

}