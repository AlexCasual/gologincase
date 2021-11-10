gologin
=====================

This is a C++ gologin test tcp-socket based chat.

## Releases
* Under `releases` folder

## Features
* tcp socket based chat
* cross-platform multi-threaded tcp server
* gui client app
* tagged embedded schemaless protocol dto serialization
* config
* logger

## Protocol
See [json](messages.json)

## Config
See [config](config/gologin.json)

`logger_level : info/debug/error`
`logger_sink : console/file`

## Requirements
* A C++ compiler with C++20 support
* POCO
* wxWidgets
* nlohmann-json
* gtest

## How to build

At the moment only `cmake-based` build systems are supported.


## How to run
cfg:

create `gologin.json` in server app executable folder

server app:

`./server`

client app:

`./client`
