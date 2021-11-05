#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"

using Distances = std::deque<std::tuple<std::string, std::string, int>>;

void ReadBaseRequests(json::Document& doc, transport_catalogue::TransportCatalogue *t_cat);

void ReadRenderSettings(json::Document& doc, renderer::MapRenderer& map_renderer);

std::unique_ptr<TransportRouter> ReadRoutingSettings(json::Document& doc, transport_catalogue::TransportCatalogue *t_cat);

json::Document ReadStatRequests(json::Document& doc, const RequestHandler& request_handler, std::unique_ptr<TransportRouter> router);

void ReadRequests(transport_catalogue::TransportCatalogue *t_cat, std::istream& in,
                  std::ostream& out, renderer::MapRenderer& map_renderer, const RequestHandler& request_handler);
