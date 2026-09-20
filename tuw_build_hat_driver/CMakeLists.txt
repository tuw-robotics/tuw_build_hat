cmake_minimum_required(VERSION 3.20)
project(tuw_build_hat_driver)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(rclcpp REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()



###############
## build_hat ##

# Library implementing the BuildHatDriver.
add_library(build_hat_driver SHARED src/tuw_build_hat_driver/build_hat.cpp src/tuw_build_hat_driver/device.cpp src/tuw_build_hat_driver/motor.cpp)
target_compile_features(build_hat_driver PUBLIC c_std_99 cxx_std_20)
target_include_directories(build_hat_driver PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(build_hat_driver PUBLIC
  Boost::headers
)

install(
  TARGETS build_hat_driver
  EXPORT export_${PROJECT_NAME}
  DESTINATION lib
)

install(
  DIRECTORY include/
  DESTINATION include
)
install(
  DIRECTORY firmware
  DESTINATION share/${PROJECT_NAME}
)

####################
## demo ##

# Minimal executable that exercises BuildHatDriver by calling its init() method.
add_executable(demo src/demo.cpp)
target_compile_features(demo PUBLIC c_std_99 cxx_std_20)
target_link_libraries(demo PUBLIC
  build_hat_driver
  Boost::program_options
)

install(
  TARGETS demo
  DESTINATION lib
)


ament_export_include_directories(
  include
)
ament_export_libraries(
  build_hat_driver
)
ament_export_targets(
  export_${PROJECT_NAME}
)

ament_export_dependencies(
  rclcpp
)
ament_package()
