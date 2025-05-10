# Copyright 2025, Past Time Engineers
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
"""ros2_speaker voice server."""

import importlib

import rclpy
from rclpy.node import Node

from ros2_speaker.base_engine import BaseEngine


class VoiceServer(Node):
    """voice server node."""

    def __init__(self, engine: str = 'ros2_speaker.voicevox_engine'):
        """Initialize voice server."""
        super().__init__('voice_server')

        # init and start engine
        self.get_logger().info('Starting server..')
        engine_module = importlib.import_module(engine)
        engine_class = engine_module.get_engine()
        self._engine: BaseEngine = engine_class(self)


def main():
    """Voice server main function."""
    rclpy.init()
    voice_server = VoiceServer()
    try:
        rclpy.spin(voice_server)
    except KeyboardInterrupt:
        print('Keyboard Interrupt, stopping voice server..')
    voice_server.destroy_node()
    rclpy.try_shutdown()


if __name__ == '__main__':
    main()
