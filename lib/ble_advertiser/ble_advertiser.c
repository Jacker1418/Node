#include "ble_advertiser.h"

static uint8_t advPacket[SIZE_BUFFER_MAX_LENGTH];

/**
 * @brief 
 * 
 * @todo
 * 1. setPacketAD() call - Flag AD
 * 2. addPacketAD() call
 * 3. setPacketHeader() call 
 * 4. setPacketAD() call - Device Name
 * 5. addPacketAD() call
 * 6. setPacketHeader() call
 * 7. setAdvPacket() call
 */
void init_ble_advertiser(void)
{
    struct pktPDU_Header    pktHeader;
    struct pktPDU_Payload   pktPayload;
    struct pktPDU_AD        pktAd;

    clearPDU_Header(&pktHeader);
    clearPDU_Payload(&pktPayload);
    clearPDU_AD(&pktAd);

    setPacketAD(&pktAd, DATA_TYPE_FLAGS, NULL);
    addPacketAD(&pktPayload, &pktAd);

    setPacketAD(&pktAd, DATA_TYPE_COMPLETE_LOCAL_NAME, "Node");
    addPacketAD(&pktPayload, &pktAd);

    setPacketHeader(&pktHeader, PDU_TYPE_ADV_IND, pktPayload.lenPayload);

    setAdvPacket(advPacket, &pktHeader, &pktPayload);
}

err_code setAdvPacket(uint8_t *out_packet, struct pktPDU_Header *in_header, struct pktPDU_Payload *in_payload)
{
    
}

static err_code setPacketHeader(struct pktPDU_Header* out_packet, uint8_t in_type, uint8_t in_payload_length)
{
    

}

static err_code setPacketAD(struct pktPDU_AD* out_ad, uint8_t in_type, uint8_t* in_data)
{
    uint8_t length = 0;

    if(out_ad == NULL) return ERROR_PARAM_NULL;

    if( (in_type != DATA_TYPE_FLAGS) & in_data == NULL ) return ERROR_PARAM_NULL;

    switch(in_type)
    {
        case DATA_TYPE_FLAGS:
            out_ad->lengthAD = SIZE_FIELD_TYPE + 1;
            out_ad->dataAD[0] = 0x05;
        break;

        case DATA_TYPE_COMPLETE_UUID_128BIT:

        break;

        case DATA_TYPE_SHORTENED_LOCAL_NAME:

        break;

        case DATA_TYPE_COMPLETE_LOCAL_NAME:
            for(uint8_t i = 0; in_data[i] != NULL; i++)
            {
                out_ad->dataAD[i] = in_data[i];
                length++;
            }
            out_ad->lengthAD = length + SIZE_FIELD_TYPE;
        break;

        case DATA_TYPE_MANUFACTURER_SPECIFIC_DATA:

        break;

        default:
            return ERROR_PARAM_WRONG;
    }

    out_ad->typeAD = in_type;

    return ERROR_NONE;
}

err_code addPacketAD(struct pktPDU_Payload* in_out_packet, struct pktPDU_AD* in_packet_ad)
{
    uint8_t lenPayload = 0;
    uint8_t index = 0;
    struct pktPDU_AD* data = NULL;

    if(in_out_packet == NULL || in_packet_ad == NULL) return ERROR_PARAM_NULL;

    index = in_out_packet->idxAD;
    
    if(index > SIZE_MAX_AD) return ERROR_PARAM_WRONG;

    data = &(in_out_packet->arrPacketAD[index]);

    memcpy(data, in_packet_ad, sizeof(struct pktPDU_AD));
    
    in_out_packet->lenPayload += data->lengthAD + SIZE_FIELD_LENGTH;

    in_out_packet->idxAD = ++index;

    return ERROR_NONE;

}

err_code delPacketAD(struct pktPDU_Payload* in_out_packet, struct pktPDU_AD* in_packet_ad)
{
    struct pktPDU_AD* data = NULL;
    struct pktPDU_AD* temp = NULL;
    uint8_t tmpIndexAD = 0;

    if(in_out_packet == NULL || in_packet_ad == NULL) return ERROR_PARAM_NULL;

    tmpIndexAD = in_out_packet->idxAD;
    
    for(uint8_t idxSearch = 0; idxSearch < tmpIndexAD; idxSearch++)
    {
        data = &(in_out_packet->arrPacketAD[idxSearch]);

        if(data->typeAD == in_packet_ad->typeAD)
        {
            for(uint8_t index = idxSearch; index < SIZE_MAX_AD - 1; index++)
            {
                temp = &(in_out_packet->arrPacketAD[index + 1]);
                memcpy(data, temp, sizeof(struct pktPDU_AD));
                data = temp;
            }

            tmpIndexAD--;
            in_out_packet->idxAD = tmpIndexAD;
            
            break;
        }
    }

    return ERROR_NONE;
}