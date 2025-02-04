import socket
import cv2
import numpy as np

cam_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
cam_sock.connect(("192.168.4.1", 8000))  # 替换为实际IP
print("got IP")

while True:
    print("reading")
    # 读取帧长度头
    header = cam_sock.recv(4)
    if len(header) != 4: break
    length = int.from_bytes(header, byteorder='little')

    # 读取JPEG数据
    img_data = b''
    while len(img_data) < length:
        packet = cam_sock.recv(length - len(img_data))
        if not packet: break
        img_data += packet

    # 解码显示
    img = cv2.imdecode(np.frombuffer(img_data, np.uint8), cv2.IMREAD_COLOR)
    cv2.imshow('ESP32-CAM', img)
    if cv2.waitKey(1) == 27:  # ESC退出
        break

cv2.destroyAllWindows()
cam_sock.close()