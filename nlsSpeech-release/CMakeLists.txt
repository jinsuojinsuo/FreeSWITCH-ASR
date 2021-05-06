cmake_minimum_required(VERSION 2.4)
project(speech-demo)

if(UNIX)
    message("this operation platform is UNIX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI -lpthread -D_GNU_ -ldl")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/openssl)

link_directories(${CMAKE_SOURCE_DIR}/lib/linux)

add_executable(nluDemo demo/nlu-demo.cpp)
add_executable(ttsDemo demo/tts-demo.cpp)
add_executable(realtimeDemo demo/realtime-demo.cpp)
add_executable(asrDemo demo/asr-demo.cpp)

target_link_libraries(nluDemo realTimeUnity)
target_link_libraries(ttsDemo realTimeUnity)
target_link_libraries(realtimeDemo realTimeUnity)
target_link_libraries(asrDemo realTimeUnity)

if(UNIX)
	target_link_libraries(asrDemo libssl.so libcrypto.so libopus.so)
	target_link_libraries(realtimeDemo libssl.so libcrypto.so libopus.so)
	target_link_libraries(ttsDemo libssl.so libcrypto.so libopus.so)
	target_link_libraries(nluDemo libssl.so libcrypto.so libopus.so)

endif()

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/demo) 

