#include "json_reader.h"

#include <sstream>

using namespace std::literals;

void ReadBaseRequests(json::Document& doc, Distances& distances, transport_catalogue::TransportCatalogue *t_cat) {
    for (const auto& request : doc.GetRoot().AsMap().at("base_requests").AsArray()) {
        if (request.AsMap().at("type").AsString() == "Stop") {
            domain::Stop stop;
            stop.stop_name = request.AsMap().at("name").AsString();
            stop.coordinates.lat = request.AsMap().at("latitude").AsDouble();
            stop.coordinates.lng = request.AsMap().at("longitude").AsDouble();
            for (auto [to, dist] : request.AsMap().at("road_distances").AsMap()) {
                distances.emplace_back(stop.stop_name, to, dist.AsInt());
            }
            t_cat->AddStop(stop);
        }
    }
    for (const auto& [stop1, stop2, dist] : distances) {
        t_cat->SetDistance(stop1, stop2, dist);
    }
    for (const auto& request : doc.GetRoot().AsMap().at("base_requests").AsArray()) {
        if (request.AsMap().at("type").AsString() == "Bus") {
            std::deque<std::string> deq;
            for (const auto& stop : request.AsMap().at("stops").AsArray()) {
                deq.push_back(stop.AsString());
            }
            if (!request.AsMap().at("is_roundtrip"s).AsBool()) {
                for (int i = deq.size() - 2; i >= 0; --i) {
                    deq.push_back(deq[i]);
                }
            }
            t_cat->AddBus(request.AsMap().at("name"s).AsString(), deq, request.AsMap().at("is_roundtrip").AsBool());
        }
    }
}

void ReadRenderSettings(json::Document& doc, renderer::MapRenderer& map_renderer) {
    renderer::RenderSettings settings;
    auto settings_map = doc.GetRoot().AsMap().at("render_settings").AsMap();

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
    } else if(settings_map.at("underlayer_color"s).AsArray().size() == 3) {
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
        } else if (color.AsArray().size() == 3) {
            settings.color_palette.emplace_back(svg::Rgb(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt()));
        } else {
            settings.color_palette.emplace_back(svg::Rgba(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt(), color.AsArray()[3].AsDouble()));
        }
    }

    map_renderer.SetSettings(std::move(settings));
}

json::Document ReadStatRequests(json::Document& doc, RequestHandler request_handler) {
    json::Array arr;
    for (const auto& request : doc.GetRoot().AsMap().at("stat_requests").AsArray()) {
        json::Dict dict;
        dict["request_id"] = request.AsMap().at("id").AsInt();
        if (request.AsMap().at("type").AsString() == "Stop") {
            const std::string& stop = request.AsMap().at("name").AsString();
            if (auto buses = request_handler.GetBusesByStop(stop)) {
                json::Array arr_buses;
                for (auto bus : buses.value()) {
                    arr_buses.emplace_back(static_cast<std::string>(bus));
                }
                dict["buses"] = move(arr_buses);
            } else {
                dict["error_message"] = "not found"s;
            }
        } else if (request.AsMap().at("type").AsString() == "Bus") {
            const std::string& bus = request.AsMap().at("name").AsString();
            if (auto Bus = request_handler.GetBusStat(bus)) {
                dict["curvature"] = Bus->curvature;
                dict["route_length"] = Bus->route_length;
                dict["stop_count"] = Bus->all_stops;
                dict["unique_stop_count"] = Bus->unique_stops;
            } else {
                dict["error_message"] = "not found"s;
            }
        } else if (request.AsMap().at("type").AsString() == "Map") {
            std::stringstream ss;
            request_handler.RenderMap().Render(ss);
            dict["map"] = ss.str();
        }
        arr.push_back(move(dict));
    }
    return {arr};
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

