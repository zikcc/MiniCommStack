import socket, threading, time

def worker(id):
    s = socket.create_connection(('127.0.0.1', 8888))
    for i in range(100):
        msg = f"hello from {id}-{i}"
        # 构造你的自定义 Packet bytes
        s.send(build_packet_bytes(msg))
        data = s.recv(4096)
        # parse_packet(data)
    s.close()

threads = []
t0 = time.time()
for i in range(200):
    t = threading.Thread(target=worker, args=(i,))
    t.start()
    threads.append(t)

for t in threads:
    t.join()
print("总耗时:", time.time() - t0)
