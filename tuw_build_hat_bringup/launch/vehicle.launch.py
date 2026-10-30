import os

from ament_index_python.packages import get_package_share_path
from launch import LaunchDescription
from launch.substitutions import Command, LaunchConfiguration, TextSubstitution
from launch.actions import DeclareLaunchArgument, SetLaunchConfiguration, IncludeLaunchDescription, OpaqueFunction
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.logging import get_logger
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.actions import Node


def generate_launch_description():
    pkg_prefix = 'tuw_build_hat'
    path_description = get_package_share_path(f'{pkg_prefix}_description')
    path_bringup = get_package_share_path(f'{pkg_prefix}_bringup')
    
    robot_controllers = os.path.join(path_bringup, 'config', 'diffdrive_controllers.yaml')


    setup_arg = DeclareLaunchArgument(
        'setup', 
        default_value=TextSubstitution(text=''), 
        description='controllers setup which should be launched (options: gz, hw)')

    serial_port_arg = DeclareLaunchArgument(
        'serial_port', 
        default_value=TextSubstitution(text='/dev/ttyUSB0'), 
        description='serial port used for the build_hat connection')
    
    entity_name_arg = DeclareLaunchArgument(
        'entity_name',
        default_value='r0',
        description=('Top-level namespace. The value will be used to replace the '
                        '<robot_namespace> keyword on the rviz config file.'))

    rviz_launch_arg = DeclareLaunchArgument(
        'rviz_launch',
        default_value='true',
        description='on true it launches rviz')

    def setup_switch(context):
        setup = context.launch_configurations['setup']
        entity_name = context.launch_configurations['entity_name']
        serial_port = context.launch_configurations['serial_port']
        path_ros_gz_sim = get_package_share_path('ros_gz_sim')
        launch_descriptions = LaunchDescription()

        if setup == "":
            get_logger().error("No setup-argument specified! (use \"setup:=[gz|hw]\")")
            exit(1)
        if setup not in ['gz', 'hw']:
            get_logger().error(f"setup \"{setup}\" unknown setup (choose from gz or hw)")
            exit(1)
        
        rviz_config_path = os.path.join(path_bringup, 'config', f'{entity_name}.rviz')
        rviz_config_path_config = SetLaunchConfiguration('rviz_config_path', rviz_config_path)

        launch_descriptions.add_action(rviz_config_path_config)
        
        xacro_path = os.path.join(path_description, 'models', 'lego_diffdrive', 'main.xacro')
        sdf_content = Command(['xacro ', str(xacro_path),
                     ' namespace:=', entity_name,
                     ' serial_port:=', serial_port,
                     ' pkg_prefix:=', pkg_prefix,
                     ' setup:=', setup]);
        robot_description = ParameterValue(sdf_content, value_type=str)

        # dump the generated description for debugging
        with open('/tmp/robot_description.urdf', 'w') as f:
            f.write(sdf_content.perform(context))

        robot_state_publisher_node = Node(
            package="robot_state_publisher",
            executable="robot_state_publisher",
            parameters=[{'robot_description': robot_description},
                        {'use_sim_time': True}],
        )
        launch_descriptions.add_action(robot_state_publisher_node)

        if setup == "gz":
            get_logger().info("Simulation selected! --> spawning model")
            spawn_model_string_config = SetLaunchConfiguration('model_string', sdf_content)
            gz_spawn_model = IncludeLaunchDescription(
                PythonLaunchDescriptionSource(os.path.join(path_ros_gz_sim, 'launch', 'gz_spawn_model.launch.py')),
                launch_arguments={}.items()
            )
            launch_descriptions.add_action(spawn_model_string_config)
            launch_descriptions.add_action(gz_spawn_model)

        if setup == 'hw':
            get_logger().info("Real Hardware selected! --> loading interfaces")
            control_manager_node = Node(
                package="controller_manager",
                executable="ros2_control_node",
                parameters=[robot_controllers],
            )
            launch_descriptions.add_action(control_manager_node)

        diff_drive_controller_spawner = Node(
            package="controller_manager",
            executable="spawner",
            arguments=["diff_drive_controller", "--param-file", robot_controllers,
                    "--controller-ros-args",
                    "-r /diff_drive_controller/cmd_vel:=cmd_vel -p use_sim_time:=true"],
            parameters=[],
        )
        joint_state_broadcaster_spawner = Node(
            package="controller_manager",
            executable="spawner",
            arguments=["joint_state_broadcaster", "--param-file", robot_controllers,
                    "--controller-ros-args",
                    "-r /diff_drive_controller/cmd_vel:=cmd_vel -p use_sim_time:=true"], # this fails when leaving out the remap (???)
            parameters=[],
        )
        launch_descriptions.add_action(diff_drive_controller_spawner)
        launch_descriptions.add_action(joint_state_broadcaster_spawner)



        return [launch_descriptions]

    setup_switch_func = OpaqueFunction(function=setup_switch) 


    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        arguments=["-d", LaunchConfiguration('rviz_config_path')],
        condition=IfCondition(LaunchConfiguration('rviz_launch')),
    )
 
    return LaunchDescription([
        setup_arg,
        entity_name_arg,
        serial_port_arg,
        rviz_launch_arg,
        setup_switch_func,
        rviz_node,
    ])
