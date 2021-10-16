#include "input_reader.h"

#include <sstream>
#include <iostream>
#include <algorithm>

using Distances = std::deque<std::tuple<std::string, std::string, int>>;
using Buses = std::unordered_map<std::string, std::deque<std::string>>;

void transport_catalogue::details::DeleteSpaces(std::string& line) {
    size_t first = line.find_first_not_of(" ");
    size_t last = line.find_last_not_of(" ");
    if (last != std::string::npos) {
        line = move(line.substr(first, last - first + 1));
    } else {
        line = "";
    }
}

std::deque<std::string> transport_catalogue::input::SplitBusLine(std::string& line) {
    std::deque<std::string> result;
    bool is_circle = false;
    char delim = '-';
    if (find(line.begin(), line.end(), '>') != line.end()) {
        delim = '>';
        is_circle = true;
    }
    std::string stop;
    std::stringstream ss(line);
    while(getline(ss, stop, delim)) {
        transport_catalogue::details::DeleteSpaces(stop);
        result.push_back(stop);
    }
    if (!is_circle) {
        for (int i = result.size() - 2; i >= 0; --i) {
            result.push_back(result[i]);
        }
    }
    return result;
}

void transport_catalogue::input::SplitStopLine(std::string& line, Distances& distances, const std::string& name) {
    std::string stop, m, to;
    std::stringstream ss(line);
    int dist;
    while(ss >> dist >> m >> to) {
        getline(ss, stop, ',');
        transport_catalogue::details::DeleteSpaces(stop);
        distances.emplace_back(name, stop, dist);
    }
}

void transport_catalogue::input::ReadStop(transport_catalogue::TransportCatalogue* t_cat, Distances& distances, std::istream& in) {
    transport_catalogue::TransportCatalogue::Stop stop;
    getline(in, stop.stop_name, ':');
    transport_catalogue::details::DeleteSpaces(stop.stop_name);
    double lat, lng;
    char comma;
    in >> lat >> comma >> lng;
    std::string line;
    getline(in, line);
    if (!line.empty()) {
        line.erase(0, 1);
        SplitStopLine(line, distances, stop.stop_name);
    }
    stop.coordinates = {lat, lng};
    t_cat->AddStop(stop);
}

void transport_catalogue::input::ReadBusLine(Buses & buses, std::istream& in) {
    std::string bus;
    std::string line;
    getline(in, bus, ':');
    transport_catalogue::details::DeleteSpaces(bus);
    getline(in, line);
    buses[move(bus)] = SplitBusLine(line);
}

void transport_catalogue::input::ReadInputRequests(int number_of_requests, TransportCatalogue* t_cat, std::istream& in) {

    Buses buses;
    Distances distances;

    for (int i = 0; i < number_of_requests; ++i) {
        std::string first_word;
        in >> first_word;
        if (first_word == "Stop") {
            ReadStop(t_cat, distances, in);
        } else if (first_word == "Bus") {
            ReadBusLine(buses, in);
        }
    }
    for (const auto& [stop1, stop2, dist] : distances) {
        t_cat->SetDistance(stop1, stop2, dist);
    }
    for (const auto& [bus, deq] : buses) {
        t_cat->AddBus(bus, deq);
    }
}