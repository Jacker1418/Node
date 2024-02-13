#ifndef __BLE_INIT_H__
#define __BLE_INIT_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "nrf52.h"

typedef uint8_t err_code;

/**
@brief
- BLE Packet의 Field별 사이즈 정보 기입
- byte 단위
@note
- SIZE_MAX_AD는 Advertising Packet의 Payload영역에 넣을 수 있는 Data Unit의 최대치이다.
@note
- 현재 Default Flag type 이외에 Device Name이나 Manufacturer Specific Data, UUID 등 추가할 Type의 개수이다
@note
- 하지만 이는 제약조건이 아닌 하단 struct pktPDU_Payload의 선언을 위한 용도로만 사용
@note
- 실제 SIZE_ADV_PAYLOAD나 SIZE_EXT_PAYLOAD의 Payload 사이즈가 제약조건이다. 
@param SIZE_ACC_ADDR PDU Header 앞단 Access Address Field의 사이즈
@param SIZE_DEV_ADDR PDU Payload 앞단 Device Address(=Advertising Address) Field의 사이즈
@param SIZE_ADV_PAYLOAD 실질적인 Advertising Packet의 Data 영역
@param SIZE_EXT_PAYLOAD 위 Advertising Packet의 Extention Packet의 Payload 사이즈 

*/
enum SIZE_PACKET{
    SIZE_ACC_ADDR = 4,
    SIZE_DEV_ADDR = 6,
    SIZE_FIELD_TYPE = 1,
    SIZE_FIELD_LENGTH = 1,
    SIZE_ADV_PAYLOAD = 31,
    SIZE_EXT_PAYLOAD = 254,
    SIZE_MAX_AD = 3,
    SIZE_BUFFER_MAX_LENGTH = 255,
    SIZE_BUFFER_COUNT = 3,
};

enum ERROR_PACKET{
    ERROR_NONE,
    ERROR_PAYLOAD_SIZE,
    ERROR_PARAM_NULL,
    ERROR_PARAM_WRONG,
};

enum PACKET_HEADER_PARAMETER{

    /**
     * @brief Header field of PDU
     * 
     */
    PDU_TYPE_ADV_IND        = 0x00,
    CHANNEL_SELECTION_2     = 0x20,
    TX_ADDRESS_TYPE_RANDOM  = 0x40,
    TX_ADDRESS_TYPE_RANDOM  = 0x80,

    DATA_TYPE_FLAGS                 = 0x01,
    DATA_TYPE_COMPLETE_UUID_128BIT  = 0x07,
    DATA_TYPE_SHORTENED_LOCAL_NAME  = 0x08,
    DATA_TYPE_COMPLETE_LOCAL_NAME   = 0x09,
    DATA_TYPE_DEVICE_ID             = 0x10,
    DATA_TYPE_MANUFACTURER_SPECIFIC_DATA = 0xFF,
};

struct pktPDU_Header{
    uint8_t def;
    uint8_t type;
    uint8_t txAddr;
    uint8_t rxAddr;
    uint8_t length;
};

struct pktPDU_AD{
    uint8_t lengthAD;
    uint8_t typeAD;
    uint8_t dataAD[SIZE_EXT_PAYLOAD];
};

struct pktPDU_Payload{
    uint8_t idxAD;
    uint8_t lenPayload;
    uint8_t deviceAddr[SIZE_DEV_ADDR];
    struct pktPDU_AD arrPacketAD[SIZE_MAX_AD];
};

const uint8_t FREQ_ADV[3] = {2, 26, 80};

#define BASE_UUID {0xCF, 0x80, 0xA5, 0xCC, 0x76, 0x7A, 0xFF, 0xA9, 0x0B, 0x41, 0xB5, 0xA5, 0x00, 0x00, 0xD4, 0xBF}

#define BLE_UUID_SERVICE 0x0001 
#define BLE_UUID_RX_CHARACTERISTIC 0x0002              
#define BLE_UUID_TX_CHARACTERISTIC 0x0003      

err_code init_ble(void);
err_code clearPDU_Header(struct pktPDU_Header* in_out_header);
err_code clearPDU_Payload(struct pktPDU_Payload* in_out_payload);
err_code clearPDU_AD(struct pktPDU_AD* in_out_ad);

#endif
