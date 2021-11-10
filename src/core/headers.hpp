#pragma once

//stl
#include <chrono>
#include <mutex>
#include <thread>
#include <future>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include <set>
#include <atomic>
#include <exception>
#include <system_error>
#include <stop_token>
#include <type_traits>
#include <shared_mutex>
#include <filesystem>
#include <queue>
#include <ranges>

//poco
#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Util/JSONConfiguration.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Net/Net.h>
#include <Poco/Net/DNS.h>
#include <Poco/Net/UDPServer.h>
#include <Poco/Net/UDPHandler.h>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/NetworkInterface.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <Poco/NullStream.h>

