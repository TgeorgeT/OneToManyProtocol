import os
from netfilterqueue import NetfilterQueue
import numpy as np

os.system("iptables -I FORWARD -j NFQUEUE --queue-num 5")

def drop(packet):
    if np.random.choice([True, False], p=[0.10, 0.90]):
        print("Dropping packet")
        packet.drop()
    else:
        print("Accepting packet")
        packet.accept()

nfqueue = NetfilterQueue()
nfqueue.bind(5, drop, 10000)

try:
    print("Starting packet processing")
    nfqueue.run()
except KeyboardInterrupt:
    print("Stopping")
    nfqueue.unbind()