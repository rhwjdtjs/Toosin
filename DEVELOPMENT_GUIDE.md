# 📘 개발 가이드 (Development Guide)

이 문서는 **Toosin** 프로젝트의 **1인 개발 워크플로우**와 **GitHub Desktop** 사용 규칙을 정의합니다.
혼자 개발하더라도 히스토리 관리와 안정적인 버전 제어를 위해 체계적인 절차를 따릅니다.

---

## 🖥️ 깃허브 데스크탑 워크플로우 (GitHub Desktop Workflow)

복잡한 명령어 대신, **GitHub Desktop**의 GUI 기능을 적극 활용하여 개발합니다.

### 1. 브랜치 전략 (Branch Strategy)

혼자 개발하므로 너무 복잡한 브랜치는 피하되, 안정성은 유지합니다.

* **`main`**: **"언제든 실행 가능한 완성본"**. 각 MVP 단계가 완전히 끝났을 때만 합칩니다.
* **`dev`**: **"개발 작업대"**. 기능 구현이 끝나면 이리로 모읍니다.
* **`feature/*`**: **"개별 기능 작업"**. `dev`에서 따와서 작업하고, 완료되면 다시 `dev`로 합칩니다.
  * 예: `feature/parry-system`, `feature/ai-learning`

### 2. 작업 순서 (Step-by-Step)

#### 1단계: 작업 시작 전 (Sync)

1. **GitHub Desktop**을 켭니다.
2. **Current Branch**가 `dev`인지 확인합니다. (아니라면 `dev`로 변경)
3. **Fetch origin**을 눌러 원격 저장소의 최신 상태를 확인하고, 변경사항이 있다면 **Pull origin**을 눌러 동기화합니다.

#### 2단계: 브랜치 생성 (New Branch)

1. 상단 메뉴의 **Branch** -> **New Branch** 클릭.
2. 이름 입력: `feature/작업명` (예: `feature/guard-logic`).
3. **Create Branch** 버튼 클릭. (이제 해당 기능 개발을 시작합니다)

#### 3단계: 개발 및 커밋 (Code & Commit)

1. 언리얼 엔진/IDE에서 코드를 작성하고 저장합니다.
2. GitHub Desktop 왼쪽 패널에 변경된 파일 목록이 뜹니다.
3. **Summary (필수)**: 작업 내용을 한글로 명확히 적습니다. (예: `feat: 우클릭 방어 로직 구현`)
4. **Description (선택)**: 상세 내용이 있다면 적습니다.
5. **Commit to feature/...** 파란 버튼 클릭.

#### 4단계: 업로드 및 병합 (Push & PR)

1. 작업이 끝나면 상단의 **Publish branch** (또는 **Push origin**) 버튼 클릭.
2. **Create Pull Request** 버튼이 생기면 클릭 -> 웹사이트로 이동됩니다.

#### 5단계: 이슈 및 PR 작성 (Documentation)

1. **이슈 작성 (Project Planning)**:
    * 작업 시작 전, `Epic Issue Document form.md` 템플릿을 참고하여 이슈를 생성합니다.
    * 작업의 목적, 세부 항목, 완료 조건을 명확히 합니다.

2. **PR 작성 (Code Review)**:
    * 작업 완료 후 PR 생성 시 `Pull Request document form.md` 템플릿을 사용합니다.
    * 제목은 `[MVP-X] 작업명` 형식으로 작성합니다.
    * `Closes #이슈번호` 를 포함하여 관련 이슈를 연결합니다.
    * 구현된 기능의 스크린샷이나 영상을 첨부합니다.

3. (혼자니까) 본인이 **Merge pull request** 버튼을 눌러 합칩니다.
4. Desktop으로 돌아와서 **Current Branch**를 `dev`로 바꾸고 **Pull origin** 하여 최신화합니다.
5. 다 쓴 `feature` 브랜치는 **Branch -> Delete**로 삭제하여 정리합니다.

---

## 📝 커밋 메시지 규칙 (Commit Convention)

나중에 "이때 뭐 했더라?"를 쉽게 알 수 있도록 규칙을 정합니다.

| 태그 | 설명 | 예시 |
| :--- | :--- | :--- |
| **feat** | 새로운 기능 추가 | `feat: 3타 콤보 공격 추가` |
| **fix** | 버그 수정 | `fix: 공격 중 이동되는 버그 수정` |
| **docs** | 문서 수정 | `docs: 로드맵 업데이트` |
| **style** | 코드 포맷/정라 | `style: 들여쓰기 정리` |
| **refactor**| 구조 개선 (기능 변화 X) | `refactor: 스탯 관리 로직 최적화` |
| **chord** | 기타 설정 | `chore: 언리얼 프로젝트 설정 변경` |

---

## 📁 파일 및 네이밍 규칙

언리얼 엔진 C++ 코딩 표준을 따릅니다.

* **클래스 파일**: `TS` (Toosin) 접두어 사용.
  * `ATSCharacter` (Actor)
  * `UTSGameInstance` (UObject)
* **변수**: 파스칼 케이스 (PascalCase)
  * `CurrentHealth`, `MaxStamina`
  * `bIsAttacking` (Boolean은 b 접두어)
* **함수**: 동사로 시작
  * `CalculateDamage()`, `PlayAttackMontage()`

---

### ⚠️ 주의사항 (Caution)

* **절대로 `main` 브랜치에 직접 커밋(Direct Commit)하지 마세요.** 실수가 생기면 되돌리기 어렵습니다.
* **퇴근 전**: 항상 작업한 내용을 Push 해두세요. (백업 효과)
