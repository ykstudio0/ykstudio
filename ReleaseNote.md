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
v0.2.5 : Display Manager 기반 마련
v0.3.0 : LCD 대시보드
v0.4.0 : Wi-Fi 모니터링
v1.0.0 : 차량 실사용 버전

