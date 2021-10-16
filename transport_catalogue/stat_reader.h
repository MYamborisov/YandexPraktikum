#pragma once

#include "input_reader.h"

namespace transport_catalogue::requests {

    void PrintBus(const std::string& bus, transport_catalogue::TransportCatalogue *t_cat, std::ostream& out);

    void PrintStop(const std::string& stop, transport_catalogue::TransportCatalogue *t_cat, std::ostream& out);

    void ReadOutputRequests(int number_output, transport_catalogue::TransportCatalogue *t_cat, std::istream& in, std::ostream& out);

}