from pathlib import Path
from xml.sax.saxutils import escape

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition, UnlessCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    share = Path(get_package_share_directory('mec356_arm'))
    gazebo = Path(get_package_share_directory('ros_gz_sim'))
    world = share / 'worlds' / 'lab.sdf'
    description = (share / 'urdf' / 'arm.urdf').read_text().replace(
        'CONTROLLER_CONFIG_PATH', escape(str(share / 'config' / 'controllers.yaml')))
    headless = LaunchConfiguration('headless')

    def simulation(args, condition):
        return IncludeLaunchDescription(
            PythonLaunchDescriptionSource(str(gazebo / 'launch' / 'gz_sim.launch.py')),
            launch_arguments={'gz_args': f'{args} "{world}"'}.items(),
            condition=condition)

    return LaunchDescription([
        DeclareLaunchArgument('headless', default_value='false'),
        DeclareLaunchArgument('hardware', default_value='false'),
        DeclareLaunchArgument('port', default_value='/dev/ttyUSB0'),
        simulation('-r -s', IfCondition(headless)),
        simulation('-r', UnlessCondition(headless)),
        Node(package='ros_gz_bridge', executable='parameter_bridge',
             arguments=['/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock']),
        Node(package='robot_state_publisher', executable='robot_state_publisher',
             parameters=[{'robot_description': ParameterValue(description, value_type=str),
                          'use_sim_time': True}]),
        Node(package='ros_gz_sim', executable='create',
             arguments=['-world', 'lab', '-name', 'mec356_arm', '-topic', '/robot_description']),
        Node(package='controller_manager', executable='spawner',
             arguments=['joint_state_broadcaster', 'arm_controller',
                        '--controller-manager-timeout', '60']),
        Node(package='rviz2', executable='rviz2', condition=UnlessCondition(headless),
             arguments=['-d', str(share / 'config' / 'arm.rviz')],
             parameters=[{'use_sim_time': True}]),
        Node(package='mec356_arm', executable='servo_bridge',
             parameters=[{'port': LaunchConfiguration('port')}],
             condition=IfCondition(LaunchConfiguration('hardware'))),
    ])
