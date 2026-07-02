# Unreal MCP Installation & Usage Guide (English)

This document explains how to install and use [monomable/unreal-mcp](https://github.com/monomable/unreal-mcp) (originally forked from [chongdashu/unreal-mcp](https://github.com/chongdashu/unreal-mcp)).

> For a summary of what was changed in this fork, see the "Modifications in this Fork" section of the top-level [README.md](../README.md). The usage examples below also cover the newly added features.

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Installing the Unreal Plugin](#2-installing-the-unreal-plugin)
3. [Installing the Python MCP Server](#3-installing-the-python-mcp-server)
4. [Configuring Your MCP Client](#4-configuring-your-mcp-client)
5. [Verifying the Connection](#5-verifying-the-connection)
6. [Basic Usage](#6-basic-usage)
7. [Using the Features Added in This Fork](#7-using-the-features-added-in-this-fork)
8. [Troubleshooting](#8-troubleshooting)

---

## 1. Prerequisites

- **Unreal Engine 5.5 or later** (this fork has been **tested only on 5.5.4**. Other 5.5.x versions have not been verified — see [Releases](https://github.com/monomable/unreal-mcp/releases))
- **Python 3.10 or later** (3.12+ recommended)
- **uv** (Python package/environment manager)
- An MCP-capable client: Claude Desktop, Cursor, Windsurf, etc.

If you don't have `uv` yet, install it first:

```bash
# macOS / Linux
curl -LsSf https://astral.sh/uv/install.sh | sh

# Windows (PowerShell)
powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"
```

---

## 2. Installing the Unreal Plugin

Choose one of the two approaches below.

### Option A — Use the bundled sample project (fastest)

The repository ships with `MCPGameProject`, a blank UE 5.5 project that already has the UnrealMCP plugin applied.

1. Right-click `MCPGameProject/MCPGameProject.uproject` → **Generate Visual Studio project files**
2. Open the generated `.sln`, set the build configuration to **Development Editor**, and build
3. Once the build finishes, double-click the `.uproject` to launch the editor

### Option B — Add the plugin to an existing project

1. Copy the entire `MCPGameProject/Plugins/UnrealMCP` folder into your own project's `Plugins` folder
   - If your project doesn't have a `Plugins` folder yet, create one: `<YourProject>/Plugins/UnrealMCP`
2. Launch the Unreal Editor and go to **Edit ▸ Plugins**
3. Find **UnrealMCP** under the **Editor** category and enable its checkbox
4. Restart the editor when prompted
5. Right-click your `.uproject` → **Generate Visual Studio project files**
6. Open the `.sln` and build for your target platform/configuration

Once the build succeeds, the plugin listens for commands on local **TCP port 55557** whenever the editor is open (you can confirm this via `MCPServerRunnable` log lines in the Output Log).

---

## 3. Installing the Python MCP Server

```bash
cd Python

# Create a virtual environment
uv venv

# Activate the virtual environment
source .venv/bin/activate      # macOS / Linux
.venv\Scripts\activate         # Windows

# Install dependencies
uv pip install -e .
```

At this point everything is ready to run the MCP server via `unreal_mcp_server.py`. You don't need to start the server manually — in step 4 below, your MCP client will launch it automatically using `uv run`.

---

## 4. Configuring Your MCP Client

Add the following to your MCP client's configuration file. Replace `<path/to/Python>` with the **absolute path** to the `Python` folder of your clone of this repository.

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

Windows example:

```json
{
  "mcpServers": {
    "unrealMCP": {
      "command": "uv",
      "args": [
        "--directory",
        "C:\\Users\\<username>\\Documents\\unreal-mcp\\Python",
        "run",
        "unreal_mcp_server.py"
      ]
    }
  }
}
```

### Configuration File Locations by Client

| MCP Client | Configuration File Location | Notes |
|---|---|---|
| Claude Desktop | `~/.config/claude-desktop/mcp.json` | On Windows: `%USERPROFILE%\.config\claude-desktop\mcp.json` |
| Cursor | `.cursor/mcp.json` | Located in your project root |
| Windsurf | `~/.config/windsurf/mcp.json` | On Windows: `%USERPROFILE%\.config\windsurf\mcp.json` |

After saving the configuration, restart your MCP client and the `unrealMCP` server should appear in its tool list.

---

## 5. Verifying the Connection

1. In the Unreal Editor, **open the project with the plugin first** (the editor must be running for the TCP server to accept commands).
2. From your MCP client, call a simple tool such as `get_actors_in_level`.
3. If it returns the current level's actor list successfully, the connection is working.

---

## 6. Basic Usage

When you ask your MCP client (e.g. Claude) something in natural language, it calls tools from the categories below under the hood:

- **Actor management**: create/delete actors (cubes, spheres, lights, cameras, etc.), set transforms (position/rotation/scale), find actors by name
- **Blueprint development**: create Blueprint classes, add/configure components, set physics properties, compile, spawn Blueprint actors
- **Blueprint node graph**: add event/function-call nodes, connect nodes, add variables
- **Editor control**: focus the viewport camera on a specific actor or location

Example prompts:

> "Create a red cube in the level and place it at (0, 0, 100)"

> "Add a BeginPlay event node to the PlayerCharacter Blueprint, and wire up logic that initializes the Health variable to 100"

See the [Tools documentation](Tools/README.md) for detailed parameters of each tool.

---

## 7. Using the Features Added in This Fork

Compared to upstream, this fork adds the following. See the top-level [README](../README.md#-modifications-in-this-fork) for the reasoning behind each change.

### 7.1 Targeting graphs other than EventGraph (`graph_name`)

Previously, node-add/connect commands always applied to the main `EventGraph`. Now you can target a custom function graph (e.g. a function named `Enable`).

```python
# Omitting graph_name preserves the old behavior of targeting EventGraph.
add_blueprint_function_node(
    blueprint_name="BP_MyActor",
    target="self",
    function_name="PrintString",
    graph_name="Enable"   # target the custom function graph named "Enable"
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

### 7.2 Listing a Blueprint's graphs

```python
list_blueprint_graphs(blueprint_name="BP_MyActor")
```
Use this to see the names of a Blueprint's function graphs before targeting one.

### 7.3 Adding variable Get/Set nodes

```python
add_blueprint_variable_get_node(blueprint_name="BP_MyActor", variable_name="Health")
add_blueprint_variable_set_node(blueprint_name="BP_MyActor", variable_name="Health")
```

You can also create a node that references a variable owned by another class (e.g. a sibling component). In that case the generated node has a `Target` pin that you'll need to wire to a reference to that owner.

### 7.4 Breaking pin links / deleting nodes

```python
break_blueprint_pin_links(blueprint_name="BP_MyActor", node_id="...", pin_name="then")
delete_blueprint_node(blueprint_name="BP_MyActor", node_id="...")
```

Useful for cleaning up a graph or undoing a mistakenly created node.

### 7.5 More reliable large responses

When dumping a graph with many nodes, responses could previously be silently truncated. TCP sends now loop until the full response is sent, so this happens automatically — no configuration needed.

---

## 8. Troubleshooting

- **Getting a connection error when calling a tool** → Make sure the Unreal Editor is running and the plugin is enabled. The plugin only listens on port 55557 while the editor is open.
- **`unrealMCP` doesn't show up in the MCP client** → Verify that the `--directory` path in `mcp.json` matches the absolute path of your `Python` folder, and fully restart the client.
- **Python dependency errors** → From the `Python` folder, re-run `uv venv` followed by `uv pip install -e .`.
- **Checking detailed logs** → Look at `Python/unreal_mcp.log` and the Unreal Editor's Output Log (search for `MCPServerRunnable`, `UnrealMCPBridge`).
- **Issues on a UE version other than 5.5.4** → This fork has only been verified on 5.5.4. Please report issues from other versions on [Issues](https://github.com/monomable/unreal-mcp/issues).
