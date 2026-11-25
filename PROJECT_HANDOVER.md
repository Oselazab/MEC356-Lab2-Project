# Lab 2 verification

| Check | Recorded result |
| --- | --- |
| ROS 2 Jazzy workspace | Colcon build passed on Ubuntu 24.04 in WSL. |
| Gazebo controller and state feedback | Active controllers and measured targets of approximately `[0.55, 0.40, 0.25, -0.20]` rad. A home command returned all four joints to zero. |
| RViz2 | Followed Gazebo's measured joint states during the recorded motion. |
| Slider GUI | A live **Send pose** action on the base and shoulder sliders moved Gazebo and changed both ESP32 targets. |
| Arduino IDE monitor | One ROS command moved Gazebo and showed 122° and 113° in Arduino IDE over the same USB connection. |
| ESP32 PWM | Both channels reported 50 Hz. Home targeted 90° on both. |
| Physical servo movement | Pending. The servos were disconnected during the laptop demo. |
| Course submission | The assignment calls for submitting the GitHub URL through a separate course form. |

[Gazebo and RViz2 recording](docs/simulation/gazebo-esp32-demo.mp4) · [Arduino IDE Serial Monitor proof](docs/simulation/arduino-serial-monitor-proof.jpg)

The recording shows Gazebo and RViz2 during a synchronized ESP32 run. The Arduino IDE screenshot was captured during a later USB-only run with the IDE providing the serial connection. PWM verification used ESP32 register readback. The electrical waveform and physical servo travel were outside the test setup.
