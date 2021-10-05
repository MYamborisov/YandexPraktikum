#include "input_reader.h"

#include <sstream>
#include <iostream>

using namespace std;

void DeleteSpaces(string& line) {
    int i = 0;
    while (line[i] == ' ') {
        ++i;
    }
    line.erase(0, i);
    if (line[line.size() - 1] == ' ') {
        i = line.size() - 1;
        while (line[i] == ' ') {
            --i;
        }
        line.erase(i + 1, line.size() - i - 1);
    }
}

std::deque<std::string> SplitBusLine(std::string& line) {
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
        DeleteSpaces(stop);
        result.push_back(stop);
    }
    if (!is_circle) {
        for (int i = result.size() - 2; i >= 0; --i) {
            result.push_back(result[i]);
        }
    }
    return result;
}

void SplitStopLine(std::string& line, std::deque<tuple<string, string, int>>& distances, const string& name) {
    string stop, m, to;
    stringstream ss(line);
    int dist;
    while(ss >> dist >> m >> to) {
        getline(ss, stop, ',');
        DeleteSpaces(stop);
        distances.emplace_back(name, stop, dist);
    }
}

void ReadStop(TransportCatalogue* t_cat, std::deque<tuple<string, string, int>>& distances) {
    TransportCatalogue::Stop stop;
    getline(cin, stop.stop_name, ':');
    DeleteSpaces(stop.stop_name);
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

void ReadBusLine(std::unordered_map<std::string, std::deque<std::string>> & buses) {
    string bus;
    string line;
    getline(cin, bus, ':');
    DeleteSpaces(bus);
    getline(cin, line);
    buses[move(bus)] = SplitBusLine(line);;
}

void ReadInputRequests(int number_of_requests, TransportCatalogue* t_cat) {

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