#pragma once

#include "router.h"
#include "domain.h"

#include <memory>

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0;
};

class TransportRouter {
public:
    TransportRouter(RoutingSettings settings, domain::DataPack data);
    void SetSettings(RoutingSettings settings);
    std::optional<domain::EfficientRoute> ComputeEfficientRoute(const std::string_view& from, const std::string_view& to) const;
private:
    RoutingSettings settings_;
    std::unordered_map<size_t, std::string_view> edge_id_to_bus;
    std::unordered_map<size_t, size_t> edge_id_to_span_count;
    std::unordered_map<std::string_view, size_t> stop_to_id;
    std::unordered_map<size_t, std::string_view> id_to_stop;
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    domain::DataPack data_;
};