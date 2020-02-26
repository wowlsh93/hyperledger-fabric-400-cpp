//
// Created by hama on 20. 2. 23..
//

#ifndef HYPERLEDGER_FABRIC_400_CPP_MIDDLEWARE_H
#define HYPERLEDGER_FABRIC_400_CPP_MIDDLEWARE_H

#endif //HYPERLEDGER_FABRIC_400_CPP_MIDDLEWARE_H

#include <string>
#include "fabric.h"

class ClientSDK {


public:
    void writeTrans(std::string key, std::string value);
    std::string getTrans(std::string key);
    void startFabric();
    void endFabric();

    Fabric* fab;

};