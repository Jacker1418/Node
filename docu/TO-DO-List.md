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
        - PHY : 

  - GATT