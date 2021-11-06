#include "json_reader.h"

#include <iostream>
#include <fstream>

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue db;
    renderer::MapRenderer map_renderer;
    TransportRouter transport_router;
    RequestHandler request_handler(db, map_renderer, transport_router);
    ReadRequests(&db, cin, cout, map_renderer, transport_router, request_handler);
    return 0;
}
