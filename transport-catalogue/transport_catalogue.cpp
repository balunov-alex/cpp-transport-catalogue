#include "transport_catalogue.h"

#include <algorithm>

namespace transport {

void TransportCatalogue::AddStop(const std::string& stop_name, const geo::Coordinates& stop_coordinates) {
    stops_.push_back({stop_name, stop_coordinates});
    stop_info_by_stop_name_[stops_.back().name] = &stops_.back();
    routes_through_stop_by_stop_name_[stops_.back().name];
}

void TransportCatalogue::AddDistance(std::string_view stop_from, std::string_view stop_to, int distance) {
    distances_between_stops_[{GetStop(stop_from), GetStop(stop_to)}] = distance;
    if (!distances_between_stops_.count({GetStop(stop_to), GetStop(stop_from)})) {
        distances_between_stops_[{GetStop(stop_to), GetStop(stop_from)}] = distance;
    }
}
    
void TransportCatalogue::AddRoute(const std::string& route_name, const std::vector<std::string>& route_stops, bool is_roundtrip) {
    routes_.push_back({route_name, route_stops, is_roundtrip});
    route_info_by_route_name_[routes_.back().name] = &routes_.back();
    for (const std::string& stop_name : route_stops) {
        routes_through_stop_by_stop_name_.at(stop_name).insert(routes_.back().name);
    }
}

const Stop* TransportCatalogue::GetStop(std::string_view stop_name) const {
    if (!stop_info_by_stop_name_.count(stop_name)) {
        return nullptr;
    }
    return stop_info_by_stop_name_.at(stop_name);
}

const Route* TransportCatalogue::GetRoute(std::string_view route_name) const {
    if (!route_info_by_route_name_.count(route_name)) {
        return nullptr;
    }
    return route_info_by_route_name_.at(route_name);
}
    
int TransportCatalogue::GetDistance(std::string_view stop_from, std::string_view stop_to) const {
    return distances_between_stops_.at({GetStop(stop_from), GetStop(stop_to)});
}    

TransportCatalogue::RouteInfo TransportCatalogue::GetRouteInfo(std::string_view route_name) const {
    if (!route_info_by_route_name_.count(route_name)) {
        return {0, 0, 0, 0.0};
    }
    Route route = *GetRoute(route_name);
    int stop_count = route.stops.size();
    if (!route.is_roundtrip) {
        stop_count = (stop_count * 2) - 1;
    }
    int unique_stop_count = std::unordered_set(route.stops.begin(), route.stops.end()).size();
    int real_route_length = CalculateRealRouteLength(*route_info_by_route_name_.at(route_name));
    double geo_route_length = CalculateGeoRouteLength(*route_info_by_route_name_.at(route_name));
    return {stop_count, unique_stop_count, real_route_length, real_route_length / geo_route_length};
}

const std::unordered_set<std::string_view>* TransportCatalogue::GetRoutesThroughStop(std::string_view stop_name) const {
    if (!routes_through_stop_by_stop_name_.count(stop_name)) {
        return nullptr;
    }
    return &routes_through_stop_by_stop_name_.at(stop_name);
}
    
const std::unordered_map<std::string_view, const Route*>& TransportCatalogue::GetAllRoutes() const {  
    return route_info_by_route_name_;
}

const std::unordered_map<std::string_view, const Stop*>& TransportCatalogue::GetAllStops() const {  
    return stop_info_by_stop_name_;
}

int TransportCatalogue::CalculateRealRouteLength(const Route& route) const {
    int route_length = 0;
    for (size_t i = 0; i < route.stops.size() - 1; ++i) {
        route_length += GetDistance(route.stops[i], route.stops[i+1]);
    }
    if (!route.is_roundtrip) {
        for (size_t i = 0; i < route.stops.size() - 1; ++i) {
            route_length += GetDistance(route.stops[i+1], route.stops[i]);
        }        
    }
    return route_length;
}
    
double TransportCatalogue::CalculateGeoRouteLength(const Route& route) const {
    std::vector<geo::Coordinates> stops_coords;
    stops_coords.reserve(route.stops.size());   
    for (const std::string& stop_name : route.stops) {
        stops_coords.push_back(GetStop(stop_name)->coordinates);
    } 
    double route_length = 0.0;
    for (size_t i = 0; i < stops_coords.size() - 1; ++i) {
        route_length += geo::ComputeDistance(stops_coords[i], stops_coords[i+1]);
    }
    if (!route.is_roundtrip) {
        route_length *= 2;        
    }    
    return route_length;
}
    
size_t TransportCatalogue::NearbyStopsHasher::operator()(std::pair<const Stop*, const Stop*> nearby_stops) const {
    return static_cast<size_t>(37*std::hash<const void*>()(nearby_stops.first) + 
                                  std::hash<const void*>()(nearby_stops.second));
}    
    
} // namespace transport
