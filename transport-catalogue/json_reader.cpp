#include "json_builder.h"
#include "json_reader.h"

#include <set>
#include <sstream>

using namespace std::literals;

void JsonReader::FillCatalogue(transport::TransportCatalogue& catalogue) const {
    const auto& base_requests_array = requests_doc_.GetRoot().AsDict().at("base_requests"s).AsArray();
    FillCatalogueWithStops(base_requests_array, catalogue);
    FillCatalogueWithDistances(base_requests_array, catalogue);
    FillCatalogueWithRoutes(base_requests_array, catalogue);
}

void JsonReader::FillRenderer(MapRenderer& renderer) const {
    const auto& render_settings_map = requests_doc_.GetRoot().AsDict().at("render_settings"s).AsDict();
    renderer.SetSettings({render_settings_map.at("width"s).AsDouble(),
                          render_settings_map.at("height"s).AsDouble(),
                          render_settings_map.at("padding"s).AsDouble(),
                          render_settings_map.at("line_width"s).AsDouble(),
                          render_settings_map.at("stop_radius"s).AsDouble(),
                          render_settings_map.at("bus_label_font_size"s).AsInt(),
                          {render_settings_map.at("bus_label_offset"s).AsArray()[0].AsDouble(),
                           render_settings_map.at("bus_label_offset"s).AsArray()[1].AsDouble()},
                          render_settings_map.at("stop_label_font_size"s).AsInt(),
                          {render_settings_map.at("stop_label_offset"s).AsArray()[0].AsDouble(),
                           render_settings_map.at("stop_label_offset"s).AsArray()[1].AsDouble()},
                          ReadColorFromJson(render_settings_map.at("underlayer_color"s)),
                          render_settings_map.at("underlayer_width"s).AsDouble(),
                          ReadArrayColorFromJson(render_settings_map.at("color_palette"s).AsArray())});
}

void JsonReader::FillTransportRouter(transport::TransportRouter& transport_router) const {
    const auto& routing_settings_map = requests_doc_.GetRoot().AsDict().at("routing_settings"s).AsDict();
    transport_router.SetSettings({routing_settings_map.at("bus_velocity"s).AsDouble(),
                                  routing_settings_map.at("bus_wait_time"s).AsInt()});
}

void JsonReader::PrintRequestsResults(const RequestHandler& handler, std::ostream& out) const {
    json::Array result;
    const auto& stat_requests_array = requests_doc_.GetRoot().AsDict().at("stat_requests"s).AsArray();
    for (const auto& stat_request : stat_requests_array) {
        const auto& stat_request_map = stat_request.AsDict();
        if (stat_request_map.at("type"s).AsString() == "Bus"s) {
            result.push_back(GetRouteRequestResult(stat_request_map.at("name"s).AsString(), 
                                                   stat_request_map.at("id"s).AsInt(), handler));
        }
        if (stat_request_map.at("type"s).AsString() == "Stop"s) {
            result.push_back(GetStopRequestResult(stat_request_map.at("name"s).AsString(), 
                                                  stat_request_map.at("id"s).AsInt(), handler));
        }
        if (stat_request_map.at("type"s).AsString() == "Map"s) {
            result.push_back(GetMapRequestResult(stat_request_map.at("id"s).AsInt(), handler));
        }
        if (stat_request_map.at("type"s).AsString() == "Route"s) {
            result.push_back(GetPathRequestResult(stat_request_map.at("from"s).AsString(),
                                                  stat_request_map.at("to"s).AsString(),
                                                  stat_request_map.at("id"s).AsInt(), handler));
        }        
    }
    json::Print(json::Document{result}, out);
}

const json::Document& JsonReader::GetDocument() const {
    return requests_doc_;
}

void JsonReader::FillCatalogueWithStops(const json::Array& base_requests, 
                                              transport::TransportCatalogue& catalogue) const {
    for (const auto& base_request : base_requests) {
        const auto& base_request_map = base_request.AsDict();
        if (base_request_map.at("type"s).AsString() == "Stop"s) {
            catalogue.AddStop(base_request_map.at("name"s).AsString(),
                             {base_request_map.at("latitude"s).AsDouble(), 
                              base_request_map.at("longitude"s).AsDouble()});
        }
    }    
}

void JsonReader::FillCatalogueWithDistances(const json::Array& base_requests, 
                                                  transport::TransportCatalogue& catalogue) const {
    for (const auto& base_request : base_requests) {
        const auto& base_request_map = base_request.AsDict();
        if (base_request_map.at("type"s).AsString() == "Stop"s) {
            for (const auto& [stop_to, distance] : base_request_map.at("road_distances"s).AsDict()) {
                catalogue.AddDistance(base_request_map.at("name"s).AsString(), stop_to, distance.AsInt());    
            }
        }
    }    
}

void JsonReader::FillCatalogueWithRoutes(const json::Array& base_requests, 
                                               transport::TransportCatalogue& catalogue) const {
    for (const auto& base_request : base_requests) {        
        const auto& base_request_map = base_request.AsDict();                
        if (base_request_map.at("type"s).AsString() == "Bus"s) {
            std::vector<std::string> stops_in_route;
            for (const auto& stop : base_request_map.at("stops"s).AsArray()) {
                stops_in_route.push_back(stop.AsString());
            }        
            catalogue.AddRoute(base_request_map.at("name"s).AsString(), stops_in_route, 
                               base_request_map.at("is_roundtrip"s).AsBool());    
        }            
    }    
}

svg::Color JsonReader::ReadColorFromJson(json::Node color_node) const {
    svg::Color color;
    if (color_node.IsString()) {
        color = std::move(color_node.AsString());
    } else if (color_node.IsArray()) {
        const json::Array& color_numeric_format = std::move(color_node.AsArray());
        if (color_numeric_format.size() == 3) {
            color = svg::Rgb{static_cast<uint8_t>(color_numeric_format[0].AsInt()),
                             static_cast<uint8_t>(color_numeric_format[1].AsInt()),
                             static_cast<uint8_t>(color_numeric_format[2].AsInt())};
        } else if (color_numeric_format.size() == 4) {
            color = svg::Rgba{static_cast<uint8_t>(color_numeric_format[0].AsInt()),
                              static_cast<uint8_t>(color_numeric_format[1].AsInt()),
                              static_cast<uint8_t>(color_numeric_format[2].AsInt()),
                              color_numeric_format[3].AsDouble()};            
        }
    }
    return color;
}

std::vector<svg::Color> JsonReader::ReadArrayColorFromJson(std::vector<json::Node> color_nodes) const {
    std::vector<svg::Color> colors;
    for (const json::Node& color_node : color_nodes) {
        colors.emplace_back(JsonReader::ReadColorFromJson(color_node));
    }
    return colors;
}

json::Node JsonReader::GetRouteRequestResult(std::string_view bus_name, int request_id, 
                                             const RequestHandler& handler) const {
    if (!handler.GetBusStat(bus_name)) {
        return json::Builder{}.StartDict()
                                  .Key("request_id"s).Value(request_id)
                                  .Key("error_message"s).Value("not found"s)
                              .EndDict()
                              .Build();
    }
    const auto route_info = *handler.GetBusStat(bus_name);
    return json::Builder{}.StartDict()
                              .Key("request_id"s).Value(request_id)
                              .Key("stop_count"s).Value(route_info.number_of_stops)
                              .Key("unique_stop_count"s).Value(route_info.number_of_unique_stops)
                              .Key("route_length"s).Value(route_info.length)
                              .Key("curvature"s).Value(route_info.curvature)
                          .EndDict()
                          .Build();      
}

json::Node JsonReader::GetStopRequestResult(std::string_view stop_name, int request_id, 
                                            const RequestHandler& handler) const {
    const auto routes_un_set_ptr = handler.GetBusesByStop(stop_name);
    if (!routes_un_set_ptr) {
        return json::Builder{}.StartDict()
                                  .Key("request_id"s).Value(request_id)
                                  .Key("error_message"s).Value("not found"s)
                              .EndDict()
                              .Build();
    }
    std::set<std::string> routes_set(routes_un_set_ptr->begin(), routes_un_set_ptr->end());
    json::Array routes_vec;
    for (const auto& route : routes_set) {
        routes_vec.push_back(route);
    }
    return json::Builder{}.StartDict()
                              .Key("request_id"s).Value(request_id)
                              .Key("buses"s).Value(routes_vec)
                          .EndDict()
                          .Build();
}

json::Node JsonReader::GetMapRequestResult(int request_id, const RequestHandler& handler) const {
    std::ostringstream oss;
    handler.RenderMap().Render(oss);
    return json::Builder{}.StartDict()
                              .Key("request_id"s).Value(request_id)
                              .Key("map"s).Value(oss.str())
                          .EndDict()
                          .Build();
}

json::Node JsonReader::GetPathRequestResult(std::string_view stop_from, std::string_view stop_to, 
                                            int request_id, const RequestHandler& handler) const {
    if (!handler.GetPathBetweenTwoStops(stop_from, stop_to)) {
        return json::Builder{}.StartDict()
                                  .Key("request_id"s).Value(request_id)
                                  .Key("error_message"s).Value("not found"s)
                              .EndDict()
                              .Build();
    }
    const auto path_info = *handler.GetPathBetweenTwoStops(stop_from, stop_to);
    json::Array items;
    for (auto& item : path_info.items) {
        items.emplace_back(json::Builder{}.StartDict()
                                              .Key("type"s).Value("Wait"s)
                                              .Key("stop_name"s).Value(std::string(item.start_stop))
                                              .Key("time"s).Value(path_info.bus_wait_time)
                                          .EndDict()
                                          .Build());
        items.emplace_back(json::Builder{}.StartDict()
                                              .Key("type"s).Value("Bus"s)
                                              .Key("bus"s).Value(std::string(item.bus_name))
                                              .Key("span_count"s).Value(item.span_count)
                                              .Key("time"s).Value(item.weight)
                                          .EndDict()
                                          .Build());        
    }
    return json::Builder{}.StartDict()
                              .Key("request_id"s).Value(request_id)
                              .Key("total_time"s).Value(path_info.total_time)
                              .Key("items"s).Value(items)
                          .EndDict()
                          .Build();    
}
