#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

namespace transport {

struct RoutingSettings {
    double bus_velocity = 0.0;
    int bus_wait_time = 0;
};

struct EdgeInfo {
    double weight = 0.0;
    std::string_view bus_name;
    int span_count = 0;
    std::string_view start_stop;
    std::string_view finish_stop;
};
    
template <typename Weight>    
struct GraphAndItsTransportData {
    graph::DirectedWeightedGraph<Weight> graph;
    std::unordered_map<std::string_view, graph::VertexId> vertex_id_by_stop_name = {};
    std::unordered_map<graph::EdgeId, EdgeInfo> edge_info_by_edge_id = {};
};

struct PathInfo {
    std::vector<EdgeInfo> items;
    int bus_wait_time = 0;
    double total_time = 0;
};

class TransportRouter {   
public:
    void SetSettings(RoutingSettings routing_settings);
    void UploadTransportData(const transport::TransportCatalogue& catalogue);
    std::optional<PathInfo> BuildPath(std::string_view stop_from, std::string_view stop_to) const;    
 
private:
    void AddVertexIdsInGraphData(const std::unordered_map<std::string_view, const Stop*>& all_stops);

    template <typename RandomIt>
    void AddRouteInGraph(const transport::TransportCatalogue& ctlg, RandomIt vec_stops_start_it, size_t vec_stops_size, std::string_view route_name) {
        const int meters_in_km = 1000;
        const int seconds_in_min = 60;
        for (size_t index_stop_from = 0; index_stop_from < vec_stops_size; ++index_stop_from) { 
            uint32_t total_distance = 0;
            for (size_t index_stop_to = index_stop_from + 1; index_stop_to < vec_stops_size; ++index_stop_to) {
                auto pos_stop_from = vec_stops_start_it + index_stop_from;
                auto pos_stop_before_to = vec_stops_start_it + index_stop_to - 1;
                auto pos_stop_to = vec_stops_start_it + index_stop_to;

                const graph::VertexId id_stop_from = graph_data_.vertex_id_by_stop_name[*pos_stop_from];
                const graph::VertexId id_stop_to = graph_data_.vertex_id_by_stop_name[*pos_stop_to];                
                total_distance += ctlg.GetDistance(*pos_stop_before_to, *pos_stop_to); 

                const double weight = ((total_distance * seconds_in_min) / 
                                      (meters_in_km * routing_settings_.bus_velocity)) + routing_settings_.bus_wait_time;
                graph_data_.graph.AddEdge({id_stop_from, id_stop_to, weight});

                const int span_count = index_stop_to - index_stop_from;
                graph_data_.edge_info_by_edge_id[graph_data_.graph.GetEdgeCount() - 1] = 
                {weight - routing_settings_.bus_wait_time, route_name, span_count, *pos_stop_from, *pos_stop_to};
            } 
        }  
    }    

    RoutingSettings routing_settings_;
    GraphAndItsTransportData<double> graph_data_;
    std::unique_ptr<graph::Router<double>> router_;          
};
    
} // namespace transport
