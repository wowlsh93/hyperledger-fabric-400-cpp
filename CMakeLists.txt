cmake_minimum_required(VERSION 3.15)
project(hyperledger_fabric_400_cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(hyperledger_fabric_400_cpp main.cpp hyperledger/fabric.h hyperledger/fabric.cpp hyperledger/middleware.h hyperledger/middleware.cpp)