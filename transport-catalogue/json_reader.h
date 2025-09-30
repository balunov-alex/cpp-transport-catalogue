#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"

class JsonReader {
public:
    JsonReader(std::istream& input)
        : requests_doc_(json::Load(input)) {
    }
    
    void FillCatalogue(transport::TransportCatalogue& catalogue) const;

    void FillRenderer(MapRenderer& renderer) const;
    
    void FillTransportRouter(transport::TransportRouter& transport_router) const;
    
    void PrintRequestsResults(const RequestHandler& handler, std::ostream& out) const;
    
    const json::Document& GetDocument() const;

private:
    void FillCatalogueWithStops(const json::Array& base_requests, 
                                transport::TransportCatalogue& catalogue) const;
    void FillCatalogueWithDistances(const json::Array& base_requests, 
                                    transport::TransportCatalogue& catalogue) const;
    void FillCatalogueWithRoutes(const json::Array& base_requests, 
                                 transport::TransportCatalogue& catalogue) const;
        
    svg::Color ReadColorFromJson(json::Node color) const;
    std::vector<svg::Color> ReadArrayColorFromJson(std::vector<json::Node> colors) const;
    
    json::Node GetRouteRequestResult(std::string_view bus_name, int request_id, 
                                     const RequestHandler& handler) const;
    json::Node GetStopRequestResult(std::string_view stop_name, int request_id, 
                                    const RequestHandler& handler) const;
    json::Node GetMapRequestResult(int request_id, const RequestHandler& handler) const;
    
    json::Node GetPathRequestResult(std::string_view stop_from, std::string_view stop_to, 
                                    int request_id, const RequestHandler& handler) const;
    
    json::Document requests_doc_;
};
