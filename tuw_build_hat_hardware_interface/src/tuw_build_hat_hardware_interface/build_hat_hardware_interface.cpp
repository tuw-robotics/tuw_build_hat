#include "tuw_build_hat_hardware_interface/build_hat_hardware_interface.hpp"
#include <ament_index_cpp/get_package_share_path.hpp>

namespace tuw_build_hat_hardware_interface {

static constexpr char FIRMWARE_FILENAME[] = "firmware.bin";
static constexpr char SIGNATURE_FILENAME[] = "signature.bin";

hardware_interface::CallbackReturn BuildHatHardwareInterface::on_init(const hardware_interface::HardwareComponentInterfaceParams &params) {
    if (hardware_interface::SystemInterface::on_init(params) != hardware_interface::CallbackReturn::SUCCESS) {
        return hardware_interface::CallbackReturn::ERROR;
    }
    std::filesystem::path firmware_directory = ament_index_cpp::get_package_share_path("tuw_build_hat_driver") / "firmware";
    RCLCPP_INFO(get_logger(), "Firmware directory: %s", firmware_directory.c_str());

    serial_port_ = "/dev/ttyUSB0";
    if (!info_.hardware_parameters.count("serial_port")) {
        RCLCPP_WARN(get_logger(), "Missing important hardware parameter 'serial_port' using default: %s", serial_port_.c_str());
    } else {
        serial_port_ = info_.hardware_parameters["serial_port"];
        RCLCPP_INFO(get_logger(), "Serial port: %s", serial_port_.c_str());
    }

    int port_motor_left = 0;
    if (!info_.hardware_parameters.count("port_motor_left")) {
        RCLCPP_WARN(get_logger(), "Missing important hardware parameter 'port_motor_left' using default: %d", port_motor_left);
    } else {
        port_motor_left = std::stoi(info_.hardware_parameters["port_motor_left"]);
    }

    int port_motor_right = 1;
    if (!info_.hardware_parameters.count("port_motor_right")) {
        RCLCPP_WARN(get_logger(), "Missing important hardware parameter 'port_motor_right' using default: %d", port_motor_left);
    } else {
        port_motor_right = std::stoi(info_.hardware_parameters["port_motor_right"]);
    }
    RCLCPP_INFO(get_logger(), "Motor left on port: %d, motor right on port: %d", port_motor_left, port_motor_right);

    msgs_loglevel_ = 1;
    if (info_.hardware_parameters.count("msgs_loglevel")) {
        msgs_loglevel_ = std::stoi(info_.hardware_parameters["msgs_loglevel"]);
    }

    if (info_.hardware_parameters.count("filename_msgs_log")) {
        filename_msgs_log_ = info_.hardware_parameters["filename_msgs_log"];
    }
    RCLCPP_INFO(get_logger(),
                "Msgs Log to: %s with loglevel: %d",
                (filename_msgs_log_.empty() ? "std::cout" : filename_msgs_log_.c_str()),
                msgs_loglevel_);

    if (info_.hardware_parameters.count("filename_serial_log")) {
        filename_serial_log_ = info_.hardware_parameters["filename_serial_log"];
    }
    RCLCPP_INFO(get_logger(), "Serial Log to: %s", (filename_serial_log_.empty() ? "std::cout" : filename_serial_log_.c_str()));

    filename_firmware_ = (std::filesystem::path(firmware_directory) / FIRMWARE_FILENAME).string();
    filename_signature_ = (std::filesystem::path(firmware_directory) / SIGNATURE_FILENAME).string();
    RCLCPP_INFO(get_logger(), "Build Firmware: %s", filename_firmware_.c_str());

    build_hat_ = std::make_shared<tuw_build_hat_driver::BuildHat>();
    motor_left_ = std::make_shared<tuw_build_hat_driver::Motor>(port_motor_left);
    motor_right_ = std::make_shared<tuw_build_hat_driver::Motor>(port_motor_right);
    build_hat_->set_device(serial_port_, 115200);
    build_hat_->set_firmware(filename_firmware_, filename_signature_);
    build_hat_->set_logfile_serial(filename_serial_log_);
    build_hat_->set_logfile_msgs(filename_msgs_log_);
    build_hat_->set_loglevel(msgs_loglevel_);
    build_hat_->add_device(motor_left_);
    build_hat_->add_device(motor_right_);
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn BuildHatHardwareInterface::on_configure(const rclcpp_lifecycle::State & /*previous_state*/) {
    if (build_hat_->init() != tuw_build_hat_driver::BuildHat::SUCCESS) {
        return hardware_interface::CallbackReturn::ERROR;
    }
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn BuildHatHardwareInterface::on_activate(const rclcpp_lifecycle::State & /*previous_state*/) {

    set_state("base_right_wheel_joint/velocity", 0.);
    set_state("base_left_wheel_joint/velocity", 0.);
    set_state("base_right_wheel_joint/position", 0.);
    set_state("base_left_wheel_joint/position", 0.);
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn BuildHatHardwareInterface::on_deactivate(const rclcpp_lifecycle::State & /*previous_state*/) {

    if (build_hat_->deactivate() != tuw_build_hat_driver::BuildHat::SUCCESS) {
        return hardware_interface::CallbackReturn::ERROR;
    }
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type BuildHatHardwareInterface::read(const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/) {
    double pos_left = motor_left_->get_position_cumulative_radian();
    double pos_right = motor_right_->get_position_cumulative_radian();
    double vel_left = motor_left_->get_velocity_radian_per_sec();
    double vel_right = motor_right_->get_velocity_radian_per_sec();
    set_state("base_right_wheel_joint/velocity", vel_left);
    set_state("base_right_wheel_joint/position", pos_left);
    set_state("base_left_wheel_joint/velocity", vel_right);
    set_state("base_left_wheel_joint/position", pos_right);
    return hardware_interface::return_type::OK;
}

hardware_interface::return_type BuildHatHardwareInterface::write(const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/) {

    motor_left_->set_target_radian_per_sec(get_command("base_right_wheel_joint/velocity"));
    motor_right_->set_target_radian_per_sec(get_command("base_left_wheel_joint/velocity"));
    build_hat_->commit();
    return hardware_interface::return_type::OK;
}

} // namespace tuw_build_hat_hardware_interface

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(tuw_build_hat_hardware_interface::BuildHatHardwareInterface, hardware_interface::SystemInterface)
