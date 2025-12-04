# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/appDashboardproject_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/appDashboardproject_autogen.dir/ParseCache.txt"
  "appDashboardproject_autogen"
  )
endif()
