#include <iostream>
#include <string>
#include "hyperledger/middleware.h"


int main() {
    std::cout << "Start !!!" << std::endl;

    ClientSDK sdk;
    sdk.startFabric();

    sdk.writeTrans("1", "bitcoin");
    sdk.writeTrans("2", "ethereum");
    sdk.writeTrans("3", "hyperledger");
    sdk.writeTrans("4", "eos");


    std::string result1 = sdk.getTrans("1");
    std::string result2 = sdk.getTrans("2");
    std::string result3 = sdk.getTrans("3");

    std::cout << "key1" <<  result1 << std::endl;
    std::cout << "key2" <<  result2 << std::endl;
    std::cout << "key3" <<  result3 << std::endl;

    return 0;
}
