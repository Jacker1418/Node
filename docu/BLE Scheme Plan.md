## Role별 코드
### Basic
- Timeslot 설정 및 요청
- Timeslot 기간 중 Event Handler 처리
- RADIO Peripheral 설정
- TIME0 Peripheral 설정
- SWI0 활용
  - Advertiser Code에서는 Scan Request에 대한 Report를 수신 시, SWI0 Interrupt를 발생
- 추후 Debug용으로 RADIO <-> GPIOTE PPI 설정도 필요할 수 있음

### Advertiser
- Advertising
- Scan Request 수신 & Filter
- Scan Response 송신 & Report 관리 
- format assemble

### Scanner
- Scanning
- Scan Request 송신 & Filter
- Scan Response 수신 & Report 관리
- RSSI
- Format Disassemble

### Connect