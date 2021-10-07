#include "transport_catalogue.h"

#include <algorithm>
#include <unordered_set>

using namespace std;
using namespace transport_catalogue;

void TransportCatalogue::AddStop(TransportCatalogue::Stop& stop) {
    stops_.push_back(move(stop));
    stop_to_coords[stops_.back().stop_name] = &stops_.back();
    stop_to_buses[stops_.back().stop_name];
}

void TransportCatalogue::AddBus(const std::string& bus, const deque<std::string>& route) {
    TransportCatalogue::Bus bus_struct;
    double straight_dist = 0;
    bus_struct.bus_name = bus;
    bus_struct.all_stops = route.size();
    for (size_t i = 0; i < route.size() - 1; ++i) {
        straight_dist +=
                ComputeDistance(stop_to_coords[route[i]]->coordinates, stop_to_coords[route[i + 1]]->coordinates);
        bus_struct.route_length += GetDistance(route[i], route[i + 1]);
    }
    bus_struct.curvature = bus_struct.route_length / straight_dist;
    unordered_set<string_view> set_of_unique_stops;
    for (const auto& stop : route) {
        set_of_unique_stops.insert(stop);
        bus_struct.route.push_back(stop_to_coords[stop]->stop_name);
    }
    bus_struct.unique_stops = set_of_unique_stops.size();
    buses_.push_back(move(bus_struct));
    bus_to_description[buses_.back().bus_name] = &buses_.back();
    for (const auto& stop : route) {
        stop_to_buses[stop_to_coords[stop]->stop_name].insert(buses_.back().bus_name);
    }
}

void TransportCatalogue::AddDistance(const std::string& stop1, const std::string& stop2, int dist) {
    auto pair = make_pair(stop_to_coords[stop1], stop_to_coords[stop2]);
    dist_between_stops[pair] = dist;
}

const TransportCatalogue::Bus* TransportCatalogue::GetBus(const std::string& bus) {
    if (bus_to_description.count(bus)) {
        return bus_to_description[bus];
    } else {
        return nullptr;
    }
}

const TransportCatalogue::Stop* TransportCatalogue::GetStop(const std::string& stop) {
    if (stop_to_coords.count(stop)) {
        return stop_to_coords[stop];
    } else {
        return nullptr;
    }
}

const std::set<std::string_view>& TransportCatalogue::GetBusesForExistingStop(const std::string& stop) {
    return stop_to_buses[stop];
}

int TransportCatalogue::GetDistance(const std::string& stop1, const std::string& stop2) {
    auto pair = make_pair(stop_to_coords[stop1], stop_to_coords[stop2]);
    if (dist_between_stops.count(pair)) {
        return dist_between_stops[pair];
    } else {
        pair = make_pair(stop_to_coords[stop2], stop_to_coords[stop1]);
        return dist_between_stops[pair];
    }
}