"""Expression stack bringup."""
from launch import LaunchDescription

from launch_ros.actions import LifecycleNode, Node


def generate_launch_description():
    """Generate launch description."""
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

    return LaunchDescription([
        speaker_node,
        face_node,
        expression_node,
        manager
    ])
