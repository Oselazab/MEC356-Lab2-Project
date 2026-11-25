# ESP32 USB setup

The NodeMCU-32S firmware uses GPIO25 and GPIO26 for 50 Hz PWM. Upload [`esp32_arm_servos.ino`](firmware/esp32_arm_servos/esp32_arm_servos.ino) with Arduino IDE and the Arduino ESP32 3.x core. The sketch accepts newline-terminated ASCII packets at **115200 baud, 8N1** and prints targets and PWM readback. For an unplugged bench check, send `122,113,2000` with **Newline** selected in Arduino Serial Monitor.

## Native Ubuntu

Close Arduino Serial Monitor, then run from a built and sourced workspace:

```bash
ros2 launch mec356_arm lab.launch.py hardware:=true port:=/dev/ttyUSB0
```

Use the device assigned to your board, such as `/dev/ttyACM0` on native USB hardware. The bridge prints the board's replies in the launch terminal. Allow at least 2.5 seconds after it connects before sending a trajectory.

## Windows and WSL with Arduino IDE Serial Monitor

In this configuration Arduino IDE owns the USB port. ROS sends the same position packets through the IDE's local monitor WebSocket. The ESP32 prints its replies in Arduino IDE, and the relay also forwards them to ROS. The verified setup used Arduino IDE 2.3.8 on Windows and Ubuntu 24.04 in WSL2.

1. Open Arduino IDE's **Serial Monitor** for the ESP32 on its Windows COM port at **115200 baud**. Keep the panel open.
2. In WSL, run `python3 esp32/wsl_serial_relay.py` from the cloned project. It prints a relay-ready message and creates `/tmp/mec356-esp32-serial`. Windows must be able to reach the WSL loopback listener on port 43562.
3. Find the Arduino IDE monitor WebSocket port. With the IDE connected, compare the loopback connections in `netstat -ano` immediately before and after opening the monitor panel. Use the new Arduino IDE backend listener that has an established connection from the IDE frontend. This port is assigned dynamically.
4. In Windows PowerShell, from the cloned project, install the Python dependency and run the relay. Replace `MONITOR_PORT` with the number identified in step 3.

   ```powershell
   python -m pip install "websockets>=15"
   python esp32/arduino_ide_usb_relay.py MONITOR_PORT
   ```

5. In another WSL terminal, source ROS and the built workspace and launch the project.

   ```bash
   ros2 launch mec356_arm lab.launch.py hardware:=true port:=/tmp/mec356-esp32-serial
   ```

6. Wait for the controllers and bridge, then use the slider GUI or send a test pose from another sourced terminal.

   ```bash
   ros2 run mec356_arm send_pose 0.55 0.40 0.25 -0.20 --seconds 2 --subscribers 2
   ```

Gazebo and RViz2 show the pose. Arduino IDE shows the two target degrees followed by the PWM readback.

```text
ESP32 target: GPIO25=122 deg GPIO26=113 deg duration=2000 ms
ESP32 PWM applied: GPIO25 duty=5498 at 50 Hz; GPIO26 duty=5335 at 50 Hz
```

If the monitor is reopened and gets a new WebSocket port, stop both relays and restart them with that port. If an interrupted WSL relay left a stale `/tmp/mec356-esp32-serial` symlink, remove the symlink after confirming no relay is running. For a return to zero, use **Send home** or the zero-pose ROS command.

## Diagnostic sketches

The [serial parser](firmware/esp32_serial_monitor/esp32_serial_monitor.ino) reports incoming CSV targets. The [standalone PWM demo](firmware/esp32_servo_demo/esp32_servo_demo.ino) runs a short 50 Hz pulse sequence on GPIO25 and GPIO26 when it receives `D`. Use the main arm sketch for ROS integration.
