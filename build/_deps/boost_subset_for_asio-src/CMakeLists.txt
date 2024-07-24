cmake_minimum_required(VERSION 3.15)

project(boost_subset_for_asio)

add_library(boost_subset_for_asio INTERFACE)
target_include_directories(boost_subset_for_asio INTERFACE .)
