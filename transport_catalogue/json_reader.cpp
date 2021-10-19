#include "json_reader.h"

#include <sstream>

using Distances = std::deque<std::tuple<std::string, std::string, int>>;

using namespace std::literals;

void ReadRequests(transport_catalogue::TransportCatalogue *t_cat, std::istream& in, std::ostream& out, renderer::MapRenderer& map_renderer) {

    Distances distances;
    json::Document doc = json::Load(in);

    for (const auto& request : doc.GetRoot().AsMap().at("base_requests").AsArray()) {
        if (request.AsMap().at("type").AsString() == "Stop") {
            domain::Stop stop;
            stop.stop_name = request.AsMap().at("name").AsString();
            stop.coordinates.lat = request.AsMap().at("latitude").AsDouble();
            stop.coordinates.lng = request.AsMap().at("longitude").AsDouble();
            for (auto [to, dist] : request.AsMap().at("road_distances").AsMap()) {
                distances.emplace_back(stop.stop_name, to, dist.AsDouble());
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

    {
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
        map_renderer.BuildMap(*t_cat);
    }

    json::Array arr;
    for (const auto& request : doc.GetRoot().AsMap().at("stat_requests").AsArray()) {
        json::Dict dict;
        dict["request_id"] = request.AsMap().at("id").AsInt();
        if (request.AsMap().at("type").AsString() == "Stop") {
            const std::string& stop = request.AsMap().at("name").AsString();
            auto Stop = t_cat->GetStop(stop);
            if (Stop != nullptr) {
                const std::set<std::string_view>& buses = t_cat->GetBusesForExistingStop(stop);
                json::Array arr_buses;
                for (auto bus : buses) {
                    arr_buses.emplace_back(static_cast<std::string>(bus));
                }
                dict["buses"] = move(arr_buses);
            } else {
                dict["error_message"] = "not found"s;
            }
        } else if (request.AsMap().at("type").AsString() == "Bus") {
            const std::string& bus = request.AsMap().at("name").AsString();
            auto Bus = t_cat->GetBusStatistics(bus);
            if (Bus != nullptr) {
                dict["curvature"] = Bus->curvature;
                dict["route_length"] = Bus->route_length;
                dict["stop_count"] = Bus->all_stops;
                dict["unique_stop_count"] = Bus->unique_stops;
            } else {
                dict["error_message"] = "not found"s;
            }
        } else if (request.AsMap().at("type").AsString() == "Map") {
            std::stringstream ss;
            map_renderer.Render().Render(ss);
            dict["map"] = ss.str();
        }
        arr.push_back(move(dict));
    }
    json::Document doc_out(move(arr));
    json::Print(doc_out, out);
}

