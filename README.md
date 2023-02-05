# 개발 환경 구성

## VSCode로 개발환경 구성 
### nRF52832 SDK 다운로드 
* 현재 Project는 SDK 15.3.0을 이용
* [SDK 다운로드 링크](https://www.nordicsemi.com/Products/Development-software/nrf5-sdk/download)에서 SDK 15.3.0을 선택하여 다운로드한다.
### Debugger (Cortex-Debug) 구성
  * 개발한 Firmware에 대해 Breakpoint 및 Step별 실행하여, 동작 Test를 할 수 있는 Debugger가 필요하다. 
  * VSCode에서 사용 가능한 Debugger 중 가능 대중적인 것은 **Cortex-Debug** 이다. 
  * Cortex-Debug는 GDB(GNU Debugger)를 이용하는 것이기 때문에, Local에 GDB를 설치해야 한다.
  * Local의 GDB 설치는 [GNU ARM Embedded Toolchain](https://developer.arm.com/downloads/-/gnu-rm)에서 다운받을 수 있으며, 개발하는 nRF SDK에서 components/toolchain/gcc 하위 경로에서 Makefile.window 사용된 GNU Version과 동일한 것을 찾아 설치를 진행한다. 
  ![GNU Version](docu/img/230121_Makefile.window_GNU%20Version%20캡쳐.png) 
  * **GNU Release** : GNU_INSTALL_ROOT의 마지막 부분이 GNU Release Date 정보이므로 해당 Date와 일치하는 GNU를 다운받는다.
  * **GNU Version** : 7.3.1
  * 여기서 가장 중요한 것은 Cortex-Debug와 GDB version 호환을 맞춰야한다.
  * Cortex-Debug의 최신 버전 1.6.10은 GDB version 9 이상부터 지원하고 있기 때문에, nRF SDK에서 사용하는 GNU Version은 7.3.1로 호환되지 않는다.
  * 문제를 해결하는 방법은 간단하다. Cortex-Debug를 Downgrade하는 것이다.
  ![Cortex-Debug Downgrade](docu/img/230121_Cortex-Debug_Install%20Another%20version.png)
  * nRF SDK VSCode의 Extension에서 Cortex-Debug를 찾아 **install Another Version...** 선택하여 버전을 낮추면 된다.
  * 필자는 어떤 버전으로 낮춰야 하는지 알아보기 위해서 Cortex-Debug에 대한 [Release Note](https://github.com/Marus/cortex-debug/blob/master/CHANGELOG.md)를 찾아서 보았지만 나와있지 않았다. (혹시 아시는 분 있으면 연락 부탁드려요)
  * 결국 하나하나씩 설치하여 찾은 버전은 **1.4.4** 버전이다.  
### Hardware 사양
  * nRF52-DK (PCA10040)에서 개발 
  * nRF52832-QFAC version 1 Chip이 내장되어 있음

# 프로젝트 생성
## Reference Project
- nRF52-SDK 중 ble-peripheral/ble-app-uart 프로젝트를 Base로 시작
- VSCode로 프로젝트 동작시키기 위해서 BLE Advertising부터 Connection 이후 Data 통신까지 되는 ble-app-uart를 선택

## VSCode로 개발하기 위한 Settings
- VSCode로 개발하기 위해서는 프로젝트에 4가지 Settings 파일(json 파일)인  **c_cpp_properties.json**, **launch.json**, **settings.json**, **tasks.json** 필요하다.
- **c_cpp_properties.json** : C 및 C++ 언어 환경 설정 파일
  - 주요 개발 설정은 여기서 작업한다.
  - 아래 설정들은 실제 Compiler에 대한 설정들이 아닌 VSCode의 IntelliSense 엔진이 참조하기 위한 설정값들로써 **실제 Compiler의 설정값을 변경하는 것은 Makefile에서 해야된다.**
  - ***nrfSDK*** : Library 경로를 지정할 때 사용할 Root 경로를 설정하는 변수로 nRF SDK 최상위 경로를 지정하면 되지만, 필자는 프로젝트 편의를 위해 상대 경로로 설정하였다. 
  - ***GNU_GCC*** : GCC Compiler 경로를 지정하는 곳으로, 각자 Local에 설치되어 있는 GCC Compiler 위치로 변경해야 한다. 
  - ***includePath*** : Library 경로를 지정할 수 있으며, 주로 nRF SDK에서 최상 폴더 중 components, modules, integration 하위 폴더에 Library Header 및 C Code가 있으므로 해당 파일을 모두 지정해주는 것이 좋다. 
  - ***forcedInclude*** : Builder에서 제외되지 않도록 강제로 포함시킬 Header Files을 지정하는데, 이는 nRF SDK로 개발 시 Library나 Hardware (Peripheral or Clock) 그리고 SoftDevice 동작까지 설정하는 ***sdk_config.h***로 지정한다. 
  - ***defines*** : Compile 옵션에 대한 Define들을 정의한다. 그러나 해당 Define들은 Makefile에서 FLAG값으로 지정하고 있기 때문에, 중복되는 값들이다. 
    - 여기서 Define을 수정해도 반영되는지 의문이다. Makefile까지 수정되어야 하는 것인데 어떻게 해야할지 알아봐야 할 듯 ...
    - 찾아봤을 떄는 VSCode의 IntelliSense 엔진을 위한 전처리기 defines들로써, 실제 컴파일에 영향을 주는 것이 아닌 Intellisense 엔진을 위한 것이다. 
- **launch.json** : Debugger 실행 설정 파일
  - Cortex-Debug의 실행 설정 파일로써, 설치 이후에 수정할 parameter는 3가지 **"executable"**, **"serverpath"**, **"armToolchainPath"** 이다. 각 parameter 값은 각자의 프로젝트의 경로와 Local에 설치된 경로로 수정해야 한다. 
- **settings.json** : 파일별 언어 설정이나 포트 설정
- **tasks.json** : 실행시킬 Task별 동작 정의 파일
  - Task는 **Makefile**에 있는 Build Rule Block을 따른다.
