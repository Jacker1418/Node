#include "ble_init.h"

err_code init_ble()
{

}

err_code clearPDU_Header(struct pktPDU_Header* in_out_header)
{
    if(in_out_header == NULL) return ERROR_PARAM_NULL;

    in_out_header->def = 0x60;
    in_out_header->type = 0x00;
    in_out_header->rxAddr = 0x00;
    in_out_header->txAddr = 0x00;
    in_out_header->length = SIZE_DEV_ADDR;

    return ERROR_NONE;
}

err_code clearPDU_Payload(struct pktPDU_Payload* in_out_payload)
{
    if(in_out_payload == NULL) return ERROR_PARAM_NULL;

    in_out_payload->idxAD = 0x00;
    in_out_payload->lenPayload = 0x00;
    
    uint32_t deviceAddress_0 = NRF_FICR->DEVICEADDR[0];
    uint32_t deviceAddress_1 = NRF_FICR->DEVICEADDR[1];

    in_out_payload->deviceAddr[0] = (uint8_t)((deviceAddress_1 & 0x0000FF00) >> 8);
    in_out_payload->deviceAddr[1] = (uint8_t)((deviceAddress_1 & 0x000000FF));
    in_out_payload->deviceAddr[2] = (uint8_t)((deviceAddress_0 & 0xFF000000) >> 24);
    in_out_payload->deviceAddr[3] = (uint8_t)((deviceAddress_0 & 0x00FF0000) >> 16);
    in_out_payload->deviceAddr[4] = (uint8_t)((deviceAddress_0 & 0x0000FF00) >> 8);
    in_out_payload->deviceAddr[5] = (uint8_t)((deviceAddress_0 & 0x000000FF));

    for(uint8_t index = 0; index < SIZE_MAX_AD; index++)
    {
        clearPDU_AD(in_out_payload->arrPacketAD);
    }

    return ERROR_NONE;
}

err_code clearPDU_AD(struct pktPDU_AD* in_out_ad)
{
    if(in_out_ad == NULL) return ERROR_PARAM_NULL;

    memset(in_out_ad, 0, sizeof(struct pktPDU_AD));

    return ERROR_NONE;
}