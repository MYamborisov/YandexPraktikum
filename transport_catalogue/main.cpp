#include "transport_catalogue.h"
#include "json_reader.h"

#include <iostream>

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue cat;
    ReadRequests(&cat, cin, cout);
    return 0;
}
