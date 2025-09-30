#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <optional>

class RequestHandler {
public:
    RequestHandler(const transport::TransportCatalogue& catalogue, const MapRenderer& renderer, transport::TransportRouter& router)
        : catalogue_(catalogue), renderer_(renderer), router_(router) {
    }
    
    std::optional<transport::TransportCatalogue::RouteInfo> GetBusStat(const std::string_view& bus_name) const;

    const std::unordered_set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;

    svg::Document RenderMap() const;
    
    void UpdateTransportRouterData();
    
    std::optional<transport::PathInfo> GetPathBetweenTwoStops(std::string_view stop_from, std::string_view stop_to) const;
    
private:
    const transport::TransportCatalogue& catalogue_;
    const MapRenderer& renderer_;
    transport::TransportRouter& router_;
};
