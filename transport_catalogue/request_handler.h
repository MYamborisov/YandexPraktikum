#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <optional>

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer, const TransportRouter& router);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<domain::BusStatistics> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    std::optional<std::set<std::string_view>> GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

    std::optional<domain::EfficientRoute> ComputeEfficientRoute(const std::string_view& from, const std::string_view& to) const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const TransportRouter& router_;
};
