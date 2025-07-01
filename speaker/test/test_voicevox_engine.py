"""Voicevox engine test."""
import time

import pytest

import rclpy
from rclpy.node import Node
from speaker.voicevox_engine import VoicevoxEngine


def test_engine():
    """Test Voicevox engine."""
    rclpy.init()
    node = Node('test_node')

    vv_engine = VoicevoxEngine(node)
    time.sleep(10)
    vv_engine.shutdown()

    rclpy.try_shutdown()


if __name__ == '__main__':
    pytest.main(['-v'])
