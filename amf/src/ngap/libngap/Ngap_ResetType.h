/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NGAP-IEs"
 * 	found in "asn.1/Information Element Definitions.asn1"
 * 	`asn1c -pdu=all -fcompound-names -fno-include-deps -findirect-choice
 * -no-gen-example -gen-APER -gen-UPER -no-gen-JER -gen-BER -D src`
 */

#ifndef _Ngap_ResetType_H_
#define _Ngap_ResetType_H_

#include <asn_application.h>

/* Including external dependencies */
#include "Ngap_ResetAll.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Ngap_ResetType_PR {
  Ngap_ResetType_PR_NOTHING, /* No components present */
  Ngap_ResetType_PR_nG_Interface,
  Ngap_ResetType_PR_partOfNG_Interface,
  Ngap_ResetType_PR_choice_Extensions
} Ngap_ResetType_PR;

/* Forward declarations */
struct Ngap_UE_associatedLogicalNG_connectionList;
struct Ngap_ProtocolIE_SingleContainer;

/* Ngap_ResetType */
typedef struct Ngap_ResetType {
  Ngap_ResetType_PR present;
  union Ngap_ResetType_u {
    Ngap_ResetAll_t nG_Interface;
    struct Ngap_UE_associatedLogicalNG_connectionList* partOfNG_Interface;
    struct Ngap_ProtocolIE_SingleContainer* choice_Extensions;
  } choice;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} Ngap_ResetType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ngap_ResetType;
extern asn_CHOICE_specifics_t asn_SPC_Ngap_ResetType_specs_1;
extern asn_TYPE_member_t asn_MBR_Ngap_ResetType_1[3];
extern asn_per_constraints_t asn_PER_type_Ngap_ResetType_constr_1;

#ifdef __cplusplus
}
#endif

#endif /* _Ngap_ResetType_H_ */
#include <asn_internal.h>
