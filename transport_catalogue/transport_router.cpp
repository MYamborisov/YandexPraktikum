#include "transport_router.h"

#include <algorithm>

void TransportRouter::SetSettings(RoutingSettings settings) {
    settings_ = settings;
}

TransportRouter::TransportRouter(RoutingSettings settings, domain::DataPack data) {
    settings_ = settings;

    data_ = data;

    std::unordered_set<std::string_view> vertecies;
    for (auto [bus, stat] : data.bus_to_statistics) {
        for (auto stop : data.bus_to_description.at(bus)->route) {
            vertecies.insert(stop);
        }
    }

    graph::DirectedWeightedGraph<double> graph(vertecies.size());

    size_t v_cntr = 0;

    for (auto [bus, stat] : data.bus_to_statistics) {
        auto route = data.bus_to_description.at(bus)->route;
        std::vector<double> prefix_sum(route.size());
        prefix_sum[0] = 0;
        double sum = 0;
        for (int i = 1; i < prefix_sum.size(); ++i) {
            double dist;
            auto p_from = data.stop_to_coords.at(route[i-1]);
            auto p_to = data.stop_to_coords.at(route[i]);
            if (data.dist_between_stops.count(std::make_pair(p_from, p_to))) {
                dist = data.dist_between_stops.at(std::make_pair(p_from, p_to));
            } else {
                dist = data.dist_between_stops.at(std::make_pair(p_to, p_from));
            }
            sum += dist / settings_.bus_velocity * 60 / 1000;
            prefix_sum[i] = sum;
        }
        for (int i = 0; i < route.size() - 1; ++i) {
            for (int j = i + 1; j < route.size(); ++j) {
                if (!stop_to_id.count(route[i])) {
                    id_to_stop[v_cntr] = route[i];
                    stop_to_id[route[i]] = v_cntr++;
                }
                if (!stop_to_id.count(route[j])) {
                    id_to_stop[v_cntr] = route[j];
                    stop_to_id[route[j]] = v_cntr++;
                }
                graph::Edge<double> edge;
                edge.from = stop_to_id.at(route[i]);
                edge.to = stop_to_id.at(route[j]);
                edge.weight = prefix_sum[j] - prefix_sum[i] + settings_.bus_wait_time;
                size_t edge_id = graph.AddEdge(edge);
                edge_id_to_bus[edge_id] = bus;
                edge_id_to_span_count[edge_id] = j - i;
            }
        }
    }
    graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(graph);
    router_ = std::make_unique<graph::Router<double>>(*graph_);
}

std::optional<domain::EfficientRoute>
TransportRouter::ComputeEfficientRoute(const std::string_view &from, const std::string_view &to) const {

    if (!data_.stop_to_buses.count(from) || !data_.stop_to_buses.count(to) || data_.stop_to_buses.at(from).empty() || data_.stop_to_buses.at(to).empty()) {
        return std::nullopt;
    }

    auto route_info = router_->BuildRoute(stop_to_id.at(from), stop_to_id.at(to));
    if (route_info) {
        domain::EfficientRoute result;
        result.total_time = route_info->weight;
        for (auto edge_id : route_info->edges) {
            domain::WaitItem wait_item;
            wait_item.time = settings_.bus_wait_time;
            wait_item.stop_name = id_to_stop.at(graph_->GetEdge(edge_id).from);
            domain::BusItem bus_item;
            bus_item.time = graph_->GetEdge(edge_id).weight - settings_.bus_wait_time;
            bus_item.bus = edge_id_to_bus.at(edge_id);
            bus_item.span_count = edge_id_to_span_count.at(edge_id);
            result.items.emplace_back(wait_item, bus_item);
        }
        return result;
    } else {
        return std::nullopt;
    }
}