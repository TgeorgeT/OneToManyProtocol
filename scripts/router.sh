# iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
# iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE

sysctl -w net.ipv4.ip_forward=1
