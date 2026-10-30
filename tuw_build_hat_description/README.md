# tuw_build_hat_description

Robot description (URDF/xacro) and meshes of the LEGO differential-drive vehicle, used for simulation in Gazebo and for the real robot with a Raspberry Pi Build HAT.

## models/lego_diffdrive

`main.xacro` builds the vehicle from:

* `body/common_properties.xacro` dimensions, masses, materials and the `box_inertia` / `surface_friction` macros
* `body/mobile_base.xacro` links and joints: base plate, Raspberry Pi, battery, two motors with wheels (`left_motor_wheel_joint`, `right_motor_wheel_joint`) and a caster ball
* `control/ros2_gz_control.xacro` ros2_control block with `gz_ros2_control/GazeboSimSystem` and the `gz_ros2_control` plugin, included with `setup:=gz`
* `control/ros2_hw_control.xacro` ros2_control block with `tuw_build_hat_hardware_interface/BuildHatHardwareInterface`, included with `setup:=hw`

Both wheel joints expose a `velocity` command interface and `position` / `velocity` state interfaces.

### xacro arguments

* `setup` `gz` (simulation) or `hw` (real robot). default `gz`
* `namespace` robot namespace. default empty
* `meshes_dir` mesh location used in the `<mesh filename>` tags, e.g. `tuw_build_hat_description/meshes`. default empty
* `serial_port` serial port of the Build HAT, only used with `setup:=hw`. default `/dev/ttyAMA0`

```bash
xacro $(ros2 pkg prefix --share tuw_build_hat_description)/models/lego_diffdrive/main.xacro \
  setup:=gz namespace:=r0 meshes_dir:=tuw_build_hat_description/meshes > /tmp/robot_description.urdf
```

In simulation the `gz_ros2_control` plugin loads the controller configuration from `tuw_build_hat_bringup/config/diffdrive_controllers.yaml`.

## meshes

`meshes/lego/*.dae` LEGO parts (plate, motors, wheel, sensors, chassis) used as visuals.

## launch

* `display.launch.py` / `display.launch.xml` show the description in rviz with `joint_state_publisher_gui` (config `rviz/urdf_config.rviz`)

## environment

The environment hooks in `hooks/` add the package `share` and `models` directories to `GZ_SIM_RESOURCE_PATH`, so Gazebo finds the meshes after sourcing the workspace.

To start the vehicle use [tuw_build_hat_bringup](../tuw_build_hat_bringup/README.md).
