#/*
# * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
# * contributor license agreements.  See the NOTICE file distributed with
# * this work for additional information regarding copyright ownership.
# * The OpenAirInterface Software Alliance licenses this file to You under
# * the OAI Public License, Version 1.1  (the "License"); you may not use this file
# * except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *      http://www.openairinterface.org/?page_id=698
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *-------------------------------------------------------------------------------
# * For more information about the OpenAirInterface (OAI) Software Alliance:
# *      contact@openairinterface.org
# */
#---------------------------------------------------------------------
#
# Dockerfile for the Open-Air-Interface UDR service
#   Valid for Ubuntu-20.04 (focal),
#             Ubuntu-22.04 (jammy)
#
#---------------------------------------------------------------------

#---------------------------------------------------------------------
# BASE IMAGE
#---------------------------------------------------------------------
ARG BASE_IMAGE=ubuntu:jammy
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
COPY ./build/scripts /openair-udr/build/scripts/
COPY ./build/common-build /openair-udr/build/common-build
COPY ./build/udr/CMakeLists.txt /openair-udr/build/udr/CMakeLists.txt

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
COPY . /openair-udr
# Building UDR
WORKDIR /openair-udr/build/scripts
RUN ./build_udr --clean --Verbose --build-type Release --jobs && \
    ldd /openair-udr/build/udr/build/udr && \
    mv /openair-udr/build/udr/build/udr /openair-udr/build/udr/build/oai_udr

FROM mikefarah/yq:latest as yq-source

#---------------------------------------------------------------------
# TARGET IMAGE
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
# healthcheck
HEALTHCHECK --interval=10s \
            --timeout=15s \
            --retries=6 \
    CMD /openair-udr/bin/healthcheck.sh

CMD ["/openair-udr/bin/oai_udr", "-c", "/openair-udr/etc/config.yaml", "-o"]
