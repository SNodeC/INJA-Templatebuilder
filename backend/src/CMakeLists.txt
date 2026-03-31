cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(snodec REQUIRED COMPONENTS http-server-express-tls-in
                                        http-server-express-legacy-in)
find_package(nlohmann_json REQUIRED)

add_executable(inja-templatebuilder main.cpp)

target_link_libraries(
  inja-templatebuilder
  PUBLIC http-server-express-tls-in http-server-express-legacy-in
         nlohmann_json::nlohmann_json)
