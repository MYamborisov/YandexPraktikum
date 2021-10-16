#include "stat_reader.h"

#include <iostream>
#include <iomanip>

void transport_catalogue::requests::PrintBus(const std::string& bus, transport_catalogue::TransportCatalogue* t_cat, std::ostream& out) {
    auto Bus = t_cat->GetBusStatistics(bus);
    if (Bus != nullptr) {
        out << "Bus " << bus << ": " << Bus->all_stops << " stops on route, "
             << Bus->unique_stops << " unique stops, "
             << Bus->route_length << " route length, "
             << std::setprecision(6) << Bus->curvature << " curvature" << std::endl;
    } else {
        out << "Bus " << bus << ": not found" << std::endl;
    }
}

void transport_catalogue::requests::PrintStop(const std::string& stop, transport_catalogue::TransportCatalogue* t_cat, std::ostream& out) {
    auto Stop = t_cat->GetStop(stop);
    if (Stop != nullptr) {
        const std::set<std::string_view>& buses = t_cat->GetBusesForExistingStop(stop);
        if (buses.empty()) {
            out << "Stop " << stop << ": no buses" << std::endl;
        } else {
            out << "Stop " << stop << ": buses" ;
            for (auto bus : buses) {
                out << " " << bus;
            }
            out << std::endl;
        }
    } else {
        out << "Stop " << stop << ": not found" << std::endl;
    }
}

void transport_catalogue::requests::ReadOutputRequests(int number_output, transport_catalogue::TransportCatalogue* t_cat, std::istream& in, std::ostream& out) {
    for (int i = 0; i < number_output; ++i) {
        std::string entity_name, instance_name;
        in >> entity_name;
        getline(in, instance_name);
        transport_catalogue::details::DeleteSpaces(instance_name);
        if (entity_name == "Bus") {
            PrintBus(instance_name, t_cat, out);
        } else if (entity_name == "Stop") {
            PrintStop(instance_name, t_cat, out);
        }
    }
}
