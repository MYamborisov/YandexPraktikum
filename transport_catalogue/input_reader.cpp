#include "input_reader.h"

#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

void transport_catalogue::details::DeleteSpaces(string& line) {
    size_t first = line.find_first_not_of(" ");
    size_t last = line.find_last_not_of(" ");
    if (last != string::npos) {
        line = move(line.substr(first, last - first + 1));
    } else {
        line = "";
    }
}

std::deque<std::string> transport_catalogue::input::SplitBusLine(std::string& line) {
    deque<string> result;
    bool is_circle;
    char delim;
    if (find(line.begin(), line.end(), '>') != line.end()) {
        delim = '>';
        is_circle = true;
    } else  {
        delim = '-';
        is_circle = false;
    }
    string stop;
    stringstream ss(line);
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

void transport_catalogue::input::SplitStopLine(std::string& line, std::deque<tuple<string, string, int>>& distances, const string& name) {
    string stop, m, to;
    stringstream ss(line);
    int dist;
    while(ss >> dist >> m >> to) {
        getline(ss, stop, ',');
        transport_catalogue::details::DeleteSpaces(stop);
        distances.emplace_back(name, stop, dist);
    }
}

void transport_catalogue::input::ReadStop(transport_catalogue::TransportCatalogue* t_cat, std::deque<tuple<string, string, int>>& distances) {
    transport_catalogue::TransportCatalogue::Stop stop;
    getline(cin, stop.stop_name, ':');
    transport_catalogue::details::DeleteSpaces(stop.stop_name);
    double lat, lng;
    char comma;
    cin >> lat >> comma >> lng;
    string line;
    getline(cin, line);
    if (!line.empty()) {
        line.erase(0, 1);
        SplitStopLine(line, distances, stop.stop_name);
    }
    stop.coordinates = {lat, lng};
    t_cat->AddStop(stop);
}

void transport_catalogue::input::ReadBusLine(std::unordered_map<std::string, std::deque<std::string>> & buses) {
    string bus;
    string line;
    getline(cin, bus, ':');
    transport_catalogue::details::DeleteSpaces(bus);
    getline(cin, line);
    buses[move(bus)] = SplitBusLine(line);;
}

void transport_catalogue::input::ReadInputRequests(int number_of_requests, TransportCatalogue* t_cat) {

    std::unordered_map<std::string, std::deque<std::string>> buses;
    std::deque<tuple<string, string, int>> distances;

    for (int i = 0; i < number_of_requests; ++i) {
        string first_word;
        cin >> first_word;
        if (first_word == "Stop") {
            ReadStop(t_cat, distances);
        } else if (first_word == "Bus") {
            ReadBusLine(buses);
        }
    }
    for (const auto& [stop1, stop2, dist] : distances) {
        t_cat->AddDistance(stop1, stop2, dist);
    }
    for (const auto& [bus, deq] : buses) {
        t_cat->AddBus(bus, deq);
    }
}