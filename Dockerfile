
FROM ubuntu:20.04

USER root

    
RUN export DEBIAN_FRONTEND=noninteractive && \
           apt-get update && \
           apt-get install -y build-essential \
                                         git \
                                         net-tools \
                                         arp-scan \
                                         python3.8 \
                                         python3-pip \
                                         tcpdump \
                                         ethtool \
                                         nmap \
                                         netcat \
                                         traceroute \
                                         iputils-ping \
                                         dnsutils \
                                         iptables \
                                         iproute2 \
                                         telnet \
                                         libnetfilter-queue-dev


RUN apt-get install -y g++ make