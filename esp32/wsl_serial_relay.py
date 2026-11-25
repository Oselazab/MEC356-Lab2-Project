import os
import pty
import select
import socket
import tty

master, slave = pty.openpty()
tty.setraw(slave)
os.symlink(os.ttyname(slave), "/tmp/mec356-esp32-serial")
try:
    with socket.socket() as server:
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind(("127.0.0.1", 43562))
        server.listen(1)
        print("Relay ready", flush=True)
        client, _ = server.accept()
        with client:
            print("Windows serial connected", flush=True)
            while True:
                ready, _, _ = select.select([master, client], [], [])
                if master in ready:
                    client.sendall(os.read(master, 4096))
                if client in ready:
                    data = client.recv(4096)
                    if not data:
                        break
                    os.write(master, data)
finally:
    os.unlink("/tmp/mec356-esp32-serial")
