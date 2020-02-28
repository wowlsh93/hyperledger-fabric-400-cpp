#include <iostream>
#include <string>
#include "hyperledger/middleware.h"
#include "hyperledger/stop_watch.h"

int main() {
    std::cout << "Start !!!" << std::endl;

    ClientSDK sdk;
    sdk.startFabric();
    std::cout << "Start 2!!!" << std::endl;

    hama::StopWatch sw;

    for (int i = 0 ; i < 1000050 ; i++) {
        sdk.writeTrans(std::to_string(i),std::to_string(i));
    }
    std::cout << "Start 3!!!" << std::endl;
    while(true) {
        string result = sdk.getTrans("1000000");
        if (result != "") {
            break;
        }
    }
    std::cout << "Start 4!!!" << std::endl;
    std::cout << sw.stop() << std::endl;

    return 0;
}
