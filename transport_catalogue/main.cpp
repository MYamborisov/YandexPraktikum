#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue cat;
    int number_input, number_output;
    cin >> number_input;
    transport_catalogue::input::ReadInputRequests(number_input, &cat, cin);
    cin >> number_output;
    transport_catalogue::requests::ReadOutputRequests(number_output, &cat, cin, cout);
    return 0;
}
