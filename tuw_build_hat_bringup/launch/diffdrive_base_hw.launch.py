import os

from ament_index_python.packages import get_package_share_path
from launch import LaunchDescription
from launch.substitutions import Command, FileContent, TextSubstitution
from launch.actions import DeclareLaunchArgument
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.actions import Node


def generate_launch_description():
    robot_description_path = get_package_share_path('tuw_build_hat_description')
    robot_bringup_path = get_package_share_path('tuw_build_hat_bringup')
    
    urdf_path = os.path.join(robot_description_path, 'models', 'diffdrive_base', 'main.xacro')
    rviz_config_path = os.path.join(robot_description_path, 'rviz', 'urdf_config.rviz')
    robot_description = ParameterValue(Command(['xacro ', str(urdf_path)]), value_type=str)
    robot_controllers = os.path.join(robot_bringup_path, 'config', 'diffdrive_controllers.yaml')


    # publish urdf for configuration of robot for ros2_control to consume
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{'robot_description': robot_description}],
    )

    control_manager_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[robot_controllers],
    )

    # let manager spawn the controller node
    diff_drive_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["diff_drive_controller", "--param-file", robot_controllers],
        parameters=[],
    )
    
    # let manager spawn joint state broadcaster
    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "--param-file", robot_controllers],
        parameters=[],
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        arguments=["-d", rviz_config_path],
    )
 
    return LaunchDescription([
        robot_state_publisher_node,
        control_manager_node,
        joint_state_broadcaster_spawner,
        diff_drive_controller_spawner,
        rviz_node,
    ])
