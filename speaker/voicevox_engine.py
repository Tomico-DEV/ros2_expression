# Copyright 2025, tomicodev
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

"""speaker voicevox_engine module.

not to be confused with the actual voicevox_engine program
that is responsible for tts
"""
import subprocess
import time

from rclpy.node import Node

from speaker.base_engine import BaseEngine, PhenomeList


class DockerPullError(Exception):
    """Voicevox engine docker image pull error."""


class DockerStartError(Exception):
    """Voicevox engine docker image start error."""


class DockerDead(Exception):
    """Voicevox engine docker image exit error."""


def get_engine():
    """Get engine class."""
    return VoicevoxEngine


class VoicevoxEngine(BaseEngine):
    """ros2 speaker voicevox engine class.

    manages the lifecycle of a voicevox engine docker
    """

    def __init__(self, server: Node):
        """initialize, pull, and start engine image."""
        super().__init__()

        self.server = server
        self.__declare_params()

        self.__log('Getting Voicevox engine docker image...')
        self._pull_docker(
            server.get_parameter('vv_use_gpu').get_parameter_value().bool_value
        )

        self.__log('Starting Voicevox engine...')
        self._start_docker(
            server.get_parameter('vv_port').get_parameter_value().string_value
        )

        self.__docker_p = None

    def __declare_params(self):
        """Attach parameters to server node."""
        self.server.declare_parameter('vv_use_gpu', False)
        self.server.declare_parameter('vv_port', '127.0.0.1:50021:50021')

    def __log(self, text: str):
        """Shorthand for node.get_logger().info() ."""
        self.server.get_logger().info('(vv engine) ' + text)

    def __warn(self, text: str):
        """Shorthand for node.get_logger().warn() ."""
        self.server.get_logger().warn('(vv engine) ' + text)

    def __error(self, text: str):
        """Shorthand for node.get_logger().error() ."""
        self.server.get_logger().error('(vv engine) ' + text)

    def _pull_docker(self, use_gpu: bool = False):
        """Pull voicevox engine's docker image.

        raises an error if the pull failed
        """
        self.image_name = (
            'voicevox/voicevox_engine:cpu-latest'
            if not use_gpu
            else 'voicevox/voicevox_engine:nvidia-latest')
        self.__log(f'use_gpu is {use_gpu}, pulling {self.image_name}')

        pull_docker_p = subprocess.Popen(
            ['docker', 'pull', self.image_name],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        interval = 2  # interval to print message
        try:
            while True:
                ret = pull_docker_p.poll()
                if ret is not None:  # pull finished
                    break
            self.__log('Pulling image....')
            time.sleep(interval)
        except KeyboardInterrupt as exc:
            self.__error('KeyboardInterrupt received, canceling pull..')
            pull_docker_p.terminate()
            pull_docker_p.wait()

            raise KeyboardInterrupt from exc

        __outs, errs = pull_docker_p.communicate()
        if pull_docker_p.returncode == 0:
            self.__log('Image pulled successfully!')
        else:
            self.__error(f'Failed to pull {self.image_name}!')
            self.__error(errs.decode())
            raise DockerPullError('Failed to pull voicevox engine image')

    def _start_docker(self, port: str = '127.0.0.1:50021:50021'):
        """Start docker process."""
        if self.image_name is None:
            self.__warn('image_name is not set (image not pulled?), ')
        self.__docker_p = subprocess.Popen(
            [
                'docker', 'run', '--rm', '-it',
                '-p', port, self.image_name
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        if self._check_docker():
            self.__log('Engine is running!')

    def _stop_docker(self):
        """Terminate docker process."""
        try:
            self._check_docker()
        except DockerDead:
            return
        else:
            self.__docker_p.terminate()
            self.__docker_p.wait()

    def _check_docker(self):
        """Check if process is running.

        returns true if it is
        """
        try:
            assert self.__docker_p.poll() is None, 'docker image is dead'
        except AttributeError as exc:
            raise DockerDead('VV engine process is non-existent!') \
                from exc
        except AssertionError as exc:
            raise DockerDead('VV engine process is dead!') \
                from exc
        else:
            return True

    def speak(self, text: str) -> PhenomeList:
        """Generate and playback audio."""

    def cancel(self):
        """Cancel audio playback."""

    def config(self, **kwargs):
        """Configure engine."""

    def shutdown(self):
        """Terminate voicevox engine process."""
        self._stop_docker()

    def __del__(self):
        """Deinit engine."""
        self._stop_docker()


if __name__ == '__main__':
    pass
