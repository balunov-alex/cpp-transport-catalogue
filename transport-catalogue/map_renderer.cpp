#include "map_renderer.h"

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}
    
void MapRenderer::SetSettings(RenderSettings settings) {
    settings_ = std::move(settings);
}
    
void MapRenderer::AddAllRoutesLines(const std::map<std::string_view, InfoForRenderRoute>& route_render_info_by_route_name, 
                                          svg::Document& document) const {
    size_t color_index = 0;
    size_t number_of_colors = settings_.color_palette.size();
    for (const auto& [route_name, route_render_info] : route_render_info_by_route_name) {
        svg::Polyline route_line;
        if (!route_render_info.coords_of_stops.empty()) {
            for (const auto coords_of_stop: route_render_info.coords_of_stops) {
                route_line.AddPoint(coords_of_stop);
            }
            route_line.SetFillColor(svg::NoneColor)
                      .SetStrokeColor(settings_.color_palette[color_index])
                      .SetStrokeWidth(settings_.line_width)
                      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            
            ++color_index;
            if (color_index == number_of_colors) {
                color_index = 0;
            }
            document.Add(route_line);
        }           
    }
}

void MapRenderer::AddAllRoutesTexts(const std::map<std::string_view, InfoForRenderRoute>& route_render_info_by_route_name,
                                          svg::Document& document) const {
    size_t color_index = 0;
    size_t number_of_colors = settings_.color_palette.size();
    for (const auto& [route_name, route_render_info] : route_render_info_by_route_name) {
        const auto& stops_coords = route_render_info.coords_of_stops;
        svg::Text route_name_text;
        svg::Text text_background;
        if (!route_render_info.coords_of_stops.empty()) {
            text_background.SetPosition(stops_coords[0])
                           .SetOffset(settings_.bus_label_offset)
                           .SetFontSize(settings_.bus_label_font_size)
                           .SetFontFamily("Verdana")
                           .SetFontWeight("bold")
                           .SetData(std::string(route_name))
                           .SetFillColor(settings_.underlayer_color)
                           .SetStrokeColor(settings_.underlayer_color)
                           .SetStrokeWidth(settings_.underlayer_width)
                           .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                           .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                
            route_name_text.SetPosition(stops_coords[0])
                           .SetOffset(settings_.bus_label_offset)
                           .SetFontSize(settings_.bus_label_font_size)
                           .SetFontFamily("Verdana")
                           .SetFontWeight("bold")
                           .SetData(std::string(route_name))
                           .SetFillColor(settings_.color_palette[color_index]);
            
            ++color_index;
            if (color_index == number_of_colors) {
                color_index = 0;
            }
            document.Add(text_background);
            document.Add(route_name_text);
            
            const int index_of_median_stop = stops_coords.size() / 2;
            if (!route_render_info.is_roundtrip && stops_coords[0] != stops_coords[index_of_median_stop]) {
                document.Add(text_background.SetPosition(stops_coords[index_of_median_stop]));
                document.Add(route_name_text.SetPosition(stops_coords[index_of_median_stop]));
            }         
        }      
    }
}

void MapRenderer::AddAllStopsPoints(const std::map<std::string_view, svg::Point>& coords_of_stop_in_route_by_stop_name,
                                          svg::Document& document) const {
    for (const auto [stop_name, stop_coords] : coords_of_stop_in_route_by_stop_name) {
        document.Add(svg::Circle().SetCenter(stop_coords).SetRadius(settings_.stop_radius).SetFillColor("white"));
    }
}

void MapRenderer::AddAllStopsTexts(const std::map<std::string_view, svg::Point>& coords_of_stop_in_route_by_stop_name,
                                         svg::Document& document) const {
    for (const auto [stop_name, stop_coords] : coords_of_stop_in_route_by_stop_name) {
        svg::Text stop_name_text;
        svg::Text text_background;
        text_background.SetPosition(stop_coords)
                       .SetOffset(settings_.stop_label_offset)
                       .SetFontSize(settings_.stop_label_font_size)
                       .SetFontFamily("Verdana")
                       .SetData(std::string(stop_name))
                       .SetFillColor(settings_.underlayer_color)
                       .SetStrokeColor(settings_.underlayer_color)
                       .SetStrokeWidth(settings_.underlayer_width)
                       .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                       .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                
        stop_name_text.SetPosition(stop_coords)
                      .SetOffset(settings_.stop_label_offset)
                      .SetFontSize(settings_.stop_label_font_size)
                      .SetFontFamily("Verdana")
                      .SetData(std::string(stop_name))
                      .SetFillColor("black");
        
        document.Add(text_background);
        document.Add(stop_name_text);
    }
}

svg::Document MapRenderer::MakeSvgDocument(const std::unordered_map<std::string_view, const transport::Stop*>& all_stops,
                                           const std::unordered_map<std::string_view, const transport::Route*>& all_routes) const {
    std::vector<geo::Coordinates> coords_of_all_stops_in_routs;
    for (const auto [stop_name, stop_info] : all_stops) {
        for (const auto [route_name, route_info] : all_routes) {
            if (std::count(route_info->stops.begin(), route_info->stops.end(), stop_name)) {
                coords_of_all_stops_in_routs.push_back(stop_info->coordinates);
                break;
            }
        }    
    }
    const SphereProjector proj_{coords_of_all_stops_in_routs.begin(), 
                                coords_of_all_stops_in_routs.end(), settings_.width, settings_.height, settings_.padding};
    
    
    std::map<std::string_view, InfoForRenderRoute> route_render_info_by_route_name;
    for (const auto [route_name, route_info] : all_routes) {
        std::vector<svg::Point> all_stops_coords_in_route;
        if (!route_info->is_roundtrip) {
            all_stops_coords_in_route.reserve(route_info->stops.size() * 2 - 1);
        } else {
            all_stops_coords_in_route.reserve(route_info->stops.size());
        }    
        for (const auto& stop_name : route_info->stops) {
            all_stops_coords_in_route.push_back(proj_(all_stops.at(stop_name)->coordinates));    
        }
        if (!route_info->is_roundtrip) {
            all_stops_coords_in_route.insert(all_stops_coords_in_route.end(), 
                                             std::next(all_stops_coords_in_route.rbegin()), all_stops_coords_in_route.rend());
        }       
        route_render_info_by_route_name[route_name] = {all_stops_coords_in_route, route_info->is_roundtrip};
    }
    
    std::map<std::string_view, svg::Point> coords_of_stop_in_route_by_stop_name;
    for (const auto [stop_name, stop_info] : all_stops) {
        for (const auto [route_name, route_info] : all_routes) {
            if (std::count(route_info->stops.begin(), route_info->stops.end(), stop_name)) {
                coords_of_stop_in_route_by_stop_name[stop_name] = proj_(stop_info->coordinates);
                break;
            }
        }
    }
            
    svg::Document all_objects;
    MapRenderer::AddAllRoutesLines(route_render_info_by_route_name, all_objects);
    MapRenderer::AddAllRoutesTexts(route_render_info_by_route_name, all_objects);
    MapRenderer::AddAllStopsPoints(coords_of_stop_in_route_by_stop_name, all_objects);
    MapRenderer::AddAllStopsTexts(coords_of_stop_in_route_by_stop_name, all_objects);
    return all_objects;
}
