# tuw_build_hat_driver

A C++ driver for the Raspberry Pi Build HAT, talking to it directly over a serial
connection (Boost.Asio). It uploads firmware to the Build HAT bootloader when needed,
and drives/reads back devices such as motors that are plugged into its ports.

## A ROS 2 package, but only at the build-system level

This is packaged as a regular `ament_cmake` package (`package.xml` +
`CMakeLists.txt` calling `find_package(ament_cmake REQUIRED)` / `ament_package()`) so
it builds and installs cleanly inside a `colcon` workspace alongside other ROS 2
packages, and exports a standard CMake target (`tuw_build_hat_driver::build_hat_driver`)
for other ament packages to depend on.

**The driver itself stays pure C++ and has no ROS dependency at runtime.**
`BuildHat`, `Device` and `Motor` don't include `rclcpp`, don't define any nodes,
topics, services or messages, and don't link against ROS middleware — their only
third-party dependency is Boost (`boost::asio` for the serial link,
`boost::program_options` in the demo for argument parsing). ROS 2 is only involved in
*how the package is built and found* (`ament_cmake`/`colcon`), not in what the driver
code does. This makes `BuildHat`/`Device`/`Motor` reusable from non-ROS code too, as
long as it links the `build_hat_driver` library directly.

## Layout

- `include/tuw_build_hat_driver/`, `src/tuw_build_hat_driver/` — the `build_hat` library:
  - `Device` (`device.hpp`/`.cpp`) — abstract base for anything attached to a Build HAT
    port: owns the port id and a queued command buffer, and declares the hooks
    (`init()`, `deactivate()`, `decode()`, `is_feedback()`) a concrete device implements.
    Also provides the speed/angle unit-conversion helpers devices commonly need.
  - `Motor` (`motor.hpp`/`.cpp`) — a `Device` that drives a Build HAT motor port in
    closed-loop speed mode and decodes its speed/position feedback.
  - `BuildHat` (`build_hat.hpp`/`.cpp`) — owns the serial connection, uploads firmware if
    the Build HAT boots into its bootloader, and dispatches commands/feedback for all
    registered devices.
- `src/demo.cpp` — a small example executable, see below.
- `firmware/` — firmware/signature binaries uploaded to the Build HAT bootloader.

## Building

Since `CMakeLists.txt` relies on `ament_cmake` (and `find_package(rclcpp REQUIRED)`),
building it requires a sourced ROS 2 environment, normally as part of a colcon
workspace:

```bash
source /opt/ros/<distro>/setup.bash
colcon build --packages-select tuw_build_hat_driver
source install/setup.bash
```

It can also be configured with plain CMake instead of colcon, as long as the ROS 2
environment is sourced first so `ament_cmake`/`rclcpp` can be found on
`CMAKE_PREFIX_PATH`:

```bash
source /opt/ros/<distro>/setup.bash
cd tuw_build_hat_driver
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Demo

`demo` is a minimal example program that registers one `Motor`, spins it forward for a
few seconds, then backward, then stops it. Run it with `--help` to see all options
(paths below assume the plain CMake build; with colcon it's
`install/tuw_build_hat_driver/lib/demo` instead):

```bash
./build/demo --help
```

```
Allowed options:
  -h [ --help ]                         produce help message
  -d [ --serial-device ] arg (=/dev/ttyUSB0)
                                        serial port the Build HAT is connected to
  -f [ --firmware-dir ] arg (=.../tuw_build_hat_driver/firmware)
                                        directory containing the firmware.bin and
                                        signature.bin files
  -s [ --serial_log ] arg               file to log the serial communication to
  -m [ --msgs_log ] arg                 file to log the msgs to if empty it will be
                                        printed to std::cout
  -l [ --loglevel ] arg (=1)            log level: 0 = debug, 1 = info, 2 = warning,
                                        3 = error
  -p [ --port ] arg (=0)                Build HAT port the motor is connected to
  -t [ --time ] arg (=5)                seconds to hold each target speed for
  -r [ --rad-per-sec ] arg (=3.14159)   target speed in radians per second
```

### Examples

Run with all defaults (serial device `/dev/ttyUSB0`, motor on port 0, info-level logging):

```bash
./build/demo
```

Drive the motor on port 1 at half the default speed for 2 seconds each direction:

```bash
./build/demo --port 1 --rad-per-sec 1.57 --time 2
```

Use a different serial device and enable debug-level logging plus a raw serial log file:

```bash
./build/demo \
  --serial-device /dev/ttyAMA0 \
  --loglevel 0 \
  --serial_log /tmp/build_hat_serial.log
```

Point at a custom firmware directory (must contain `firmware.bin` and `signature.bin`):

```bash
./build/demo --firmware-dir /path/to/firmware
```
