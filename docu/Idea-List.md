# Idea List

## 01. switch-case문을 함수포인터 배열로 대체
- BLE의 GAP 및 GATT 관련 Events 처리는 Hanlder 함수 내에서 각 Event별 switch-case문으로 처리해왔다.
- 하지만, GAP와 GATT의 Event별로 상수값을 인덱스로 이용한 함수포인터 배열을 만든다면?
- 장점 : switch-case문에서 모든 event를 case문으로 나열할 필요없이 event의 상수값을 배열의 Index로 사용하여 해당 event를 처리할 함수를 함수 포인터를 사용하여 호출한다.
- 단점 : BLE Event별 enum 상수값을 확인해보니... 0x00 ~ 0x8F 무려 144까지 상수로 사용 중이였다. 상수값을 변경하는 실험을 해볼까 했지만, 일단은 145 크기의 함수 포인터 배열을 만들어야한다. **즉 Event별 enum 상수값을 그대로 배열의 인덱스로 사용하기 위해서 불필요한 공간 낭비가 있다는 점이 단점**

## 02. State-Context용 struct 설계
- state machine 패턴은 각 state별 동작할 task가 존재하고, Event가 발생했을 때 변경해야할 state별로 사전 작업이나 종료해야할 작업을 진행한 후, state전용 task로 context-switch가 진행되어야 한다. 
- 사전 작업 및 종료 작업 그리고 state전용 task에 대한 context-switch를 관리할 수 있는 struct가 필요하다.
- 예를 들어, Enter-Function / Exit-Function / Task-Swicth 3가지로 나누고, 각 State별로 해당 부분별 function을 정의된 것을 함수 포인터로 연결한다면 어떻게 될까?
``` C
enum STATE
{
    INIT,
    ADVERTISER,
    SCANNER,
    NONE
};

enum EVENT
{
    evtAdvertiser,
    evtScanner,
    None
}

struct State-Context 
{
    STATE stCurrent = INIT;

    ret_code_t (*ptrEnter)(void);
    ret_code_t (*pttExit)(void);
    ret_code_t (*ptrTask)(void);
}

ret_code_t enterInit(void){ ... }
ret_code_t exitInit(void){ ... }
ret_code_t taskInit(void){ ... }

ret_code_t enterAdvertiser(void){ ... }
ret_code_t exitAdvertiser(void){ ... }
ret_code_t taskAdvertiser(void){ ... }

int main(void)
{
    State-Context context;

    context.ptrEnter = enterInit;
    context.ptrExit = exitInit;
    context.ptrTask = taskInit;

    while(1)
    {
        context.ptrTask();
    }
}

void event_handler(EVENT in_action)
{
    if(in_action == evtAdvertiser) 
    {
        context.ptrEnter = enterAdvertiser;
        context.ptrExit = exitAdvertiser;
        context.ptrTask = taskAdvertiser;
    }

    context.ptrExit();
    context.ptrEnter();
}
```
- `struct State-Context` 구조체와 같이 각 state별로 사전 작업, 종료 작업, Main Task에 대한 함수 포인터를 정의하였다.
- main의 while(1)문에서는 사전에 초기화한 함수 포인터 `context.ptrTask`를 그대로 실행시킨다.
- `event_handler()` 함수에서는 event에 맞춰 변경할 state를 확인하고, 해당 state의 Enter-Function / Exit-Function / Task-Function 각 함수를 함수 포인터 변수에 정의한다. 

## 03. GPIO에 대한 라이브러리화가 의미 있을까?
- 미치겄다. 중요한게 아닌지만 GPIO를 Library로 만들려고 하는데, 애매하다.
- Library는 디바이스 드라이버의 어뎁터 방식으로 개발하고자 했다. 그래서 Open, Close, Read, Write, ioctl으로 따로 구현하고자 했다.
- 그런데, 보통 펌웨어서는 GPIO를 Init하는 함수에서 System에서 사용하는 모든 GPIO Pin에 대하여 한번에 정의한다.
- 정의하는 것 중에는 순수하게 Output, Input 모드로 설정하는 것도 있지만 개발자의 Event Handler를 등록하는 것도 존재한다.
- 여기서 문제가 된다. GPIO 라이브러리의 Open 함수를 기존의 Init 함수처럼 코딩한다면, 이는 개발하는 Firmware에 종속성이 발생한다.
- 종속성이 발생한다는 것은 다른 Firmware를 개발할 때 GPIO Library를 가져온다면 Open 함수를 결국 System에 맞춰 전면 수정해야 한다.
- 아이러니? 하게도 GPIO는 사실 System에 종속될 수 밖에 없다. 왜냐하면 System마다 회로가 다르기 때문에 회로에 맞춰 설정하는 것이 GPIO이다.
- 게다가 라이브러리화하는 것도 결국 nRF SDK의 GPIO 라이브러리가 정의가 더 잘 되어 있다는 것이다. 
- nRF SDK의 GPIO 라이브러리도 사용자가 정의한 Event Handler를 호출하기 전에 전처리를 하는 부분이 불필요한 동작이라고 생각이 들어서 수정하고 싶은데.
- 괜한 고민하는 것이 아닌가 싶기도...
- 그래두 범용적으로 맛갈나게 사용할 수 있는 라이브러리를 만들고 싶어서 고민을 해봤다.
- 결국 생각나는 것은 사용자가 정의한 Event Handler를 매개변수로 넘겨받는 Open 함수인데, 문제는 결국 Event Handler도 어떤 GPIO Pin을 사용하여, Event를 어떻게 처리하고, System의 Flow에 영향을 줄 것인지 여러 요소가 System에 종속적이다. 
- 결론은 GPIO는 System에 맞춰 Init하는 함수로 코딩한다. 단, nRF SDK의 GPIO 라이브러리를 사용하지 않고, Event Handler는 __WEAK으로 선정의한 후, Task 코딩 영역에서 정의할 수 있도록 변경한다. 

## 04. UARTE 개발 
- UARTE는 UART와 DMA가 합쳐진 Peripheral로써, TX와 RX Data 처리를 DMA를 이용한다.
- nRF SDK 16.0.0 이상부터 공식으로 libuarte Library를 지원하고 있으며, 이를 래퍼런스하여 Node의 동작 특성에 맞춰 UARTE Library를 개발한다.

### 개발 요소
- UARTE Library는 UART Configuration, Time & PPI, Queue 크게 3가지 영역으로 개발되어야 한다. 
- UART Configuration
  - Buadrate : 921600
  - Parity Bit : None
  - Data Bit : 8 bit
  - Stop Bit : 1 bit
  - Flow-control : None (나중에 사용할 수도 있는 여지가 있음)
- Timer & PPI
  - Timer 2와 3만 사용
  - Timer 2 [ RX Data 수신 확인용 Timer ]
    - UART RXD로 설정한 특정 시간안에 Data가 수신되지 않아 RX Buffer에 Data를 확인하라는 알람 기능을 제공한다.
    - 먼저 UARTE의 RXD 설정 중 RXD.MAXCNT Reg를 설정하는 이는 DMA 기능으로 RXD로 Data를 받을 최대 개수를 설정하는 것으로 4 ~ 255까지 설정할 수 있다.
    - RXD.MAXCNT에 설정한 개수만큼 Data를 수신한 경우, UARTE에서 ENDRX event가 발생하고, User는 ENDRX evnet 시점에서 RXD.PTR에 설정한 Buffer로부터 Data를 처리할 수 있다.
    - 즉, RXD.MAXCNT 값만큼 Data가 수신되지 않는다면, UARTE에서 ENDRX event는 발생되지 않으며, 소량의 Data가 Buffer에 쌓여있지만 확인할 수 있는 Trigger가 존재하지 않는다.
    - 이를 해결하기 위하여 Timer와 PPI Peripheral를 이용한다.
    - Timer는 Timer Mode로 설정 후, TIMER_START_TASK trigger와 UARTE의 RXDRDY event를 PPI로 연결한다.
    - 이럴 경우, RXD로부터 Data를 받을 때마다, UARTE의 RXDRDY event가 발생할 것이고 event는 Timer의 START Trigger로 전달되어 가장 최근 Data를 받은 시점부터 Timer가 동작한다.
    - 이후 Timer가 Timeout이 되기 전까지 Data를 수신하면 Timer는 초기화 되지만, Timeout이 발생하면 Timer의 COMPARE event를 통해 Buffer를 확인할 수 있다.
    - Timer를 이용하지 않는 다른 방법은 UARTE의 RXDRDY event를 직접 이용하는 것이다.
    - UARTE의 RXDRDY event가 발생할 때마다 RXD로부터 수신한 Data를 직접 확인할 수 있기 때문에, 바로 사용이 가능하다.
    - 그렇다면 `Timer & PPI 방식` vs `UARTE's RXDRDY event 방식` 둘 중 괜찮은 녀석은 난 `Timer & PPI 방식`이라고 생각한다.
    - `Timer & PPI 방식`은 UARTE 이외에 Timer와 PPI Peripheral까지 사용해야하기 때문에, 소비전류를 더 사용하겠지만, UARTE의 본연의 DMA 기능을 최대한 활용할 수 있는 방식이다.
    - DMA의 사용 목적은 처리에 대한 무관섭이다! Core에서 Task를 처리하는 동안 UARTE의 DMA를 통해 특정 Buffer에 Data를 송수신하는 것이다. 그렇다면 `Timer & PPI 방식`도 Core의 처리가 없이도 스스로 동작이 가능한 방식이기 때문에, 특정 Event에서만 Core가 처리함으로 UARTE를 사용하는 목적에 부합된다.
    - 하지만 `UARTE's RXDRDY event` 방식 UARTE의 RXDRDY event를 Interrupt로 활성화한 후, Event Handler에서 매번 처리하는 방식이기 때문에 Core의 처리가 필요하다.
    - 당연히 일반적인 상황에서는 배보다 배꼽이 커보이지만 만약에 말야 Core가 무진장 바쁘다면 어떻게 될까? UARTE의 RXDRDY event가 우선순위에 밀려서 처리를 못하는 상황이 된다면?
    - 그 상황이 바로 BLE Event들일 것이다. 우리의 BLE Event는 Timing 싸움이기 때문에 우선순위가 가장 높은 Task일 것이고 Core 처리 비중도 앞도적으로 높을 것이다
  - Timer 3 [ RX Data 수신 Counting용 Counter ]
    - RXD로부터 수신 개수를 Counting하기 위한 Counter로 사용
    - UARTE의 RXDRDY event를 Timer의 TIMER_TASK_COUNT에 PPI로 연결k하여 RXD로부터 Data를 수신할 때마다 Counting될 수 있도록 설정
    - 위 기능은 UARTE의 RXD.AMOUNT와 동일한 기능이지만, RXD.AMOUT reg는 UARTE의 ENDRX or RXTO event가 발생한 다음부터 읽을 수 있기 때문에 제약이 있다.
    - 그러므로 아무때나 읽을 수 있는 Counter를 이용하여, RXD로부터 수신한 데이터 개수를 알아낼 수 있다.
  - Queue
    - UARTE의 최대 수신 가능 개수는 255개
    - 최대 개수보다 많은 Data를 받을 때는 연속적으로 받기 위해서 빠르게 Buffer를 교체해야 한다.
    - Buffer를 교체하는 시기는 
    - 이를 위해서 Buffer Pool를 만들고, Buffer Pool을 관리하는 Queue를 개발
    - Buffer의 변경 시점은 UARTE의 RXSTARTED event가 발생했을 때

### nRF SDK's libuarte library
- libuarte의 전체 구성은 Queue, Timer, PPI, UART DMA 설정으로 나뉜다.
  - Queue 구성
    - `main.c` 에서 `NRF_LIBUARTE_ASYNC_DEFINE()` macro 함수를 통해 Instance를 생성하고, 여기서 Queue를 생성된다.
    - Queue 생성 또한 `NRF_QUEUE_DEF()` macro 함수를 통해서 Instance로 생성되는데, 매개변수로 `_type`, `_name`, `_size`, `_mode` 4가지를 넘겨받는다.
    - `_type` : Queue Buffer를 생성할 때, Data 자료형을 지정
    - `_name` : Queue Instance에 대한 이름을 지정하는 것으로 `CONCAT_2()` macro 함수를 이용하여 `_name` + `_nrf_queue_buffer[(_size) + 1]`를 이용된다.
    - 또한, 
    - `_size` : 위 `_name`에서 `CONCAT_2()` macro 함수로 buffer 이름 지정과 함께 크기를 지정한다.
    - 