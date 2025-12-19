cmake_minimum_required(VERSION 3.22.1)
project(LocalChatApp)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Socket utility library
add_library(socketUtil lib/socketUtilImp.cpp)
target_include_directories(socketUtil PUBLIC lib/)

# Client helper library
add_library(clientHelper client/clientHelper/clientImp.cpp)
target_include_directories(clientHelper PUBLIC client/clientHelper/)
target_link_libraries(clientHelper socketUtil)

# Server helper library  
add_library(serverHelper server/serverHelper/serverImp.cpp)
target_include_directories(serverHelper PUBLIC server/serverHelper/)
target_link_libraries(serverHelper socketUtil)

# Client executable
add_executable(client client/main.cpp)
target_link_libraries(client clientHelper socketUtil)
  
# Server executable
add_executable(server server/main.cpp)
target_link_libraries(server serverHelper socketUtil pthread)
