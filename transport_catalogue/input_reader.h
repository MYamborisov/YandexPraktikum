#pragma once

#include "transport_catalogue.h"

void DeleteSpaces(std::string& line);

std::deque<std::string> SplitBusLine(std::string& line);

void SplitStopLine(std::string& line, std::deque<std::tuple<std::string, std::string, int>>& distances, const std::string& name);

void ReadStop(TransportCatalogue* t_cat);

void ReadBusLine(std::unordered_map<std::string, std::deque<std::string>> & buses);

void ReadInputRequests(int number_of_requests, TransportCatalogue* t_cat);