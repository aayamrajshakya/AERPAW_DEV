


########## BUILDING AMF #############################


#---------------------------------------------------------------------
# BASE IMAGE
#---------------------------------------------------------------------
ARG BASE_IMAGE=ubuntu:jammy
ARG CACHEBUST=1
FROM $BASE_IMAGE as oai-amf-base
RUN ls 
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
ENV IS_DOCKERFILE=1
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
      psmisc \
      git \
  && rm -rf /var/lib/apt/lists/*
RUN apt-get update && \
    apt-get install -y \
        cmake \
        # Add any other packages you need here \
    && rm -rf /var/lib/apt/lists/*
# Some GIT configuration commands quite useful
RUN git config --global https.postBuffer 123289600 && \
    git config --global http.sslverify false


RUN git clone https://github.com/aayamrajshakya/containerization.git 
RUN cd containerization/
RUN ls
# Copying source code
WORKDIR /openair-amf/
COPY amf/build/scripts /openair-amf/build/scripts
COPY amf/build/common-build /openair-amf/build/common-build
COPY amf/build/amf/CMakeLists.txt /openair-amf/build/amf/CMakeLists.txt

# Installing all the needed libraries/packages to build and run AMF
WORKDIR /openair-amf/build/scripts
RUN ./build_amf --install-deps --force

#---------------------------------------------------------------------
# BUILDER IMAGE
#---------------------------------------------------------------------
FROM oai-amf-base as oai-amf-builder
# Copying source code
COPY amf/ /openair-amf
# Building AMF
WORKDIR /openair-amf/build/scripts
RUN ldconfig && \
    ./build_amf --clean --Verbose --build-type Release --jobs && \
    ldd /openair-amf/build/amf/build/amf && \
    mv /openair-amf/build/amf/build/amf /openair-amf/build/amf/build/oai_amf

FROM mikefarah/yq:latest as yq-source

#---------------------------------------------------------------------
# TARGET IMAGE
#---------------------------------------------------------------------
FROM $BASE_IMAGE as oai-amf
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
# We install some debug tools for the moment in addition of mandatory libraries
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
# Debug Tools that are not mandatory
      tzdata \
      psmisc \
      net-tools \
      tcpdump \
      iputils-ping \
      bc \
# Mandatory packages for the NF to run
      libssh-4 \
      openssl \
      libbrotli1 \
      netbase \
# Ubuntu 20 --> libasan5
# Ubuntu 22 --> libasan6
#      libasan? \
      libgssapi-krb5-2 \
# Ubuntu 20 --> libldap-2.4-2
# Ubuntu 22 --> libldap-2.5-0
      libldap-2.?-? \
      libconfig++9v5 \
      libsctp1 \
      librtmp1 \
      libpsl5 \
# Ubuntu 20 --> mysqlclient20
# Ubuntu 22 --> mysqlclient21
      libmysqlclient2? \
      libsqlite3-0 \
      libcurl4-gnutls-dev \
# Ubuntu 20 --> boost71
# Ubuntu 22 --> boost74
      libboost-thread1.7?.0 \
      libboost-chrono1.7?.0 \
  && rm -rf /var/lib/apt/lists/*

# Copying executable and generated libraries
WORKDIR /openair-amf/bin
COPY --from=oai-amf-builder \
    /openair-amf/build/amf/build/oai_amf \
    /openair-amf/scripts/healthcheck.sh \
    ./
COPY --from=yq-source /usr/bin/yq /usr/bin

WORKDIR /usr/local/lib
COPY --from=oai-amf-builder \
# Copying only the packages built from source
    /usr/local/lib/libnghttp2.so.14 \
    /usr/local/lib/libnghttp2_asio.so.1 \
    /usr/local/lib/libpistache.so \
    /usr/local/lib/libfmt.so \
    /usr/local/lib/libyaml-cpp.so.0.? \
    ./

RUN ldconfig && \
    ldd /openair-amf/bin/oai_amf

# Copying template configuration files
WORKDIR /openair-amf/etc
COPY --from=oai-amf-builder \
    /openair-amf/etc/amf.conf \
    /openair-amf/etc/config.yaml ./

WORKDIR /openair-amf

# expose ports
EXPOSE 80/tcp 9090/tcp 38412/sctp




##### BUILD AUSF IMAGE #########



ARG BASE_IMAGE=oai-amf
FROM $BASE_IMAGE as oai-ausf-base

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
ENV IS_DOCKERFILE=1
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
      psmisc \
      git \
  && rm -rf /var/lib/apt/lists/*

# Some GIT configuration command quite useful
RUN git config --global https.postBuffer 123289600 && \
    git config --global http.sslverify false

# Copying source code
WORKDIR /openair-ausf
COPY ausf/build/scripts /openair-ausf/build/scripts/
COPY ausf/build/common-build /openair-ausf/build/common-build
COPY ausf/build/ausf/CMakeLists.txt /openair-ausf/build/ausf/CMakeLists.txt

# Installing all the needed libraries/packages to build and run AUSF
WORKDIR /openair-ausf/build/scripts
RUN ./build_ausf --install-deps --force && \
    ldconfig

#---------------------------------------------------------------------
# BUILDER IMAGE
#---------------------------------------------------------------------
FROM oai-ausf-base as oai-ausf-builder
# Copying source code
COPY ausf/ /openair-ausf
# Building AUSF
WORKDIR /openair-ausf/build/scripts
RUN ./build_ausf --clean --Verbose --build-type Release --jobs && \
    ldd /openair-ausf/build/ausf/build/ausf && \
    mv /openair-ausf/build/ausf/build/ausf /openair-ausf/build/ausf/build/oai_ausf

FROM mikefarah/yq:latest as yq-source

#---------------------------------------------------------------------
# TARGET IMAGE
#---------------------------------------------------------------------
FROM $BASE_IMAGE as oai-ausf
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
# We install some debug tools for the moment in addition of mandatory libraries
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
# Debug Tools that are not mandatory
      tzdata \
      psmisc \
      net-tools \
      tcpdump \
      iputils-ping \
      bc \
# Mandatory packages for the NF to run
      netbase \
# Ubuntu 20 --> libasan5
# Ubuntu 22 --> libasan6
#      libasan? \
      libconfig++9v5 \
# Ubuntu 20 --> libnettle7
# Ubuntu 22 --> libnettle8
      libnettle? \
# Ubuntu 20 --> libhogweed5
# Ubuntu 22 --> libhogweed6
      libhogweed? \
# Ubuntu 20 --> libldap-2.4-2
# Ubuntu 22 --> libldap-2.5-0
      libldap-2.?-? \
      libkrb5-3 \
      libgssapi-krb5-2 \
      libcurl?-gnutls \
      librtmp1 \
      libpsl5 \
# Ubuntu 20 --> boost71
# Ubuntu 22 --> boost74
      libboost-thread1.7?.0 \
  && rm -rf /var/lib/apt/lists/*

# Copying executable and generated libraries
WORKDIR /openair-ausf/bin
COPY --from=oai-ausf-builder \
    /openair-ausf/build/ausf/build/oai_ausf \
    /openair-ausf/scripts/healthcheck.sh \
    ./
COPY --from=yq-source /usr/bin/yq /usr/bin

COPY --from=oai-ausf-builder \
# Copying only the packages built from source
    /usr/local/lib/libnghttp2.so.14 \
    /usr/local/lib/libnghttp2_asio.so.1 \
    /usr/local/lib/libpistache.so \
    /usr/local/lib/libfmt.so \
    /usr/local/lib/libyaml-cpp.so.0.? \
    /usr/local/lib/

RUN ldconfig && \
    ldd /openair-ausf/bin/oai_ausf

WORKDIR /openair-ausf/etc
COPY --from=oai-ausf-builder \
    /openair-ausf/etc/ausf.conf \
    /openair-ausf/etc/config.yaml ./

WORKDIR /openair-ausf

# expose ports
EXPOSE 80/tcp


################# BUILD FED ##################################


ARG BASE_IMAGE=oai-ausf

FROM $BASE_IMAGE as trf-gen-cn5g
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
# We install some debug tools for the moment in addition of mandatory libraries
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
      psmisc \
      net-tools \
      ethtool \
      tshark \
      tzdata \
      iperf \
      iproute2 \
      iputils-ping \
      iptables \
      traceroute \
      tcpdump \
      iperf3 \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp



###### BUILD IMS IMAGE #########################

ARG BASE_IMAGE=trf-gen-cn5g

FROM $BASE_IMAGE as ims

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
      psmisc \
      git \
      asterisk \
  && rm -rf /var/lib/apt/lists/*






###################### BUILD NRF IMAGE #########################



ARG BASE_IMAGE=ims
FROM $BASE_IMAGE as oai-nrf-base

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
ENV IS_DOCKERFILE=1
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
      psmisc \
      git \
  && rm -rf /var/lib/apt/lists/*

# Some GIT configuration command quite useful
RUN git config --global https.postBuffer 123289600 && \
    git config --global http.sslverify false

# Copying source code
WORKDIR /openair-nrf
COPY nrf/build/scripts /openair-nrf/build/scripts
COPY nrf/build/common-build /openair-nrf/build/common-build
COPY nrf/build/nrf/CMakeLists.txt /openair-nrf/build/nrf/CMakeLists.txt

# Installing all the needed libraries/packages to build and run NRF
WORKDIR /openair-nrf/build/scripts
RUN ./build_nrf --install-deps --force && \
    ldconfig

#---------------------------------------------------------------------
# BUILDER IMAGE
#---------------------------------------------------------------------
FROM oai-nrf-base as oai-nrf-builder
# Copying source code
COPY nrf/ /openair-nrf
# Building NRF
WORKDIR /openair-nrf/build/scripts
RUN ./build_nrf --clean --Verbose --build-type Release --jobs && \
    ldd /openair-nrf/build/nrf/build/nrf && \
    mv /openair-nrf/build/nrf/build/nrf /openair-nrf/build/nrf/build/oai_nrf

FROM mikefarah/yq:latest as yq-source

#---------------------------------------------------------------------
# TARGET IMAGE
#---------------------------------------------------------------------
FROM $BASE_IMAGE as oai-nrf
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
# We install some debug tools for the moment in addition of mandatory libraries
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
# Debug Tools that are not mandatory
      tzdata \
      psmisc \
      net-tools \
      tcpdump \
      iputils-ping \
      bc \
# Mandatory packages for the NF to run
# Ubuntu 20 --> libasan5
# Ubuntu 22 --> libasan6
#      libasan? \
# Ubuntu 20 --> libldap-2.4-2
# Ubuntu 22 --> libldap-2.5-0
      libldap-2.?-? \
      libkrb5-3 \
      libgssapi-krb5-2 \
      libcurl4-gnutls-dev \
      librtmp1 \
      libpsl5 \
# Ubuntu 20 --> boost71
# Ubuntu 22 --> boost74
      libboost-thread1.7?.0 \
      libboost-chrono1.7?.0 \
  && rm -rf /var/lib/apt/lists/*

# Copying executable and generated libraries
WORKDIR /openair-nrf/bin
COPY --from=oai-nrf-builder \
    /openair-nrf/build/nrf/build/oai_nrf \
    /openair-nrf/scripts/healthcheck.sh \
    ./
COPY --from=yq-source /usr/bin/yq /usr/bin

COPY --from=oai-nrf-builder \
# Copying only the packages built from source
    /usr/local/lib/libnghttp2.so.14 \
    /usr/local/lib/libnghttp2_asio.so.1 \
    /usr/local/lib/libyaml-cpp.so.0.? \
    /usr/local/lib/libpistache.so \
    /usr/local/lib/libfmt.so \
    /usr/local/lib/

RUN ldconfig && \
    ldd /openair-nrf/bin/oai_nrf

# Copying template configuration files
COPY --from=oai-nrf-builder /openair-nrf/etc/config.yaml /openair-nrf/etc/

WORKDIR /openair-nrf

# expose ports
EXPOSE 80/tcp 8080/tcp









###################### BUILD SMF IMAGE #########################



ARG BASE_IMAGE=oai-nrf
FROM $BASE_IMAGE as oai-smf-base

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
ENV IS_DOCKERFILE=1
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
      psmisc \
      git \
  && rm -rf /var/lib/apt/lists/*

# Some GIT configuration commands quite useful
RUN git config --global https.postBuffer 123289600 && \
    git config --global http.sslverify false

# Copying source code
WORKDIR /openair-smf
COPY smf/build/scripts /openair-smf/build/scripts
COPY smf/build/common-build /openair-smf/build/common-build
COPY smf/build/smf/CMakeLists.txt /openair-smf/build/smf/CMakeLists.txt

# Installing all the needed libraries/packages to build and run SMF
WORKDIR /openair-smf/build/scripts
RUN ./build_smf --install-deps --force

#---------------------------------------------------------------------
# BUILDER IMAGE
#---------------------------------------------------------------------
FROM oai-smf-base as oai-smf-builder
# Copying source code
COPY smf/ /openair-smf
# Building SMF
WORKDIR /openair-smf/build/scripts
RUN ldconfig && \
    ./build_smf --clean --Verbose --build-type Release --jobs && \
    ldd /openair-smf/build/smf/build/smf && \
    mv /openair-smf/build/smf/build/smf /openair-smf/build/smf/build/oai_smf

FROM mikefarah/yq:latest as yq-source

#---------------------------------------------------------------------
# TARGET IMAGE
#---------------------------------------------------------------------
FROM $BASE_IMAGE as oai-smf
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
# We install some debug tools for the moment in addition of mandatory libraries
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
# Debug Tools that are not mandatory
      tzdata \
      psmisc \
      net-tools \
      tcpdump \
      iputils-ping \
      bc \
# Mandatory packages for the NF to run
      libssh-4 \
      openssl \
      libbrotli1 \
      netbase \
# Ubuntu 20 --> libasan5
# Ubuntu 22 --> libasan6
#      libasan? \
      libgssapi-krb5-2 \
# Ubuntu 20 --> libldap-2.4-2
# Ubuntu 22 --> libldap-2.5-0
      libldap-2.?-? \
      libsctp1 \
      librtmp1 \
      libpsl5 \
      libcurl?-gnutls \
# Ubuntu 20 --> boost71
# Ubuntu 22 --> boost74
      libboost-thread1.7?.0 \
      libboost-chrono1.7?.0 \
  && rm -rf /var/lib/apt/lists/*

# Copying executable and generated libraries
WORKDIR /openair-smf/bin
COPY --from=oai-smf-builder \
    /openair-smf/build/smf/build/oai_smf \
    /openair-smf/scripts/healthcheck.sh \
    ./
COPY --from=yq-source /usr/bin/yq /usr/bin

WORKDIR /usr/local/lib/
COPY --from=oai-smf-builder \
# Copying only the packages built from source
    /usr/local/lib/libnghttp2.so.14 \
    /usr/local/lib/libnghttp2_asio.so.1 \
    /usr/local/lib/libyaml-cpp.so.0.? \
    /usr/local/lib/libpistache.so \
    /usr/local/lib/libfmt.so \
    /openair-smf/build/smf/build/nas/libNAS.so \
    ./

RUN ldconfig && \
    ldd /openair-smf/bin/oai_smf

# Copying configuration files
# The configuration folder will be flat
WORKDIR /openair-smf/etc
COPY --from=oai-smf-builder /openair-smf/etc/config.yaml .

WORKDIR /openair-smf

# expose ports
EXPOSE 80/tcp 9090/tcp 8805/udp


###################### BUILD UDM IMAGE #########################

ARG BASE_IMAGE=oai-smf
FROM $BASE_IMAGE as oai-udm-base

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
ENV IS_DOCKERFILE=1
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
      psmisc \
      git \
  && rm -rf /var/lib/apt/lists/*

# Some GIT configuration command quite useful
RUN git config --global https.postBuffer 123289600 && \
    git config --global https.maxRequestBuffer 123289600 && \
    git config --global core.compression 0 && \
    git config --global http.sslverify false

# Copying source code
WORKDIR /openair-udm
COPY udm/build/scripts /openair-udm/build/scripts/
COPY udm/build/common-build /openair-udm/build/common-build
COPY udm/build/udm/CMakeLists.txt /openair-udm/build/udm/CMakeLists.txt

# Installing all the needed libraries/packages to build and run UDM
WORKDIR /openair-udm/build/scripts
RUN ./build_udm --install-deps --force && \
    ldconfig

#---------------------------------------------------------------------
# BUILDER IMAGE
#---------------------------------------------------------------------
FROM oai-udm-base as oai-udm-builder
# Building UDM
COPY udm/ /openair-udm
# Copying source code
WORKDIR /openair-udm/build/scripts
RUN ./build_udm --clean --Verbose --build-type Release --jobs && \
    ldd /openair-udm/build/udm/build/udm && \
    mv /openair-udm/build/udm/build/udm /openair-udm/build/udm/build/oai_udm

FROM mikefarah/yq:latest as yq-source

#---------------------------------------------------------------------
# TARGET IMAGE
#---------------------------------------------------------------------
FROM $BASE_IMAGE as oai-udm
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
# We install some debug tools for the moment in addition of mandatory libraries
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
# Debug Tools that are not mandatory
      tzdata \
      psmisc \
      net-tools \
      tcpdump \
      iputils-ping \
      bc \
# Mandatory packages for the NF to run
      netbase \
# Ubuntu 20 --> libasan5
# Ubuntu 22 --> libasan6
#      libasan? \
      libconfig++9v5 \
# Ubuntu 20 --> libnettle7
# Ubuntu 22 --> libnettle8
      libnettle? \
# Ubuntu 20 --> libhogweed5
# Ubuntu 22 --> libhogweed6
      libhogweed? \
# Ubuntu 20 --> libldap-2.4-2
# Ubuntu 22 --> libldap-2.5-0
      libldap-2.?-? \
      libkrb5-3 \
      libgssapi-krb5-2 \
      libcurl?-gnutls \
      librtmp1 \
      libpsl5 \
# Ubuntu 20 --> boost71
# Ubuntu 22 --> boost74
      libboost-thread1.7?.0 \
  && rm -rf /var/lib/apt/lists/*

# Copying executable and generated libraries
WORKDIR /openair-udm/bin
COPY --from=oai-udm-builder \
    /openair-udm/build/udm/build/oai_udm \
    /openair-udm/scripts/healthcheck.sh \
    ./
COPY --from=yq-source /usr/bin/yq /usr/bin

COPY --from=oai-udm-builder \
# Copying only the packages built from source
    /usr/local/lib/libnghttp2.so.14 \
    /usr/local/lib/libnghttp2_asio.so.1 \
    /usr/local/lib/libpistache.so \
    /usr/local/lib/libfmt.so \
    /usr/local/lib/libyaml-cpp.so.0.? \
    /usr/local/lib/

RUN ldconfig && \
    ldd /openair-udm/bin/oai_udm

WORKDIR /openair-udm/etc
COPY --from=oai-udm-builder \
    /openair-udm/etc/udm.conf \
    /openair-udm/etc/config.yaml ./

WORKDIR /openair-udm

# expose ports
EXPOSE 80/tcp

#---------------------------------------------------------------------
# BASE IMAGE UDR
#---------------------------------------------------------------------
ARG BASE_IMAGE=oai-udr
FROM $BASE_IMAGE as oai-udr-base

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
ENV IS_DOCKERFILE=1
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
      psmisc \
      git \
  && rm -rf /var/lib/apt/lists/*

# Some GIT configuration command quite useful
RUN git config --global https.postBuffer 123289600 && \
    git config --global http.sslverify false

# Copying source code
WORKDIR /openair-udr
COPY udr/build/scripts /openair-udr/build/scripts/
COPY udr/build/common-build /openair-udr/build/common-build
COPY udr/build/udr/CMakeLists.txt /openair-udr/build/udr/CMakeLists.txt

# wait for it
WORKDIR /
RUN git clone https://github.com/vishnubob/wait-for-it.git

# Installing all the needed libraries/packages to build and run UDR
WORKDIR /openair-udr/build/scripts
RUN ./build_udr --install-deps --force && \
    ldconfig

#---------------------------------------------------------------------
# BUILDER IMAGE
#---------------------------------------------------------------------
FROM oai-udr-base as oai-udr-builder
# Copying source code
COPY udr/ /openair-udr
# Building UDR
WORKDIR /openair-udr/build/scripts
RUN ./build_udr --clean --Verbose --build-type Release --jobs && \
    ldd /openair-udr/build/udr/build/udr && \
    mv /openair-udr/build/udr/build/udr /openair-udr/build/udr/build/oai_udr

FROM mikefarah/yq:latest as yq-source

#---------------------------------------------------------------------
# TARGET IMAGE UDR
#---------------------------------------------------------------------
FROM $BASE_IMAGE as oai-udr
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
# We install some debug tools for the moment in addition of mandatory libraries
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
# Debug Tools that are not mandatory
      tzdata \
      psmisc \
      net-tools \
      tcpdump \
      iputils-ping \
      bc \
# Mandatory packages for the NF to run
      libmysqlclient2? \
      netbase \
# Ubuntu 20 --> libasan5
# Ubuntu 22 --> libasan6
#      libasan? \
      libconfig++9v5 \
# Ubuntu 20 --> libnettle7
# Ubuntu 22 --> libnettle8
      libnettle? \
# Ubuntu 20 --> libhogweed5
# Ubuntu 22 --> libhogweed6
      libhogweed? \
# Ubuntu 20 --> libldap-2.4-2
# Ubuntu 22 --> libldap-2.5-0
      libldap-2.?-? \
      libkrb5-3 \
      libgssapi-krb5-2 \
      libcurl?-gnutls \
      librtmp1 \
      libpsl5 \
# Ubuntu 20 --> boost71
# Ubuntu 22 --> boost74
      libboost-thread1.7?.0 \
  && rm -rf /var/lib/apt/lists/*

# Copying executable and generated libraries
WORKDIR /openair-udr/bin
COPY --from=oai-udr-builder \
    /openair-udr/build/udr/build/oai_udr \
    /openair-udr/scripts/healthcheck.sh \
    /wait-for-it/wait-for-it.sh \
    ./
COPY --from=yq-source /usr/bin/yq /usr/bin

COPY --from=oai-udr-builder \
# Copying only the packages built from source
    /usr/local/lib/libnghttp2.so.14 \
    /usr/local/lib/libnghttp2_asio.so.1 \
    /usr/local/lib/libpistache.so \
    /usr/local/lib/libfmt.so \
    /usr/local/lib/libyaml-cpp.so.0.? \
    /usr/local/lib/libmongocxx.so._noabi \
    /usr/local/lib/libbsoncxx.so._noabi \
    /usr/local/lib/libmongoc-1.0.so.0 \
    /usr/local/lib/libbson-1.0.so.0 \
    /lib/x86_64-linux-gnu/libsnappy.so.1 \
    /usr/local/lib/

RUN ldconfig && \
    ldd /openair-udr/bin/oai_udr

WORKDIR /openair-udr/etc
COPY --from=oai-udr-builder \
    /openair-udr/etc/udr.conf \
    /openair-udr/etc/config.yaml ./

WORKDIR /openair-udr

# expose ports
EXPOSE 80/tcp



###################### BUILD spgwu_tiny IMAGE #########################


ARG BASE_IMAGE=oai-udr
FROM $BASE_IMAGE as oai-spgwu-tiny-base

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
ENV IS_DOCKERFILE=1
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
      psmisc \
      git \
  && rm -rf /var/lib/apt/lists/*

# Some GIT configuration commands quite useful
RUN git config --global https.postBuffer 123289600 && \
    git config --global http.sslverify false

# Copy installation scripts
WORKDIR /openair-spgwu-tiny
COPY spgwu_tiny/build/scripts /openair-spgwu-tiny/build/scripts/
COPY spgwu_tiny/build/spgw_u/CMakeLists.txt /openair-spgwu-tiny/build/spgw_u/CMakeLists.txt
COPY spgwu_tiny/build/cmake_modules /openair-spgwu-tiny/cmake_modules/

# Installing all the needed libraries/packages to build and run SPGWU-TINY
WORKDIR /openair-spgwu-tiny/build/scripts
RUN ./build_spgwu --install-deps --force

#---------------------------------------------------------------------
# BUILDER IMAGE
#---------------------------------------------------------------------
FROM oai-spgwu-tiny-base as oai-spgwu-tiny-builder
# Copy the rest of source code
COPY spgwu_tiny/ /openair-spgwu-tiny
# Building SPGW-U-TINY
WORKDIR /openair-spgwu-tiny/build/scripts
RUN ./build_spgwu --clean --build-type Release --jobs --Verbose && \
    ldd /openair-spgwu-tiny/build/spgw_u/build/spgwu && \
    mv /openair-spgwu-tiny/build/spgw_u/build/spgwu /openair-spgwu-tiny/build/spgw_u/build/oai_spgwu

#---------------------------------------------------------------------
# TARGET IMAGE
#---------------------------------------------------------------------
FROM $BASE_IMAGE as oai-spgwu-tiny
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
# We install some debug tools for the moment in addition of mandatory libraries
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes \
      python3 \
      python3-jinja2 \
      tzdata \
      psmisc \
      net-tools \
      iproute2 \
      ethtool \
      iptables \
      netbase \
      libgssapi-krb5-2 \
# Ubuntu 18/20 --> libldap-2.4-2
# Ubuntu 22    --> libldap-2.5-0
      libldap-2.?-? \
      libgoogle-glog0v5 \
# Ubuntu 18 --> libdouble-conversion1
# Ubuntu 20 --> libdouble-conversion3
      libdouble-conversion? \
      libconfig++9v5 \
      librtmp1 \
      libpsl5 \
      libnghttp2-14 \
      libcurl?-gnutls \
      libboost-thread1.7?.0 \
  && rm -rf /var/lib/apt/lists/*

# Copying executable and generated libraries
WORKDIR /openair-spgwu-tiny/bin
COPY --from=oai-spgwu-tiny-builder \
    /openair-spgwu-tiny/build/spgw_u/build/oai_spgwu \
    /openair-spgwu-tiny/scripts/entrypoint.py \
    /openair-spgwu-tiny/scripts/healthcheck.sh \
    ./

WORKDIR /usr/local/lib
COPY --from=oai-spgwu-tiny-builder \
    /usr/local/lib/libfmt.so \
    ./

RUN ldconfig && \
    ldd /openair-spgwu-tiny/bin/oai_spgwu

# Copying template configuration files
# The configuration folder will be flat
WORKDIR /openair-spgwu-tiny/etc
COPY --from=oai-spgwu-tiny-builder /openair-spgwu-tiny/etc/spgw_u.conf .

WORKDIR /openair-spgwu-tiny

# use these labels for CI purpose
LABEL support-multi-sgwu-instances="true"
LABEL support-nrf-fdqn="true"

# expose ports
EXPOSE 2152/udp 8805/udp



