#!/usr/bin/env python3
"""Manual controls for the Lab 2 arm and its two servo channels."""
import tkinter as tk
from tkinter import ttk

import rclpy
from trajectory_msgs.msg import JointTrajectory, JointTrajectoryPoint

TOPIC = '/arm_controller/joint_trajectory'
JOINTS = ['joint1', 'joint2', 'joint3', 'joint4']
LIMITS = [(-1.2, 1.2), (-0.25, 1.2), (-0.2, 1.2), (-0.35, 1.2)]
LABELS = ['Base yaw', 'Shoulder pitch', 'Elbow pitch', 'Wrist pitch']


def main():
    rclpy.init()
    node = rclpy.create_node('lab2_arm_gui')
    publisher = node.create_publisher(JointTrajectory, TOPIC, 10)
    try:
        window = tk.Tk()
        window.title('MEC356 Lab 2 Arm Control')
        panel = ttk.Frame(window, padding=16)
        panel.grid(sticky='nsew')
        angles = [tk.DoubleVar(window, 0) for _ in JOINTS]
        duration = tk.DoubleVar(window, 2)
        status = tk.StringVar(window, 'Waiting for a trajectory subscriber')

        def send():
            if not publisher.get_subscription_count():
                status.set('No subscriber. Launch Lab 2 and wait for the controllers.')
                return
            seconds = duration.get()
            point = JointTrajectoryPoint(positions=[round(a.get(), 2) for a in angles])
            point.time_from_start.sec, point.time_from_start.nanosec = divmod(
                round(seconds * 1e9), 10**9)
            publisher.publish(JointTrajectory(joint_names=JOINTS, points=[point]))
            status.set(f'Sent {point.positions} rad over {seconds:.1f} s')

        def home():
            for angle in angles:
                angle.set(0)
            send()

        def spin():
            if rclpy.ok():
                rclpy.spin_once(node, timeout_sec=0)
                window.after(40, spin)
            else:
                window.destroy()

        for row, (label, angle, bounds) in enumerate(zip(LABELS, angles, LIMITS)):
            ttk.Label(panel, text=label).grid(row=row, column=0, sticky='w')
            tk.Scale(panel, variable=angle, from_=bounds[0], to=bounds[1],
                     resolution=0.01, length=270, orient='horizontal').grid(row=row, column=1)
        ttk.Label(panel, text='Duration (s)').grid(row=4, column=0, sticky='w')
        tk.Scale(panel, variable=duration, from_=1.25, to=10, resolution=0.25,
                 length=270, orient='horizontal').grid(row=4, column=1)
        ttk.Button(panel, text='Send pose', command=send).grid(row=5, column=0, sticky='w')
        ttk.Button(panel, text='Send home', command=home).grid(row=5, column=1, sticky='w')
        ttk.Label(panel, textvariable=status).grid(row=6, columnspan=2, pady=(12, 0), sticky='w')
        status.set('Set joint angles in radians, then send the pose.')
        window.after(0, spin)
        window.mainloop()
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
