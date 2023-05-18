## Role별 코드
### Basic
- Radio Notification 설정 및 Handler
  - Distance : 800us
  - SWI1 Interrupt 활성화
  - SWI1 IRQ Handler 정의

### Scanner
- 사실상 수신자로 구성되어야함
- Radio Notification
  - Active : 없음
  - nActive : Advertiser 동작 준비
- Scanning
    - Scanning 시작 <-> Radio Notification Active : 2.2ms
    - Scanning Active <-> nActive : Scan Window와 거의 동일
    - Distance에 따라 증가함
  - Scan Window : 실험값 적용
  - Scan Interval : 사용안함
- Scan Request 송신 & Filter
- Scan Response 수신 & Report 관리
- RSSI

### Advertiser
- 사실상 송신자로 구성되어야함
- Radio Notification
  - Active : Scanner로부터 수신한 Data가 있다면, Adv Packet Payload에 적용
  - nActive 
    - Connection이 없을 경우 : TIMER1 시작
    - Connection이 있을 경우 : 없음
- Advertising
  - Advertising이 차지하는 시간 : Radio Notification Active 이후 Distance + Random Delay + Advertising channel 37 + T_IFS + Advertising channel 38 + T_IFS + Advertising channel 39
    - Distance : 800us
    - Random Delay : 3000us ~ 10000us (SoftDevice 사양) ? 하지만 실제 측정 시간은 2900us도 나오기 때문에 맞지 않는 것 같다.
    - 각 Channel 별 동작 시간 : 340us
    - T_IFS : 300us
    - 총 계산값 : 5420us ~ 12420us
    - 측정된 시간 : 2900us ~ 3480us
  - Advertising Interval : 사용안함
  - Advertising Duration : 사용안함
- Scan Request 수신 & Filter
- Scan Response 송신 & Report 관리 

### Timer
- TIMER0 : Block
- TIMER1 : Scanner -> Advertiser 이후, 다음 Scanner 동작을 위한 TIMER 동작 
- TIMER2 : Connection PER Count
- TIMER3 : UARTE Rx Timeout
- TIMER4 : UARTE Rx Count

### Connect
- Connection 
  - Connection은 Routing Table의 유무의 따라 동작 차이가 있다.
  - 

### Packet Assemble & Disassemble

### Link Report
- Scanner Report
  - Node Address & RSSI & Service Quality 요구사항
- Connect PER
  - Connection 동작 중, TIMER2를 통한 PER Counting 수행

### UARTE