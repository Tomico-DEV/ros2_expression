"""Expression graph bringup node."""

from lifecycle_msgs.msg import State, Transition
from lifecycle_msgs.srv import ChangeState, GetState

from rcl_interfaces.msg import ParameterDescriptor

import rclpy
from rclpy.node import Node


class LifecycleManager(Node):
    """Lifecycle manager node."""

    def __init__(self):
        """Node constructor."""
        super().__init__('lifecycle_manager')

        ParameterDescriptor()

        self.declare_parameter(
            'node_names',
            ['voicevox_speaker', 'face2d', 'expression_node'],
            ParameterDescriptor(
                description='Nodes to activate'
            )
        )
        self.declare_parameter(
            'autostart',
            True,
            ParameterDescriptor(
                description='Whether or not to start on creation'
            )
        )

        self.node_names = self.get_parameter('node_names').get_parameter_value().string_array_value
        self.autostart = self.get_parameter('autostart').get_parameter_value().bool_value

        self.get_logger().info(f'Managing nodes: {self.node_names}')
        self.current_index = 0  # node index

    def activate_nodes(self):
        """Activate nodes specified in node_names."""
        if not rclpy.ok():
            return
        while self.current_index < len(self.node_names):
            node = self.node_names[self.current_index]
            if not self.is_available(node):
                self.get_logger().info(f"{node}'s services are not available yet")
                continue

            if self.is_active(node):
                self.get_logger().info(f'{node} is already active, skipping..')
                self.current_index += 1
                continue

            self.get_logger().info(f'Configuring and activating {node}...')

            if self.configure_and_activate(node):
                self.get_logger().info(f'Cofigured and activated {node}!')
                self.current_index += 1
            else:
                self.get_logger().error(f'Failed to configure and activate {node}! Retrying..')

        self.get_logger().info('All nodes have been activated!')

    def cleanup_nodes(self):
        """Deactivate and cleanup nodes in the opposite order."""
        if not rclpy.ok():
            return
        self.get_logger().info('Deactivating and cleaning up nodes...')
        for node in reversed(self.node_names):
            if not self.is_available(node):
                self.get_logger().warn(f"{node}'s services are not available, skipping...")
                continue

            if self.is_unconfigured(node):
                self.get_logger().info(f'{node} is already unconfigured, skipping..')
                continue

            self.get_logger().info(f'Deactivating and cleaning up {node}...')

            if self.deactivate_and_cleanup(node):
                self.get_logger().info(f'Deactivated and cleaned up {node}!')
            else:
                self.get_logger().error(f'Failed to deactivate and clean {node}, skipping..')

    def is_available(self, name: str):
        """Check if node 'name' is available."""
        if not rclpy.ok():
            return False
        client = self.create_client(GetState, f'/{name}/get_state')
        return client.wait_for_service(timeout_sec=0.5)

    def is_active(self, name: str):
        """Check if node is already active."""
        if not rclpy.ok():
            return
        client = self.create_client(GetState, f'/{name}/get_state')
        if not client.wait_for_service(timeout_sec=0.5):
            return False
        req = GetState.Request()
        future_result = self.make_req_and_wait(client, req)

        if future_result is None:
            return False

        return future_result.current_state.id == State.PRIMARY_STATE_ACTIVE

    def configure_and_activate(self, name: str):
        """Configure and activate node."""
        if not rclpy.ok():
            return False
        client = self.create_client(ChangeState, f'/{name}/change_state')
        if not client.wait_for_service(timeout_sec=10.0):
            self.get_logger().error(f'{name} change_state not available')
            return False

        def send(transition_id):
            req = ChangeState.Request()
            req.transition.id = transition_id
            future_result = self.make_req_and_wait(client, req)

            return future_result.success

        res1 = send(Transition.TRANSITION_CONFIGURE)  # configure node
        res2 = send(Transition.TRANSITION_ACTIVATE)  # activate node

        return res1 and res2

    def is_unconfigured(self, name: str):
        """Check if node is already unconfigured."""
        if not rclpy.ok():
            return True
        client = self.create_client(GetState, f'/{name}/get_state')
        if not client.wait_for_service(timeout_sec=0.5):
            return False
        req = GetState.Request()
        future_result = self.make_req_and_wait(client, req)

        if future_result is None:
            return False

        return future_result.current_state.id == State.PRIMARY_STATE_UNCONFIGURED

    def deactivate_and_cleanup(self, name: str):
        """Deactivate and clean up node."""
        if not rclpy.ok():
            return True
        client = self.create_client(ChangeState, f'/{name}/change_state')
        if not client.wait_for_service(timeout_sec=10.0):
            self.get_logger().error(f'{name} change_state not available')
            return False

        def send(transition_id):
            req = ChangeState.Request()
            req.transition.id = transition_id
            future_result = self.make_req_and_wait(client, req)

            return future_result.success

        res1 = send(Transition.TRANSITION_DEACTIVATE)  # deactivate node
        res2 = send(Transition.TRANSITION_CLEANUP)  # clean up node

        return res1 and res2

    def make_req_and_wait(self, client, req):
        """Make request and wait for result."""
        try:
            future = client.call_async(req)
            rclpy.spin_until_future_complete(self, future)
            return future.result()
        except (rclpy.exceptions.ROSInterruptException,
                rclpy.executors.ExternalShutdownException):
            return None


def main(args=None):
    """Node entry point."""
    rclpy.init(args=args)
    node = LifecycleManager()
    node.activate_nodes()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info('Keyboard interrupt received, shutting down..')
    else:
        node.cleanup_nodes()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
