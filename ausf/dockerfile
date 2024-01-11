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
# Dockerfile for the Open-Air-Interface AUSF service
#   Valid for Ubuntu-20.04 (focal),
#             Ubuntu-22.04 (jammy)
#
#---------------------------------------------------------------------

#---------------------------------------------------------------------
# BASE IMAGE
#---------------------------------------------------------------------
ARG BASE_IMAGE=ubuntu:jammy
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
COPY ./build/scripts /openair-ausf/build/scripts/
COPY ./build/common-build /openair-ausf/build/common-build
COPY ./build/ausf/CMakeLists.txt /openair-ausf/build/ausf/CMakeLists.txt

# Installing all the needed libraries/packages to build and run AUSF
WORKDIR /openair-ausf/build/scripts
RUN ./build_ausf --install-deps --force && \
    ldconfig

#---------------------------------------------------------------------
# BUILDER IMAGE
#---------------------------------------------------------------------
FROM oai-ausf-base as oai-ausf-builder
# Copying source code
COPY . /openair-ausf
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
# healthcheck
HEALTHCHECK --interval=10s \
            --timeout=15s \
            --retries=6 \
    CMD /openair-ausf/bin/healthcheck.sh

CMD ["/openair-ausf/bin/oai_ausf", "-c" , "/openair-ausf/etc/config.yaml", "-o"]
