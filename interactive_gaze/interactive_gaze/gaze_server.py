"""Interactive gaze server."""
import rclpy
from rclpy.node import Node

from math import sqrt

from visualization_msgs.msg import InteractiveMarkerControl, InteractiveMarker
from visualization_msgs.msg import Marker

from interactive_markers.interactive_marker_server import InteractiveMarkerServer

from geometry_msgs.msg import TransformStamped

from tf2_ros import TransformBroadcaster


class GazeMarker(Node):
    """Gaze marker server node."""

    def __init__(self):
        super().__init__('gaze_marker')

        self.tf_broadcaster = TransformBroadcaster(self)
        self.marker_server = InteractiveMarkerServer(self, 'gaze')

        self.sphere_scale = 0.1

        self.make_marker()
        
    def make_marker(self):
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

        # set marker's pose to local
        self.marker_server.setPose('gaze', pose)
        self.marker_server.setPose('gaze_rot', pose)
        self.marker_server.applyChanges()

        t = TransformStamped()
        t.header.stamp = self.get_clock().now().to_msg()
        t.header.frame_id = 'base_link'
        t.child_frame_id = 'gaze'

        t.transform.translation.x = pose.position.x
        t.transform.translation.y = pose.position.y
        t.transform.translation.z = pose.position.z
        t.transform.rotation = pose.orientation

        self.tf_broadcaster.sendTransform(t)


def main(args=None):
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
