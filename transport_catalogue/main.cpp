#include "json_reader.h"

#include <iostream>

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue db;
    renderer::MapRenderer map_renderer;
    TransportRouter router;
    RequestHandler request_handler(db, map_renderer, router);
    ReadRequests(&db, cin, cout, map_renderer, request_handler, router);
    return 0;
}
