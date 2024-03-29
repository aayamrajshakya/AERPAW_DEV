/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NGAP-IEs"
 * 	found in "asn.1/Information Element Definitions.asn1"
 * 	`asn1c -pdu=all -fcompound-names -fno-include-deps -findirect-choice
 * -no-gen-example -gen-APER -gen-UPER -no-gen-JER -gen-BER -D src`
 */

#ifndef _Ngap_UPTransportLayerInformation_H_
#define _Ngap_UPTransportLayerInformation_H_

#include <asn_application.h>

/* Including external dependencies */
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Ngap_UPTransportLayerInformation_PR {
  Ngap_UPTransportLayerInformation_PR_NOTHING, /* No components present */
  Ngap_UPTransportLayerInformation_PR_gTPTunnel,
  Ngap_UPTransportLayerInformation_PR_choice_Extensions
} Ngap_UPTransportLayerInformation_PR;

/* Forward declarations */
struct Ngap_GTPTunnel;
struct Ngap_ProtocolIE_SingleContainer;

/* Ngap_UPTransportLayerInformation */
typedef struct Ngap_UPTransportLayerInformation {
  Ngap_UPTransportLayerInformation_PR present;
  union Ngap_UPTransportLayerInformation_u {
    struct Ngap_GTPTunnel* gTPTunnel;
    struct Ngap_ProtocolIE_SingleContainer* choice_Extensions;
  } choice;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} Ngap_UPTransportLayerInformation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ngap_UPTransportLayerInformation;
extern asn_CHOICE_specifics_t asn_SPC_Ngap_UPTransportLayerInformation_specs_1;
extern asn_TYPE_member_t asn_MBR_Ngap_UPTransportLayerInformation_1[2];
extern asn_per_constraints_t
    asn_PER_type_Ngap_UPTransportLayerInformation_constr_1;

#ifdef __cplusplus
}
#endif

#endif /* _Ngap_UPTransportLayerInformation_H_ */
#include <asn_internal.h>
