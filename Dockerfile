
FROM ubuntu:20.04

USER root

    
RUN export DEBIAN_FRONTEND=noninteractive && \
           apt-get update && \
           apt-get install -y build-essential \
                                         git \
                                         net-tools \
                                         arp-scan \
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
                                         libnetfilter-queue-dev \
                                         python3 \
                                         python3-pip \
                                         python3-dev \
                                         build-essential
                                    
RUN pip3 install NetfilterQueue numpy
WORKDIR /protocol

COPY . .

RUN chmod +x ./scripts/router.sh
RUN chmod +x ./scripts/client.sh


RUN apt-get install -y g++ make