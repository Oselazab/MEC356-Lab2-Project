"""Forward ROS serial packets through Arduino IDE's open Serial Monitor."""
import argparse
import asyncio
import json
import re

from websockets.asyncio.client import connect


async def send_commands(reader, monitor):
    while packet := await reader.readline():
        if not re.fullmatch(rb"\d{1,3},\d{1,3},\d{1,5}\n", packet):
            raise ValueError(f"Invalid Lab 2 servo packet: {packet!r}")
        await monitor.send(json.dumps({"command": "SEND_MESSAGE", "data": packet.decode("ascii")}))


async def return_feedback(monitor, writer):
    async for message in monitor:
        reply = json.loads(message)
        if isinstance(reply, list):
            for fragment in reply:
                writer.write(fragment.encode("utf-8"))
            await writer.drain()


async def main(monitor_port):
    async with connect(f"ws://localhost:{monitor_port}", proxy=None) as monitor:
        reader, writer = await asyncio.open_connection("127.0.0.1", 43562)
        print("Connected to the Arduino IDE monitor and ROS relay.", flush=True)
        tasks = [
            asyncio.create_task(send_commands(reader, monitor)),
            asyncio.create_task(return_feedback(monitor, writer)),
        ]
        try:
            done, _ = await asyncio.wait(tasks, return_when=asyncio.FIRST_COMPLETED)
            for task in done:
                task.result()
        finally:
            for task in tasks:
                task.cancel()
            await asyncio.gather(*tasks, return_exceptions=True)
            writer.close()
            await writer.wait_closed()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("monitor_port", type=int, help="Arduino IDE's local monitor WebSocket port")
    asyncio.run(main(parser.parse_args().monitor_port))
