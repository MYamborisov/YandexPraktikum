#pragma once

#include "transport_catalogue.h"

namespace transport_catalogue::details {

    void DeleteSpaces(std::string &line);

}

namespace transport_catalogue::input {

    std::deque<std::string> SplitBusLine(std::string &line);

    void SplitStopLine(std::string &line, std::deque<std::tuple<std::string, std::string, int>> &distances,
                       const std::string &name);

    void ReadStop(transport_catalogue::TransportCatalogue *t_cat, std::deque<std::tuple<std::string, std::string, int>>& distances, std::istream& in);

    void ReadBusLine(std::unordered_map<std::string, std::deque<std::string>> &buses, std::istream& in);

    void ReadInputRequests(int number_of_requests, transport_catalogue::TransportCatalogue *t_cat, std::istream& in);

}