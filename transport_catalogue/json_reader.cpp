#include "json_reader.h"
#include "json_builder.h"

#include <sstream>

using namespace std::literals;

domain::Stop ReadStopInfo(Distances& distances, const json::Node request) {
    domain::Stop stop;
    stop.stop_name = request.AsDict().at("name"s).AsString();
    stop.coordinates.lat = request.AsDict().at("latitude"s).AsDouble();
    stop.coordinates.lng = request.AsDict().at("longitude"s).AsDouble();
    for (auto [to, dist] : request.AsDict().at("road_distances"s).AsDict()) {
        distances.emplace_back(stop.stop_name, to, dist.AsInt());
    }
    return stop;
}

std::deque<std::string> ReadBusInfo(const json::Node request) {
    std::deque<std::string> route;
    for (const auto& stop : request.AsDict().at("stops"s).AsArray()) {
        route.push_back(stop.AsString());
    }
    if (!request.AsDict().at("is_roundtrip"s).AsBool()) {
        for (int i = route.size() - 2; i >= 0; --i) {
            route.push_back(route[i]);
        }
    }
    return route;
}

void ReadBaseRequests(json::Document& doc, transport_catalogue::TransportCatalogue *t_cat) {
    Distances distances;
    for (const auto& request : doc.GetRoot().AsDict().at("base_requests"s).AsArray()) {
        if (request.AsDict().at("type"s).AsString() == "Stop"s) {
            domain::Stop stop = std::move(ReadStopInfo(distances, request));
            t_cat->AddStop(stop);
        }
    }
    for (const auto& [stop_from, stop_to, dist] : distances) {
        t_cat->SetDistance(stop_from, stop_to, dist);
    }
    for (const auto& request : doc.GetRoot().AsDict().at("base_requests"s).AsArray()) {
        if (request.AsDict().at("type"s).AsString() == "Bus"s) {
            std::deque<std::string> route = std::move(ReadBusInfo(request));
            t_cat->AddBus(request.AsDict().at("name"s).AsString(), route, request.AsDict().at("is_roundtrip"s).AsBool());
        }
    }
}

void ReadRenderSettings(json::Document& doc, renderer::MapRenderer& map_renderer) {
    renderer::RenderSettings settings;
    auto settings_map = doc.GetRoot().AsDict().at("render_settings"s).AsDict();

    settings.width = settings_map.at("width"s).AsDouble();
    settings.height = settings_map.at("height"s).AsDouble();
    settings.padding = settings_map.at("padding"s).AsDouble();
    settings.line_width = settings_map.at("line_width"s).AsDouble();
    settings.stop_radius = settings_map.at("stop_radius"s).AsDouble();
    settings.bus_label_font_size = settings_map.at("bus_label_font_size"s).AsInt();
    settings.bus_label_offset = {settings_map.at("bus_label_offset"s).AsArray()[0].AsDouble(),
                                 settings_map.at("bus_label_offset"s).AsArray()[1].AsDouble()};
    settings.stop_label_font_size = settings_map.at("stop_label_font_size"s).AsInt();
    settings.stop_label_offset = {settings_map.at("stop_label_offset"s).AsArray()[0].AsDouble(),
                                  settings_map.at("stop_label_offset"s).AsArray()[1].AsDouble()};
    if (settings_map.at("underlayer_color"s).IsString()) {
        settings.underlayer_color = settings_map.at("underlayer_color"s).AsString();
    } else if(settings_map.at("underlayer_color"s).AsArray().size() == 3u) {
        auto arr = settings_map.at("underlayer_color"s).AsArray();
        settings.underlayer_color = svg::Rgb(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
    } else {
        auto arr = settings_map.at("underlayer_color"s).AsArray();
        settings.underlayer_color = svg::Rgba(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
    }
    settings.underlayer_width = settings_map.at("underlayer_width"s).AsDouble();
    for (const auto& color : settings_map.at("color_palette"s).AsArray()) {
        if (color.IsString()) {
            settings.color_palette.emplace_back(color.AsString());
        } else if (color.AsArray().size() == 3u) {
            settings.color_palette.emplace_back(svg::Rgb(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt()));
        } else {
            settings.color_palette.emplace_back(svg::Rgba(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt(), color.AsArray()[3].AsDouble()));
        }
    }

    map_renderer.SetSettings(std::move(settings));
}

void ReadRoutingSettings(json::Document& doc, transport_catalogue::TransportCatalogue *t_cat, TransportRouter& router) {
    RoutingSettings settings;
    auto route_settings = doc.GetRoot().AsDict().at("routing_settings"s).AsDict();
    settings.bus_velocity = route_settings.at("bus_velocity"s).AsDouble();
    settings.bus_wait_time = route_settings.at("bus_wait_time"s).AsInt();
    router.SetSettings(settings);
    router.ComputeGraph(t_cat->GetDataPack());
}

void ReadStopRequest(json::Array& arr, const json::Node& request, const RequestHandler& request_handler) {
    json::Dict result;
    const std::string& stop = request.AsDict().at("name"s).AsString();
    if (auto buses = request_handler.GetBusesByStop(stop)) {
        json::Array arr_buses;
        for (auto bus : buses.value()) {
            arr_buses.emplace_back(static_cast<std::string>(bus));
        }
        result = json::Builder{}
                .StartDict()
                .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                .Key("buses"s).Value(move(arr_buses))
                .EndDict()
                .Build().AsDict();
    } else {
        result = json::Builder{}
                .StartDict()
                .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                .Key("error_message"s).Value("not found"s)
                .EndDict()
                .Build().AsDict();
    }
    arr.emplace_back(move(result));
}

void ReadBusRequest(json::Array& arr, const json::Node& request, const RequestHandler& request_handler) {
    json::Dict result;
    const std::string& bus = request.AsDict().at("name"s).AsString();
    if (auto bus_stat = request_handler.GetBusStat(bus)) {
        result = json::Builder{}
                .StartDict()
                .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                .Key("curvature"s).Value(bus_stat->curvature)
                .Key("route_length"s).Value(bus_stat->route_length)
                .Key("stop_count"s).Value(bus_stat->all_stops)
                .Key("unique_stop_count"s).Value(bus_stat->unique_stops)
                .EndDict()
                .Build().AsDict();
    } else {
        result = json::Builder{}
                .StartDict()
                .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                .Key("error_message"s).Value("not found"s)
                .EndDict()
                .Build().AsDict();
    }
    arr.emplace_back(move(result));
}

void ReadMapRequest(json::Array& arr, const json::Node& request, const RequestHandler& request_handler) {
    json::Dict result;
    std::stringstream ss;
    request_handler.RenderMap().Render(ss);
    result = json::Builder{}
            .StartDict()
            .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
            .Key("map"s).Value(ss.str())
            .EndDict()
            .Build().AsDict();
    arr.emplace_back(move(result));
}

void ReadRouteRequest(json::Array& arr, const json::Node& request,const RequestHandler& request_handler) {
    json::Dict result;
    const std::string& from = request.AsDict().at("from"s).AsString();
    const std::string& to = request.AsDict().at("to"s).AsString();
    if (auto eff_route = request_handler.ComputeEfficientRoute(from, to)) {
        json::Array items;
        for (const auto& item : eff_route->items) {
            json::Dict wait_item_dict = json::Builder{}
                        .StartDict()
                        .Key("type"s).Value("Wait"s)
                        .Key("stop_name"s).Value(item.first.stop_name)
                        .Key("time"s).Value(item.first.time)
                        .EndDict()
                        .Build().AsDict();
            items.emplace_back(move(wait_item_dict));
            json::Dict bus_item_dict = json::Builder{}
                        .StartDict()
                        .Key("type"s).Value("Bus"s)
                        .Key("bus"s).Value(item.second.bus)
                        .Key("span_count"s).Value(item.second.span_count)
                        .Key("time"s).Value(item.second.time)
                        .EndDict()
                        .Build().AsDict();
            items.emplace_back(move(bus_item_dict));
        }
        result = json::Builder{}
                .StartDict()
                .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                .Key("total_time"s).Value(eff_route->total_time)
                .Key("items"s).Value(move(items))
                .EndDict()
                .Build().AsDict();
    } else {
        result = json::Builder{}
                .StartDict()
                .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                .Key("error_message"s).Value("not found"s)
                .EndDict()
                .Build().AsDict();
    }
    arr.emplace_back(move(result));
}

json::Document ReadStatRequests(json::Document& doc, const RequestHandler& request_handler) {
    json::Array arr;
    for (const auto& request : doc.GetRoot().AsDict().at("stat_requests"s).AsArray()) {
        if (request.AsDict().at("type"s).AsString() == "Stop"s) {
            ReadStopRequest(arr, request, request_handler);
        } else if (request.AsDict().at("type"s).AsString() == "Bus"s) {
            ReadBusRequest(arr, request, request_handler);
        } else if (request.AsDict().at("type"s).AsString() == "Map"s) {
            ReadMapRequest(arr, request, request_handler);
        } else if (request.AsDict().at("type"s).AsString() == "Route"s) {
            ReadRouteRequest(arr, request, request_handler);
        }
    }
    return json::Document(json::Node(arr));
}

void ReadRequests(transport_catalogue::TransportCatalogue *t_cat, std::istream& in,
                  std::ostream& out, renderer::MapRenderer& map_renderer, TransportRouter& router, const RequestHandler& request_handler) {

    json::Document doc = json::Load(in);

    ReadBaseRequests(doc, t_cat);
    ReadRenderSettings(doc,map_renderer);
    ReadRoutingSettings(doc, t_cat, router);
    json::Document doc_out = ReadStatRequests(doc, request_handler);

    json::Print(doc_out, out);
}

