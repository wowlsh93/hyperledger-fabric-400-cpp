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

void ClientSDK::writeTrans(std::string key, std::string value) {



}


std::string ClientSDK::getTrans(std::string key) {


}