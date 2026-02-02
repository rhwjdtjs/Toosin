# 📘 개발 가이드 (Development Guide)

이 문서는 `Toosin` 프로젝트의 1인 개발 워크플로우와 규칙을 정의합니다.

## 🔄 깃허브 워크플로우 (GitHub Workflow)

본 프로젝트는 혼자 개발하지만, 체계적인 관리를 위해 **Feature Branch Workflow**를 따릅니다.

### 1. 브랜치 전략 (Branch Strategy)
- **`main`**:
    - 언제나 실행 가능하고 배포 가능한 상태를 유지합니다.
    - 기능 개발이 완료되고 테스트가 끝난 코드만 병합(Merge)합니다.
- **`feature/*`**:
    - 새로운 기능 개발 시 생성하는 브랜치입니다.
    - 예: `feature/combat_system`, `feature/ai_basic`, `feature/inventory`
- **`fix/*`**:
    - 버그 수정 시 생성합니다.
    - 예: `fix/attack_collision`

### 2. 작업 순서 (Process)
1. **이슈 생성 (Issue)**: 작업할 내용을 이슈 탭에 등록합니다. (예: "플레이어 기본 공격 구현")
2. **브랜치 생성**: `main` 브랜치에서 새로운 기능 브랜치를 팝니다.
   ```bash
   git checkout -b feature/new-feature
   ```
3. **코드 작성 및 커밋**:
   - 커밋 메시지는 명확하게 작성합니다.
4. **PR (Pull Request)**:
   - 작업이 완료되면 GitHub에서 `main` 브랜치로 PR을 생성합니다.
   - 스스로 코드를 검토하고 병합(Merge)합니다.

## 📝 커밋 컨벤션 (Commit Convention)
일관성 있는 히스토리를 위해 다음 접두어를 사용합니다.

| 태그 | 설명 | 예시 |
| :--- | :--- | :--- |
| `feat` | 새로운 기능 추가 | `feat: Add player jump logic` |
| `fix` | 버그 수정 | `fix: Resolve crash on death` |
| `docs` | 문서 수정 | `docs: Update README` |
| `style` | 코드 포맷팅 (로직 변경 없음) | `style: Edit coding convention` |
| `refactor` | 코드 리팩토링 | `refactor: Optimize AI pathfinding` |
| `chore` | 빌드 수정, 패키지 매니저 설정 등 | `chore: Update UE5 project settings` |

## 📁 파일 관리 규칙
- **C++ 클래스**: `PascalCase` (예: `MyCharacter.cpp`)
- **변수명**: `CamelCase` (예: `healthPoint`)
- **블루프린트**: 접두사 `BP_` 사용 (예: `BP_GladitorCharacter`)
