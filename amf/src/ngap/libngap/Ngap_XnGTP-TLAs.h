/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NGAP-IEs"
 * 	found in "asn.1/Information Element Definitions.asn1"
 * 	`asn1c -pdu=all -fcompound-names -fno-include-deps -findirect-choice
 * -no-gen-example -gen-APER -gen-UPER -no-gen-JER -gen-BER -D src`
 */

#ifndef _Ngap_XnGTP_TLAs_H_
#define _Ngap_XnGTP_TLAs_H_

#include <asn_application.h>

/* Including external dependencies */
#include "Ngap_TransportLayerAddress.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Ngap_XnGTP-TLAs */
typedef struct Ngap_XnGTP_TLAs {
  A_SEQUENCE_OF(Ngap_TransportLayerAddress_t) list;

  /* Context for parsing across buffer boundaries */
  asn_struct_ctx_t _asn_ctx;
} Ngap_XnGTP_TLAs_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ngap_XnGTP_TLAs;
extern asn_SET_OF_specifics_t asn_SPC_Ngap_XnGTP_TLAs_specs_1;
extern asn_TYPE_member_t asn_MBR_Ngap_XnGTP_TLAs_1[1];
extern asn_per_constraints_t asn_PER_type_Ngap_XnGTP_TLAs_constr_1;

#ifdef __cplusplus
}
#endif

#endif /* _Ngap_XnGTP_TLAs_H_ */
#include <asn_internal.h>
