cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(QGeoView LANGUAGES C CXX)

option(BUILD_GDAL_EXAMPLE "Build the GeoTIFF Raster Example (Requires GDAL library)" OFF)

add_subdirectory(demo)
add_subdirectory(lib)
