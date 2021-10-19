#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

void ReadRequests(transport_catalogue::TransportCatalogue *t_cat, std::istream& in, std::ostream& out, renderer::MapRenderer& map_renderer);
