#include "json_reader.h"

using Distances = std::deque<std::tuple<std::string, std::string, int>>;

using namespace std::literals;

void ReadRequests(transport_catalogue::TransportCatalogue *t_cat, std::istream& in, std::ostream& out) {

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
            if (!request.AsMap().at("is_roundtrip").AsBool()) {
                for (int i = deq.size() - 2; i >= 0; --i) {
                    deq.push_back(deq[i]);
                }
            }
            t_cat->AddBus(request.AsMap().at("name").AsString(), deq);
        }
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
        }
        arr.push_back(move(dict));
    }
    json::Document doc_out(move(arr));
    json::Print(doc_out, out);
}

