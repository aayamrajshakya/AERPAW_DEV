

# # AERPAW containerization with open5gs


# ## building the container:

Clone the Repository
> git clone https://github.ncsu.edu/agurses/AERPAW-Dev.git

Move to docker directory
> cd AERPAW-Dev/DCS/platform_control/container_image_gen

configure environment variables
> export GIT_SECRET=<github secret key> \
> && export BUILD_DATE=$(date -u +'%Y-%m-%dT%H:%M:%SZ')

build the base image
> docker build --no-cache=true --network=host --build-arg GIT_SECRET=$GIT_SECRET  --build-arg BUILD_DATE=$BUILD_DATE  -t aerpaw_base:latest -f Dockerfile.base_image .

build the evm base image
> docker build --no-cache=true --network=host --build-arg GIT_SECRET=$GIT_SECRET  --build-arg BUILD_DATE=$BUILD_DATE -t aerpaw_evm_base:latest -f Dockerfile.evm_base .

build primary image 
> docker build --no-cache=true --network=host --build-arg GIT_SECRET=$GIT_SECRET --build-arg BUILD_DATE=$BUILD_DATE -t aerpaw_evm:latest -f Dockerfile.evm .



# ## running the container
 
> sudo docker run -it --privileged  -v /dev/bus/usb:/dev/bus/usb --net none --cap-add=NET_ADMIN --cap-add=SYS_NICE <Image ID> /bin/bash



# ## Device A: running the gNB and open5gs

Start open5gs
> cd ~/Profiles/ProfileScripts/Radio/Helpers && \
> ./startOpen5GS.sh /opt/open5gs/build/configs/open5gs_nr_core.yaml


# ## Device A: running the UE

start NR softmodem
> /opt/oai5g/cmake_targets/ran_build/build/nr-uesoftmodem -r 106 --numerology 1 --band 78 -C 3619200000 --ue-fo-compensation --sa -E --uicc0.imsi 208990100001100 --uicc0.key fec86ba6eb707ed08905757b1bb44b8f --uicc0.opc C42449363BBAD02B66D16BC975D77CC1 


