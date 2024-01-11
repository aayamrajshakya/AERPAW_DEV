# RELEASE NOTES: #

## v2.0.0 -- December 2023 ##

* Features
  - Support YAML configuration file
    * Yaml validation default value
  - Added heartbeat parameter
* Fixes
  - Returning only REGISTERED NFs from Search result
  - Removing suspended NFs after timeout
* Tech debt
  - Updated common models to 3GPP TS 29.571 Release 16.13.0 and moved them to the shared common submodule
  - Switching to clang-format-12
  - Published image on Docker-Hub is using now Ubuntu-22 as base image
    * Ubuntu-18 is no longer supported

## v1.5.1 -- May 2023 ##

* Tentative fix for a failed assertion of Boost signal
* HTTP2 fix for json response data formatting
* Code Refactoring for:
  * Logging mechanism (runtime log level selection)
  * Installation / build scripts
  * Continuous Integration scripts
* Published image on Docker-Hub is using now Ubuntu-20 as base image
  * We will soon obsolete the build system for Ubuntu18.04

## v1.5.0 -- January 2023 ##

* Enhance UPF profile in NRF and fix bug in SBI when body is too long
* Added Status change notification
* Docker image improvements
* Fixed docker exit by catching SIGTERM
* release mode does not use libasan anymore --> allocation of 20T virtual memory is no longer done
* Ubuntu22 and cgroup2 support

## v1.4.0 -- July 2022 ##

* Remove unused packages
* Add NF profile handling for PCF
* HTTP/2 support for SBI interface
* Official images produced by CI are pushed to `oaisoftwarealliance` Docker-Hub Team account
* Reduce image size

## v1.3.0 -- January 2022 ##

* NF profile update
* Docker optimization

## v1.2.1 -- October 2021 ##

* Fix build issue

## v1.2.0 -- September 2021 ##

* Added AUSF, UDR, UDM profiles
* Updated UPF profile

## v1.1.0 -- July 2021 ##

* FQDN DNS resolution
* Bug fixes
* Full support for Ubuntu18 and RHEL8

## v1.0.0 -- Jan 2021 ##

* Initial release

