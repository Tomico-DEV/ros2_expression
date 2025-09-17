"""Expression stack bringup."""
from launch import LaunchDescription
from launch.substitutions import PathJoinSubstitution

from launch_ros.actions import LifecycleNode, Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    """Generate launch description."""
    this_pkg = FindPackageShare('expression_launch')
    
    speaker_node = LifecycleNode(
        package='voicevox_speaker',
        executable='speaker_node',
        name='speaker_node',
        namespace='',
        output='screen'
    )

    face_node = LifecycleNode(
        package='face2d',
        executable='face2d',
        name='face_node',
        namespace='',
        output='screen'
    )

    expression_node = LifecycleNode(
        package='expression',
        executable='expression_node',
        name='expression_node',
        namespace='',
        output='screen'
    )

    manager = Node(
        package='expression_launch',
        executable='lifecycle_manager',
        parameters=[{
            'node_names': ['speaker_node', 'face_node', 'expression_node'],
            'autostart': True
        }],
        output='screen'
    )

    base_link_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_transform_publisher',
        arguments=['0', '0', '0', '0', '0', '0', '1', 'world', 'base_link'],
    )

    face_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_transform_publisher',
        arguments=['0', '0', '0.5', '0', '0', '0', '1', 'base_link', 'face'],
    )

    gaze_server = Node(
        package='interactive_gaze',
        executable='gaze_server',
        name='gaze_server',
        output='screen'
    )

    rviz_config = PathJoinSubstitution([this_pkg, 'config', 'visualize.rviz'])
    rviz2 = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['d', rviz_config],
        output='screen'
    )

    return LaunchDescription([
        base_link_tf,
        face_tf,
        gaze_server,
        rviz2,
        speaker_node,
        face_node,
        expression_node,
        manager
    ])
