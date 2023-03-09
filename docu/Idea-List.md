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