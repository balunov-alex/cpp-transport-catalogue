#include "request_handler.h"

using namespace std::literals;

std::optional<transport::TransportCatalogue::RouteInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    const auto bus_stat = catalogue_.GetRouteInfo(bus_name);
    if (bus_stat.number_of_stops == 0) {
        return std::nullopt;
    }
    return bus_stat;
}

const std::unordered_set<std::string_view>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return catalogue_.GetRoutesThroughStop(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.MakeSvgDocument(catalogue_.GetAllStops(), catalogue_.GetAllRoutes());
}

void RequestHandler::UpdateTransportRouterData() {
    router_.UploadTransportData(catalogue_);
}

std::optional<transport::PathInfo> RequestHandler::GetPathBetweenTwoStops(std::string_view stop_from, 
                                                                          std::string_view stop_to) const {
    return router_.BuildPath(stop_from, stop_to);
}
