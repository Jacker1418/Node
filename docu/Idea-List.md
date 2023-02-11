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