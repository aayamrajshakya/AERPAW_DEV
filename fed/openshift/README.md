<table style="border-collapse: collapse; border: none;">
  <tr style="border-collapse: collapse; border: none;">
    <td style="border-collapse: collapse; border: none;">
      <a href="http://www.openairinterface.org/">
         <img src="../docs/images/oai_final_logo.png" alt="" border=3 height=50 width=150>
         </img>
      </a>
    </td>
    <td style="border-collapse: collapse; border: none; vertical-align: center;">
      <b><font size = "5">OpenAirInterface: Building UBI Container Images</font></b>
    </td>
  </tr>
</table>

**TABLE OF CONTENTS**

0.  [Pre-requisites](#0-pre-requisites)
1.  [How to Build UBI Images of Core Network Function](#1-how-to-build-ubi-images-of-core-network-functions)
2.  [How to Build UBI Images of gNB and UE](#2-how-to-build-ubi-images-of-gnb-and-ue)


## 0. Pre-requisites

We assume that there is already a project name `oai-tutorial` in case there is no project like that then create a new project `oc new-project oai-tutorial`. 

Also you need to create configmaps and secrets because that is needed for creating the images of the network functions. In the future we will try to avoid using the entitlements. Run below commands from a RHEL8 host machine which have access to openshift cluster,

```bash
oc create configmap rhsm-conf --from-file /etc/rhsm/rhsm.conf
oc create configmap rhsm-ca --from-file /etc/rhsm/ca/redhat-uep.pem
oc create secret generic etc-pki-entitlement --from-file /etc/pki/entitlement/<pem>.pem --from-file /etc/pki/entitlement/<key>-key.pem
```

*NOTE*: The entitlements have to be renewed every month 


## 1. How to Build UBI Images of Core Network Functions?

Create the build configs for each core network function, the build config yamls will also create the image streams.

```bash
oc create -f oai-amf-build-config.yaml
oc create -f oai-ausf-build-config.yaml
oc create -f oai-udr-build-config.yaml
oc create -f oai-udm-build-config.yaml
oc create -f oai-smf-build-config.yaml
oc create -f oai-spgwu-tiny-build-config.yaml
oc create -f oai-nrf-build-config.yaml
```

You can do `oc get bc` to see all the build configs in `oai5g` project. Once all the build config definations are there, you can start building the network function images parallel or one by one its a choice, 

``` bash
oc start-bc oai-amf 
oc start-bc oai-smf 
oc start-bc oai-ausf 
oc start-bc oai-nrf
oc start-bc oai-spgwu-tiny 
oc start-bc oai-udm
oc start-bc oai-udr
```

For the moment the dockerfiles used in these build configs are based on v1.4.0 release tag of all the network functions. 

## 2. How to Build UBI Images of gNB and UE?

For the moment CU and DU are using the same image with different configuration parameters so you just need to build one monolythic gNB image. Also CU and DU are running fine in simulated mode but not with RRU, there is a problem which we will fix soon. 

The gNB image is build in three steps
- Base image
- Builder image
- Final/Target image

Every image has its own build config. For the moment you will see inside the build config embedded dockerfiles but this change in sometime and there will only one build config which will fetch dockerfile directly from the gitlab repository. 

#### 2.1 Start with building the base image

Develop branch is mostly stable and there is a new merge every week. First start with creating base image

```bash
oc create -f oai-ran-base-build-config.yaml
oc start-build oai-ran-base
```

Once done you need to build builder image

#### 2.2 Creating builder image

```bash
oc create -f ran-build/ran-build.yaml 
oc start-build ran-build
```

#### 2.3 Creating target image for oai-gNB

This target image is used by oai-gnb, oai-cu, oai-cu-cp and oai-du. But for oai-cu-up there is another target image. 

If you are using another project than `oai-tutorial` then you need to make a small change in the [build config file](./oai-gnb-build-config.yaml). Change the YOUR_PROJECT_NAME variable with your project name (oc project)

```
$: sed -i 's/oai-tutorial/$YOUR_PROJECT_NAME/g' oai-gnb-build-config.yaml
```

If you are using the project name as `oai-tutorial` then you can directly start building the target image. 

```bash
oc create -f oai-gnb-build-config.yaml
oc start-build oai-gnb
```

#### 2.4 Creating target image for oai-cu-up

This target image is only for oai-cu-up

If you are using another project than `oai-tutorial` then you need to make a small change in the [build config file](./oai-gnb-build-config.yaml). Change the YOUR_PROJECT_NAME variable with your project name (oc project)

```
$: sed -i 's/oai-tutorial/$YOUR_PROJECT_NAME/g' oai-cu-up-build-config.yaml
```

If you are using the project name as `oai-tutorial` then you can directly start building the target image. 

```bash
oc create -f oai-cu-up-build-config.yaml
oc start-build oai-cu-up
```

#### 2.5 Creating NR-UE image

If you are using another project than `oai-tutorial` then you need to make a small change in the [build config file](./oai-nr-ue-build-config.yaml). Change the YOUR_PROJECT_NAME variable with your project name (oc project)

```
$: sed -i 's/oai-tutorial/$YOUR_PROJECT_NAME/g' oai-nr-ue-build-config.yaml
```

If you are using the project name as `oai-tutorial` then you can directly start building the target image. 


``` bash
oc create -f oai-nr-ue-build-config.yaml
oc start-build oai-nr-ue 
```

You can follow [our tutorial](../docs/DEPLOY_SA5G_HC.md) on how to deploy OAI5g Core, gNB and NR-UE via [helm-charts](../charts) and helm-spray.