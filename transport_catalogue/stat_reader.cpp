#include "stat_reader.h"

#include <iostream>
#include <iomanip>

using namespace std;

void ReadOutputRequests(int number_output, TransportCatalogue* t_cat) {
    for (int i = 0; i < number_output; ++i) {
        string delimeter;
        cin >> delimeter;
        if (delimeter == "Bus") {
            string bus;
            getline(cin, bus);
            DeleteSpaces(bus);
            auto Bus = t_cat->GetBus(bus);
            if (Bus != nullptr) {
                cout << "Bus " << Bus->bus_name << ": " << Bus->all_stops << " stops on route, "
                     << Bus->unique_stops << " unique stops, "
                     << Bus->route_length << " route length, "
                     << setprecision(6) << Bus->curvature << " curvature" << endl;
            } else {
                cout << "Bus " << bus << ": not found" << endl;
            }
        } else if (delimeter == "Stop") {
            string stop;
            getline(cin, stop);
            DeleteSpaces(stop);
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
    }
}
