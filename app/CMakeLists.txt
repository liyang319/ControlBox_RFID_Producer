# include sub directories
include_directories(${PROJECT_SOURCE_DIR}/include/base)
include_directories(${PROJECT_SOURCE_DIR}/include/config)
include_directories(${PROJECT_SOURCE_DIR}/include/log)
include_directories(${PROJECT_SOURCE_DIR}/include/network)
include_directories(${PROJECT_SOURCE_DIR}/include/core)
include_directories(${PROJECT_SOURCE_DIR}/include/device)
include_directories(${PROJECT_SOURCE_DIR}/include/device/can)
include_directories(${PROJECT_SOURCE_DIR}/include/device/rs485)
include_directories(${PROJECT_SOURCE_DIR}/include/device/serial)
include_directories(${PROJECT_SOURCE_DIR}/include/device/sensor)
include_directories(${PROJECT_SOURCE_DIR}/include/device/gps)
include_directories(${PROJECT_SOURCE_DIR}/include/device/screen)
include_directories(${PROJECT_SOURCE_DIR}/include/device/rfid)
include_directories(${PROJECT_SOURCE_DIR}/include/utility)
include_directories(${PROJECT_SOURCE_DIR}/include/socket)
include_directories(${PROJECT_SOURCE_DIR}/include/database)
include_directories(${PROJECT_SOURCE_DIR}/include/thirdparty)
include_directories(${PROJECT_SOURCE_DIR}/include/thirdparty/rapidjson)
include_directories(${PROJECT_SOURCE_DIR}/include/thirdparty/mosquitto)
include_directories(${PROJECT_SOURCE_DIR}/include/thirdparty/sqlite)
include_directories(${PROJECT_SOURCE_DIR}/include/vendor)
include_directories(${PROJECT_SOURCE_DIR}/include/vendor/XCMG)

# add compile tree
add_subdirectory(config)
add_subdirectory(log)
add_subdirectory(network)
add_subdirectory(utility)
add_subdirectory(device)
add_subdirectory(vendor)

# all source files
file(GLOB_RECURSE ALL_SOURCE "*.cpp" "*.c")

# find_library(CURL
#   NAMES curl
#   PATHS ${PROJECT_SOURCE_DIR}/lib
# )
# set(ARM OFF) 

if(${ARM})
  link_directories(${PROJECT_SOURCE_DIR}/lib/arm/openssl)
  link_directories(${PROJECT_SOURCE_DIR}/lib/arm/mosquitto)
  link_directories(${PROJECT_SOURCE_DIR}/lib/arm/sqlite)
  link_directories(${PROJECT_SOURCE_DIR}/lib/arm/libcurl)
elseif(${LINUXBUILD})
  link_directories(${PROJECT_SOURCE_DIR}/lib/linux/openssl)
  link_directories(${PROJECT_SOURCE_DIR}/lib/linux/mosquitto)
  link_directories(${PROJECT_SOURCE_DIR}/lib/linux/sqlite)
  link_directories(${PROJECT_SOURCE_DIR}/lib/linux/libcurl)
elseif(${MACBUILD})
  link_directories(${PROJECT_SOURCE_DIR}/lib/macos/openssl)
endif()


# create the executable
set(EXECUTABLE_NAME WeighBox)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/output)

# execute file 
add_executable(${EXECUTABLE_NAME} ${ALL_SOURCE})

target_link_libraries(${EXECUTABLE_NAME} mosquitto)
target_link_libraries(${EXECUTABLE_NAME} ssl)
target_link_libraries(${EXECUTABLE_NAME} crypto)
target_link_libraries(${EXECUTABLE_NAME} dl)
target_link_libraries(${EXECUTABLE_NAME} sqlite3)
target_link_libraries(${EXECUTABLE_NAME} curl)
#target_link_libraries(${EXECUTABLE_NAME} mosquitto_static)
target_link_libraries(${EXECUTABLE_NAME} pthread)
target_link_libraries(${EXECUTABLE_NAME} rt)
