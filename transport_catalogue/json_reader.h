#pragma once

#include "json.h"
#include "transport_catalogue.h"

void ReadRequests(transport_catalogue::TransportCatalogue *t_cat, std::istream& in, std::ostream& out);
