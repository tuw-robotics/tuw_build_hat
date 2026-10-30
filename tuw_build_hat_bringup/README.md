# tuw_build_hat_bringup

Launch files and configuration to bring up the LEGO diffdrive vehicle with [ros2_control](https://control.ros.org), either in Gazebo or on the real robot with a Raspberry Pi Build HAT.

The robot description is taken from `tuw_build_hat_description` (`models/lego_diffdrive/main.xacro`).

## vehicle.launch.py

Generates the robot description and starts:

* `robot_state_publisher` with the generated `robot_description`
* `setup:=gz` spawns the model into a running Gazebo world; the `gz_ros2_control` plugin inside Gazebo provides the controller manager
* `setup:=hw` starts a `ros2_control_node` (controller manager) using the Build HAT hardware interface on `serial_port`
* the `diff_drive_controller` and `joint_state_broadcaster` spawners
* `rviz2` (optional)

### arguments

* `setup` `gz` for simulation or `hw` for the real robot. required, no default
* `entity_name` robot name, also selects the rviz config `config/<entity_name>.rviz`. default `r0`
* `serial_port` serial port of the Build HAT, only used with `setup:=hw`. default `/dev/ttyUSB0`
* `rviz_launch` launches rviz on `true`. default `true`
* `x`, `y`, `z`, `R`, `P`, `Y` spawn pose, only used with `setup:=gz` (passed on to `ros_gz_sim gz_spawn_model.launch.py`)

The generated description is also written to `/tmp/robot_description.urdf` for debugging.

### usage

```bash
# simulation: start an empty world (-r runs it right away) and bridge the simulation clock to ROS
ros2 launch ros_gz_sim gz_sim.launch.py gz_args:="-r empty.sdf"
ros2 run ros_gz_bridge parameter_bridge /clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock

# then spawn the vehicle
ros2 launch tuw_build_hat_bringup vehicle.launch.py setup:=gz x:=0.0 y:=0.0 Y:=0.0

# real robot
ros2 launch tuw_build_hat_bringup vehicle.launch.py setup:=hw serial_port:=/dev/ttyAMA0 rviz_launch:=false

# drive the vehicle
ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args -p stamped:=true
```

A tmux setup which starts all of this at once is described in the [repository README](../README.md#tmux-demo).

### topics

* `/cmd_vel` velocity command for the `diff_drive_controller` (`geometry_msgs/msg/TwistStamped`)
* `/diff_drive_controller/odom` wheel odometry, the controller also publishes the `odom` → `base_link` transform
* `/joint_states` wheel joint states from the `joint_state_broadcaster`

## config

* `diffdrive_controllers.yaml` controller manager and controller parameters (wheel separation `0.112 m`, wheel radius `0.028 m`, velocity limits), shared by simulation and hardware. In simulation the `gz_ros2_control` plugin loads it via `$(find tuw_build_hat_bringup)` in `tuw_build_hat_description/models/lego_diffdrive/control/ros2_gz_control.xacro`
* `r0.rviz` rviz configuration for `entity_name:=r0`
