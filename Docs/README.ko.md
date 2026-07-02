# Unreal MCP 설치 및 사용 가이드 (한국어)

이 문서는 [monomable/unreal-mcp](https://github.com/monomable/unreal-mcp)(원본: [chongdashu/unreal-mcp](https://github.com/chongdashu/unreal-mcp))를 설치하고 사용하는 방법을 한국어로 정리한 가이드입니다.

> 이 포크에서 추가/변경된 사항은 최상위 [README.md](../README.md)의 "Modifications in this Fork" 항목을 참고하세요. 아래 사용법에도 새로 추가된 기능을 함께 설명합니다.

## 목차

1. [사전 준비물](#1-사전-준비물)
2. [Unreal 플러그인 설치](#2-unreal-플러그인-설치)
3. [Python MCP 서버 설치](#3-python-mcp-서버-설치)
4. [MCP 클라이언트 설정](#4-mcp-클라이언트-설정)
5. [동작 확인](#5-동작-확인)
6. [기본 사용법](#6-기본-사용법)
7. [이 포크에서 추가된 기능 사용법](#7-이-포크에서-추가된-기능-사용법)
8. [문제 해결](#8-문제-해결)

---

## 1. 사전 준비물

- **Unreal Engine 5.5 이상** (이 포크는 **5.5.4에서만 테스트**되었습니다. 다른 5.5.x 버전은 검증되지 않았습니다 — [Releases](https://github.com/monomable/unreal-mcp/releases) 참고)
- **Python 3.10 이상** (권장: 3.12+)
- **uv** (Python 패키지/가상환경 관리자)
- MCP를 지원하는 클라이언트: Claude Desktop, Cursor, Windsurf 등

`uv`가 없다면 먼저 설치합니다.

```bash
# macOS / Linux
curl -LsSf https://astral.sh/uv/install.sh | sh

# Windows (PowerShell)
powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"
```

---

## 2. Unreal 플러그인 설치

두 가지 방법 중 하나를 선택합니다.

### 방법 A — 동봉된 샘플 프로젝트 사용 (가장 빠름)

저장소에 포함된 `MCPGameProject`는 UnrealMCP 플러그인이 이미 적용된 UE 5.5 빈 프로젝트입니다.

1. `MCPGameProject/MCPGameProject.uproject`를 우클릭 → **Generate Visual Studio project files**
2. 생성된 `.sln` 파일을 열고 빌드 구성을 **Development Editor**로 설정 후 빌드
3. 빌드가 끝나면 `.uproject`를 더블클릭해 에디터 실행

### 방법 B — 기존 프로젝트에 플러그인 추가

1. `MCPGameProject/Plugins/UnrealMCP` 폴더 전체를 자신의 프로젝트의 `Plugins` 폴더로 복사
   - 프로젝트에 `Plugins` 폴더가 없다면 새로 만듭니다: `<YourProject>/Plugins/UnrealMCP`
2. 언리얼 에디터를 실행하고 **Edit ▸ Plugins**로 이동
3. **Editor** 카테고리에서 **UnrealMCP** 플러그인을 찾아 체크박스로 활성화
4. 에디터 재시작 안내가 뜨면 재시작
5. `.uproject` 파일을 우클릭 → **Generate Visual Studio project files**
6. `.sln`을 열고 플랫폼/구성에 맞춰 빌드

빌드가 성공하면 에디터가 켜져 있는 동안 플러그인이 로컬 **TCP 55557 포트**에서 명령을 대기합니다 (`Output Log`에서 `MCPServerRunnable` 관련 로그로 확인 가능).

---

## 3. Python MCP 서버 설치

```bash
cd Python

# 가상환경 생성
uv venv

# 가상환경 활성화
source .venv/bin/activate      # macOS / Linux
.venv\Scripts\activate         # Windows

# 의존성 설치
uv pip install -e .
```

이 단계까지 완료하면 `unreal_mcp_server.py`를 통해 MCP 서버를 실행할 준비가 된 것입니다. 서버는 별도로 수동 실행할 필요 없이, 아래 4단계에서 MCP 클라이언트가 `uv run`으로 자동 구동합니다.

---

## 4. MCP 클라이언트 설정

사용 중인 MCP 클라이언트의 설정 파일에 아래 내용을 추가합니다. `<path/to/Python>` 부분은 이 저장소를 클론한 경로의 `Python` 폴더 **절대 경로**로 바꿔주세요.

```json
{
  "mcpServers": {
    "unrealMCP": {
      "command": "uv",
      "args": [
        "--directory",
        "<path/to/Python>",
        "run",
        "unreal_mcp_server.py"
      ]
    }
  }
}
```

Windows 예시:

```json
{
  "mcpServers": {
    "unrealMCP": {
      "command": "uv",
      "args": [
        "--directory",
        "C:\\Users\\<사용자명>\\Documents\\unreal-mcp\\Python",
        "run",
        "unreal_mcp_server.py"
      ]
    }
  }
}
```

### 클라이언트별 설정 파일 위치

| MCP 클라이언트 | 설정 파일 위치 | 비고 |
|---|---|---|
| Claude Desktop | `~/.config/claude-desktop/mcp.json` | Windows: `%USERPROFILE%\.config\claude-desktop\mcp.json` |
| Cursor | `.cursor/mcp.json` | 프로젝트 루트 기준 |
| Windsurf | `~/.config/windsurf/mcp.json` | Windows: `%USERPROFILE%\.config\windsurf\mcp.json` |

설정 저장 후 MCP 클라이언트를 재시작하면 `unrealMCP` 서버가 도구 목록에 나타납니다.

---

## 5. 동작 확인

1. Unreal 에디터에서 플러그인이 적용된 프로젝트를 **먼저 실행**해 둡니다 (에디터가 켜져 있어야 TCP 서버가 명령을 받을 수 있습니다).
2. MCP 클라이언트에서 `get_actors_in_level` 같은 간단한 도구를 호출해봅니다.
3. 정상적으로 현재 레벨의 액터 목록이 반환되면 연결 성공입니다.

---

## 6. 기본 사용법

MCP 클라이언트(Claude 등)에게 자연어로 요청하면 내부적으로 아래와 같은 카테고리의 도구가 호출됩니다.

- **액터 관리**: 큐브/스피어/라이트/카메라 생성 및 삭제, 트랜스폼(위치/회전/크기) 설정, 이름으로 액터 검색
- **블루프린트 개발**: 블루프린트 클래스 생성, 컴포넌트 추가/설정, 물리 속성 설정, 컴파일, 블루프린트 액터 스폰
- **블루프린트 노드 그래프**: 이벤트 노드/함수 호출 노드 추가, 노드 연결, 변수 추가
- **에디터 제어**: 뷰포트 카메라를 특정 액터/위치로 포커스

예시 프롬프트:

> "레벨에 붉은색 큐브를 만들고 (0, 0, 100) 위치에 배치해줘"

> "PlayerCharacter 블루프린트에 BeginPlay 이벤트 노드를 추가하고, Health 변수를 100으로 초기화하는 로직을 연결해줘"

각 도구의 상세 파라미터는 [Tools 문서](Tools/README.md)를 참고하세요.

---

## 7. 이 포크에서 추가된 기능 사용법

원본 대비 이 포크에서 추가된 기능은 다음과 같습니다. 자세한 변경 이유는 [최상위 README](../README.md#-modifications-in-this-fork)를 참고하세요.

### 7.1 EventGraph 외의 그래프 대상 지정 (`graph_name`)

기존에는 노드 추가/연결 명령이 항상 메인 `EventGraph`에만 적용되었지만, 이제 커스텀 함수 그래프(예: `Enable`이라는 이름의 함수)를 지정할 수 있습니다.

```python
# graph_name을 생략하면 기존과 동일하게 EventGraph를 사용합니다.
add_blueprint_function_node(
    blueprint_name="BP_MyActor",
    target="self",
    function_name="PrintString",
    graph_name="Enable"   # "Enable"이라는 커스텀 함수 그래프를 대상으로 지정
)

connect_blueprint_nodes(
    blueprint_name="BP_MyActor",
    source_node_id="...",
    source_pin="then",
    target_node_id="...",
    target_pin="execute",
    graph_name="Enable"
)
```

### 7.2 블루프린트의 그래프 목록 조회

```python
list_blueprint_graphs(blueprint_name="BP_MyActor")
```
블루프린트에 어떤 함수 그래프들이 있는지 이름을 미리 확인할 때 사용합니다.

### 7.3 변수 Get/Set 노드 추가

```python
add_blueprint_variable_get_node(blueprint_name="BP_MyActor", variable_name="Health")
add_blueprint_variable_set_node(blueprint_name="BP_MyActor", variable_name="Health")
```

다른 컴포넌트(형제 컴포넌트 등)가 소유한 변수를 참조하는 노드를 만들 수도 있습니다. 이 경우 생성된 노드에 `Target` 핀이 생기므로, 해당 오너에 대한 참조 노드와 연결해줘야 합니다.

### 7.4 핀 연결 해제 / 노드 삭제

```python
break_blueprint_pin_links(blueprint_name="BP_MyActor", node_id="...", pin_name="then")
delete_blueprint_node(blueprint_name="BP_MyActor", node_id="...")
```

노드 그래프를 정리하거나 잘못 만든 노드를 되돌릴 때 사용합니다.

### 7.5 대용량 응답 처리 안정성

노드가 많은 그래프를 통째로 덤프하는 등 응답이 큰 경우에도, 이제 TCP 전송이 끊기지 않고 끝까지 전송되도록 수정되었습니다. 별도로 설정할 것은 없으며 자동으로 적용됩니다.

---

## 8. 문제 해결

- **도구 호출 시 연결 오류가 난다** → Unreal 에디터가 실행 중인지, 플러그인이 활성화되어 있는지 확인하세요. 플러그인은 에디터가 켜져 있을 때만 55557 포트에서 대기합니다.
- **MCP 클라이언트에 `unrealMCP`가 안 보인다** → `mcp.json`의 `--directory` 경로가 실제 `Python` 폴더의 절대 경로와 일치하는지 확인하고, 클라이언트를 완전히 재시작하세요.
- **Python 의존성 오류** → `Python` 폴더에서 `uv venv` 후 `uv pip install -e .`를 다시 실행해보세요.
- **자세한 로그 확인** → `Python/unreal_mcp.log` 파일과 Unreal 에디터의 `Output Log`(검색어: `MCPServerRunnable`, `UnrealMCPBridge`)를 확인하세요.
- **UE 5.5.4가 아닌 버전에서 문제가 생긴다** → 이 포크는 5.5.4에서만 검증되었습니다. 다른 버전에서 발생한 이슈는 [Issues](https://github.com/monomable/unreal-mcp/issues)에 제보해주세요.
