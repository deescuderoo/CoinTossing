cmake_minimum_required(VERSION 3.5)
project(BCast)

set(CMAKE_CXX_STANDARD 11)
set("${CMAKE_CXX_FLAGS} -g")


INCLUDE_DIRECTORIES($ENV{HOME} $ENV{HOME}/libscapi/install/include/ $ENV{HOME}/libscapi/include/)

set(SOURCE_FILES main.cpp CoinTossing.cpp)

link_directories(/usr/ssl/lib/ $ENV{HOME}/libscapi/install/lib)

add_executable(CoinTossing ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(CoinTossing $ENV{HOME}/libscapi/libscapi.a gmp gmpxx libboost_system.a pthread crypto dl ssl z)