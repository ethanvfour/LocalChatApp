cmake_minimum_required(VERSION 3.22.1)
project(LocalChatApp)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(socketUtil lib/socketUtilImp.cpp)
target_include_directories(socketUtil PUBLIC lib/)

add_executable(client client/main.cpp)
target_link_libraries(client socketUtil)
  
add_executable(server server/main.cpp)
target_link_libraries(server socketUtil pthread)
