cmake_minimum_required(VERSION 3.28)
project(tuw_build_hat_hardware_interface)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(tuw_build_hat_driver REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(Boost REQUIRED CONFIG COMPONENTS program_options)

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

include_directories(./include)

####################
## demo ##

# Minimal executable that exercises BuildHatDriver by calling its init() method.
add_executable(demo src/demo.cpp)
target_compile_features(demo PUBLIC c_std_99 cxx_std_20)
target_link_libraries(demo PUBLIC
  tuw_build_hat_driver::build_hat_driver
  Boost::program_options
)


add_library(
  ${PROJECT_NAME}
  SHARED
  src/tuw_build_hat_hardware_interface/build_hat_hardware_interface.cpp
)

target_link_libraries(
  ${PROJECT_NAME} PUBLIC
  rclcpp::rclcpp
  rclcpp_lifecycle::rclcpp_lifecycle
  hardware_interface::hardware_interface
  pluginlib::pluginlib
  tuw_build_hat_driver::build_hat_driver
  ament_index_cpp::ament_index_cpp
)

pluginlib_export_plugin_description_file(hardware_interface tuw_build_hat_hardware_interface.xml)

install(
  TARGETS demo ${PROJECT_NAME} 
  DESTINATION lib
)

install(
  DIRECTORY include
  DESTINATION include
)

ament_export_libraries(
  ${PROJECT_NAME}
)

ament_export_dependencies(
  hardware_interface
  pluginlib
  rclcpp
  rclcpp_lifecycle
  ament_index_cpp
)

ament_package()
