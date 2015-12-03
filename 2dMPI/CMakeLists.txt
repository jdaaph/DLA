project(mytest)


# Require MPI for this project:
find_package(MPI REQUIRED)
set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
include_directories(MPI_INCLUDE_PATH)


add_executable(mytest 	main.cc
			GlobalDLA.cc
			LocalDLA.cc
			Vec2D.h
	      )
target_link_libraries(mytest ${MPI_LIBRARIES})
