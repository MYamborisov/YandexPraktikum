#include "request_handler.h"

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue &db,
                               const renderer::MapRenderer &renderer) : db_(db), renderer_(renderer) {}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.Render(db_.GetDataPack());
}

std::optional<domain::BusStatistics> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    if (db_.GetBusStatistics(bus_name)) {
        return *db_.GetBusStatistics(bus_name);
    }
    return std::nullopt;
}

std::optional<std::set<std::string_view>> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return db_.GetBusesByStop(stop_name);
}
