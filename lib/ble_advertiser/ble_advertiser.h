#ifndef __BLE_ADVERTISER_H__
#define __BLE_ADVERTISER_H__

#include "ble_init.h"

/* Define functions [Form] */
/**@brief  The description of function 
 *
 * @param[in]   This is the description about input of parameter
 * @param[out]  output through parameter using pointer
 *
 * @retval  NRF_SUCCESS    
 * 
 * @warning
 * 
 * @note
 */
void init_ble_advertiser(void);

err_code setAdvPacket(uint8_t *out_packet, struct pktPDU_Header *in_header, struct pktPDU_Payload *in_payload);
static err_code setPacketHeader(struct pktPDU_Header* out_packet, uint8_t in_type, uint8_t in_payload_length);

static err_code setPacketAD(struct pktPDU_AD* out_ad, uint8_t in_type, uint8_t* in_data);
err_code addPacketAD(struct pktPDU_Payload* in_out_packet, struct pktPDU_AD* in_packet_ad);
err_code delPacketAD(struct pktPDU_Payload* in_out_packet, struct pktPDU_AD* in_packet_ad);

#endif

