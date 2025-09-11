# Copyright 2025 TomicoDEV
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#                    -
#   /\                 \       _________________
#  //\\                 \     /                 \
# //  \\          <<<    |   |  ROS2 EXPRESSION  |
#             <<<<        |   \ ________________/
#          <<             |   |/
#             <<<<        |
#                 <<<    |
#                       /
#                      /
#                    -
#
"""
Voicevox speaker lifecycle launchfile.

Author: TomicoDEV
"""

from launch import LaunchDescription
from launch.actions import EmitEvent, RegisterEventHandler

from launch_ros.actions import LifecycleNode
from launch_ros.event_handlers import OnStateTransition
from launch_ros.events import matches_node_name
from launch_ros.events.lifecycle import ChangeState

from lifecycle_msgs.msg import Transition


def generate_launch_description():
    """Generate launch description."""
    speaker_node = LifecycleNode(
        package='voicevox_speaker',
        executable='speaker_node',
        name='voicevox_speaker',
        namespace='',
        output='screen'
    )

    # request configure immediately after startup
    configure_event = EmitEvent(
        event=ChangeState(
            lifecycle_node_matcher=matches_node_name('voicevox_speaker'),
            transition_id=Transition.TRANSITION_CONFIGURE
        )
    )

    # inactive to active
    activate_handler = RegisterEventHandler(
        OnStateTransition(
            target_lifecycle_node=speaker_node,
            goal_state='inactive',
            entities=[
                EmitEvent(
                    event=ChangeState(
                        lifecycle_node_matcher=matches_node_name('voicevox_speaker'),
                        transition_id=Transition.TRANSITION_ACTIVATE
                    )
                )
            ]
        )
    )

    return LaunchDescription([
        speaker_node,
        configure_event,
        activate_handler,
    ])
