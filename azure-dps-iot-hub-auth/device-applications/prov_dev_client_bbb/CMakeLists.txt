cmake_minimum_required(VERSION 2.8.11)

set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 11)

#set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")

include_directories(./inc)
include_directories(${IOTHUB_CLIENT_INC_FOLDER})
include_directories(${DEV_AUTH_MODULES_CLIENT_INC_FOLDER})
include_directories(${SHARED_UTIL_INC_FOLDER})
include_directories(${CMAKE_CURRENT_LIST_DIR}/adapters)

#set(AZUREIOT_INC_FOLDER ".." "/usr/local/include/azureiot" "/usr/local/include/azureiot/inc")
#set(PROV_CLIENT_DEV_INC_FOLDER $CMAKE_CURRENT_SOURCE_DIR/../../inc/azure_prov_client)

#include_directories(${AZUREIOT_INC_FOLDER} ${PROV_CLIENT_DEV_INC_FOLDER})

set(prov_dev_client_bbb_c_files
    ./src/main.c
)

set(prov_dev_client_bbb_h_files

)

add_executable(prov_dev_client_bbb ${prov_dev_client_bbb_c_files} ${prov_dev_client_bbb_h_files})

target_link_libraries(prov_dev_client_bbb
    serializer
    iothub_client
    iothub_client_amqp_transport
    prov_device_client
    prov_mqtt_transport
    prov_amqp_transport
    uamqp
    aziotsharedutil
    pthread
    curl
    ssl
    crypto
    m
)
