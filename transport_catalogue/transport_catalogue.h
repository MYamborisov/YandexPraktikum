#pragma once

#include "domain.h"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <deque>
#include <optional>

namespace transport_catalogue {

    class TransportCatalogue {
    public:
        void AddStop(domain::Stop &stop);

        void AddBus(const std::string &bus, const std::deque<std::string> &route, bool is_roundtrip);

        void SetDistance(const std::string &from, const std::string &to, int dist);

        const domain::BusStatistics *GetBusStatistics(const std::string_view &bus) const;

        std::optional<std::set<std::string_view>> GetBusesByStop(const std::string_view &stop) const;

        int GetDistance(const std::string &from, const std::string &to) const;

        domain::DataPack GetDataPack() const;

    private:

        std::deque<domain::Bus> buses_;
        std::deque<domain::Stop> stops_;
        std::deque<domain::BusStatistics> statistics_;
        std::unordered_map<std::string_view, const domain::Stop *> stop_to_coords;
        std::unordered_map<std::string_view, const domain::Bus *> bus_to_description;
        std::unordered_map<std::string_view, const domain::BusStatistics *> bus_to_statistics;
        std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses;
        std::unordered_map<std::pair<const domain::Stop *, const domain::Stop *>, int, domain::PairStopHasher> dist_between_stops;
    };

}