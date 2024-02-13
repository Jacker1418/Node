# 개발 Check-List

## Part 00. Code 설계
- 코드를 어떻게 설계해할까? ...
- MVC 패턴? 코드 분리? 해야하는 부분과 필요가 없는 부분을 나누자.
  - MVC 패턴은 Model-View-Contorller로 나누어 설계하는 방식인데, 펌웨어 개발에 적용을 어떻게 해야 할까?
  - Model이 Data 및 관리 시스템이라면, Device Information이나 BLE configuration, Routing tables, Data Packet buffer 등이 될 것이다.
  - View는 Model의 Data에 대한 제어 Interface를 제공한다. 
  - Controller는 View의 Action에 따라 Model의 어떤 Data에 접근하고, Read/Write/Modify/Delete를 할지 결정한다. 
  - 음 설계 반영하는 것이 중요하다.
- 그렇다면 코드 분리는?
  - main의 while()과 각 Task별 사용되는 기능인 Advertiser, Scanner, BLE Connection, UARTE, Timer 등 각 기능별로 Event Handler가 존재한다.
  - 그렇다면, 분리는 이렇게 하자.
    - main
    - 전체 Task 관리 및 제어, Task 전환을 위한 Interface 제공 (기능별 Event Handler에서 Interface를 통해 Task 제어)
    - Advertiser
    - Scanner
    - BLE Connection
    - UARTE
    - Timer
    - Log 및 Protocol
    - Peripheral (Crystal, GPIO, Power)

---

## Part 01.  State
-  State Machine 설계
    - [ ] State 변경 시, 수행해야할 사전 작업
    - [ ] State 변경 시, 이전 State가 어떤 것이였는지에 따라 작업 마무리 또는 사전 작업
    - [ ] State별로 동작해야할 Task 
    - [ ] State의 Flowchart

---

## Part 02. Hardware Init
- Peripheral Init
  - [x] Crystal Init
    - LFCLK (Low Frequency Clock) : Synth로 설정
  - [ ] GPIO Init
    - Target Board별로 Pin-map 초기화
    - NRF52_DK or MBN52_DK or MBN52_Node or MBN52_PI 4가지로 Board 구성
  - [x] UARTE Init
    - DMA 방식으로 초기화
  - [x] Timer Init
    - Debugging 용도의 Tick-Counting Timer으로 초기화
    - System에서 사용할 Timer으로 초기화
  - [ ] Device Information Init
    - UICR에서 Device ID read
    - NVMC Init 후 Flash에 저장되어 있는 Device Information load
  - [ ] Log Init
    - Log Format 만들기
    - Log 출력은 RTT로 할지 UARTE로 할지 선택할 수 있도록 설계
  - [ ] Protocol Init
    - Packet을 Capsule 단위로 저장할 수 있는 Buffer 초기화
  - [ ] Packet assemble & disassemble 함수 정의 (유선용)
    - 유선용은 Log 용도나 System Analyzer용도로 사용
    - 추후 MBN52-Node에서 Sensor(STM32)와 통신하는 방식으로 변경할 수 있음
    - 또한, IoT Hub(Raspberry-PI)와 통신하는 방식을 추가해야 함
  -  [ ] Power Init
     - DC/DC Convertor 활성화
     - Sleep Mode 설정은 추후 작업 예정
- BLE Softdevice Init
  - [ ] Advertiser Init
  - [ ] Scanner Init
  - [ ] GAP Init
  - [ ] GATT Init
  
---

## Part 03. BLE 동작
- Scanner   
   - BLE의 RX 채널과 같음
   - active scanning 모드로 동작
   - uuid or device name으로 filtering 설정 [링크](https://jimmywongiot.com/2021/01/05/ble-scan-request-filter-demo/)
   - filtering된 device 중에서 advertiser들의 packet을 packet capsule단위로 buffer에 저장
   + packet assemble 및 disassemble 함수 정의되어야 함 (무선용)
   + Scan Request Event 처리 [관련 링크](https://devzone.nordicsemi.com/f/nordic-q-a/38045/s132-v6-how-to-get-ble_gap_evt_scan_req_report-event)
  
- Advertiser
   - BLE의 TX 채널과 같음
   - scanner으로부터 scan request를 수신할 수 있음
   - uuid or device name으로 filtering하고 scan response 응답
   - advertiser는 2가지 packet으로 나눔 -> advertiser packet + scan response packet

- BLE Parameter 관리
  - SoftDevice 설정
    - Central 1개 / Peripheral 1개를 기준으로 설정
    - 이에 맞춰 RAM 사이즈 설정 
    - Radio Notification 설정 포함

  - GAP
    - GAP는 Connection과 관련된 Parameter를 관리해야 한다.
    - Connection Parameter
      - Connection Interval : MTU 사이즈와 Packet 전송개수에 맞춰서 계산된 값을 입력
        - 계산에 필요한 데이터는 SoftDevice Datasheet를 참고하여 설정
      - Slave Latancy : 위와 동일하게 최대한 빠르게 Discoonection을 감지하기 위해서는 0으로 설정
      - Supervision Timeout : 기본 예제에서는 40s로 설정되어 있으나, 설정값에 따른 영향을 확인해야 함
    - Connection 이후 Data 통신과 관련된 Parameter도 GAP Parameter로써 관리되어야 한다.
      - Data 통신 Parameter
        - MTU & Data Length : Data 통신간의 Packet의 Data Payload 크기이며, Data Throughput를 위해서 MTU 247byte / Data Length 251byte 로 설정할 것이다.
        - PHY : 1Mbps or 2Mbps 중에서 선택하지만, BLE Throughput을 위해 2Mbps로 고정하도록 설정. 단, 선택할 수 있도록 코드 작성
    - Parameter Update Procedure
      - BLE Parameter Update는 상대방 Peer에 따라 달라진다
      - Android 및 iOS는 Parameter Update 허용 가능한 순서 및 설정값이 존재하지만, 우리는 신경쓰지 않는다.
      - Peer는 무조건 nRF52832이므로, Procedure는 우리가 정할 수 있다.
      - Update Procedure는 2가지 시나리오가 존재한다.
        - Connection 직후
          - Connection 직후 바로 Update를 진행한다.
          - Connection Interval -> MTU & Data Length -> PHY
          - Connection Inerval은 최초 CON_REQ Packet을 전송 시, 설정값으로 포함되어 있다. 
          - CON_REQ Packet에 포함된 Parameter : Connection Interval, Supervision Timeout, Slave Latency, Window size, Window offset값이 있다. 
        - Data 통신 Overhead 발생
          - Data 통신 이후 Buffer Full로 인한 Overhead가 발생할 경우, Connection Interval를 조정할 수도 있다.
          - 이 시나리오는 아직 설계단계이며, 추후 개발 예정
      - Nordic SDK에서는 Parameter Update 중 최초 Fail 발생 시, 

  - GATT
    - GATT는 Connection 이후 Data 통신을 위해 Service와 Characteristic 설정하는 영역이다.
    - Service
      - Primary Service 1개만 설정
      - UUID는 128bit 대신 32bit를 사용
      - UUID 32bit는 등록된 UUID이지만, Advertising Packet은 Default 23byte (Payload 27byte - Header 4byte)이기 때문에, 최대한 공간 절약이 되야 한다.
      - Scanner 및 Advertiser는 Filer를 UUID를 기준으로 동작할 것이다.
    - Characteristic
      - 양방향 통신을 위해 2가지 RX / TX Characteristic으로 설정한다.
      - RX Characteristic : Peripheral -> Central 방향의 Data 통신용이며, Notify로 생성
      - TX Characteristic : Central -> Peripheral 방향의 Data 통신용이며, Write without response로 설정
    - Service Discovery Procedure
      - 위 GAP에서 Parameter Update Procedure가 모두 완료된 이후, Service Discovery가 동작되며, Notification Enable도 동시에 실행한다. 

## Part 04. Code
- BLE Initialize 관련해서 가장 Main은 Event Handler를 하나로 통일시키는 것이다.
- BLE Initailze에서 Event Handler 등록되는 개수는 NUS Example 기준 3개이다.
  - 01. Connection Parameter negotiation
  - 02. GATT (data length, MTU, PHY등) 관련 
  - 03. NUS Service Data
- 위 Event Handler 관련하여 하나의 통합 Event Handler로 동작되도록 해야 한다.
- nRF SDK의 BLE Stack과 관련된 Event들은 NRF_SDH_BLE_OBSERVER() 매크로 함수를 통해 등록이 가능하다.
- 그러므로 하나의 Event Handler에는 모든 BLE Event를 처리하는 코드가 들어가야한다.
- init_gap()
  - GAP Parameter 관리 및 Update Procedure 동작
  - GAP Parameter 관리 대상
    - Connection Interval 
    - Supervision Timeout
    - Slave Latency
    - MTU & Data length
    - PHY
  - Update Procedure
    - Connection Interval / Supervision Timeout / Slave Latency : 최초 CON_REQ Packet에 포함되어 Peripheral에게 전달되므로 Update 완료
    - MTU & Data length : BLE_GAP_EVT_CONNECTED Event 발생 후, MTU & Data length Update 요청
    - PHY : MTU 및 Data length Update 완료 후 
  - Connection Parameter negotiation 처리는 Evnet Handler에서 처리
- init_gatt()
  - Service 등록 및 Characteristic 등록
  - Characteristic을 통한 Data 통신은 Event Handler 및 전송 함수에서 처리


## Part 05-1. BLE Stack Layer
- NimBLE Stack Open-Source를 통해 이제는 BLE Stack을 만들 수 있다.
- 하지만 NimBLE는 Full BLE Stack이므로, 여기서 필요한 부분만 BLE Stack으로 구현하고자 한다.
- BLE Stack은 Host와 Controller로 나뉜다.
  - Host
    - Host는 nRF52832의 Physical 동작을 통해 송수신할 Data를 처리하는 Software 영역이다
    - 크게 GAP / GATT / SM(Security) / L2CAP (Logical Link Control and Adaption)으로 나뉜다.
    - 이 부분은 Software 영역이지만, 실제 BLE Application을 개발할 때는 뼈대가 되는 Framework이기 때문에 개발이 필요하다.
    - 여기서 구현할 Component는 GAP / GATT / L2CAP이다
    - GAP
      - Logical Role를 구현하는 영역으로 Central이냐, Peripheral이냐를 결정하는 영역으로 어떻게 동작할 것인지 큰 그림의 Function Grouping하는 영역이다
    - GATT
      - Logical Data Communication을 구현하는 영역으로, Connection 이후 Data를 어떻게 송수신할지 구현하는 영역으로 Read, Write, WriteWithoutResponse, Notify등을 구현하고, ATT Protocol을 구현
    - L2CAP
      - 사실상 가장 중요한 부분으로 Controller과 상호 교류로 관리해야할 Data를 관리하는 부분이다.
      - Low Data에 대해 Packet 관점에서 해석하고, Segmentation (재조립) / Retransmission / Flow Control / Encapsulation / Scheduling / Fragmentation 등이 주요 동작이다.
  - Controller
    - 실제 nRF52832에서 Data를 송수신하기 위한 Physical 동작을 담당하는 영역이다
    - 크게 HCI(Host-Controller Interface)와 LL(Link Layer), 그리고 PHY(Physical Layer)로 나뉜다.
    - HCI(Host-Controller Interface)
      - 사실상 Physical 동작을 제어하기 위한 Interface이다.
      - 이는 상위 Host와 Physical 동작을 분리시키위한 Interface이다.
      - 그러나 실제 구현 관점에서는 nRF52832상에서만 동작할 BLE Stack을 구현하는 것이기 떄문에, Physical 동작을 제어하기 위한 직접적입 Interface로 구현할 것이다.
    - LL (Link Layer)
      - 가장 애매한 부분이다
      - Physiacl 동작을 관리하는 State-Machine이다.
      - 관리할 State는 Standy / Advertising / Scanning / Initiating / Connected이 있다.
      - 즉 각 State별로 Physical Layer로부터 Radio 동작을 호출할 것이다.
  
- 그러므로 나만의 BLE Stack은 다음과 같은 구조를 갖는다.
  - Host
    - GAP
      - Central 
      - Broadcast 
      - Peripheral 
      - Observer
    - GATT
      - ATT 구현
    - L2CAP
      - Segmentation (재조립)
      - Retransmission
      - Flow Control
      - Encapsulation
      - Scheduling
      - Fragmentation
  - Controller
    - LL
      - Standy 
      - Advertising
      - Scanning
      - Initiating
      - Connected
    - PHY
      - Radio RX
      - Radio TX
- 위 BLE Stack 구현 우선순위는 Controller -> Host 영역으로 진행한다.
- 가장 중요한 구현은 PHY의 RADIO RX & RADIO TX이고, 이를 관리할 LL의 State-Machine이다.
- 이후 Test Application을 통해 동작을 확인하고, L2CAP을 통해 전체 관리를 진행

## Part 05-2. PHY 및 LL Layer 구현
- Reference로 nRF52832의 SoftDevice가 있다는 가정하에 Radio Timeslot을 이용한 Broadcaster와 Observer를 구현한 Code가 있었다.
- 하지만 현재는 이를 Reference로 하기 어렵다.
  - SoftDevice가 있다는 전제하에 Radio Timeslot API를 이용한다는 점
  - BLE Connection은 SoftDevice로 처리한다는 점
- 그러므로 일부분만 Reference로 잡을 것인데 그 부분이 Radio RX & Radio TX 부분이다.1
- 여기서 Broadcaster와 Observer의 동작을 RADIO Peripheral를 통해 직접 구현하였기 때문에, 해당 부분을 이용하여 PHY의 Radio RX & TX를 구현할 수 있다.
- 또한, LL의 Advertising과 Scanning까지 구현되어 있기 때문에, Radio Timeslot의 구현 방식을 걷어내고 Timer 0를 직접 이용한 나만의 State별 Radio Scheduling을 구현하면 된다.

- 구현 01. Advertiser
  - Advertiser에는 기본적인 Radio TX가 주 동작이지만, Scan Request를 받기 위해 Radio RX까지 필요한 영역이다.
  - 그러므로 먼저 Radio TX를 구현하여 Advertising이 되는지 Debugging을 하고
  - 이후 Radio RX를 구현하여 Scan Request를 수신 후, 바로 Scan Response를 받을 수 있도록 구현하는 방향으로 간다.
  - NimBLE 관련 Code 리스트
    - LL(Link Layer) 관련 Code
      - Project/nimble/controller/src 하위 폴더에 Code 존재함
      - ble_ll_adv.c : Advertiser 관련 Code
    - PHY(Physical Layer) 관련 Code
      - Project/nimble/drivers/nrf5x 하위 폴더에 Code 존재함
      - nrf52/phy_ppi.h : Radio Peripheral와 관련된 PPI 설정 Header
      - nrf52/phy.c : Radio Peripheral와 관련된 PPI 설정 Code
      - ble_hw.c
      - ble_phy_trace.c
      - ble_phy.c
- 구현 02. Scanner
  - 위 Advertiser를 구현한다면, Scanner는 빠르게 구현될 것이다. 


## Part 05-3. RADIO TX 및 RX 동작 설정
### 설정할 Peripheral 
1. TIMER0
2. RTC
3. PPI
4. RADIO

### Packet 사전 설정
- 아래 Packet에 대하여 일부를 제외하고, 고정된 데이터로 설정이 가능하다.

- Advertiser Packet 수정사항
  -  Payload  : Data Type에 따라 Flag / Length / Data 설정
  -  Header   : Length 설정
-  Scan Request Packet 수정사항
   -  Payload : Advertiser Address 설정
-  Scan Response Packet 수정사항
   -  Payload : Advertiser Address 설정
   -  Payload : Data Type에 따라 Flag / Length / Data 설정
   -  Header  : Length 수정

1. Advertising Packet 
- Header
   - PDU Type : ADV_IND   (0b     0000)
   - Reserved : x         (0b   0 0000)
   - ChSel    : 1         (0b  10 0000)
   - TxAdd    : 1(Random) (0b 110 0000)
   - RxAdd    : 0         (0b0110 0000)
   - Length   : 6 + 알파   -> 알파 : 추후 Advertising Packet Data 길이를 추가해야 함
- Payload
  - Tx Address : 0xXX XX XX XX XX XX
  - 필수 Generic Attribute Payload 설정
    - Lnegth : 0x02
    - Flag : 0x01 (Generic Attribute)
    - Generic Attribute : 0x05
      - Reserved : 0                                                            (0b000      )
      - Simultaneous LE and BR/EDR to Same Device Capable (Host) : false        (0b0000     ) 
      - Simultaneous LE and BR/EDR to Same Device Capable (Controller) : false  (0b0000 0   ) 
      - BR/EDR Not Supported : true                                             (0b0000 01  )
      - LE General Discoverable Mode : false                                    (0b0000 010 )
      - LE Limited Discoverable Mode : true                                     (0b0000 0101)
  - 이후 Data Type에 따라 설정 ex) Device Name
    - Length : 12
    - Flag : 0x09 (Device Name)
    - Data : Nordic_UART
  
2. Scan Request Packet
- Header
   - PDU Type : SCAN_REQ  (0b     0011)
   - Reserved : x         (0b   0 0011)
   - ChSel    : 0         (0b  00 0011)
   - TxAdd    : 1(Random) (0b 100 0011)
   - RxAdd    : 1(Random) (0b1100 0011)
   - Length   : 12        (고정 = SCAN_REQ는 Scanner Address와 Advertiser Address만 넣을 수 있음)
- Payload
  - Scanner Address : 0x00 00 00 00 00 00
  - Advertiser Address : 0x00 00 00 00 00 00

3. Scan Response Packet
- Header
   - PDU Type : SCAN_RSP  (0b     0100)
   - Reserved : x         (0b   0 0100)
   - ChSel    : 0         (0b  00 0100)
   - TxAdd    : 1(Random) (0b 100 0100)
   - RxAdd    : 0         (0b1100 0100)
   - Length   : 6 + 알파   -> 알파 : 추후 Advertising Packet Data 길이를 추가해야 함
- Payload
  - Advertiser Address : 0xXX XX XX XX XX XX
  - 이후 Data Type에 따라 설정 ex) 128bit-UUID
    - Length : 17
    - Flag : 0x07 (128-bit Service Class UUIDs)
    - Data : 6e400001-b5a3-f393-e0a9-e50e24dcca9e

### RADIO TX 설정 [Advertiser의 최초 TX]
1. NRF_RADIO->POWER = 1             / Radio Peripheral Enable
2. NRF_RADIO->EVNETS_DISABLED = 0   / Clear

3. NRF_TIMER0->PRESCALER = 4        / Timer 0 1us unit 설정
4. NRF_TIMER0->TASKS_STOP = 1       / Timer 0 Stop trigger

5. NRF_RADIO->TXPOWER = 0x00        / Tx power 0dBm
6. NRF_RADIO->MODE = 3              / 1Mbps BLE
7. channel = 37
8. NRF_RADIO->DATAWHITEIV = channel      / Data Whitening 채널값과 동일하게 설정 *** 추후 설정값 찾아보기
9. NRF_RADIO->FREQUENCY = frequency[channel - 37] / Channel 37 2402MHz 설정
10. NRF_RADIO->PREFIX0 = 0x8e
11. NRF_RADIO->BASE0 = 0x89BED600   / TX Address = 0x8e 89 BE D6 00 
12. NRF_RADIO->TXADDRESS = 0x00     / TX Address = PREFIX0 + BASE0
13. NRF_RADIO->RXADDRESSES = 0x01   / Rx Address = ADDR1 Enable / 해당 Address는 Access Address로 Packet의 Header에 있는 4byte address이다.
14. NRF_RADIO->PCNF0 = 0x0002 0106  (0b0010 0000 0001 0000 0110) / S0LEN = 1, S1LEN = 2, LFLEN = 6
15. NRF_RADIO->PCNF1 = 0x0203 0025  (0b0011 0000 0000 0010 0101) / MAXLEN = 37, STATLEN = 0, BALEN = 3, EDIAN = 0, WHITEEN = 1
16. NRF_RADIO->CRCCNF = 0x0103      (0b0000 0001 0000 0011) / LEN = 3, SKIPADDR = 1
17. NRF_RADIO->CRCINIT = 0x555555   / CRC 최초 Init
18. NRF_RADIO->CRCPOLY = 0x00065B   (0b0000 0000 0000 0110 0101 1011) / Polynomial 수식
19. NRF_RADIO->TIFS = 145           / TIFS = 145us
20. NRF_RADIO->PACKETPTR = (uint32_t) &packets[idxAdv] / Packet 연결

21. NVIC_EnabledIRQ(RADIO_IRQn)      / Radio Peripheral 관련 인터럽트 활성화
    
22. channel = 37                    / 추후 Channel 변경을 위해 변수로 Channel값을 저장하여 관리
23. NRF_RADIO->TASKS_TXEN = 1       / Radio TX를 시작하기 위해 TXEN Trigger 전달
24. NRF_RADIO->SHORTS = 0x0B        (0b0000 1011) / READY_START Enable, END_DISABLE Enable, DISABLED_RXEN Enable
25. NRF_RADIO->INTENSET = 0x10      (0b0001 0000) / DISABLED Event를 인터럽트로 설정

### RADIO TX Interrupt 발생 [최초 TX 이후 Scan Request 수신을 위한 RX 준비]
1. NRF_RADIO->EVENTS_DISABLED = 0   / 현재 발생한 RADIO Interrupt는 DISABLED이므로 Clear 진행
2. NRF_RADIO->PACKETPTR = &packets[idxSCAN_REQ] / 수신할 Packet을 위한 Buffer 포인터 연결
3. NRF_RADIO->SHORT = 0x17          (0b0001 0111) / READY_START Enable, END_DISABLE Enable, DISABLED_TXEN Enable, ADDRESS_RSSISTART Enable
4. NRF_RADIO->INTENSET = 0x10       (0b0001 0000) / DISABLED Event를 인터럽트로 설정
5. NRF_RADIO->TIFS = 148            / RADIO RX -> TX로 전환되는 시간 148us로 설정

6. NRF_TIMER0->TASKS_START = 1      / Timer 0 Enable
7. NRF_TIMER0->TASKS_CLEAR = 1      / Timer 0 Clear
8. NRF_TIMER0->EVENTS_COMPARE[0]    / Timer 0 Compare Event Clear
9. NRF_TIMER0->INTENSET = 0x01      / Timer 0 Compare Channel 0 Event를 인터럽트로 설정
10. NVIC_EnableIRQ(TIMER0_IRQn)     / Timer 0 인터럽트 활성화
11. NRF_TIMER0->CC[0] = 200         / Timer 0 CC[0] 200us Timeout 설정
12. NRF_PPI->CH[0].EEP = NRF_RADIO->EVENTS_ADDRESS
13. NRF_PPI->CH[0].TEP = NRF_TIMER0->TASKS_STOP / RADIO `ADDRESS` Event 발생 시, TIMER0 `STOP` Trigger 전달
14. NRF_PPI->CHENSET = 0x01         / PPI Channel 0 활성화

### RADIO RX Interrupt 발생 [Scan Request를 수신한 후 Scan Response를 위한 TX 준비]
1. NRF_TIMER0->TASKS_STOP = 1       / 위 6.번에서 실행한 200us Timer Stop
2. NRF_TIMER0->INTENCLR = 0x01      / Timer 0 Compare channel 0 Interrupt Clear
3. NRF_PPI->CHENCLR = 0x01          / 위 11번, 12번의 NRF_RADIO->EVENTS_ADDRESS -> NRF_TIMER0->TASKS_STOP PPI 설정 해제
4. NRF_RADIO->INTENCLR = 0x10       / RADIO DISABLED Interrupt Disable
5. NRF_RADIO->EVENTS_DISABLED = 0   / DISABLED Event Clear
   
6. NRF_RADIO->PACKETPTR = &packets[idxSCAN_RSP] / Scan Response Packet 포인터 연결
7. NRF_RADIO->SHORTS = 0x03         (0b0000 0011) / READY_START Enable, END_DISABLE Enable
8. NRF_RADIO->TIFS = 150us
9. memcpy(addrScanner, packets[idxSCAN_REQ][3], 6) / 위 2번에서 설정한 Scan Request Packet을 수신한 Buffer에서 Scanner Address를 추출
10. NRF_RADIO->EVENTS_RSSIEND = 0   / 위 3번 SHORT로 RSSI 실행된 시점에서 EVENETS_RSSIEND CLear
11. valueRSSI = NRF_RADIO->RSSISAMPLE / RSSI Sample Data 저장
12. channel = NRF_RADIO->DATAWHITEIV & 0x3F / 현재 Scan Request를 수신한 Chennel 값 저장

### RADIO TX Interrupt 발생 [Scan Response 전송 후, 다음 Channel로 전송 준비]
1. NRF_RADIO->INTENCLR = 0x10       / RADIO DISABLED Interrupt Disable
2. NRF_RADIO->EVENTS_DISABLED = 0   / DISABLED Event Clear
3. channel++                        / 37->38, 38->39, 39->37 channel 변경
4. NRF_RADIO->FREQUENCY = frequency[channel - 37] / channel 38에 맞춰 frequency 배열에서 추출 (channel 38 : 2426MHz)
5. NRF_RADIO->DATAWHITEIV = channel / channel 값으로 Whitening 설정
6. NRF_RADIO->TASKS_TXEN = 1        / Radio TX를 시작하기 위해 TXEN Trigger 전달
7. NRF_RADIO->PACKETPTR = &packets[idxADV] / Packet 연결
8. NRF_RADIO->SHORTS = 0x0B        (0b0000 1011) / READY_START Enable, END_DISABLE Enable, DISABLED_RXEN Enable
9. NRF_RADIO->INTENSET = 0x10      (0b0001 0000) / DISABLED Event를 인터럽트로 설정

### RADIO Timeslot Request [Channel 37, 38, 39 Advertising 완료 후, 새로운 Advertise Event 요청]
- g_timeslot_req_normal.params.normal.distance_us = ADV_INTERVAL_TRANSLATE(adv_int_min) + 1000 * ((rng_pool[pool_index++]) % (ADV_INTERVAL_TRANSLATE(adv_int_max - adv_int_min)))
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
  - 위 advDelay는  0 ~ 10ms 범위내에서 랜덤하게 Delay를 지정한다. 
  - Bluetooth Core Specifiaction v5.2 중 2939 page 4.4.2.2.1 Advertising interval 중 advDelay 설명 참조
- 위 Timeslot의 요청에 새롭게 시작되는 시점부터 Advertising을 chanel 37부터 다시 시작

### TIMER0 EVENTS_COMPARE[0] Interrupt 발생 [Scan Response 수신 대기 중 Time-out 발생]
1. NRF_TIMER0->TASKS_STOP = 1       / 위 6.번에서 실행한 200us Timer Stop
2. NRF_TIMER0->INTENCLR = 0x01      / Timer 0 Compare channel 0 Interrupt Clear
   
3. NRF_PPI->CHENCLR = 0x01          / 위 11번, 12번의 NRF_RADIO->EVENTS_ADDRESS -> NRF_TIMER0->TASKS_STOP PPI 설정 해제
   
4. NRF_RADIO->INTENCLR = 0x10       / RADIO DISABLED Interrupt Disable
5. NRF_RADIO->EVENTS_DISABLED = 0   / DISABLED Event Clear
6. NRF_RADIO->INTENSET = 0xq0       / DISABLED Event Interrupt 활성화 (코드상에서 Race Condition 방지를 위함이라고 되어 있지만, 실제로는 필요하지 않는다.)
7. NRF_RADIO->SHORT = 0x00          / RADIO SHORTS 설정 초기화???
 - Time-out이 발생하면, 다음 Channel으로 Radio TX가 발생해야하는데, 이전에 설정한 DISABLED->TXEN SHORT 설정까지 Clear 시키면 괜찮을까???
8. NRF_RADIO->TASKS_DISABLE         / RADIO DISABLE Trigger 전달

9. Time-out 발생 이후 
  - Time-out 발생 시점은 Scan Request 수신 대기 중에 Timer 200us가 Expried된 시점이다.
  - 반대로 Timer가 Stop되는 시점은 NRF_RADIO->EVENTS_ADDRESS가 발생했을 떄, PPI 설정에 따라 NRF_TIMER0->TASKS_STOP으로 연결되어 Trigger를 전달한다. 
  - Time-out 발생했다면, RADIO RX를 멈추고 RADIO TX로 넘어가야하는데, SHORT를 Clear 시키면 될까?
  - 개발 코드에서는 SHORT Clear는 무시

#### 정리할거
1. packets[]의 Advertising 및 Scan Response Packet에 Data 넣기
2. Scan Request에서 데이터 추출
3. Radnom Peripheral

#### 알아야할 것
1. Advertising Event가 종료되면, Interval + 0 ~ 10ms Random Delay를 진행