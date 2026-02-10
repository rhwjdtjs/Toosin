# 📅 개발 로드맵 (Development Roadmap) - Updated

## 📌 목표 (Objectives)

- **완성 (Completion)**: 2026년 6월 말
- **출시 (Release)**: 2026년 7월 ~ 8월 (Steam)

> **일정 산정 기준**: 1인 개발 및 학습 병행 속도를 고려하여 여유 있게 산정했습니다.

---

## 🚀 상세 개발 일정 (Detailed Schedule)

### ✅ MVP-1: 캐릭터 파운데이션 (Character Foundation)

- **기간**: 2026.02.02 ~ 2026.02.08 (완료됨)
- **완료 항목**:
  - [x] 캐릭터 셋업 및 이동 (WASD, Strafe)
  - [x] 데이터 기반 스탯 (Data Table)
  - [x] 입력 시스템 (Enhanced Input)
  - [x] 기본 3타 콤보 및 애니메이션

---

### ✅ MVP-2: 전투 - 방어와 리액션 (Combat Mechanics)

- **기간**: 2026.02.09 ~ 2026.02.28 (3주)
- **완료 항목**:
  - [x] **우클릭/Space 방어 (Guard)**: 스태미나 소모, 데미지 경감.
  - [x] **패링 (Just Guard)**: 성공 시 이펙트(Time Dilation) 및 데미지 무효화.
  - [x] **샌드백 (Dummy)**: 테스트용 AI 및 데미지 플로팅 UI.
- **⚠️ 이관된 항목 (Deferred)**:
  - **UI(HUD)**: 체력/스태미나 바 -> **MVP-4**로 이관.

---

### 🟠 MVP-3: 학습하는 AI & 기본 전투 완성 (AI & Combat Completion)
>
> **핵심**: "플레이어를 학습하고 상대하는 '똑똑한 적'을 구현한다."

- **기간**: 2026.03.01 ~ 2026.03.31 (4주)

#### 🧠 AI 기초 (AI Foundation) - `Due: 03.15`

- [ ] **Behavior Tree**: 순찰(Patrol), 추적(Chase), 공격(Attack) 상태 머신 구현.
- [ ] **Perception**: 시각/청각 감지 시스템.

#### 🤖 적응형 AI (Adaptive AI) - `Due: 03.31`

- [ ] **패턴 학습**: 플레이어의 공격 빈도 분석 (1타 위주 vs 콤보 위주).
- [ ] **대응 로직**: 학습된 데이터에 따라 가드/공격 확률 조정.

---

### 🟠 MVP-4: 게임 루프 & 시스템 (Game Loop)

- **기간**: 2026.04.01 ~ 2026.04.20 (3주)

#### 📊 UI 시스템 (HUD) - `Due: 04.10`

- [ ] **Status Bar**: 무기 내구도, 체력/스태미나 바 구현.
- [ ] **Round UI**: 현재 라운드 및 적 정보 표시.

#### 🔄 게임 모드 (Game Mode) - `Due: 04.20`

- [ ] **Win/Loss**: 사망 시 결과창, 승리 시 라운드 이동.
- [ ] **Arena Map**: 테스트 맵을 실제 아레나 레벨로 교체.

---

### 🔴 MVP-5: 로그라이크 특성 시스템 (Roguelike Traits)

- **기간**: 2026.04.21 ~ 2026.05.10 (3주)

#### 🃏 특성 시스템 - `Due: 05.10`

- [ ] **Trait Data**: 공격력, 스피드, 특수 효과 특성 데이터 테이블.
- [ ] **Card Select**: 라운드 종료 후 보상 선택 UI.

---

### 🟣 MVP-6: 성장 심화 & 디테일 폴리싱 (Advanced Progression & Detail Polish)
>
> **핵심**: "모든 상호작용의 퀄리티를 높이고, 숨겨진 기능들을 해금한다."

- **기간**: 2026.05.11 ~ 2026.05.31 (3주)

#### ⚡ 비주얼 & 액션 완성 (Visual & Action Polish) - `Due: 05.20`

- [ ] **Shift Special Ability (특수기)**: (기존 회피 대체) Shift 키 사용 시 발동되는 캐릭터 고유 특수 기술 구현.
- [ ] **Stamina Recovery (스태미나 회복)**: 패링/공격 성공 시 스태미나 회복 로직(C++) 구현.
- [ ] **Weapon Retaliation (무기 반작용)**: 가드/패링 시 무기 튕김 IK 및 스파크.
- [ ] **Dynamic Aiming (에임 오프셋)**: 가드/대기 시 시선 처리.
- [ ] **Impact Feedback**: 타격감(역경직, 카메라 쉐이크, 혈흔) 강화.
- [ ] **Sound Design**: 상황별 사운드(Footstep, Clash, Whoosh) 적용.

#### ⚡ 성장 시너지 (Growth Synergy) - `Due: 05.25`

- [ ] **Attack Speed Scaling**: 민첩 특성 투자 시 공격 속도 증가.
- [ ] **Combo Extension**: 레벨 달성 시 4타, 5타 히든 콤보 해금.
- [ ] **Elemental Effects**: 속성 부여 시 이펙트 변경.

#### 📈 경험치 & 레벨업 (Leveling) - `Due: 05.31`

- [ ] **XP System**: 경험치 획득 플로팅 텍스트.
- [ ] **Stat Distribution**: 스탯 분배 UI.

---

### 🏁 MVP-7: 출시 준비 (Polish & Release)

- **기간**: 2026.06.01 ~ 2026.06.30 (4주)

#### ✨ 최종 점검 - `Due: 06.15`

- [ ] **BGM**: 배경음악.
- [ ] **UX**: 메뉴, 튜토리얼.

#### 📦 배포 - `Due: 06.30`

- [ ] **Optimization**: 최적화.
- [ ] **Steam Build**: 스팀 런칭.
