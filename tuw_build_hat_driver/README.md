# tuw_build_hat_driver

A C++ driver for the Raspberry Pi Build HAT, talking to it directly over a serial
connection (Boost.Asio). It uploads firmware to the Build HAT bootloader when needed,
and drives/reads back devices such as motors that are plugged into its ports.

## Not a ROS package (in spirit)

This is a plain, ROS-independent CMake project — its `CMakeLists.txt` uses only
standard CMake (`find_package`/`install`/`GNUInstallDirs`), with no `ament_cmake` and no
`ament_*` calls anywhere. **The driver code does not link against any ROS libraries**
either (no `rclcpp`, no ROS messages, no middleware). `BuildHat`, `Device` and `Motor`
are plain C++ classes with no ROS dependency at all — their only third-party dependency
is Boost (`boost::asio` for the serial link, `boost::program_options` in the demo for
argument parsing). This makes the driver usable standalone, or from any other
framework, not just ROS 2 nodes.

A `package.xml` is still included purely so the package can sit inside a ROS 2
workspace and be found by `colcon`/rosdep alongside other packages there; it is not
needed to build this package on its own.

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

As a standalone CMake project, without colcon or ROS involved at all:

```bash
cd tuw_build_hat_driver
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

If it happens to sit inside a ROS 2 workspace, it also builds fine through colcon:

```bash
colcon build --packages-select tuw_build_hat_driver
source install/setup.bash
```

## Demo

`demo` is a minimal example program that registers one `Motor`, spins it forward for a
few seconds, then backward, then stops it. Run it with `--help` to see all options
(paths below assume the standalone CMake build; with colcon it's
`install/tuw_build_hat_driver/bin/demo` instead):

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
