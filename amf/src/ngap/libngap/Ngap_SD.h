/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NGAP-IEs"
 * 	found in "asn.1/Information Element Definitions.asn1"
 * 	`asn1c -pdu=all -fcompound-names -fno-include-deps -findirect-choice
 * -no-gen-example -gen-APER -gen-UPER -no-gen-JER -gen-BER -D src`
 */

#ifndef _Ngap_SD_H_
#define _Ngap_SD_H_

#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Ngap_SD */
typedef OCTET_STRING_t Ngap_SD_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_Ngap_SD_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_Ngap_SD;
asn_struct_free_f Ngap_SD_free;
asn_struct_print_f Ngap_SD_print;
asn_constr_check_f Ngap_SD_constraint;
ber_type_decoder_f Ngap_SD_decode_ber;
der_type_encoder_f Ngap_SD_encode_der;
xer_type_decoder_f Ngap_SD_decode_xer;
xer_type_encoder_f Ngap_SD_encode_xer;
oer_type_decoder_f Ngap_SD_decode_oer;
oer_type_encoder_f Ngap_SD_encode_oer;
per_type_decoder_f Ngap_SD_decode_uper;
per_type_encoder_f Ngap_SD_encode_uper;
per_type_decoder_f Ngap_SD_decode_aper;
per_type_encoder_f Ngap_SD_encode_aper;

#ifdef __cplusplus
}
#endif

#endif /* _Ngap_SD_H_ */
#include <asn_internal.h>
