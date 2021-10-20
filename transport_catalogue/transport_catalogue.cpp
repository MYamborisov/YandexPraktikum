#include "transport_catalogue.h"

#include <algorithm>
#include <unordered_set>

namespace transport_catalogue {

    void TransportCatalogue::AddStop(domain::Stop &stop) {
        stops_.push_back(std::move(stop));
        stop_to_coords[stops_.back().stop_name] = &stops_.back();
        stop_to_buses[stops_.back().stop_name];
    }

    void TransportCatalogue::AddBus(const std::string &bus, const std::deque<std::string> &route, bool is_roundtrip) {
        domain::Bus bus_struct;
        domain::BusStatistics bus_stat_struct;
        bus_stat_struct.is_roundtrip = is_roundtrip;
        double straight_dist = 0;
        bus_struct.bus_name = bus;
        bus_stat_struct.all_stops = route.size();
        for (size_t i = 0; i < route.size() - 1; ++i) {
            straight_dist +=
                    ComputeDistance(stop_to_coords[route[i]]->coordinates, stop_to_coords[route[i + 1]]->coordinates);
            bus_stat_struct.route_length += GetDistance(route[i], route[i + 1]);
        }
        bus_stat_struct.curvature = bus_stat_struct.route_length / straight_dist;
        std::unordered_set<std::string_view> set_of_unique_stops;
        for (const auto &stop: route) {
            set_of_unique_stops.insert(stop);
            bus_struct.route.push_back(stop_to_coords[stop]->stop_name);
        }
        bus_stat_struct.unique_stops = set_of_unique_stops.size();
        buses_.push_back(std::move(bus_struct));
        statistics_.push_back(bus_stat_struct);
        bus_to_statistics[buses_.back().bus_name] = &statistics_.back();
        bus_to_description[buses_.back().bus_name] = &buses_.back();
        for (const auto &stop: route) {
            stop_to_buses[stop_to_coords[stop]->stop_name].insert(buses_.back().bus_name);
        }
    }

    void TransportCatalogue::SetDistance(const std::string &from, const std::string &to, int dist) {
        auto pair = std::make_pair(stop_to_coords[from], stop_to_coords[to]);
        dist_between_stops[pair] = dist;
    }

    const domain::BusStatistics *TransportCatalogue::GetBusStatistics(const std::string_view &bus) const {
        if (bus_to_statistics.count(bus)) {
            return bus_to_statistics.at(bus);
        }
        else {
            return nullptr;
        }
    }

    std::optional<std::set<std::string_view>> TransportCatalogue::GetBusesByStop(const std::string_view &stop) const {
        if (stop_to_buses.count(stop)) {
            return stop_to_buses.at(stop);
        }
        return std::nullopt;
    }

    int TransportCatalogue::GetDistance(const std::string &from, const std::string &to) const {
        auto pair = std::make_pair(stop_to_coords.at(from), stop_to_coords.at(to));
        if (dist_between_stops.count(pair)) {
            return dist_between_stops.at(pair);
        } else {
            pair = std::make_pair(stop_to_coords.at(to), stop_to_coords.at(from));
            return dist_between_stops.at(pair);
        }
    }

    domain::DataPack TransportCatalogue::GetDataPack() const {
        return {stop_to_coords,
                bus_to_description,
                bus_to_statistics,
                stop_to_buses};
    }
}