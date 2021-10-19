#include "map_renderer.h"

#include <utility>

namespace renderer {

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    void MapRenderer::SetSettings(RenderSettings settings) {
        settings_ = std::move(settings);
    }

    const svg::Document& MapRenderer::Render() const {
        return document_;
    }

    void MapRenderer::BuildMap(const transport_catalogue::TransportCatalogue &db) {
        std::vector<std::string_view> bus_names = db.GetBusNames();
        sort(bus_names.begin(), bus_names.end());
        std::vector<std::string_view> stop_names = db.GetStops();
        sort(stop_names.begin(), stop_names.end());
        auto bus_to_descr = db.GetBusToDescription();
        auto stop_to_coords = db.GetStopToCoords();
        auto bus_to_statistics = db.GetBusToStatistics();
        auto stop_to_buses = db.GetStopToBuses();
        int cntr = 0;
        int palette_size = settings_.color_palette.size();
        std::vector<geo::Coordinates> points = db.GetCoordsOfStopsWithBuses();
        SphereProjector projector(points.begin(), points.end(), settings_.width, settings_.height, settings_.padding);

        //lines

        for (const auto& bus : bus_names) {
            if (!bus_to_descr[bus]->route.empty()) {
                svg::Polyline route_line;
                route_line.SetFillColor(svg::NoneColor)
                          .SetStrokeWidth(settings_.line_width)
                          .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                          .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                          .SetStrokeColor(settings_.color_palette[cntr % palette_size]);
                ++cntr;
                for (const auto& stop : bus_to_descr[bus]->route) {
                    route_line.AddPoint(projector(stop_to_coords[stop]->coordinates));
                }
                document_.Add(route_line);
            }
        }
        cntr = 0;
        //names

        for (const auto& bus : bus_names) {
            if (!bus_to_descr[bus]->route.empty()) {
                svg::Text background;
                background.SetPosition(projector(stop_to_coords[bus_to_descr[bus]->route[0]]->coordinates))
                          .SetOffset({settings_.bus_label_offset.first, settings_.bus_label_offset.second})
                          .SetFontSize(settings_.bus_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetFontWeight("bold")
                          .SetData(static_cast<std::string>(bus));
                svg::Text text = background;
                background.SetFillColor(settings_.underlayer_color)
                          .SetStrokeColor(settings_.underlayer_color)
                          .SetStrokeWidth(settings_.underlayer_width)
                          .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                          .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                text.SetFillColor(settings_.color_palette[cntr % palette_size]);
                ++cntr;
                document_.Add(background);
                document_.Add(text);
                if (!bus_to_statistics[bus]->is_roundtrip && bus_to_descr[bus]->route[0] != bus_to_descr[bus]->route[bus_to_descr[bus]->route.size() / 2]) {
                    auto stop = bus_to_descr[bus]->route[bus_to_descr[bus]->route.size() / 2];
                    background.SetPosition(projector(stop_to_coords[stop]->coordinates));
                    text.SetPosition(projector(stop_to_coords[stop]->coordinates));
                    document_.Add(background);
                    document_.Add(text);
                }
            }
        }

        //stop symbols

        for (const auto& stop : stop_names) {
            if (!stop_to_buses[stop].empty()) {
                document_.Add(svg::Circle().SetCenter(projector(stop_to_coords[stop]->coordinates))
                         .SetRadius(settings_.stop_radius)
                         .SetFillColor("white"));
            }
        }

        //stop names

        for (const auto& stop : stop_names) {
            if (!stop_to_buses[stop].empty()) {
                svg::Text background;
                background.SetPosition(projector(stop_to_coords[stop]->coordinates))
                        .SetOffset({settings_.stop_label_offset.first, settings_.stop_label_offset.second})
                        .SetFontSize(settings_.stop_label_font_size)
                        .SetFontFamily("Verdana")
                        .SetData(static_cast<std::string>(stop));
                svg::Text text = background;
                background.SetFillColor(settings_.underlayer_color)
                        .SetStrokeColor(settings_.underlayer_color)
                        .SetStrokeWidth(settings_.underlayer_width)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                text.SetFillColor("black");
                document_.Add(background);
                document_.Add(text);
            }
        }

    }
}