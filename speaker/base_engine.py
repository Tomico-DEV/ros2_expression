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


"""Abstract engine class."""
from abc import ABC, abstractmethod

type PhenomeList = list[str]


def get_engine():
    """Get engine class."""
    return BaseEngine


class BaseEngine(ABC):
    """ros2 speaker abstract engine class.

    All tts engines should inherit from this
    (or use duck typing). Offers no implementation
    """

    @abstractmethod
    def speak(self, text: str) -> PhenomeList:
        """Take in text and start audio playback.

        Returns a list of phenomes and their timestamps
        """

    @abstractmethod
    def cancel(self):
        """Stop current audio playback, if any."""

    @abstractmethod
    def config(self, **kwargs):
        """Set engine config."""

    @abstractmethod
    def shutdown(self):
        """Deinit engine."""
