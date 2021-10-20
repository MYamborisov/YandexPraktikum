#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

using Distances = std::deque<std::tuple<std::string, std::string, int>>;

void ReadBaseRequests(json::Document& doc, Distances& distances, transport_catalogue::TransportCatalogue *t_cat);

void ReadRenderSettings(json::Document& doc, renderer::MapRenderer& map_renderer);

json::Document ReadStatRequests(json::Document& doc, RequestHandler request_handler);

void ReadRequests(transport_catalogue::TransportCatalogue *t_cat, std::istream& in,
                  std::ostream& out, renderer::MapRenderer& map_renderer, RequestHandler request_handler);
