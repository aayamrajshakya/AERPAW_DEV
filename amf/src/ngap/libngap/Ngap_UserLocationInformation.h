/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NGAP-IEs"
 * 	found in "asn.1/Information Element Definitions.asn1"
 * 	`asn1c -pdu=all -fcompound-names -fno-include-deps -findirect-choice
 * -no-gen-example -gen-APER -gen-UPER -no-gen-JER -gen-BER -D src`
 */

#ifndef _Ngap_UserLocationInformation_H_
#define _Ngap_UserLocationInformation_H_

#include <asn_application.h>

/* Including external dependencies */
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Ngap_UserLocationInformation_PR {
  Ngap_UserLocationInformation_PR_NOTHING, /* No components present */
  Ngap_UserLocationInformation_PR_userLocationInformationEUTRA,
  Ngap_UserLocationInformation_PR_userLocationInformationNR,
  Ngap_UserLocationInformation_PR_userLocationInformationN3IWF,
  Ngap_UserLocationInformation_PR_choice_Extensions
} Ngap_UserLocationInformation_PR;

/* Forward declarations */
struct Ngap_UserLocationInformationEUTRA;
struct Ngap_UserLocationInformationNR;
struct Ngap_UserLocationInformationN3IWF;
struct Ngap_ProtocolIE_SingleContainer;

/* Ngap_UserLocationInformation */
typedef struct Ngap_UserLocationInformation {
  Ngap_UserLocationInformation_PR present;
  union Ngap_UserLocationInformation_u {
    struct Ngap_UserLocationInformationEUTRA* userLocationInformationEUTRA;
    struct Ngap_UserLocationInformationNR* userLocationInformationNR;
    struct Ngap_UserLocationInformationN3IWF* userLocationInformationN3IWF;
    struct Ngap_ProtocolIE_SingleContainer* choice_Extensions;
  } choice;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} Ngap_UserLocationInformation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ngap_UserLocationInformation;
extern asn_CHOICE_specifics_t asn_SPC_Ngap_UserLocationInformation_specs_1;
extern asn_TYPE_member_t asn_MBR_Ngap_UserLocationInformation_1[4];
extern asn_per_constraints_t asn_PER_type_Ngap_UserLocationInformation_constr_1;

#ifdef __cplusplus
}
#endif

#endif /* _Ngap_UserLocationInformation_H_ */
#include <asn_internal.h>
