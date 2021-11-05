#pragma once

#include "router.h"
#include "domain.h"

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0;
};

class TransportRouter {
public:
    void SetSettings(RoutingSettings settings);
    std::optional<domain::EfficientRoute> ComputeEfficientRoute(const std::string_view& from, const std::string_view& to, domain::DataPack data) const;
private:
    RoutingSettings settings_;
};