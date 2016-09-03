cmake_minimum_required(VERSION 2.8)
project( DisplayImage )
find_package( OpenCV REQUIRED )

set(CMAKE_MODULE_PATH "cmake_modules")
#find_package(sqlite3 REQUIRED)
#MESSAGE( STATUS "Found : ${SQLITE3_LIBRARIES}" )


SET(CMAKE_CXX_FLAGS "-std=gnu++1y -O2 -march=ivybridge -m64")

file(GLOB LBP_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/LBP/*.hpp)
file(GLOB LBP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/LBP/*.cpp)

set(SOURCE
   ${CMAKE_CURRENT_SOURCE_DIR}/ImageEntropyMethods/ImageEntropyMethods.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/ParseFolder/ParseFolder.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/BubbleLocalizer/L3Localizer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/common/UtilityFunctions.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/AlgorithmTraining/Trainer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/AnalyzerUnit.cpp
)
set(HEADERS
   ${CMAKE_CURRENT_SOURCE_DIR}/ImageEntropyMethods/ImageEntropyMethods.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/ParseFolder/ParseFolder.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/common/CommonDatatypes.h
   ${CMAKE_CURRENT_SOURCE_DIR}/BubbleLocalizer/L3Localizer.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/common/UtilityFunctions.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/AlgorithmTraining/Trainer.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/AnalyzerUnit.hpp

)

add_library(lbpuser SHARED ${LBP_SOURCES})
target_link_libraries(lbpuser ${OpenCV_LIBS})
set_property(TARGET lbpuser PROPERTY PUBLIC_HEADER ${LBP_HEADERS})

add_executable( abub3 AutoBubStart3.cpp )
target_link_libraries( abub3 ${OpenCV_LIBS} ${SOURCE} ${HEADERS} lbpuser )
#target_link_libraries( abub ${OpenCV_LIBS} ${SOURCE} ${HEADERS} )

