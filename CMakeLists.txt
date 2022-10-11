cmake_minimum_required(VERSION 3.20)
project(Logger)
add_library(${PROJECT_NAME} STATIC logger.h logger.cpp)