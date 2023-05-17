## Advertiser Code 분석
### main
- SoftDevice Init 및 SWI0 설정은 Advertiser Code의 main()과 동일하다
- btle_scan_init() [User]
  - Scanner를 통해 받은 Advertising Packet을 저장하기 위한 설정 및 Timeslot 요청 전 Session 활성화를 위해 `sd_radio_session_open()`를 요청한다.

  - sd_radio_session_open() [Library]
    - `@param`
      - nrf_radio_signal_callback_return_param_t *radio_cb (uint8_t sig) : Timeslot Event Handler 등록

- btle_scan_param_set() [User]
  - Scan Window 및 Interval 설정값에 맞춰 Timeslot Length 및 Distance를 설정한다.
  - earliest.length_us = param.scan_window : Scan Window를 Timeslot의 Length 매칭, 10000us 설정
  - normal.length_us = param.scan_window : Scan Window를 Next Timeslot의 Length와 매칭,10000us 설정 
  - normal.distance_us = param.scan_interval : Scan interval를 Timeslot간의 Interval과 매칭, 20000us 설정

- btle_scan_enable_set() [User]
  - Scanner 동작을 시키기 위하여, `sd_radio_request()` 함수를 통해 Timerslot 요청을 시작한다.

### Scanner의 Callback 함수 (nrf_scan.c)
- nrf_radio_signal_callback_return_param_t *radio_cb (uint8_t sig) [User]
  - `NRF_RADIO_CALLBACK_SIGNAL_TYPE_START` Signal
    - Timeslot 시작 후 Timer 설정 및 Scanning 동작을 시작한다.
    - Timer 설정
      - NRF_TIMER0->TASKS_CLEAR = 1 : TIMER0에 대한 초기화 Task 진행
      - NRF_TIMER0->EVENTS_COMPARE[0] = 0 : EVENTS_COMPARE 초기화
      - NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Msk : TIMER0의 COMPARE0 event Interrupt 설정
      - NRF_TIMER0->CC[0] = m_timeslot_req_normal.params.normal.length_us - 500 : Timeout 시간을 Scan windwo (10000us)보다 500us 짧게 설정함
        - 이유는 Timeslot은 Timeslot 기간이 끝났을 때, 특별한 Event가 없기 때문에, 다은 Timeslot을 요청하기 위해서 Timer의 Timeout을 이용해야 한다.

    - ll_scan_start() [User]
      - RADIO 동작에 맞춰 Debug용 PPI 설정 및 RADIO Peripheral 설정한다.
      - Debug용 PPI 설정은 설명에서 제외

      - radio_init (channel++) [User]
        - 

      - radio_rx_timeout_init () [User]
        - 

      - m_state_receive_adv_entry () [User]
        - 

  - `NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO` Signal
    - 
  - `NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0` Signal
    - 