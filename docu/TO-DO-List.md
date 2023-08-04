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