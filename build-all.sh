#!/bin/bash

cmake --preset=linux
cmake --build --preset=build-linux

cmake --preset=windows
cmake --build --preset=build-windows
