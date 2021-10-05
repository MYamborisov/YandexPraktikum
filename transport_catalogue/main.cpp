#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>

using namespace std;

int main() {
    TransportCatalogue cat;
    int number_input, number_output;
    cin >> number_input;
    ReadInputRequests(number_input, &cat);
    cin >> number_output;
    ReadOutputRequests(number_output, &cat);
    return 0;
}
