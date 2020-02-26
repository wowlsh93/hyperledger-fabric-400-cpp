cmake_minimum_required(VERSION 3.15)
project(hyperledger_fabric_400_cpp)

set(CMAKE_CXX_STANDARD 17)
find_library(PTHREAD_PATH pthread)

add_executable(hyperledger_fabric_400_cpp main.cpp hyperledger/fabric.h hyperledger/fabric.cpp hyperledger/middleware.h hyperledger/middleware.cpp hyperledger/sha256.h hyperledger/sha256.cpp hyperledger/stop_watch.h hyperledger/string_format.h)
target_link_libraries(hyperledger_fabric_400_cpp ${PTHREAD_PATH})