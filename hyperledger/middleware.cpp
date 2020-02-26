//
// Created by hama on 20. 2. 23..
//
#include "middleware.h"


void ClientSDK::startFabric() {

    fab = new Fabric;
    fab->start();
}

void ClientSDK::endFabric() {
    fab->stop();
    delete fab;
}

string ClientSDK::writeTrans(std::string key, std::string value) {
    std::tuple<RWSet, RWSet> rwsets = fab->writeTransaction(key, value, fab->MSP_org1);

    if (get<0>(rwsets).msp == fab->MSP_peer1 && get<1>(rwsets).msp == fab->MSP_peer1 ) {
        vector<string> msps;
        msps.push_back(get<0>(rwsets).msp);
        msps.push_back(get<1>(rwsets).msp);

        RWSet rwset(key, value, "", msps);
        fab->sendToOrderer(rwset);
        return "ok";
    }

    return "failed";
}

std::string ClientSDK::getTrans(std::string key) {
    string rwset = fab->readTranaction(key, fab->MSP_org1);
    return rwset;

}