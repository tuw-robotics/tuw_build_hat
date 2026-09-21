#include "tuw_build_hat_hardware_interface/build_hat_hardware_interface.hpp"
#include <ament_index_cpp/get_package_share_path.hpp>

namespace tuw_build_hat_hardware_interface {

static constexpr char FIRMWARE_FILENAME[] = "firmware.bin";
static constexpr char SIGNATURE_FILENAME[] = "signature.bin";

hardware_interface::CallbackReturn BuildHatHardwareInterface::on_init(const hardware_interface::HardwareComponentInterfaceParams &params) {
    if (hardware_interface::SystemInterface::on_init(params) != hardware_interface::CallbackReturn::SUCCESS) {
        return hardware_interface::CallbackReturn::ERROR;
    }

    if (params.hardware_info.joints.size() != 2) {
        RCLCPP_ERROR(get_logger(), "Not both joints specified");
        return CallbackReturn::ERROR;
    }
    std::string joint_right = params.hardware_info.joints[0].name;
    std::string joint_left = params.hardware_info.joints[1].name;
    RCLCPP_INFO(get_logger(), "Left joint name: %s", joint_left.c_str());
    RCLCPP_INFO(get_logger(), "Right joint name: %s", joint_right.c_str());


    shm_velocity_left_ = joint_left + "/velocity";
    shm_velocity_right_ = joint_right + "/velocity";
    shm_position_left_ = joint_left + "/position";
    shm_position_right_ = joint_right + "/position";
    

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

    kp_ = 0.003, ki_ = 0.01, kd_ = 0.0;
    if (info_.hardware_parameters.count("kp")) {
        kp_ = std::stod(info_.hardware_parameters["kp"]);
    }
    if (info_.hardware_parameters.count("ki")) {
        ki_ = std::stod(info_.hardware_parameters["ki"]);
    }
    if (info_.hardware_parameters.count("kd")) {
        kd_ = std::stod(info_.hardware_parameters["kd"]);
    }
    RCLCPP_INFO(get_logger(), "PID Parameters are: %4.3lf, %4.3lf, %4.3lf", kp_, ki_, kd_);


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

    set_state(shm_velocity_left_, 0.);
    set_state(shm_velocity_right_, 0.);
    set_state(shm_position_left_, 0.);
    set_state(shm_position_right_, 0.);
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn BuildHatHardwareInterface::on_deactivate(const rclcpp_lifecycle::State & /*previous_state*/) {

    if (build_hat_->deactivate() != tuw_build_hat_driver::BuildHat::SUCCESS) {
        return hardware_interface::CallbackReturn::ERROR;
    }
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type BuildHatHardwareInterface::read(const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/) {
    set_state(shm_velocity_left_, -motor_left_->get_velocity_radian_per_sec());
    set_state(shm_velocity_right_, motor_right_->get_velocity_radian_per_sec());
    set_state(shm_position_left_, -motor_left_->get_position_cumulative_radian());
    set_state(shm_position_right_, motor_right_->get_position_cumulative_radian());
    return hardware_interface::return_type::OK;
}

hardware_interface::return_type BuildHatHardwareInterface::write(const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/) {
    
    motor_left_->set_target_radian_per_sec(-get_command(shm_velocity_left_), kp_, ki_, kd_);
    motor_right_->set_target_radian_per_sec(get_command(shm_velocity_right_), kp_, ki_, kd_);
    build_hat_->commit();
    return hardware_interface::return_type::OK;
}

} // namespace tuw_build_hat_hardware_interface

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(tuw_build_hat_hardware_interface::BuildHatHardwareInterface, hardware_interface::SystemInterface)
