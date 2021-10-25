#include "json_reader.h"
#include "json_builder.h"

#include <sstream>

using namespace std::literals;

domain::Stop ReadStopInfo(Distances& distances, const json::Node request) {
    domain::Stop stop;
    stop.stop_name = request.AsDict().at("name").AsString();
    stop.coordinates.lat = request.AsDict().at("latitude").AsDouble();
    stop.coordinates.lng = request.AsDict().at("longitude").AsDouble();
    for (auto [to, dist] : request.AsDict().at("road_distances").AsDict()) {
        distances.emplace_back(stop.stop_name, to, dist.AsInt());
    }
    return stop;
}

std::deque<std::string> ReadBusInfo(const json::Node request) {
    std::deque<std::string> route;
    for (const auto& stop : request.AsDict().at("stops").AsArray()) {
        route.push_back(stop.AsString());
    }
    if (!request.AsDict().at("is_roundtrip"s).AsBool()) {
        for (int i = route.size() - 2; i >= 0; --i) {
            route.push_back(route[i]);
        }
    }
    return route;
}

void ReadBaseRequests(json::Document& doc, Distances& distances, transport_catalogue::TransportCatalogue *t_cat) {
    for (const auto& request : doc.GetRoot().AsDict().at("base_requests").AsArray()) {
        if (request.AsDict().at("type").AsString() == "Stop") {
            domain::Stop stop = std::move(ReadStopInfo(distances, request));
            t_cat->AddStop(stop);
        }
    }
    for (const auto& [stop_from, stop_to, dist] : distances) {
        t_cat->SetDistance(stop_from, stop_to, dist);
    }
    for (const auto& request : doc.GetRoot().AsDict().at("base_requests").AsArray()) {
        if (request.AsDict().at("type").AsString() == "Bus") {
            std::deque<std::string> route = std::move(ReadBusInfo(request));
            t_cat->AddBus(request.AsDict().at("name"s).AsString(), route, request.AsDict().at("is_roundtrip").AsBool());
        }
    }
}

void ReadRenderSettings(json::Document& doc, renderer::MapRenderer& map_renderer) {
    renderer::RenderSettings settings;
    auto settings_map = doc.GetRoot().AsDict().at("render_settings").AsDict();

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

void ReadStopRequest(json::Array& arr, const json::Node& request, RequestHandler request_handler) {
    json::Dict result;
    const std::string& stop = request.AsDict().at("name").AsString();
    if (auto buses = request_handler.GetBusesByStop(stop)) {
        json::Array arr_buses;
        for (auto bus : buses.value()) {
            arr_buses.emplace_back(static_cast<std::string>(bus));
        }
        result = json::Builder{}
                .StartDict()
                .Key("request_id").Value(request.AsDict().at("id").AsInt())
                .Key("buses").Value(move(arr_buses))
                .EndDict()
                .Build().AsDict();
    } else {
        result = json::Builder{}
                .StartDict()
                .Key("request_id").Value(request.AsDict().at("id").AsInt())
                .Key("error_message").Value("not found")
                .EndDict()
                .Build().AsDict();
    }
    arr.emplace_back(move(result));
}

void ReadBusRequest(json::Array& arr, const json::Node& request, RequestHandler request_handler) {
    json::Dict result;
    const std::string& bus = request.AsDict().at("name").AsString();
    if (auto bus_stat = request_handler.GetBusStat(bus)) {
        result = json::Builder{}
                .StartDict()
                .Key("request_id").Value(request.AsDict().at("id").AsInt())
                .Key("curvature").Value(bus_stat->curvature)
                .Key("route_length").Value(bus_stat->route_length)
                .Key("stop_count").Value(bus_stat->all_stops)
                .Key("unique_stop_count").Value(bus_stat->unique_stops)
                .EndDict()
                .Build().AsDict();
    } else {
        result = json::Builder{}
                .StartDict()
                .Key("request_id").Value(request.AsDict().at("id").AsInt())
                .Key("error_message").Value("not found")
                .EndDict()
                .Build().AsDict();
    }
    arr.emplace_back(move(result));
}

void ReadMapRequest(json::Array& arr, const json::Node& request, RequestHandler request_handler) {
    json::Dict result;
    std::stringstream ss;
    request_handler.RenderMap().Render(ss);
    result = json::Builder{}
            .StartDict()
            .Key("request_id").Value(request.AsDict().at("id").AsInt())
            .Key("map").Value(ss.str())
            .EndDict()
            .Build().AsDict();
    arr.emplace_back(move(result));
}

json::Document ReadStatRequests(json::Document& doc, RequestHandler request_handler) {
    json::Array arr;
    for (const auto& request : doc.GetRoot().AsDict().at("stat_requests").AsArray()) {
        if (request.AsDict().at("type").AsString() == "Stop") {
            ReadStopRequest(arr, request, request_handler);
        } else if (request.AsDict().at("type").AsString() == "Bus") {
            ReadBusRequest(arr, request, request_handler);
        } else if (request.AsDict().at("type").AsString() == "Map") {
            ReadMapRequest(arr, request, request_handler);
        }

    }
    return json::Document(json::Node(arr));
}

void ReadRequests(transport_catalogue::TransportCatalogue *t_cat, std::istream& in,
                  std::ostream& out, renderer::MapRenderer& map_renderer, RequestHandler request_handler) {

    Distances distances;
    json::Document doc = json::Load(in);

    ReadBaseRequests(doc, distances, t_cat);
    ReadRenderSettings(doc,map_renderer);
    json::Document doc_out = ReadStatRequests(doc, request_handler);

    json::Print(doc_out, out);
}

