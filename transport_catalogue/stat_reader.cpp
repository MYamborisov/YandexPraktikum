#include "stat_reader.h"

#include <iostream>
#include <iomanip>

using namespace std;

void transport_catalogue::requests::PrintBus(const string& bus, transport_catalogue::TransportCatalogue* t_cat) {
    auto Bus = t_cat->GetBus(bus);
    if (Bus != nullptr) {
        cout << "Bus " << Bus->bus_name << ": " << Bus->all_stops << " stops on route, "
             << Bus->unique_stops << " unique stops, "
             << Bus->route_length << " route length, "
             << setprecision(6) << Bus->curvature << " curvature" << endl;
    } else {
        cout << "Bus " << bus << ": not found" << endl;
    }
}

void transport_catalogue::requests::PrintStop(const string& stop, transport_catalogue::TransportCatalogue* t_cat) {
    auto Stop = t_cat->GetStop(stop);
    if (Stop != nullptr) {
        const std::set<std::string_view>& buses = t_cat->GetBusesForExistingStop(stop);
        if (buses.empty()) {
            cout << "Stop " << stop << ": no buses" << endl;
        } else {
            cout << "Stop " << stop << ": buses" ;
            for (auto bus : buses) {
                cout << " " << bus;
            }
            cout << endl;
        }
    } else {
        cout << "Stop " << stop << ": not found" << endl;
    }
}

void transport_catalogue::requests::ReadOutputRequests(int number_output, transport_catalogue::TransportCatalogue* t_cat) {
    for (int i = 0; i < number_output; ++i) {
        string entity_name, instance_name;
        cin >> entity_name;
        getline(cin, instance_name);
        transport_catalogue::details::DeleteSpaces(instance_name);
        if (entity_name == "Bus") {
            PrintBus(instance_name, t_cat);
        } else if (entity_name == "Stop") {
            PrintStop(instance_name, t_cat);
        }
    }
}
