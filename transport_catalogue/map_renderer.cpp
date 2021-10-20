#include "map_renderer.h"

#include <utility>

namespace renderer {

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    void MapRenderer::SetSettings(RenderSettings settings) {
        settings_ = std::move(settings);
    }

    RenderData MapRenderer::PreProcessData(const domain::DataPack& raw_data) const {
        RenderData result;
        for (const auto& [stop, buses] : raw_data.stop_to_buses) {
            if (!buses.empty()) {
                result.stops.push_back(stop);
                result.points.push_back(raw_data.stop_to_coords.at(stop)->coordinates);
            }
        }
        for (const auto& [bus, descr] : raw_data.bus_to_description) {
            if (!descr->route.empty()) {
                result.buses.push_back(bus);
            }
        }
        sort(result.buses.begin(), result.buses.end());
        sort(result.stops.begin(), result.stops.end());
        return result;
    }

    void MapRenderer::RenderRouteLines(const SphereProjector& projector, svg::Document& document,
                                       const domain::DataPack& raw_data, const RenderData& data) const {
        for (size_t i = 0; i < data.buses.size(); ++i) {
            svg::Polyline route_line;
            route_line.SetFillColor(svg::NoneColor)
                    .SetStrokeWidth(settings_.line_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetStrokeColor(settings_.color_palette[i % settings_.color_palette.size()]);
            for (const auto& stop : raw_data.bus_to_description.at(data.buses[i])->route) {
                route_line.AddPoint(projector(raw_data.stop_to_coords.at(stop)->coordinates));
            }
            document.Add(route_line);
        }
    }

    void MapRenderer::RenderBusNames(const SphereProjector& projector, svg::Document& document,
                                     const domain::DataPack& raw_data, const RenderData& data) const {
        for (size_t i = 0; i < data.buses.size(); ++i) {
            svg::Text background;
            background.SetPosition(projector(raw_data.stop_to_coords.at(raw_data.bus_to_description.at(data.buses[i])->route[0])->coordinates))
                    .SetOffset({settings_.bus_label_offset.first, settings_.bus_label_offset.second})
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetData(static_cast<std::string>(data.buses[i]));
            svg::Text text = background;
            background.SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            text.SetFillColor(settings_.color_palette[i % settings_.color_palette.size()]);
            document.Add(background);
            document.Add(text);
            if (!raw_data.bus_to_statistics.at(data.buses[i])->is_roundtrip &&
                    raw_data.bus_to_description.at(data.buses[i])->route[0] !=
                    raw_data.bus_to_description.at(data.buses[i])->route[raw_data.bus_to_description.at(data.buses[i])->route.size() / 2]) {
                auto stop = raw_data.bus_to_description.at(data.buses[i])->route[raw_data.bus_to_description.at(data.buses[i])->route.size() / 2];
                background.SetPosition(projector(raw_data.stop_to_coords.at(stop)->coordinates));
                text.SetPosition(projector(raw_data.stop_to_coords.at(stop)->coordinates));
                document.Add(background);
                document.Add(text);
            }
        }
    }

    void MapRenderer::RenderStopSymbols(const SphereProjector &projector, svg::Document &document,
                                        const domain::DataPack &raw_data, const RenderData &data) const {
        for (const auto& stop : data.stops) {
            document.Add(svg::Circle().SetCenter(projector(raw_data.stop_to_coords.at(stop)->coordinates))
                                  .SetRadius(settings_.stop_radius)
                                  .SetFillColor("white"));
        }
    }

    void MapRenderer::RenderStopNames(const SphereProjector &projector, svg::Document &document,
                                      const domain::DataPack &raw_data, const RenderData &data) const {
        for (const auto& stop : data.stops) {
            svg::Text background;
            background.SetPosition(projector(raw_data.stop_to_coords.at(stop)->coordinates))
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
            document.Add(background);
            document.Add(text);
        }
    }

    svg::Document MapRenderer::Render(domain::DataPack raw_data) const {

        RenderData data = PreProcessData(raw_data);
        SphereProjector projector(data.points.begin(), data.points.end(), settings_.width, settings_.height, settings_.padding);

        svg::Document document;

        RenderRouteLines(projector, document, raw_data, data);
        RenderBusNames(projector, document, raw_data, data);
        RenderStopSymbols(projector, document, raw_data, data);
        RenderStopNames(projector, document, raw_data, data);

        return document;
    }
}