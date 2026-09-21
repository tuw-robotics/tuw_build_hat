#ifndef TUW_BUILD_HAT_HARDWARE_INTERFACE__BUILD_HAT_HARDWARE_INTERFACE_HPP
#define TUW_BUILD_HAT_HARDWARE_INTERFACE__BUILD_HAT_HARDWARE_INTERFACE_HPP


#include "hardware_interface/system_interface.hpp"

#include "tuw_build_hat_driver/build_hat.hpp"
#include "tuw_build_hat_driver/motor.hpp"


namespace tuw_build_hat_hardware_interface {

    class BuildHatHardwareInterface : public hardware_interface::SystemInterface {
        public:
            // Lifecycle node override
            hardware_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State &previous_state) override;
            hardware_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State &previous_state) override;
            hardware_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State &previous_state) override;

            // SystemInterface override
            hardware_interface::CallbackReturn on_init(const hardware_interface::HardwareComponentInterfaceParams & params) override;
            hardware_interface::return_type read(const rclcpp::Time &time, const rclcpp::Duration &period) override;
            hardware_interface::return_type write(const rclcpp::Time &time, const rclcpp::Duration &period) override;

        private:
            //hardware_interface::HardwareInfo info_;
            std::shared_ptr<tuw_build_hat_driver::BuildHat> build_hat_;
            std::shared_ptr<tuw_build_hat_driver::Motor> motor_left_;
            std::shared_ptr<tuw_build_hat_driver::Motor> motor_right_;
            std::string serial_port_;
            std::string filename_firmware_;
            std::string filename_signature_;
            std::string filename_serial_log_;
            std::string filename_msgs_log_;
            std::string joint_left_;
            std::string joint_right_;
            int msgs_loglevel_;

    };


}  // namespace tuw_build_hat_hardware_interface
#endif // TUW_BUILD_HAT_HARDWARE_INTERFACE__BUILD_HAT_HARDWARE_INTERFACE_HPP