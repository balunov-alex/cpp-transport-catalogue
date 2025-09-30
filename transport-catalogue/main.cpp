#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <sstream>

int main () {
    transport::TransportCatalogue ctlg;
    MapRenderer renderer;
    transport::TransportRouter router;
    
    JsonReader reader(std::cin);
    
    reader.FillCatalogue(ctlg);
    reader.FillRenderer(renderer);
    reader.FillTransportRouter(router);
    
    RequestHandler handler(ctlg, renderer, router);
    
    handler.UpdateTransportRouterData();
    
    reader.PrintRequestsResults(handler, std::cout);
}
