#include "transport_router.h"

#include <cmath>
#include <stdexcept>

namespace transport {
    
void TransportRouter::SetSettings(RoutingSettings routing_settings) {
    routing_settings_ = routing_settings;
}

void TransportRouter::UploadTransportData(const transport::TransportCatalogue& ctlg) {
    graph_data_ = std::move(GraphAndItsTransportData<double>{graph::DirectedWeightedGraph<double>(ctlg.GetAllStops().size())});
    AddVertexIdsInGraphData(ctlg.GetAllStops());
    for (const auto [route_name, route_ptr] : ctlg.GetAllRoutes()) {
        const auto& vec_stops = route_ptr->stops;
        if (route_ptr->is_roundtrip) {
            AddRouteInGraph(ctlg, vec_stops.begin(), vec_stops.size(), route_name);
        } else {
            AddRouteInGraph(ctlg, vec_stops.begin(), vec_stops.size(), route_name);
            AddRouteInGraph(ctlg, vec_stops.rbegin(), vec_stops.size(), route_name);            
        }
    }
    router_ = std::make_unique<graph::Router<double>>(graph_data_.graph);
}

std::optional<PathInfo> TransportRouter::BuildPath(std::string_view stop_from, std::string_view stop_to) const {
    if (!router_) {
        return std::nullopt;
    }        
    const auto route_info = router_->BuildRoute(graph_data_.vertex_id_by_stop_name.at(stop_from), 
                                                graph_data_.vertex_id_by_stop_name.at(stop_to));
    if (!route_info) {
        return std::nullopt;   
    } else {
        std::vector<EdgeInfo> items;
        for (graph::EdgeId edge_id : route_info->edges) {
            items.push_back(graph_data_.edge_info_by_edge_id.at(edge_id));
        }
        return PathInfo{items, routing_settings_.bus_wait_time, route_info->weight};
    }
}

void TransportRouter::AddVertexIdsInGraphData(const std::unordered_map<std::string_view, const Stop*>& all_stops) {
    size_t index_number_of_stop = 0;
    for (const auto [stop_name, stop_ptr] : all_stops) {
        graph_data_.vertex_id_by_stop_name[stop_ptr->name] = index_number_of_stop;
        ++index_number_of_stop;
    }
}
    
} // namespace transport
