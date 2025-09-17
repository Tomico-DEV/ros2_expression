"""Interactive gaze server."""

from geometry_msgs.msg import TransformStamped

from interactive_markers.interactive_marker_server import InteractiveMarkerServer

import rclpy
from rclpy.node import Node

from tf2_ros import TransformBroadcaster

from visualization_msgs.msg import InteractiveMarker, InteractiveMarkerControl
from visualization_msgs.msg import Marker


class GazeMarker(Node):
    """Gaze marker server node."""

    def __init__(self):
        """Create tf broadcaster and server."""
        super().__init__('gaze_marker')

        self.tf_broadcaster = TransformBroadcaster(self)
        self.marker_server = InteractiveMarkerServer(self, 'gaze')

        self.sphere_scale = 0.1

        self.current_pose = None

        self._make_marker()

        self.timer = self.create_timer(1.0 / 30.0, self._broadcast_tf)

    def _make_marker(self):
        int_marker = InteractiveMarker()
        int_marker.header.frame_id = 'base_link'
        int_marker.name = 'gaze'
        int_marker.description = 'Drag to move gaze'
        int_marker.scale = 0.4

        sphere = Marker()
        sphere.type = Marker.SPHERE

        sphere.scale.x = self.sphere_scale
        sphere.scale.y = self.sphere_scale
        sphere.scale.z = self.sphere_scale
        sphere.color.r = 0.0
        sphere.color.g = 1.0
        sphere.color.b = 0.0
        sphere.color.a = 1.0

        sphere_control = InteractiveMarkerControl()
        sphere_control.always_visible = True
        sphere_control.markers.append(sphere)
        int_marker.controls.append(sphere_control)

        int_marker.controls.append(GazeMarker._plane_control(1, 0, 1, 0))
        int_marker.controls.append(GazeMarker._plane_control(1, 1, 0, 0))

        rot_marker = InteractiveMarker()
        rot_marker.header.frame_id = 'base_link'
        rot_marker.name = 'gaze_rot'
        rot_marker.scale = 0.6

        control_rotate_z = InteractiveMarkerControl()
        control_rotate_z.interaction_mode = InteractiveMarkerControl.ROTATE_AXIS
        control_rotate_z.orientation.w = 1.0
        control_rotate_z.orientation.x = 0.0
        control_rotate_z.orientation.y = 1.0
        control_rotate_z.orientation.z = 0.0
        rot_marker.controls.append(control_rotate_z)

        self.marker_server.insert(int_marker, feedback_callback=self._feedback_cb)
        self.marker_server.insert(rot_marker, feedback_callback=self._feedback_cb)
        self.marker_server.applyChanges()

    @staticmethod
    def _plane_control(w, x, y, z):
        control_plane = InteractiveMarkerControl()
        control_plane.orientation.w = float(w)
        control_plane.orientation.x = float(x)
        control_plane.orientation.y = float(y)
        control_plane.orientation.z = float(z)
        control_plane.interaction_mode = InteractiveMarkerControl.MOVE_PLANE

        return control_plane

    def _feedback_cb(self, feedback):
        pose = feedback.pose
        self.current_pose = pose

        # set marker's pose to local
        self.marker_server.setPose('gaze', pose)
        self.marker_server.setPose('gaze_rot', pose)
        self.marker_server.applyChanges()

    def _broadcast_tf(self):
        if self.current_pose is None:
            return

        t = TransformStamped()
        t.header.stamp = self.get_clock().now().to_msg()
        t.header.frame_id = 'base_link'
        t.child_frame_id = 'gaze/0'

        t.transform.translation.x = self.current_pose.position.x
        t.transform.translation.y = self.current_pose.position.y
        t.transform.translation.z = self.current_pose.position.z
        t.transform.rotation = self.current_pose.orientation

        self.tf_broadcaster.sendTransform(t)


def main(args=None):
    """Start gaze server."""
    rclpy.init(args=args)
    gaze_server = GazeMarker()
    try:
        rclpy.spin(gaze_server)
    except KeyboardInterrupt:
        print('Keyboard interrupt, exiting..')
    else:
        gaze_server.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
