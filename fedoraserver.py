import socket
import threading
from datetime import datetime
import time

host = "0.0.0.0"
port = 4311

players = []
addrlist = []

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_data(newdata, sender):
    global players

    if newdata == "":
        return
    data = newdata
    if data == "ALIVE":
        return
    try:
        command = newdata.split(":")[0]
        args = newdata.split(":")[1]
    except:
        s.sendto("Unknown command / Invalid format".encode("utf-8"), sender)
        return

    if args == "":
        s.sendto("Unknown command / Invalid format".encode("utf-8"), sender)
        return

    if command == "SPAWN":
        pass
    elif command == "LOAD":
        for player in players:
            if player.addr != sender:
                data = "SPAWN:"+str(player.x)+","+str(player.y)+","+str(player.addr[0])+"-"+str(player.addr[1])
                s.sendto(data.encode("utf-8"), sender)
        return
    elif command == "MOV":

        x = int(args.split(",")[0])
        y = int(args.split(",")[1])
        for player in players:
            if player.addr == sender:
                if x - player.x > 3 or x - player.x < -3:
                    player.x = x
                elif y - player.y > 3 or y - player.y < -3:
                    player.y = y
                else:
                    data = ""
                    return
        data = data + "," + str(sender[0]) + "-" + str(sender[1])
    elif command == "DESPAWN":
        for player in players:
            if player.addr == sender:
                players.remove(player)
        for addr in addrlist:
            if addr == sender:
                addrlist.remove(sender)
    else:
        s.sendto("Unknown command / Invalid format".encode("utf-8"), sender)
        return

    for player in players:
        if player.addr != sender:
            s.sendto(data.encode("utf-8"), player.addr)


class Player:
    def __init__(self, addr, y,x, last_time):
        self.x = x
        self.y = y
        self.addr = addr
        self.last_time = last_time

def spam_check(addr):
    clients = 0
    for adr in addrlist:
        if adr[0] == addr[0]:
            clients += 1
    return clients

def check_alive(): # Checks each connection is still alive every 3 seconds by comparing last transaction time
    global players
    while True:
        time.sleep(3)
        now = datetime.now()
        for player in players:
            if (now-player.last_time).total_seconds() > 10:
                print(f"Player {player.addr[0]} timed out / Disconnected")
                adr = str(player.addr[0]) + "-" + str(player.addr[1])
                send_data(f"DESPAWN:{adr}",player.addr)


s.bind((host, port))

print("Server has started")

t1 = threading.Thread(target=check_alive, args=[])
t1.start()

while True:
    data, addr = s.recvfrom(1024)
    data = data.decode("utf-8")
    if not addr in addrlist:
        if spam_check(addr) < 10:
            if data == "SPAWN":
                print("Client connected from ", addr)
                s.sendto("SUCCESS".encode("utf-8"), addr)
                x = Player(addr,0,0,datetime.now())
                players.append(x)
                addrlist.append(addr)
                adr = str(addr[0]) + "-" + str(addr[1])
                send_data(f"SPAWN:0,0,{adr}", addr)
                send_data("LOAD:0,0",addr)
            else:
                s.sendto("Unknown command".encode("utf-8"), addr)
    else:
        send_data(data, addr)
        for player in players:
            if player.addr == addr:
                player.last_time = datetime.now()

