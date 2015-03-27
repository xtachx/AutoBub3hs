cmake_minimum_required(VERSION 2.8)
project( DisplayImage )
find_package( OpenCV REQUIRED )

set(CMAKE_MODULE_PATH "cmake_modules")
#find_package(sqlite3 REQUIRED)
#MESSAGE( STATUS "Found : ${SQLITE3_LIBRARIES}" )


SET(CMAKE_CXX_FLAGS "-std=gnu++11")

file(GLOB JMO_BGS_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/BGS/methods/jmo/*.h)
file(GLOB JMO_BGS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/BGS/methods/jmo/*.cpp)

set(SOURCE
   ${CMAKE_CURRENT_SOURCE_DIR}/ImageEntropyMethods/ImageEntropyMethods.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/ParseFolder/ParseFolder.cpp
   #${CMAKE_CURRENT_SOURCE_DIR}/SQLiteDBstorage/imageAnalysisResults.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/BubbleLocalizer/TplMatchLocalizer.cpp
   #${CMAKE_CURRENT_SOURCE_DIR}/BGS/methods/DPZivkovicAGMMBGS.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/BGS/methods/AdaptiveSelectiveBackgroundLearning.cpp
   #${JMO_BGS_SOURCES}

)
set(HEADERS
   ${CMAKE_CURRENT_SOURCE_DIR}/ImageEntropyMethods/ImageEntropyMethods.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/ParseFolder/ParseFolder.hpp
   #${CMAKE_CURRENT_SOURCE_DIR}/SQLiteDBstorage/imageAnalysisResults.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/common/CommonDatatypes.h
   ${CMAKE_CURRENT_SOURCE_DIR}/BubbleLocalizer/TplMatchLocalizer.hpp
   #${CMAKE_CURRENT_SOURCE_DIR}/BGS/methods/DPZivkovicAGMMBGS.h
   ${CMAKE_CURRENT_SOURCE_DIR}/BGS/methods/AdaptiveSelectiveBackgroundLearning.h
   ${CMAKE_CURRENT_SOURCE_DIR}/BGS/bgs.hpp
   #${JMO_BGS_HEADERS}

)

add_library(bgs SHARED ${JMO_BGS_SOURCES})
target_link_libraries(bgs ${OpenCV_LIBS})
set_property(TARGET bgs PROPERTY PUBLIC_HEADER ${bgs_include})

add_executable( abub AutoBubStart.cpp )
target_link_libraries( abub ${OpenCV_LIBS} ${SOURCE} ${HEADERS} bgs )
