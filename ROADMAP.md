# 📅 개발 로드맵 (Development Roadmap)

## 목표 (Objectives)
- **완성**: 2026년 6월 ~ 7월
- **출시**: 2026년 7월 ~ 8월 (Steam)

## 📌 상세 개발 일정 (Detailed Schedule)

### 🏁 단계별 목표 (Milestones)

현재 **MVP-1** 단계에서 캐릭터의 기본 움직임뿐만 아니라 **애니메이션 및 기초 전투(콤보)**까지 구현 범위를 확장하여 진행 중입니다.

#### 🟢 MVP-1: 캐릭터 파운데이션 & 기본 액션 (Foundation & Basic Actions)
> **핵심**: "플레이어가 심리스하게 움직이고, 기본적인 공격 콤보를 수행할 수 있어야 한다."
- **기간**: ~ 2026-02-07 (Target)
- **완료된 작업 (Done)**
    - [x] **Character Setup**: `ATSCharacter` 클래스 구조 및 `UCombatComponent` 설계
    - [x] **Movement**: WASD 이동, **Strafe(시선 고정)** 모드, 마우스 시점 반전
    - [x] **Data Driven**: `DataTable` 기반 스탯 관리 (Health, Stamina, Speed)
    - [x] **Input**: Enhanced Input 시스템 구축 (Dodge, Guard, Attack 바인딩)
- **진행 중 (In Progress)**
    - [x] **Animation**: `TSAnimInstance` 구현, 블렌드 스페이스(Idle/Walk/Run), 몽타주 슬롯 설정
    - [x] **Combo System**: 3연타 콤보 로직 (Light/Heavy), 콤보 타이밍 판정(`AnimNotify`)
    - [ ] **Action Montages**: 회피(구르기/스텝), 공격 애니메이션 연동

#### 🟡 MVP-2: 전투 심화 & 상호작용 (Advanced Combat & Interaction)
> **핵심**: "적을 때리고 막는 '타격감'과 '공방'을 구현한다."
- [ ] **Defense Mechanics**:
    - 우클릭 방어(Guard) 상태 로직 및 피해 감소
    - 패링(Parry) 타이밍 판정 및 실행
- [ ] **Hit Reaction**:
    - 피격 몽타주(앞/뒤/좌/우) 재생
    - 히트 스톱(Hit Stop) 및 카메라 쉐이크 연출
- [ ] **Dummy Test**:
    - 샌드백(Dummy) 적 배치 및 데미지 플로터(UI) 표시
    - `TakeDamage` 인터페이스 구현

#### 🟠 MVP-3: AI 기초 & 행동 패턴 (AI Foundation)
> **핵심**: "적이 나를 인지하고 위협적으로 행동해야 한다."
- [ ] **AI Controller**: Behavior Tree & Blackboard 구조 설계
- [ ] **Perception**: 시각/청각 감지 시스템
- [ ] **Combat AI**: 거리 조절, 공격/방어 선택, 추적 오차(Human-like error) 구현

#### 🟠 MVP-4: 게임 루프 & 시스템 (Game Loop)
> **핵심**: "승패가 나뉘고 게임이 순환되어야 한다."
- [ ] **HUD**: 체력바, 스태미나바, 무기 슬롯 UI
- [ ] **Game Mode**: 라운드 시작/종료, 사망 및 리스폰 처리
- [ ] **Level Design**: 테스트 아레나 맵 폴리싱

#### 🔴 MVP-5: 로그라이크 특성 시스템 (Trait System)
> **핵심**: "성장의 재미를 더한다."
- [ ] **Trait Data**: 특성 데이터 테이블 설계 (공격력 증가, 흡혈 등)
- [ ] **Selection UI**: 라운드 종료 시 보상 선택 시스템
- [ ] **Stats Modifier**: 특성 획득에 따른 스탯 실시간 변화 로직

#### 🟣 MVP-6: 출시 준비 (Polish & Release)
> **핵심**: "상품성을 갖춘다."
- [ ] **VFX/SFX**: 전체 이펙트 및 사운드 적용
- [ ] **Optimization**: 프레임 최적화 및 버그 수정
- [ ] **Packaging**: Steam Build 배포 테스트
