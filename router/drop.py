import os
from netfilterqueue import NetfilterQueue
import numpy as np
import atexit

os.system("iptables -I FORWARD -j NFQUEUE --queue-num 5")

def drop(packet):
    if np.random.choice([True, False], p=[0.01, 0.99]):
        print("dropping")
        packet.drop()
    else:
        print("accepting")
        packet.accept()

nfqueue = NetfilterQueue()
nfqueue.bind(5, drop, 10000)

def cleanup():
    print("Cleaning up")
    nfqueue.unbind()
    # Remove iptables rule (adjust command as needed)
    os.system("sudo iptables -D INPUT -j NFQUEUE --queue-num 5")

atexit.register(cleanup)

try:
    print("Starting packet processing")
    nfqueue.run()
except KeyboardInterrupt:
    print("Stopping")