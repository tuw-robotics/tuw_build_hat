import os

from ament_index_python.packages import get_package_share_path
from launch import LaunchDescription
from launch.substitutions import Command, TextSubstitution
from launch.actions import DeclareLaunchArgument, SetLaunchConfiguration, IncludeLaunchDescription, OpaqueFunction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.logging import get_logger
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.actions import Node


def generate_launch_description():
    robot_description_path = get_package_share_path('tuw_build_hat_description')
    robot_bringup_path = get_package_share_path('tuw_build_hat_bringup')
    
    rviz_config_path = os.path.join(robot_description_path, 'rviz', 'urdf_config.rviz')
    robot_controllers = os.path.join(robot_bringup_path, 'config', 'diffdrive_controllers.yaml')


    setup_arg = DeclareLaunchArgument('setup', 
                default_value=TextSubstitution(text=''), 
                description='controllers setup which should be launched (options: hw, gz, gz_running)')

    # Return a set of launch entities to
    #   publish urdf for configuration of robot for ros2_control to consume 
    #   AND, dedepending on configuration:
    #   - hw: bring up control manager to orchestrate upcoming nodes
    #   - gz: spawn robot using gz_spawn_model.launch.py, inheriting all launch parameters
    #       --> this causes gazebo to create the control manager
    #   - gz_running: do nothing (assuming that there is already a running control manager)

    def setup_switch(context):
        setup = context.launch_configurations['setup']
        
        if setup == 'gz' or setup == 'gz_running':
            urdf_path = os.path.join(robot_description_path, 'models', 'diffdrive_base', 'main_gz.xacro')
        else:
            urdf_path = os.path.join(robot_description_path, 'models', 'diffdrive_base', 'main_hw.xacro')
        robot_description = ParameterValue(Command(['xacro ', str(urdf_path)]), value_type=str)
        
        if setup == "":
            get_logger().error("No setup-argument specified! (use \"setup:=[hw|gz|gz_running]\")")
            exit(1)
        if setup not in ['hw', 'gz', 'gz_running']:
            get_logger().error(f"setup \"{setup}\" unknown (choose from hw, gz or gz_running)")
            exit(1)

        robot_state_publisher_node = Node(
            package="robot_state_publisher",
            executable="robot_state_publisher",
            parameters=[{'robot_description': robot_description},
                        {'use_sim_time': True}],
        )
        launch_descriptions = [robot_state_publisher_node]
        if setup == 'hw':
            control_manager_node = Node(
                package="controller_manager",
                executable="ros2_control_node",
                parameters=[robot_controllers],
            )
            launch_descriptions.append(control_manager_node)
        elif setup == "gz":
            spawn_model_string_config = SetLaunchConfiguration('model_string', Command(['xacro ', str(urdf_path)]))
            gz_spawn_model = IncludeLaunchDescription(
                PythonLaunchDescriptionSource(os.path.join(get_package_share_path('ros_gz_sim'), 'launch', 'gz_spawn_model.launch.py')),
                launch_arguments={}.items()
            )
            launch_descriptions.append(spawn_model_string_config)
            launch_descriptions.append(gz_spawn_model)
        return launch_descriptions

    setup_switch_func = OpaqueFunction(function=setup_switch) 
    
    # let manager spawn the controller node
    diff_drive_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["diff_drive_controller", "--param-file", robot_controllers,
                   "--controller-ros-args",
                   "-r /diff_drive_controller/cmd_vel:=cmd_vel -p use_sim_time:=true"],
        parameters=[],
    )
    
    # let manager spawn joint state broadcaster
    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "--param-file", robot_controllers,
                   "--controller-ros-args",
                   "-r /diff_drive_controller/cmd_vel:=cmd_vel -p use_sim_time:=true"], # this fails when leaving out the remap (???)
        parameters=[],
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        arguments=["-d", rviz_config_path],
    )
 
    return LaunchDescription([
        setup_arg,
        setup_switch_func,
        joint_state_broadcaster_spawner,
        diff_drive_controller_spawner,
        rviz_node,
    ])
