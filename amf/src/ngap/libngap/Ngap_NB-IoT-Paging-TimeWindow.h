/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NGAP-IEs"
 * 	found in "asn.1/Information Element Definitions.asn1"
 * 	`asn1c -pdu=all -fcompound-names -fno-include-deps -findirect-choice
 * -no-gen-example -gen-APER -gen-UPER -no-gen-JER -gen-BER -D src`
 */

#ifndef _Ngap_NB_IoT_Paging_TimeWindow_H_
#define _Ngap_NB_IoT_Paging_TimeWindow_H_

#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Ngap_NB_IoT_Paging_TimeWindow {
  Ngap_NB_IoT_Paging_TimeWindow_s1  = 0,
  Ngap_NB_IoT_Paging_TimeWindow_s2  = 1,
  Ngap_NB_IoT_Paging_TimeWindow_s3  = 2,
  Ngap_NB_IoT_Paging_TimeWindow_s4  = 3,
  Ngap_NB_IoT_Paging_TimeWindow_s5  = 4,
  Ngap_NB_IoT_Paging_TimeWindow_s6  = 5,
  Ngap_NB_IoT_Paging_TimeWindow_s7  = 6,
  Ngap_NB_IoT_Paging_TimeWindow_s8  = 7,
  Ngap_NB_IoT_Paging_TimeWindow_s9  = 8,
  Ngap_NB_IoT_Paging_TimeWindow_s10 = 9,
  Ngap_NB_IoT_Paging_TimeWindow_s11 = 10,
  Ngap_NB_IoT_Paging_TimeWindow_s12 = 11,
  Ngap_NB_IoT_Paging_TimeWindow_s13 = 12,
  Ngap_NB_IoT_Paging_TimeWindow_s14 = 13,
  Ngap_NB_IoT_Paging_TimeWindow_s15 = 14,
  Ngap_NB_IoT_Paging_TimeWindow_s16 = 15
  /*
   * Enumeration is extensible
   */
} e_Ngap_NB_IoT_Paging_TimeWindow;

/* Ngap_NB-IoT-Paging-TimeWindow */
typedef long Ngap_NB_IoT_Paging_TimeWindow_t;

/* Implementation */
extern asn_per_constraints_t
    asn_PER_type_Ngap_NB_IoT_Paging_TimeWindow_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_Ngap_NB_IoT_Paging_TimeWindow;
extern const asn_INTEGER_specifics_t
    asn_SPC_Ngap_NB_IoT_Paging_TimeWindow_specs_1;
asn_struct_free_f Ngap_NB_IoT_Paging_TimeWindow_free;
asn_struct_print_f Ngap_NB_IoT_Paging_TimeWindow_print;
asn_constr_check_f Ngap_NB_IoT_Paging_TimeWindow_constraint;
ber_type_decoder_f Ngap_NB_IoT_Paging_TimeWindow_decode_ber;
der_type_encoder_f Ngap_NB_IoT_Paging_TimeWindow_encode_der;
xer_type_decoder_f Ngap_NB_IoT_Paging_TimeWindow_decode_xer;
xer_type_encoder_f Ngap_NB_IoT_Paging_TimeWindow_encode_xer;
oer_type_decoder_f Ngap_NB_IoT_Paging_TimeWindow_decode_oer;
oer_type_encoder_f Ngap_NB_IoT_Paging_TimeWindow_encode_oer;
per_type_decoder_f Ngap_NB_IoT_Paging_TimeWindow_decode_uper;
per_type_encoder_f Ngap_NB_IoT_Paging_TimeWindow_encode_uper;
per_type_decoder_f Ngap_NB_IoT_Paging_TimeWindow_decode_aper;
per_type_encoder_f Ngap_NB_IoT_Paging_TimeWindow_encode_aper;

#ifdef __cplusplus
}
#endif

#endif /* _Ngap_NB_IoT_Paging_TimeWindow_H_ */
#include <asn_internal.h>
