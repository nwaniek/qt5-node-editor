cmake_minimum_required(VERSION 2.8.8)
project(node-editor)

add_definitions(-std=c++11) 
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(EIGEN3_INCLUDE_DIR "/usr/include/eigen3" CACHE STRING "Eigen3 include directory")

find_package(
	Boost 1.55.0
	COMPONENTS
		system
		program_options
	REQUIRED)

find_package(Qt5Widgets REQUIRED)

set(SRC
	main.cpp
	mainwindow.cpp
	node.cpp
	graphicsbezieredge.cpp
	graphicsnode.cpp
	graphicsnodeview.cpp
	graphicsnodescene.cpp
	graphicsnodesocket.cpp
	)
set(HEADERS
	mainwindow.hpp
	node.hpp
	graphicsbezieredge.hpp
	graphicsnode.hpp
	graphicsnodeview.hpp
	graphicsnodescene.hpp
	graphicsnodesocket.hpp
)

# include(${QT_USE_FILE})
include_directories(
	${EIGEN3_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
	)
add_executable(${PROJECT_NAME} ${SRC} ${HEADERS_MOC} ${HEADERS})
target_link_libraries(${PROJECT_NAME}
	m
	pthread
	hdf5
	hdf5_cpp
	${Boost_LIBRARIES}
)
qt5_use_modules(${PROJECT_NAME} Widgets)
