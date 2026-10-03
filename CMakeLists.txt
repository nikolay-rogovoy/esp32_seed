cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Собирать только main и то, от чего он зависит (быстрее сборка)
idf_build_set_property(MINIMAL_BUILD ON)

project(esp32_seed)
