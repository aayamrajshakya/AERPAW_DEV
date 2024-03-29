/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NGAP-IEs"
 * 	found in "asn.1/Information Element Definitions.asn1"
 * 	`asn1c -pdu=all -fcompound-names -fno-include-deps -findirect-choice
 * -no-gen-example -gen-APER -gen-UPER -no-gen-JER -gen-BER -D src`
 */

#ifndef _Ngap_SerialNumber_H_
#define _Ngap_SerialNumber_H_

#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Ngap_SerialNumber */
typedef BIT_STRING_t Ngap_SerialNumber_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_Ngap_SerialNumber_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_Ngap_SerialNumber;
asn_struct_free_f Ngap_SerialNumber_free;
asn_struct_print_f Ngap_SerialNumber_print;
asn_constr_check_f Ngap_SerialNumber_constraint;
ber_type_decoder_f Ngap_SerialNumber_decode_ber;
der_type_encoder_f Ngap_SerialNumber_encode_der;
xer_type_decoder_f Ngap_SerialNumber_decode_xer;
xer_type_encoder_f Ngap_SerialNumber_encode_xer;
oer_type_decoder_f Ngap_SerialNumber_decode_oer;
oer_type_encoder_f Ngap_SerialNumber_encode_oer;
per_type_decoder_f Ngap_SerialNumber_decode_uper;
per_type_encoder_f Ngap_SerialNumber_encode_uper;
per_type_decoder_f Ngap_SerialNumber_decode_aper;
per_type_encoder_f Ngap_SerialNumber_encode_aper;

#ifdef __cplusplus
}
#endif

#endif /* _Ngap_SerialNumber_H_ */
#include <asn_internal.h>
