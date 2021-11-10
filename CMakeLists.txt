cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(gologin CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

find_package(nlohmann_json CONFIG REQUIRED)
find_package(wxWidgets REQUIRED)
find_package(Poco REQUIRED Foundation Util Net)
find_package(Threads REQUIRED)
find_package(GTest CONFIG REQUIRED)

include(${wxWidgets_USE_FILE})

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(CONTRIB_DIR ${CMAKE_SOURCE_DIR}/contrib)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${SRC_DIR})
include_directories(${CONTRIB_DIR})
include_directories(${CONTRIB_DIR}/spdlog/include)

#spdlogger
add_library(spdlogger INTERFACE)
target_include_directories(spdlogger INTERFACE "$<BUILD_INTERFACE:${CONTRIB_DIR}/spdlog/include>")

#core
add_library(core STATIC
    ${SRC_DIR}/core/core.hpp
	${SRC_DIR}/core/error.hpp
	${SRC_DIR}/core/headers.hpp
	${SRC_DIR}/core/packer.hpp
	${SRC_DIR}/core/dispatcher.hpp
	${SRC_DIR}/core/logger.hpp
	${SRC_DIR}/core/error.cpp)
	
add_library(tcp STATIC
    ${SRC_DIR}/tcp/core/core.hpp
	${SRC_DIR}/tcp/core/dispatcher.hpp
	${SRC_DIR}/tcp/client/client.hpp
	${SRC_DIR}/tcp/server/server.hpp
	${SRC_DIR}/tcp/core/core.cpp
	${SRC_DIR}/tcp/client/client.cpp
	${SRC_DIR}/tcp/server/server.cpp)
		
#auth
add_library(auth STATIC
    ${SRC_DIR}/auth/provider/provider.hpp
	${SRC_DIR}/auth/provider/provider.cpp)
	
#server
add_executable(server
				${SRC_DIR}/messages/messages.hpp
				${SRC_DIR}/messages/json/packer.hpp
				${SRC_DIR}/server/app.hpp
				${SRC_DIR}/server/main.cpp
				${SRC_DIR}/server/app.cpp)
				
target_link_libraries(server PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(server PRIVATE ${SRC_DIR})
target_link_libraries(server PRIVATE nlohmann_json nlohmann_json::nlohmann_json)
target_link_libraries(server PRIVATE Poco::Util Poco::Net)
target_link_libraries(server PRIVATE spdlogger)
target_link_libraries(server PRIVATE core tcp auth)

#client
if(WIN32)
add_executable(client WIN32
				${SRC_DIR}/messages/messages.hpp
				${SRC_DIR}/messages/json/packer.hpp
				${SRC_DIR}/client/app.hpp
				${SRC_DIR}/client/frame.hpp
				${SRC_DIR}/client/main.cpp
				${SRC_DIR}/client/app.cpp
				${SRC_DIR}/client/frame.cpp)
else()
add_executable(client
				${SRC_DIR}/messages/messages.hpp
				${SRC_DIR}/messages/json/packer.hpp
				${SRC_DIR}/client/app.hpp
				${SRC_DIR}/client/frame.hpp
				${SRC_DIR}/client/main.cpp
				${SRC_DIR}/client/app.cpp
				${SRC_DIR}/client/frame.cpp)				
endif()
				
target_link_libraries(client PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(client PRIVATE ${wxWidgets_LIBRARIES})
target_include_directories(client PRIVATE ${SRC_DIR})
target_link_libraries(client PRIVATE nlohmann_json nlohmann_json::nlohmann_json)
target_link_libraries(client PRIVATE Poco::Util Poco::Net)
target_link_libraries(client PRIVATE spdlogger)
target_link_libraries(client PRIVATE core tcp auth)

