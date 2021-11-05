#include "transport_router.h"

#include <algorithm>

void TransportRouter::SetSettings(RoutingSettings settings) {
    settings_ = settings;
}

std::optional<domain::EfficientRoute>
TransportRouter::ComputeEfficientRoute(const std::string_view &from, const std::string_view &to,
                                       domain::DataPack data) const {

    if (!data.stop_to_buses.count(from) || !data.stop_to_buses.count(to)) {
        return std::nullopt;
    }

    auto buses_from = data.stop_to_buses.at(from);
    auto buses_to = data.stop_to_buses.at(to);

    std::vector<std::string_view> buses_union;

    set_union(buses_from.begin(), buses_from.end(),
              buses_to.begin(), buses_to.end(),
              back_inserter(buses_union));

    std::unordered_set<std::string_view> vertecies;
    for (auto bus : buses_union) {
        for (auto stop : data.bus_to_description.at(bus)->route) {
            vertecies.insert(stop);
        }
    }

    graph::DirectedWeightedGraph<double> graph(vertecies.size());

    std::unordered_map<size_t, std::string_view> edge_id_to_bus;
    std::unordered_map<size_t, size_t> edge_id_to_span_count;
    std::unordered_map<std::string_view, size_t> stop_to_id;
    std::unordered_map<size_t, std::string_view> id_to_stop;

    size_t v_cntr = 0;

    for (auto bus : buses_union) {
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

    graph::Router<double> router(graph);
    auto route_info = router.BuildRoute(stop_to_id.at(from), stop_to_id.at(to));
    if (route_info) {
        domain::EfficientRoute result;
        result.total_time = route_info->weight;
        for (auto edge_id : route_info->edges) {
            domain::WaitItem wait_item;
            wait_item.time = settings_.bus_wait_time;
            wait_item.stop_name = id_to_stop[graph.GetEdge(edge_id).from];
            domain::BusItem bus_item;
            bus_item.time = graph.GetEdge(edge_id).weight - settings_.bus_wait_time;
            bus_item.bus = edge_id_to_bus[edge_id];
            bus_item.span_count = edge_id_to_span_count[edge_id];
            result.items.emplace_back(wait_item, bus_item);
        }
        return result;
    } else {
        return std::nullopt;
    }
}