# SVEMS Development History
v0.1.0 : 개발환경 구축
    ESP32-S3 DevKitC 개발환경 구축
    VS Code + PlatformIO 설정 완료
    프로젝트 생성
    platformio.ini 기본 설정
    USB CDC 문제 해결 (ARDUINO_USB_CDC_ON_BOOT=1)
    빌드 성공
    업로드 성공
    Serial Monitor 정상 출력
    ESP32와 첫 번째 통신 성공
v0.1.1 : SystemInfo
    SystemInfo 모듈 생성
    Hardware Diagnostic 구현
    Flash/PSRAM 검증
    Heap/SDK 정보 출력
    프로젝트 모듈화 시작
v0.1.2
    ESP32-S3 DevKitC(N16R8) 지원
    WS2812 RGB LED(GPIO38) 제어 성공
    StatusLED 클래스 구현
    Begin(), Task(), SetState() 인터페이스 확립
    PlatformIO에서 Build / Upload / Serial Monitor 정상 동작 확인
    상태별 LED 색상 순환 테스트 완료
v0.1.3: RS485 통신 계층 구축
    Logger 모듈 생성
    RS485 Driver 기본 구조 생성
    Common 모듈 생성
        Pins.h
        Version.h
        Types.h
    GPIO 하드코딩 제거
    Core 폴더 생성
        System.h
        System.cpp
    프로젝트 구조 리팩터링
v0.1.4: EPEVER Modbus 프레임 송신,CRC16 구현
    CRC16 클래스 작성
    Modbus CRC16 알고리즘 구현
    CRC 검증 완료 (F7CA)
    프로젝트 개발 원칙 수립
        검증 우선
        계층 구조 유지
        재사용 가능한 모듈 설계
v0.1.5: Modbus RTU 송신 성공
v0.1.6: MPPT와 최초의 양방향 통신 성공
    Config.h 생성
    Pins.h 리팩터링
    GPIO 매직 넘버 제거
    MODBUS_BAUDRATE 적용
    EpeverRegisters.h 작성
    lib/Epever 폴더 생성
    Epever.h
    Epever.cpp
    Begin() 구현
    Getter 함수 뼈대 작성
    빌드 성공
    main.cpp에서 Epever::Begin() 호출 및 Driver Ready 로그 확인
v0.1.7: Battery / PV Driver 완성
    EpeverRealtimeData 구조체 생성
    EpeverStatus 구조체 생성
    ReadBattery() 완성
    Epever::Data 캐시에 실제 배터리 전압 저장
    ModbusRTU 계층 정리
    RS485 / MODBUS 로그 역할 분리
    main.cpp에서 캐시 데이터를 이용한 출력 확인
    실제 MPPT에서 12.38V 읽기 성공
v0.1.8 : 안정적인 Modbus 통신 계층
    ReceiveFrame()
    CRC Verify
    Modbus Exception 처리
    Expected Length 자동 계산
    응답의 Slave ID/Function Code 검증 정리
v0.1.9 : EPEVER Driver 완성(PV, Battery, Load)
    EpeverData 하나로 통합 완료
    online / lastUpdate 추가 완료
    ReadRegisters() 선언 이름 일치
    ReadLoad / ReadTemperature / ReadSOC 선언 추가 완료
    static EpeverData Data 선언 완료
    Epever Driver 구조 정리
    Update() 구조 확립
    DeviceManager 설계 시작
    Epever::Begin() 추가
    Data Cache 구조 완성
v0.2.0 : 차량 에너지 관리 코어
    RegisterMap 기반으로 코드 리팩터링
    Solar 구조 변경
    Battery 구조 변경
    DeviceManager 완료
    Scheduler 클래스 생성
    DeviceManager가 Scheduler를 호출하도록 변경
    Scheduler가 1초 주기로 Epever::Update() 실행
v0.2.1
    RS485 Driver 안정화
    Modbus RTU 통신 안정화
    CRC 검증 완료
    Epever Driver 분리
    RegisterMap 적용
    DeviceManager 구축
    Poll Scheduler 구축
    실장 테스트 성공 (1초 Solar / 5초 Battery Poll 확인)
v0.2.2 : Poll Task 분리
v0.2.3 : Poll 결과 처리 및 Logger 분리
v0.2.4 : Device Health(통신 상태, 오류 횟수)
    updated 플래그를 별도 구조체로 분리
    Logger를 Service Layer로 이동
    Poll과 Service의 역할을 분리하기 시작
    ClearUpdates()를 통해 이벤트 소비 모델의 기반 마련
    NO Response 해결
    Retry 제거
    CRC 안정
    통신 속도 안정
    SVEMS v0.2.4
    RS485 Driver Stable
    Modbus RTU Stable
    PV/Battery Read Stable
    Timeout Fixed
    Receive Engine Fixed
v0.2.5 : SVEMS의 실시간 데이터 계층(Data Acquisition Layer) 완성
    DataManager 리팩터링 1단계 진행 중
    SolarData, BatteryData, LoadData, TemperatureData 구조 분리 완료
    DataManager.cpp에 정적 인스턴스 4개 정의 완료
    ClearUpdates() 구현 완료
    빌드 성공
    ReadSolar() 데이터를 DataManager::Solar 기반으로 변경 후 정상 동작 확인
    ReadBattery() 데이터도 DataManager로 이동 후 로그 정상 출력 확인
    현재 로그에서 PV와 Battery 값이 정상적으로 읽히는 상태
    LoadData 구현
    ReadLoad() 구현 및 검증
    TemperatureData 구현
    ReadTemperature() 구현 및 검증
    Common/Units.h로 변환 함수 통합
    실제 EPEVER Tracer3906BP에서 Load/Temperature/Soc 값 검증 완료
    Scheduler 구조에 자연스럽게 통합 가능함을 확인
v0.3.0 : Display/UI 계층
    DisplayLayout
    DisplayTheme
    DisplayPages
    DisplayTypes
    Main 화면의 구조 설계
    각 DrawXXX() 함수의 역할 정의
v0.3.1 : Display Theme
    ✔ DisplayTheme
    ✔ DisplayThemeTests
    Display Framework
    ───────────────
    DisplayPages        ✔
    DisplayLayout       ✔
    DisplayTypes        ✔
    DisplayTheme        ✔

    Tests
    ───────────────
    DisplayTests        23 PASS
    DisplayThemeTests   25 PASS
    TOTAL               48 PASS
v0.3.2 : Display Model
    ✔ DisplayPages
    ✔ DisplayLayout
    ✔ DisplayTypes
    ✔ DisplayTheme
    ✔ DisplayModel
    DisplayTests        23 PASS
    DisplayThemeTests   25 PASS
    DisplayModelTests   42 PASS
    -----------------------------
    TOTAL               90 PASS
v0.3.3 : Display Connecting
    ILI9341V 초기화 성공
    SPI 통신 정상
    배선 정상
    CS/DC/RST 모두 정상
    백라이트 PWM 정상
    해상도 320×240 정상 (LCD width : 320, LCD height : 240)
    LovyanGFX 설정도 거의 맞음
v0.3.4 : TFTRenderTarget
    TFTRenderTarget 완성
    DisplayRenderer를 단 한 줄도 수정하지 않고 TFT 연결 성공
    DisplayModel → DisplayRenderer → TFTRenderTarget → ILI9341 전체 파이프라인 검증
    Header, Divider, Row Layout 정상 동작 확인
    DrawText() / DrawValue() / FormatValue() 정상 동작 확인
v0.4.0 : Design Rule
v0.4.1 : Charging Status Parser
    Display Framework
    Page Manager
    Overview
    Solar Page
    Dynamic Rendering
    Page Clear
v0.4.2 : Charging Status DataManager
v0.4.3 : Charging Status Display
v0.4.4 : Renderer Framework
    Renderer Framework Completed
    Generic Page Change Detection
    Unified Value/Text Rendering
    Overview Rendering Completed
v0.4.5 : Overview Energy Status
v0.4.6 :Display Refactoring
    HeaderWidget 분리
        DrawEnergy()
        DrawTime()
        DrawStatus()
    Renderer에서 HasTextChanged()로 변경 감지
    변경된 항목만
        FillRect()
        DrawTextBg()
v0.4.7 SVEMS Display Engine v0.4.7 (Overview Refactoring)
v0.5.0 Device Layer
    DS3231 연동
    SHT40 연동
    실제 센서 데이터 표시
    DS3231 Driver Complete
    SHT40 Driver Complete
    SVEMS 최초의 Device Layer 완성
    Device
    ↓
    Service
    ↓
    DataManager
    ↓
    Display
    전체 데이터 파이프라인이 실제 하드웨어에서 검증됨.
v0.5.0 : EPEVER Full Status
v0.5.1 RTC Service
    DS3231 Driver Complete
    RTC Display
    Header Real-time Clock
v0.5.2  Environment Layer
    SHT40 Device Driver 완료
    EnvironmentService 추가
    DataManager(Environment) 추가
    DisplayModelBuilder 연동
    LCD 실시간 Cabin Temperature/Humidity 표시
    SVEMS 최초의 환경(Environment) 데이터 파이프라인 완성
    SHT40
        ↓
    EnvironmentService
        ↓
    DataManager
        ↓
    DisplayModel
        ↓
    Renderer
        ↓
    LCD
v0.5.3 Device Framework
    Device Layer Refactoring
    Generic Device Pattern
    Driver Standardization
v0.5.4
## 2026-08-01
    - DS3231 Driver Complete
    - SHT40 Driver Complete
    - Real-time Clock displayed on LCD
    - Real-time Cabin Temperature displayed on LCD
    - Real-time Cabin Humidity displayed on LCD
v0.5.4.1 Renderer Optimization & Display Stability
## 2026-08-02
    DisplayText Framework 완성
    DisplayText Color 적용
    Stage / Input Status 표시 완료
    모든 페이지에 m_pageChanged 반영
    반복 조건을 ShouldDraw()로 통합
    빌드 성공
    ESP32 업로드 성공
    LCD 페이지 전환 정상
    Solar / Battery / Load / Temperature 값 정상 출력
    변경된 항목만 갱신하는 구조 유지
    Battery Status는 적산계/BMS 연동 전까지 의도적으로 보류
    HeaderData 모델 추가
    Header 시간 문자열을 자체 버퍼로 안전하게 보관
    BuildHeader()에서 Time / Status / Energy 생성
    Renderer는 HeaderData만 소비
    Time 생성 로직을 Builder로 이동
    Status 생성 로직을 Builder로 이동
    Energy 생성 로직을 Builder로 이동
    DrawHeader() 인자 단순화
    BuildTimeText() 제거
    BuildStatus() 제거
    DrawHeaderStatus() 제거
    BuildEnergyDisplay() 제거
    DrawHeaderStatus() 제거
    Header Static 호출을 DrawStatic()으로 일원화
    빌드 성공
    LCD 정상
    TimeService / DataManager
            ↓
    DisplayModelBuilder::BuildHeader()
            ↓
    DisplayModel::HeaderData
            ↓
    Renderer::DrawHeader()
            ↓
    HeaderWidget
v0.5.4.2 UI Framework Enhancement
    Header
        HeaderData 도입
        BuildHeader()
        Legacy 제거
        Renderer 단순화
    Footer
        FooterWidget 정리
        실제 페이지 번호 표시
        하드코딩 제거
        Renderer 단순화
    Architecture
        Driver
            ↓
        Service
            ↓
        DataManager
            ↓
        DisplayModelBuilder
            ↓
        DisplayModel
            ↓
        Renderer
            ↓
        Widget
v0.5.5 WifiServicde
    ✅ WiFi Service
        비차단 연결
        자동 재접속
        Header Status
    ✅ NTP Service
        NTP 동기화
        한국 시간 적용
        비차단 처리
    ✅ RTC 자동 보정
        DS3231 기록
        TimeService 즉시 갱신
        LCD 즉시 반영
        초 단위까지 정확한 동기화
        날짜 Tick 보강
        윤년 처리
        24시간 자동 재동기화
        오프라인 지속 운전
        RTC 재보정 정책 검증
v0.5.6 Touch Device & System Page Enhancement
    ✅ FT6336G 터치 IC 인식 (0x38)
    ✅ TouchDevice Driver 완료
    ✅ TouchManager Framework 완료
    ✅ 좌표 변환(Calibration) 완료
    ✅ Pressed / Released 이벤트 완료
    ✅ Tap 이벤트 완료
    ✅ 초기화 중복 제거
    ✅ 좌표 정규화
    ✅ Rect HitTest
    ✅ Footer HitTest
    ✅ Footer Navigation
    ✅ UiAction
    ✅ WidgetEventManager
    ✅ Footer Previous / Next 페이지 전환
    ✅ Footer Center Action 예약
    ✅ 본문 터치와 Footer 터치 분리
    ✅ RSSI 표시
    ✅ RS485 / Modbus Status
    ✅ Heap
    ✅ Uptime
    ✅ Polling Enable Switch
    ✅ Test/Vehicle Mode 분리
    ✅ v0.5.6 빌드 성공
    ✅ 모든 기존 테스트(Display, Theme, Model, Renderer) PASS 유지  
v0.5.7 implement Quantumcat BMS UART protocol parser
    Y케이블 설계
    핀 분석
    GND 찾기
    UART 후보 찾기
    GPIO 변경
    ESP32 재부팅 해결
    분압회로 설계
    96바이트 프레임 정상 수신
    헤더 동기화 정상
    Big-endian float 해석 정상
    셀 4개 전압 정상
    팩 전압과 셀 합계 일치
    SOC와 잔여 용량 관계 일치
    방전 전류 음수 표기 정상
    세 온도값 정상
    Touch 및 페이지 전환과 충돌 없음
    Wi-Fi/NTP와 병행 동작 정상
v0.5.8 
    ✅ Temperature 데이터 소스별 Status 분리
    ✅ Controller / Controller Board 데이터 분리
    ✅ PowerBank Battery / BMS / External 온도 구조 확보
    ✅ Temperature 기본 페이지 → Cabin / Battery / BMS / Controller
    ✅ Overview의 Cabin Temperature / Humidity 유지
    ✅ ControllerBattery를 EPEVER 진단 데이터로 독립 보존
    ✅ 빌드 / LCD / Touch / Page 전환 정상
v0.5.9 
    통신 안정화 + timeout 수정 + Offline 누적 진단
    12h continuous vehicle test: 0 offline events
v0.6.0 IoT Service
v0.6.1 JSON Schema
v0.6.x
    EPEVER Modbus 통신 완료
    태양광 실시간 데이터
v0.7.x
    DeviceManager 안정화
    Wi-Fi / IoT
v1.0.0 : 차량 실사용 버전(첫 번째 정식 릴리스)

Architecture Status
──────────────────────────────────────

Core            ██████████ 100%
RS485           ██████████ 100%
Modbus          ██████████ 100%
EPEVER Driver   ████████░░  80%
RTC Driver      ██████████ 100%
SHT40 Driver    ██████████ 100%
DeviceManager   █████████░  90%
Scheduler       █████████░  90%
DataManager     █████████░  90%
Display Engine  ██████████ 100%
IoT             ░░░░░░░░░░   0%