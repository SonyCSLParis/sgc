cmake_minimum_required(VERSION 2.6)
project(libsgc)

set(libr_VERSION_MAJOR 0)
set(libr_VERSION_MINOR 1)

set(CMAKE_BUILD_TYPE DEBUG)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -shared")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_DEBUG} -shared ")

set(SOURCES
  src/config.h
  src/sgc_allocator.c
  src/sgc_allocator.h
  src/sgc_area_heap.c
  src/sgc_area_heap.h
  src/sgc.c
  src/sgc_dlist.c
  src/sgc_dlist.h
  src/sgc_fifo.h
  src/sgc.h
  src/sgc_interval.c
  src/sgc_interval.h
  src/sgc_lifo.h
  src/sgc_list.c
  src/sgc_list.h
  src/sgc_log.c
  src/sgc_log.h
  src/sgc_mem_heap.c
  src/sgc_mem_heap.h
  src/sgc_mess.c
  src/sgc_mess.h
  src/sgc_priv.h
  src/sgc_sys.h
  src/sgc_thread.c
  src/sgc_thread.h
  src/sgc_trace.c
  src/sgc_types.h
  src/sgc_var_heap.c
  src/sgc_var_heap.h)

add_library(sgc SHARED ${SOURCES})
target_link_libraries(sgc pthread)

set_target_properties(sgc PROPERTIES PUBLIC_HEADER "src/sgc.h")

INSTALL(TARGETS sgc 
        LIBRARY DESTINATION "lib"
        PUBLIC_HEADER DESTINATION "include")
