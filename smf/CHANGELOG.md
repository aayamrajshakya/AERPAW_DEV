# RELEASE NOTES: #

## v2.0.0 -- December 2023 ##

* Features
  - Support YAML configuration file
    * Yaml validation default value
    * Make DNS configurable per DNN
    * Add CIDR DNN configuration
    * implement UpfInfo parsing for YAML config
  - Support getting SMF configuration with the new SMF configuration API
  - Handled traffic redirection PCF policy
  - Giving users possibility to configure SMF info themselves
  - Removed SBI DNS lookup upon start of SMF
  - De-register to NRF on exit
  - Enabled event exposure in HTTP/2 API server
* Fixes
  - Fixed ULCL support in SM context update procedure
  - Fixed SEID assignment in N4 session establishment request
  - Fixed Service Request handling
  - Fixed YAML config issues
  - Fixed Handle IE Usage Report in session deletion response
  - Fixed SMF selection (NRF registration) when local subscription info is not used
* Tech debt
  - Updated common models to 3GPP TS 29.571 Release 16.13.0 and moved them to the shared common submodule
  - Updated SMF-specific NRF models to Release 16.15.0 and moved them to the shared common submodule
  - Updated PCF models to Release 16.17.0 and moved them to the shared common submodule
  - Switching to clang-format-12
  - Published image on Docker-Hub is using now Ubuntu-22 as base image
    * Ubuntu-18 is no longer supported

## v1.5.1 -- May 2023 ##

* Fix N1N2 Content message
* Remove ranInfo field in n2infocontainer
* Fix encoding NAS header
* Resolved QoS Flow info missing in N2 Handover Preparation
* Fix SUPI format: Use IMSI as SUPI
* Fix: Send PDU Session Reject when PDU session fails before SMF procedure is started
* Fix HTTP header
* Fixing config parameter read
* Fix for IE Redirect Information
* Fix: Associate node id based on node id type
* Use N3 ip address from NF profile if provided
* Fix: UL TEID for multiple created PDR
* Fix for encode/decode of create QER IE
* Support for DL PDR in session establishment
* Use NWI from config file when no UPF profile is associated
* Fix HTTP/2 issue with PCF
* Code Refactoring for:
  * Logging mechanism (runtime log level selection)
  * Installation / build scripts
  * Continuous Integration scripts
* Published image on Docker-Hub is using now Ubuntu-20 as base image
  * We will soon obsolete the build system for Ubuntu18.04

## v1.5.0 -- January 2023 ##

* Feature improvements:
  * PDU SESS EST + Qos Monitoring events exposure
  * Giving some time for FQDN resolution
  * PFCP: removing dependency to folly
  * Add IEs to support UE Rel 16
  * Added and integrated UPF graph instead of PFCP associations list
    * Supported scenarios:
      - I-UPF / A-UPF N9
      - UL CL
  * Implement Npcf_SMPolicyControl API client
  * Changing the configuration file to jinja2-based approach
    * At least one slice (slice #0) shall be completely defined
  * Ubuntu22 and cgroup2 support
* Fixes:
  * Fix openapi cause
  * Fix small issue for Mime parser for normal message
  * Fix Volume Threshold IE to PFCP Create URR IE
  * Fix mcc mnc 00 udr
  * Fix issues to work with Release-16 COTS UE
  * Fix issue when there's no valid reply from UPF for PDU session release
  * Fix Protocol identifier Selected Bearer Control Mode
  * Fix issue for Static UE IP Addr
  * Fix URR ID in IE Create URR (post UPF graph implementation)
  * Fix issue for Full DNN (APN Operator Identifier)
* Docker image improvements
  * Fixed docker exit by catching SIGTERM
  * release mode does not use libasan anymore --> allocation of 20T virtual memory is no longer done

## v1.4.0 -- July 2022 ##

* Fix for handling S-NSSAI 
* Add support for IMS
* HTTP/2 support for SBI interface
* Add usage reporting feature
* Flexible MTU
* Support for Ubuntu 20.04 bare-metal installation
* Official images produced by CI are pushed to `oaisoftwarealliance` Docker-Hub Team account
* Reduce image size


## v1.3.0 -- January 2022 ##

* Update NWI from NF profile
* Update SMF profile
* Added retry for PFCP association request
* More flexible DNN/IP ranges when deploying container
* Fix retrieving the list of NWI
* Fix entreprise IE decoding
* Fix issue for UPF selection
* Fix for IE Measurement Period
* Docker optimization

## v1.2.1 -- October 2021 ##

* Fix build issue
* Fix UPF selection
* Timers handling: T3591/T3952

## v1.2.0 -- September 2021 ##

* Event Exposure implemented
* UPF profile update
* Support for URR query
* Bug fixes

## v1.1.0 -- July 2021 ##

* PFCP Release 16 support
* NRF registration
  - with FQDN DNS resolution
* support for multiple UPF instances
* Dotted DNN support
* use (SST, SD) to get the subscription information
* Bug fixes
* Full support for Ubuntu18 and RHEL8

## v1.0.0 -- September 2020 ##

* Initial release, SMF can handle basic PDU session-related procedures e.g., session establishment request, session modification and session release.

