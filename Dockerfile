FROM ubuntu:22.04


RUN apt-get update && apt-get install -y sudo


# Copy the bash script to the container
COPY open5g_bash.sh /usr/local/bin/
COPY start_trf.sh /usr/local/bin/
COPY start5gcore.sh /usr/local/bin
COPY buildgnb.sh /usr/local/bin

# Set execute permissions on the script
RUN chmod +x /usr/local/bin/open5g_bash.sh
RUN chmod +x /usr/local/bin/start5gcore.sh
RUN chmod +x /usr/local/bin/buildgnb.sh
RUN chmod +x /usr/local/bin/start_trf.sh


#build core network services
RUN /usr/local/bin/open5g_bash.sh
#RUN /usr/local/bin/start5gcore.sh
RUN /usr/local/bin/buildgnb.sh

#expose needed ports
EXPOSE 80/tcp 9090/tcp 38412/sctp 8080/tcp 8805/udp 2152/udp
EXPOSE 3306 33060
