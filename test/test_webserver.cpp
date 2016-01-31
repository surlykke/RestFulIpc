#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "service.h"
#include "jsonresource.h"
#include "webserver.h"

int main(int argc, char** argv)
{
    std::cout << "test_webserver\n";
    using namespace org_restfulipc;
    Service service(7938);
    //WebServer webServer("/home/christian/NetBeansProjects/GenericRipcClient/public_html");
    JsonResource resource("/service");
    resource.json["time"] = time(NULL);
    resource.setResponseStale();

    std::cout << "map..\n";
    //service.map("/client", &webServer, true);
    service.map("/service", &resource);
    //service.map("/service/notify", &notifier);
    service.runInBackground();

    for(;;) {
        resource.json["time"] = time(NULL);
        resource.setResponseStale();
        sleep(3);
    }
}