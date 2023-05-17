## Advertiser Code 분석
### main
  - sd_softdevice_enable((uint32_t)NRF_CLOCK_LFCLKSRC_XTAL_75_PPM, sd_assert_cb) [SoftDevice]
    - SoftDevice 활성화
    - NRF_CLOCK_LFCLKSRC_XTAL_75_PPM : Low Frequency 설정 
    - sd_assert_cb : SoftDevice Init error 발생 시, Debugging용 Callback 함수
  
  - sd_nvic_EnableIRQ(SD_EVT_IRQn) [SoftDevice]
    - SoftDevice 관련 인터럽트 활성화
    - SD_EVT_IRQn : SoftDevice 인터럽트
  
  - ble_setup() [User]
    - Advertiser 모드로 Advertising Packet 및 RADIO Peripheral 설정
    - btle_hci_adv_init(SWI0_IRQn) [User]
      - SWI0_IRQn : User용 SW 인터럽트 0번 
      - nrf_report_disp_init(btle_hci_adv_evt_irq) [User]
        - 이벤트 Dispatch 설정
        - btle_hci_adv_evt_irq = SWI0_IRQn
        - NVIC_SetPriority(irq, 3) [Library]
          - 인터럽트 Priority 설정
          - irq = SWI0_IRQn
          - 3 : User 인터럽트 중 Low priority로 설정 
        - NVIC_EnableIRQ(irq) [Library]
          - 인터럽트 활성화
          - irq = SWI0_IRQn
          - nrf_report_fifo_init(&report_fifo, &evt_buffer[0], EVENT_DISPATCHER_FIFO_SIZE); [User]
            - ctrl_init() [User]
            - sd_radio_session_open(&radio_signal_callback) [SoftDevice]
              - radio_signal_callback : Timeslot Callback 함수
    - Advertiser Address 설정
      - ble_addr[] = {0x4e, 0x6f, 0x72, 0x64, 0x69, 0x63}
    - Advertiser Parameter 설정
      - adv_params.channel_map = BTLE_CHANNEL_MAP_ALL : Advertising할 Channel 설정으로 Channel 37, 38, 39 사용으로 설정 
        - 추후 `channel_iterate()`함수에서 Channel 변경 시, 해당 변수가 이용됨
      - adv_params.direct_address_type = BTLE_ADDR_TYPE_RANDOM : Advertiser의 Address의 Type
      - adv_params.filter_policy = BTLE_ADV_FILTER_ALLOW_ANY : Filter는 없음
      - adv_params.interval_min = BLE_ADV_INTERVAL_100MS : Advertising Interval의 MIN
      - adv_params.interval_max = BLE_ADV_INTERVAL_150MS  : Advertising Interval의 MAX
        - MIN, MAX 값은 추후 Timeslot 요청 시, Distance 계산에서 사용됨
      - adv_params.own_address_type = BTLE_ADDR_TYPE_RANDOM
      - adv_params.type = BTLE_ADV_TYPE_SCAN_IND : Advertising Packet의 Type으로 `ADV_SCAN_IND`으로 설정하였다.
        - `ADV_SCAN_IND` Type은 Scannable Undirected Advertising으로써, Connection Request를 수락하지 않고, Scan Request만 수락하는 모드이다
        - BLUETOOTH CORE SPECIFICATION Version 5.2 | Vol 6, Part B, Table 4.2 참고
    - Advertising Packet 설정
    ``` C
    ble_adv_data[] =
    {
      /* Flags: */
      0x02,                   /* length */
      0x01,                   /* type (flags) */
      0x00,                   /* AD Flags*/
      /* Appearance: */
      0x03,                   /* length */
      0x19,                   /* type (Appearance) */
      0x00, 0x00,             /* Generic unspecified */
      /* Role: */
      0x02,                   /* length */
      0x1c,                   /* type (LE role) */
      0x00,                   /* Only peripheral role supported */
      /* Complete local name: */
      0x0D,                   /* length */
      0x09,                   /* type (complete local name) */
      'T', 'i', 'm', 'e', 's', 'l', 'o', 't', ' ', 'a', 'd', 'v',
    };
    ```
    - Advertising 시작을 위한 Timeslot 요청
      - `btle_hci_adv_enable()` 함수 -> `ctrl_timeslot_order()` 함수 -> `timeslot_req_initial()` 함수 -> 최종 `sd_radio_request(&g_timeslot_req_earliest)` 함수 요청
      - Timeslot 요청 Parameter로 `g_timeslot_req_earliest` 설정은 아래 설명 참고

  - nrf_adv_conn_init() [User]
    - Nordic SoftDevice를 통한 Advertising 설정하는 함수로써, Connectable Scannable Advertising 모드로 설정한다
    - 전체 Code Scheme는 BLE + Advertiser + Observer이다.
    - BLE는 Connection을 담당하고, Advertiser는 사실상 Beacon이며, Observer는 Scanner를 수행한다.
  
### Advertiser의 Callback 함수 (ts_controller.c)
- nrf_radio_signal_callback_return_param_t* radio_signal_callback(uint8_t sig) [User]
  - RADIO TIMESOLT용 Signal Handler 함수
  - RADIO Timeslot을 사용할 경우, 위 Handler 함수를 무조건 정의해야 하며 이를 사용해야 한다.

  - `@return`
    - nrf_radio_signal_callback_return_param_t 자료형으로 Handler 처리 후 SoftDevice에게 처리 결과를 전달해야한다.
    - `NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE` : Timeslot 처리를 아직 하지 못했다. SoftDevice는 아무런 Action을 취하지 않는다.
    - `NRF_RADIO_SIGNAL_CALLBACK_ACTION_END` : 현재 Timeslot은 마무리하였다. SoftDevice는 다른 Activity를 재개한다.
    - `NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END` : 현재 Timeslot은 마무리하였다. SoftDevice는 새로운 Timeslot을 요청하고, 이후부터 기존의 Activity를 재개한다.
    - `NRF_RADIO_SIGNAL_CALLBACK_ACTION_EXTEND` : 현재의 Timeslot를 더 연장하도록 SoftDevice에게 요청한다.

  - `@param` 
    - SoftDevice가 RADIO Timeslot Handler에게 전달하는 Signal이다
    - `NRF_RADIO_CALLBACK_SIGNAL_TYPE_START` : Timeslot의 시작 포인트. App은 Timeslot의 기간동안 모든 Peripheral에 접근을 독점한다.
    - `NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO` : RADIO 인터럽트
    - `NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0` : TIMER0 인터럽트
    - `NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_SUCCEEDED` : 마지막 Timeslot 확장 요청이 성공함
    - `NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_FAILED` : 마지막 Timeslot 확장 요청이 실패함

  - `@action` 
    - Handler의 return은 `NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE`으로 설정한다.
    - ctrl_signal_handler() [User]
      - `inline`함수로써, 실제 Signal을 처리하는 함수이다.
      - Signal에 대해 Switch-Case 문으로 처리함

        - `NRF_RADIO_CALLBACK_SIGNAL_TYPE_START`
          - adv_evt_setup() [User]
            - 최초 adv_send State가 되기전에 설정하기위한 함수

            - periph_radio_setup() [User]
              - Radio Peripheral 설정 함수
              - NRF_RADIO->POWER = 1 : RADIO Peripheral 활성화로 해당 Register를 0->1되면 모든 상태가 Reset됨
              - NRF_RADIO->EVENTS_DISABLED = 0 : RADIO State 중 DISABLED State일 때 발생하는 Event Reg로, 최초 설정 시 0으로 Unset
              - NRF_TIMER0->PRESCALER = 4 : TIMER0를 1us 해상도를 가진 타이머로 설정
              - NRF_TIMER0->TASKS_STOP = 1 : 일단 정지로 설정

              - radio_init() [User]
                - 상세 RADIO Peripheral 설정 함수
                - NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos) : RADIO TX Power를 설정하는 Reg로, 0dBm로 설정
                - NRF_RADIO->MODE = (RADIO_MODE_MODE_Ble_1Mbit << RADIO_MODE_MODE_Pos) : RADIO의 PHY 설정으로 1Mbps로 설정
                - NRF_RADIO->FREQUENCY = 2 : RADIO Channel에 맞춰 Frequency를 직접 설정하는 Reg로, 0 ~ 100까지 설정이 가능하고, 40MHz Unit으로 계산하여 2400MHz + (n * 40MHz)으로 Channel의 Frquency를 맞춘다.
                  - Channel 39 = 2480MHz이며, 현재 설정한 Channel Frquency이다
                - NRF_RADIO->DATAWHITEIV = 37 : Data Whitening 값을 설정하는 Reg (Bluetooth Core Specification Version 5.2 | Vol 6, Part B, 3.2 참고)
                - NRF_RADIO->PREFIX0 = 0x8e : RADIO Packet 중에서 Address에 해당되는 영역 중 Prefix0 주소값을 설정하는 Reg
                - NRF_RADIO->BASE0 = 0x89bed600 : RADIO Packet 중에서 Address에 해당되는 영역 중 Base0 주소값을 설정하는 Reg
                  - BASE address + Prefix0 Address = BLE Packet Address
                - NRF_RADIO->TXADDRESS = 0x00 : RADIO TX Packet의 Address를 BASE0와 PREFIX0 조합을 사용한다.
                - NRF_RADIO->RXADDRESSES = 0x01 : RADIO RX Packet의 Address를 BASE1과 PREFIX1 조합을 사용한다.
                - NRF_RADIO->PCNF0 : Packet의 S0, LENGTH, S1 Field를 설정하는 구간
                  - Packet의 전체 길이를 S0, LENGTH, S1의 설정값으로 결정한다.
                  - S0LEN = 1 : byte unit으로 값 입력 / 0 ~ 1 설정 가능 
                  - S1LEN = 2 : bit unit으로 값 입력 / 0 ~ 8 설정 가능
                  - LFLEN = 6 : bit unit으로 값 입력 / 0 ~ 8 설정 가능
                  - Code에서는 `S0LEN = 1`, `S1LEN = 2`, `LFLEN = 6`으로 설정하였고, 이는 1byte + 2bit + 6bit = 2byte로 설정되었다.
                  - 나머지 `S1INCL`과 `PLEN` Field는 초기화하지 않았다. 그러므로 나머지 영역은 0으로 설정
                  - S1INCL = 0 : S1 Field를 RAM에 둘것인지 배제할것인지 결정
                    - 0 : S1 Field를 RAM에 둔다.
                  - PLEN = 0 : RADIO Packet의 Preamble의 길이를 결정
                    - 0 : Preamble Length를 8bit로 설정
                    - 위는 PHY에 따라 달라진다. 1Mbps이면 Preamble Length = 8bit, 2Mbps이면 Preamble Length = 16bit로 설정한다.
                - NRF_RADIO->PCNF1 : Packet에 대한 여러 설정을 하는 Reg
                  - MAXLEN = 37UL : 송수신할때, Packet의 전체 Length로써 실제 전송되는 Packet의 Length이며, Packet의 LENGTH 영역의 값이 MAXLEN의 값보다 클 경우 Packet을 MAXLEN 단위로 나누어 전송
                  - STATLEN = 0UL : Static payload add-on 영역의 Length를 결정하는 영역 / Static add-on 영역은 Packet의 구조에서 PAYLOAD와 CRC 영역 사이에 존재하지만 BLE Packet에서는 존재하지 않는 영역이므로 Static payload add-on 영역의 Length값을 0으로 설정
                  - BALEN = 3UL : Base address를 byte 단위로 입력, address 영역은 base address + 1byte의 prefix address / 현재 설정은 Base address 3Byte + Prefix address 1Byte = 총 Address Length 4Byte
                  - ENDIAN = RADIO_PCNF1_ENDIAN_Little : S0, LENGTH, S1, PAYLOAD 영역의 Endian를 LSB First인 Little Endian으로 설정
                  - WHITEEN = 1UL : Packet Whiteen 가능 활성화
                - NRF_RADIO->CRCCNF
                  - LEN = RADIO_CRCCNF_LEN_Three : CRC의 Length를 byte단위로 설정 / CRC Length = 3byte
                  - SKIPADDR = RADIO_CRCCNF_SKIPADDR_Skip : CRC의 계산에서 Address영역은 제외
                - NRF_RADIO->CRCINIT = 0x555555 : 최초 CRC Initial Value
                - NRF_RADIO->CRCPOLY = 0x00065B : CRC polynomial 공식 설정 
                - NRF_RADIO->TIFS = 145 : Frame간의 사이 간격(Inter Frame Spacing) 시간을 us단위로 설정 / 145us 설정 (Bluetooth Core Specificatin v5.2 p2927에서는 IFS = 150us로 설정) / 뭐지???
              - NRF_RADIO->PACKETPTR = (uint32_t) &packet[0] : 전송할 Packet의 시작 주소값을 설정하는 Reg
              
              - NVIC_EnableIRQ(RADIO_IRQn) [Library]
                - RADIO_IRQn 인터럽트 활성화
                
            - channel = 36 : 
            - channel_iterate() [User]
              - 다음 Channel로 계산하기 위한 함수
              - 함수 호출 전 위 `channel` 변수를 36으로 초기화하였고, 함수 호출을 통해 다음 Channel 37로 변수 값을 변경
              - BLE의 Advertising Channel은 3개로 37, 38, 39이다.
              
          - sm_enter_adv_send() [User]
            - Advertising으로 Data Packet을 전송하기 위한 설정 및 활성화 
            - sm_state = STATE_ADV_SEND : 추후 `ctrl_signal_handler()` 함수에서 `NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO` event를 처리할 때, 기준이 되는 State machine 변수이다.
            
            - periph_radio_ch_set(channel) [User]
              - 인자값으로 넘긴 `channel`를 기준으로 RADIO Peripheral를 설정
              - NRF_RADIO->FREQUENCY = freq_bins[channel - 37] : Channel별 실제 Frequency를 설정해야 하므로, 각 Channel별 주파수값을 저장한 배열 `freq_bin[]`을 이용한다.
              - Channel 37 = 2402MHz /  Channel 38 = 2426MHz / Channel = 2480MHz으로 설정되어야 한다. 
              - NRF_RADIO->DATAWHITEIV = channel : Data whitening 초기값 설정

            - PERIPHERAL_TASK_TRIGGER(NRF_RADIO->TASKS_TXEN) [User]
              - macro 함수로써 매개변수에 True값 전달하는 함수
              - RADIO Peripheral에 TX 전송 ENABLE Trigger를 보내 Advertising Packet이 전송되도록 설정

            - periph_radio_packet_ptr_set(&ble_adv_data[0]) [User]
              - 위 `periph_radio_setup()` 함수에서 정의를 했었지만 main문에서 호출한 `ctrl_init()`에서 설정한 `ble_adv_data[0]`을 Advertising하기 위한 Packet으로 설정한다.

            - periph_radio_shorts_set() [User]
              - Scan Response를 활성화할 경우, RADIO Peripheral의 SHORT를 활성화하여 Packet을 바로 전송할 수 있도록 준비한다.
              - Bluetooth Core specification Version 5.2 | Vol 6, Part B, Fig 4.10 2942p 참고
              - Sequence는 Advertising Packet 전송 -> T_IFS 150us 지연 -> SCan Request 수신 -> T_IFS 150us 지연 -> Scan Response 송신이다
              - 그러므로 Scan Request를 수신하자마자 Scan Response를 송신해야하므로, 빠르게 동작하기 위한 SHORT가 설정되어야 한다.
              - `RADIO_SHORTS_READY_START_Msk` : EVENTS_READY Event -> TASKS_START task로 Trigger SHORT 설정
                - RADIO State (nRF52832 Product Specification v1.4 209p Fig34 : Radio states) 중 TXRU 또는 RXRU state일 때, EVENTS_READY가 발생하면 TXRU -> TXIDLE과 RXRU -> RXIDLE로 State가 이동하고, 각 TXIDLE과 RXIDLE State에서 TASKS_START Trigger로 TXIDLE -> TX와 RXIDLE -> RX로 넘어갈 수 있다.
                - TXRU & RXRU State : Radio 송수신을 준비하는 단계
                - TXIDLE & RXIDLE State : Radio를 송수신 시작할 수 있도록 준비된 단계
                - TX & RX State : Radio Packet 송수신 작업
              - `RADIO_SHORTS_END_DISABLE_Msk` : EVENTS_END Event -> TASKS_DISABLE task zTrigger SHORT 설정
                - TX & RX State에서 Packet을 송수신했다면, EVENTS_END Event가 발생하고 TX -> TXIDLE State 및 RX -> RXIDLE로 State가 바뀌게 된다.
                - 여기서 TASKS_DISABLE Trigger를 전달하면, TXIDLE -> TXDISABLE 및 RXIDLE -> RXDISABLE로 변경된다.
                - 이를 SHORT로 설정하여, TX & RX State에서 Packet을 송수신했다면, 바로 TX -> TXIDLE -> TXDISABLE 그리고 RX -> RXIDLE -> RXDISABLE로 State가 변경 가능하다.
              - `RADIO_SHORTS_DISABLED_RXEN_Msk` : EVENTS_DISABLED Event -> TASKS_RXEN task Trigger SHORT 설정 
                - TXDISABLE & RXDISABLE State에서 DISABLED State로 변경될때, EVENTS_DISABLED Event가 발생한다.
                - 이를 SHORT로 설정하여, EVENT_DISABLED가 발생하면 TASKS_RXEN Trigger로 전달하여, DISABLED -> RXRU State로 이동하도록 설정한다.
                - 바로 수신 준비를 하도록 SHORT를 설정

            - periph_radio_intenset(RADIO_INTENSET_DISABLED_Msk) [User]
              - RADIO Peripheral에서 EVENTS_DISABLED에 대해서 Interrupt를 활성화함

        - `NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO`
          - Timeslot이 유효한 동안 발생하는 RADIO Interrupt Signal
          - RADIO Interrupt 발생 시, State마다 Switch-case문으로 처리한다.
          - `STATE_ADV_SEND` state
            - `sm_enter_adv_send()` 함수 호출 시, `sm_state`를 STATE_ADV_SEND로 변경된다.
            - Signale 처리는 EVENTS_DISABLED일 때만 처리하고, 이는 `sm_enter_adv_send()` 함수에서 설정한 `TASKS_TXEN` Trigger로 Packet을 송신을 시작하고 이후 설정된 SHORT로 인해 TXDISABLE까지 State가 변경되게 된다. 

            - sm_exit_adv_send() [User]
              - 위 State machine에서 Packet을 전송한 후 Advertising Packet 전송 단계를 종료시키기 위하여, EVENTS_DISABLED event에 대해 Interrut Clear를 진행한다.

              - PERIPHERAL_EVENT_CLR(NRF_RADIO->EVENTS_DISABLED) [User]
                - `NRF_RADIO->EVENTS_DISABLED`에 대해 Clear 진행한다.

            - sm_enter_scan_req_rsp() [User]
              - Scan Request으로 Packet을 수신하기 위한 설정 및 활성화 
              - sm_state = STATE_SCAN_REQ_RSP : 추후 `ctrl_signal_handler()` 함수에서 `NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO` event를 처리할 때, 기준이 되는 State machine 변수이다.

              - periph_radio_packet_ptr_set(&ble_rx_buf[0]) [User]
                - Scan Request Packet을 수신하기 위한 Buffer 설정
            
              - periph_radio_shorts_set() [User]
                - Scan Response를 활성화할 경우, RADIO Peripheral의 SHORT를 활성화하여 Scan Request를 수신한 즉시 바로 Scan Response Packet을 전송할 수 있도록 준비한다.
                - Bluetooth Core specification Version 5.2 | Vol 6, Part B, Fig 4.10 2942p 참고
                - Sequence는 Advertising Packet 전송 -> T_IFS 150us 지연 -> Scan Request 수신 -> T_IFS 150us 지연 -> Scan Response 송신이다
                - 그러므로 Scan Request를 수신하자마자 Scan Response를 송신해야하므로, 빠르게 동작하기 위한 SHORT가 설정되어야 한다.
                - `RADIO_SHORTS_READY_START_Msk` : 위 설명 참조
                - `RADIO_SHORTS_END_DISABLE_Msk` : 위 설명 참조
                - `RADIO_SHORTS_DISABLED_TXEN_Msk` : EVENTS_DISABLED Event -> TASKS_TXEN task Trigger SHORT 설정 
                  - TXDISABLE & RXDISABLE State에서 DISABLED State로 변경될때, EVENTS_DISABLED Event가 발생한다.
                  - 이를 SHORT로 설정하여, EVENT_DISABLED가 발생하면 TASKS_TXEN Trigger로 전달하여, DISABLED -> TXRU State로 이동하도록 설정한다.
                  - 이를 통해 Scan Request Packet을 수신하면, Scan Response를 바로 전송할 수 있도록 준비할 수 있게 된다.
                - `RADIO_SHORTS_ADDRESS_RSSISTART_Msk` : EVENTS_ADDRESS Event -> TASKS_RSSISTART Task Trigger SHORT 설정
                  - RX State에서 Packet의 Address까지 받게 되면, EVENTS_ADDRESS가 발생한다. 이때, TASKS_RSSISTART Task에 Trigger로 전달하여, RSSI Sampling을 시작한다.

              - periph_radio_intenset(RADIO_INTENSET_DISABLED_Msk) [User]
                - RADIO Peripheral에서 EVENTS_DISABLED에 대해서 Interrupt를 활성화함

              - periph_radio_tifs_set(148) [User]
                - T_IFS를 148us으로 설정

              - periph_timer_start(0, 200, true) [User]
                - 만약 어떤 Address도 수신하지 못했을 때, RX를 중지하기 위한 Timer를 설정한다.
                - RADIO와 관련된 Timer는 Timer 0로 고정되어 있기 때문에 Timer 0를 설정한다.
                - `@param`
                  - uint8_t timer = 0 : NRF_TIMER0->EVENTS_COMPARE[timer] = 0에서 COMPARE의 Index를 결정하는 인자값이다.
                  - uint16_t value = 200 : NRF_TIMER0->CC[timer] = value에서 Timeout 시간 설정하는 인자값으로 200us로 설정
                  - bool interrupt = true : TIMER0의 COMPARE Event Interrupt를 허용할지 선택할 수 있으며, 현재는 Compare Event에 대한 Interrupt를 활성화함
                
              - periph_ppi_set(0, &(NRF_TIMER0->TASKS_STOP), &(NRF_RADIO->EVENTS_ADDRESS)) [User]
                - PPI Peripherak 설정으로 EVENTS_ADDRESS가 발생했을 떄, Timer0에 TASKS_STOP Trigger가 전달되도록 설정함
                - `@param`
                  - uint8_t ppi_ch = 0 : PPI Channel
                  - volatile uint32_t* task = NRF_TIMER0->TASKS_STOP : Trigger를 전달할 TASK 주소
                  - volatile uint32_t* event = NRF_RADIO->EVENTS_ADDRESS : Trigger로 사용될 Event의 주소

          - `STATE_SCAN_REQ_RSP` state
            - `sm_enter_scan_req_rsp()` 함수 호출 시, `sm_state`를 STATE_SCAN_REQ_RSP로 변경된다.
            - Signale 처리는 EVENTS_DISABLED일 때만 처리하고, 이는 `sm_enter_scan_req_rsp()` 함수 Scan Request Packet을 수신할 준비를 한 후, Packet을 수신한 경우 처리하기 위함이다.

            - sm_exit_scan_req_rsp() [User]
              - 위 State machine에서 Scan Request Packet을 수신하여, Scan Request 수신 단계를 종료시키기 위하여, EVENTS_DISABLED event에 대해 Interrut Clear를 진행한다.
              - 또한, Address 수신 확인을 위한 Timer 및 PPI 설정도 함꼐 초기화한다.

            - sm_enter_wait_for_idle(is_scan_req_for_me()) [User]
              - RXIDLE state에서 Scan Request를 수신한 경우, Scan Response를 전송을 위한 설정
              - 먼저 인자값으로 `is_scan_req_for_me()` 함수의 Return 값으로, `is_scan_req_for_me()` 함수는 Scan Request Packet을 받았는지 검사하는 함수이다.

              - `is_scan_req_for_me()` [User]
                - 0 == NRF_RADIO->CRCSTATUS : 먼저 Packer이 유효한 Packet인지를 CRCSTATUS Reg로 확인
                - (0x03 != (ble_rx_buf[0] & 0x0F)) || (0x0C != ble_rx_buf[1]) : ble_rx_buf[]은 수신한 Packet으로 ble_rx_buf[0]은 Type Feild이고, ble_rx_buf[1]은 Length Feild로써 Scan Request Packet인지 확인한다. 
                - 이후 Advertising Packet의 Address와 Scan Request Packet의 Address가 동일한지 확인한다
                - 위 3가지 조건이 모두 유효하다면, Scan Request Packet을 수신한 것으로 판단한다

              - `@param`
                - bool req_rx_accepted = is_scan_req_for_me() : Scan Request Packet을 수신한 경우, `is_scan_req_for_me()`의 Return 값은 True가 되고, 이를 통해 Scan Response Packet을 전송할 준비를 한다.
                - `is_scan_req_for_me()`의 Return 값이 False일 경우, 모든 SHORT 설정을 초기화하고, TASKS_DISABLE Trigger를 전달하여 Radio Peripheral 동작을 중지한다.
              - `@action`
                - periph_radio_intenset(RADIO_INTENSET_DISABLED_Msk) : EVENTS_DISABLED 인터럽트 활성화
                - periph_radio_packet_ptr_set(&ble_scan_rsp_data[0]) : Scan Response Packet을 전송할 Buffer 주소값 설정
                - periph_radio_shorts_set()
                  - `RADIO_SHORTS_READY_START_Msk` : EVENTS_READY -> TASKS_START으로 SHORT 설정
                  - `RADIO_SHORTS_END_DISABLE_Msk` : EVENTS_END -> TASKS_DISABLE으로 SHORT 설정
                - periph_radio_tifs_set(150) : T_IFS 150us로 설정

                - scan_req_evt_dispatch() [User]
                  - Scan Request Packet에 대한 수집 정보를 처리하는 함수이다
                  - Scan Request Packet에 대한 정보로 Packet 수신 개수 / Packet 송신 Address / RSSI 값 / 수신 Channel를 수집한다.
                  - the BLE Core specification Version 5.2, VOL 6, Part B, Section 2.3.2를 보면 Scan Request Packet에는 Scanner의 Address와 Advertisier의 Address 값만 들어가고, 나머지 Payload에는 어떤 Custom Data를 넣을 수 없다.
                  - 즉 Scan Request Packet의 Payload에는 Scanner의 Address만 들어가있다.
                  - Scan Request로 부터 수집할 내용은 위 내용 중 Scanner의 Address, RSSI 값만 수집하자
                  - `@action`
                    - scan_req_report.valid_packets = packet_count_valid : Scan Request로 받은 Packet 중에서 유효한 Packet의 개수를 Counting한다.

                    - scan_addr_get() [User]
                      - Scan Request Packet으로부터 Address Tyoe과 Address를 얻어오는 함수이다
                      - `@action` 
                        - btle_address_type_t *addr_type = ((ble_rx_buf[BLE_TYPE_OFFSET] & BLE_ADDR_TYPE_MASK) > 0 ? BTLE_ADDR_TYPE_RANDOM : BTLE_ADDR_TYPE_PUBLIC) : Packet의 Type Field를 읽어서 확인
                        - memcpy((void*) addr, (void*) &ble_rx_buf[BLE_ADDR_OFFSET], BLE_ADDR_LEN) : Packet의 Address Field에서 Address Data를 읽어온다

                    - periph_radio_rssi_read() [User]
                      - RADIO Peripheral의 RSSISAMPLE reg를 읽어온다.
                      - RSSI는 `sm_enter_scan_req_rsp()` 함수에서 SHORT로 설정한 `RADIO_SHORTS_ADDRESS_RSSISTART_Msk` 으로 Address와 Matching이 확인되면, TASKS_RSSISTART가 실행되도록 설정되어 있었다.
                      - `@action`
                        - NRF_RADIO->EVENTS_RSSIEND = 0 : TASKS_RSSISTART에 대한 처리 후, EVENTS_RSSI 클리어
                        - *rssi = NRF_RADIO->RSSISAMPLE : RSSI 값 확인

          - `STATE_WAIT_FOR_IDLE` state
            - `sm_enter_wait_for_idle()` 함수 호출 시, `sm_state`를 STATE_WAIT_FOR_IDLE로 변경된다.
            - Signale 처리는 EVENTS_DISABLED일 때만 처리하고, Scan Requst Packet을 받고 Scan Response까지 보낸 시점에서 오는 State이다.
            - Advertising은 Channel 37,38,39 3개 Channel에서 Advertising을 해야하므로 3개 Channel로 할때까지, 작업을 반복한다.

            - sm_exit_wait_for_idle() [User]
              - 위 Channel 37,38,39까지 Advertising을 했는지 확인하는 함수이다
              - `@action`
                - periph_radio_intenclr(RADIO_INTENCLR_DISABLED_Msk) : 먼저 EVENTS_DISABLED에 대한 Interrupt를 Clear한다
                - PERIPHERAL_EVENT_CLR(NRF_RADIO->EVENTS_DISABLED) : EVENTS_DISABLED도 Clear한다

                - channel_iterate() [User]
                  - Chaneel를 변경하는 함수로 37부터 39까지 순서대로 다음 Channel로 변경한다.
                  - return (channel > 39) : Channel 39까지 Advertising을 헀는지 Bool값으로 반환한다.

            - adv_evt_done = sm_exit_wait_for_idle() : `sm_exit_wait_for_idle()` 함수에서 Channel를 확인하여, Channel 37, 38, 39까지 Advertising을 했는지 유무에 따라 다음 동작을 준비한다.
              - True : Channel 39까지 Advertising을 완료하였다.

                - next_timeslot_schedule() [User]
                  - 모든 Channel로 Advertising을 마무리했기 때문에, 다음 Timeslot을 요청한다. 
                  - Timeslot의 distance는 Timeslot간의 간격을 설정하는 것으로 현재 할당된 Timeslot의 시작 시점부터 다음 Timeslot의 시작시점간의 간격을 설정하는 변수이다
                  - 아래 계산식을 통해 Advertising의 Interval과 Timeslot간의 Interval를 맞추기 위함이다
                  - 단, Advertising의 Interval은 Min과 Max를 설정하고, Min과 Max 사의 시간으로 Interval이 랜덤으로 설정되어 Advertising을 수행한다.
                  - 이를 매커니즘을 맞추기 위해 아래 계산식을 수행한다. 
                  - ADV_INTERVAL_TRANSLATE(adv_int_min) + 1000 * ((rng_pool[pool_index++]) % (ADV_INTERVAL_TRANSLATE(adv_int_max - adv_int_min)))
                    - ADV_INTERVAL_TRANSLATE(adv_int_min) : Advertisng Interval의 최소값 계산으로 설정값의 625us단위로 환산하는 macro함수이다
                    - 1000 : 뒤에서 계산되는 us단위를 ms로 상향하기 위함
                    - rng_pool[pool_index++] : Random Number Generate (RNG) Peripheral를 통해 랜덤 숫자를 생성한 배열을 이용한다
                    - ADV_INTERVAL_TRANSLATE(adv_int_max - adv_int_min) : Advertising Interval로 설정한 Max에서 Min값을 뺀 후, 이를 625us단위로 환산한다
                    - 위 계산식은 기본 Advertising Interval의 Min에서 설정한 Max보다 적거나 같은 Interval를 랜덤으로 설정하게 된다
                  - `NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END` : 현재의 Timeslot를 마무리하고, 새로운 Timeslot을 요청한다.
                  - NRF_TIMER0->TASKS_STOP = 1 : 실행되고 있을 TIMER0에 대해 중지시키며, TIMER0의 시작은 `sm_enter_scan_req_rsp()` 함수에서 실행하였다. 

              - False : Channel 39까지 Advertising을 하지 못했다.
                - sm_enter_adv_send() [User]
                  - 위에서 `sm_exit_wait_for_idle()` 함수를 통해 변경된 Channel로 다시 Advertising을 시작한다

        - `NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0`
          - TIMER0의 실행은 `sm_enter_scan_req_rsp()`함수에서 실행하였고, Timeout의 시간은 200us이고, Timer의 종료 조건은 2가지이다.
          - Timeout이 발생하거나, RADIO의 EVNETS_ADDRESS가 발생할때이다.
          - 그러므로 RADIO로부터 Scan Request Packet을 수신하지 않았다면, `NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0` Event가 발생하게 된다.
          - 그러므로 Scan Request Packet을 수신받지 못했으므로, 바로 다음 Advertising을 진행하기 위해 다음 함수를 호출한다.
          - `sm_exit_scan_req_rsp()` 및 `sm_enter_wait_for_idle()` 함수 호출
          - PERIPHERAL_TASK_TRIGGER(NRF_RADIO->TASKS_DISABLE) : TASKS_DISABLE Trigger로 RADIO 동작은 중지

    #### Timeslot 동작
    - Timeslot은 earliest request와 normal request 2가지로 나뉜다.
    - 2가지 request에 맞춰 struct 변수를 설정한다
      - earliest request
        - 최초 Timeslot을 요청
        - hfclk : High Frequency 설정 변수로 기본 XTAL의 Frequency로 설정한다.
        - length_us : Timeslot의 동작 시간을 us단위로 설정하며, 4300us으로 설정했다.
        - priority : Timslot의 Priority 설정으로 NRF_RADIO_PRIORITY_NORMAL으로 설정
        - timeout_us :
      - normal request
        - 이전 Timeslot에서 다음 Timeslot을 요청
        - hfclk : High Frequency 설정 변수로 기본 XTAL의 Frequency로 설정한다.
        - length_us : Timeslot의 동작 시간을 us단위로 설정하며, 4300us으로 설정했다.
        - priority : Timslot의 Priority 설정으로 NRF_RADIO_PRIORITY_NORMAL으로 설정
        - distance_us : Timeslot간의 간격을 설정하는 것으로 현재 할당된 Timeslot의 시작 시점부터 다음 Timeslot의 시작시점간의 간격을 설정하는 변수이다

