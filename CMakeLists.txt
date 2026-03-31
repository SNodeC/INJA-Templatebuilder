cmake_minimum_required(VERSION 3.20)
project(INJA_Templatebuilder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(snodec REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(inja REQUIRED)

add_executable(inja-templatebuilder
  backend/src/main.cpp
)

target_link_libraries(inja-templatebuilder
  PRIVATE
    snodec::express
    nlohmann_json::nlohmann_json
    pantor::inja
)
