# tuw_build_hat_hardware_interface

[ros2_control](https://control.ros.org) hardware interface for a Raspberry Pi Build HAT with two motors, based on [tuw_build_hat_driver](../tuw_build_hat_driver/README.md).

Plugin: `tuw_build_hat_hardware_interface/BuildHatHardwareInterface` (`hardware_interface::SystemInterface`)

## behaviour

* expects exactly two joints in the `<ros2_control>` block, the **first one is the right**, the second one the left wheel
* each joint has a `velocity` command interface and `position` / `velocity` state interfaces, in rad/s and rad
* `on_configure` opens the serial port and uploads the Build HAT firmware if needed (`firmware.bin` / `signature.bin` from `tuw_build_hat_driver/firmware`)
* `write` sends the velocity commands to the motors using the PID gains below, `read` returns the motor velocities and cumulative positions
* the left motor is mounted mirrored, its direction is inverted internally
* `on_deactivate` stops both motors

## hardware parameters

Set as `<param>` tags in the `<hardware>` block.

* `serial_port` serial port of the Build HAT. default `/dev/ttyUSB0`
* `port_motor_left` Build HAT port of the left motor. default `0`
* `port_motor_right` Build HAT port of the right motor. default `1`
* `kp`, `ki`, `kd` motor speed PID gains. default `0.003`, `0.01`, `0.0`
* `msgs_loglevel` log level of the driver messages. default `1`
* `filename_msgs_log` file for driver messages. default empty (`std::cout`)
* `filename_serial_log` file for the raw serial communication. default empty (`std::cout`)

### example

```xml
<ros2_control name="MobileBase" type="system">
  <hardware>
    <plugin>tuw_build_hat_hardware_interface/BuildHatHardwareInterface</plugin>
    <param name="serial_port">/dev/ttyAMA0</param>
    <param name="port_motor_left">0</param>
    <param name="port_motor_right">1</param>
  </hardware>
  <joint name="right_motor_wheel_joint">
    <command_interface name="velocity" />
    <state_interface name="position" />
    <state_interface name="velocity" />
  </joint>
  <joint name="left_motor_wheel_joint">
    <command_interface name="velocity" />
    <state_interface name="position" />
    <state_interface name="velocity" />
  </joint>
</ros2_control>
```

The vehicle uses this in `tuw_build_hat_description/models/lego_diffdrive/control/ros2_hw_control.xacro`, started with `ros2 launch tuw_build_hat_bringup vehicle.launch.py setup:=hw`.

## demo

`demo` drives two motors directly through the driver, without ros2_control, to test the hardware. It is installed to `lib/` (not `lib/<package>`), so it is started by path instead of `ros2 run`:

```bash
$(ros2 pkg prefix tuw_build_hat_hardware_interface)/lib/demo --help
```
