# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\TodoList_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\TodoList_autogen.dir\\ParseCache.txt"
  "TodoList_autogen"
  )
endif()
