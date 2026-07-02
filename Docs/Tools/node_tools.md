# Unreal MCP Node Tools

This document provides detailed information about the Blueprint node tools available in the Unreal MCP integration.

## Overview

Node tools allow you to manipulate Blueprint graph nodes and connections programmatically, including adding event nodes, function nodes, variables, and creating connections between nodes.

### Targeting custom function graphs (`graph_name`)

> Added in this fork.

Most node tools accept an optional `graph_name` parameter. When omitted, the tool targets the Blueprint's main `EventGraph`, matching the original behavior. Pass the name of a custom function graph (e.g. `"Enable"`) to target that graph instead — useful for wiring up logic inside a Blueprint function rather than the event graph.

Use [`list_blueprint_graphs`](#list_blueprint_graphs) to confirm the exact name of a function graph before targeting it.

## Node Tools

### add_blueprint_event_node

Add an event node to a Blueprint's event graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `event_type` (string) - Type of event (BeginPlay, Tick, etc.)
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])

**Returns:**
- Response containing the node ID and success status

**Example:**
```json
{
  "command": "add_blueprint_event_node",
  "params": {
    "blueprint_name": "MyActor",
    "event_type": "BeginPlay",
    "node_position": [100, 100]
  }
}
```

### add_blueprint_input_action_node

Add an input action event node to a Blueprint's event graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `action_name` (string) - Name of the input action to respond to
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])

**Returns:**
- Response containing the node ID and success status

**Example:**
```json
{
  "command": "add_blueprint_input_action_node",
  "params": {
    "blueprint_name": "MyActor",
    "action_name": "Jump",
    "node_position": [200, 200]
  }
}
```

### add_blueprint_function_node

Add a function call node to a Blueprint's graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `target` (string) - Target object for the function (component name or self)
- `function_name` (string) - Name of the function to call
- `params` (object, optional) - Parameters to set on the function node
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])
- `graph_name` (string, optional) - *Fork addition.* Name of a custom function graph to target (e.g. `"Enable"`). Defaults to the main EventGraph if omitted.

**Returns:**
- Response containing the node ID and success status

**Example:**
```json
{
  "command": "add_blueprint_function_node",
  "params": {
    "blueprint_name": "MyActor",
    "target": "Mesh",
    "function_name": "SetRelativeLocation",
    "params": {
      "NewLocation": [0, 0, 100]
    },
    "node_position": [300, 300],
    "graph_name": "Enable"
  }
}
```

### connect_blueprint_nodes

Connect two nodes in a Blueprint's graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `source_node_id` (string) - ID of the source node
- `source_pin` (string) - Name of the output pin on the source node
- `target_node_id` (string) - ID of the target node
- `target_pin` (string) - Name of the input pin on the target node
- `graph_name` (string, optional) - *Fork addition.* Name of a custom function graph to target (e.g. `"Enable"`). Defaults to the main EventGraph if omitted.

**Returns:**
- Response indicating success or failure

**Example:**
```json
{
  "command": "connect_blueprint_nodes",
  "params": {
    "blueprint_name": "MyActor",
    "source_node_id": "node_1",
    "source_pin": "exec",
    "target_node_id": "node_2",
    "target_pin": "exec",
    "graph_name": "Enable"
  }
}
```

### add_blueprint_variable

Add a variable to a Blueprint.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `variable_name` (string) - Name of the variable
- `variable_type` (string) - Type of the variable (Boolean, Integer, Float, Vector, etc.)
- `default_value` (any, optional) - Default value for the variable
- `is_exposed` (boolean, optional) - Whether to expose the variable to the editor (default: false)

**Returns:**
- Response indicating success or failure

**Example:**
```json
{
  "command": "add_blueprint_variable",
  "params": {
    "blueprint_name": "MyActor",
    "variable_name": "Health",
    "variable_type": "Float",
    "default_value": 100.0,
    "is_exposed": true
  }
}
```

### add_blueprint_variable_get_node

> Added in this fork.

Add a "Get \<Variable\>" node to a Blueprint's graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `variable_name` (string) - Name of the variable to get
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])
- `graph_name` (string, optional) - Name of a custom function graph to target (e.g. `"Enable"`). Defaults to the main EventGraph if omitted.
- `owner_blueprint` (string, optional) - Name of a different Blueprint that actually declares the variable (e.g. a sibling component like `"AC_PlayerStatus"`). When set, the created node references the variable on that class instead of on `blueprint_name`, and gets a `Target` pin that must be wired to a reference to that component/object. Omit this to get a variable declared directly on `blueprint_name` itself.

**Returns:**
- Response containing the node ID and success status

**Example:**
```json
{
  "command": "add_blueprint_variable_get_node",
  "params": {
    "blueprint_name": "MyActor",
    "variable_name": "Health",
    "node_position": [100, 400]
  }
}
```

Referencing a variable owned by a sibling component:
```json
{
  "command": "add_blueprint_variable_get_node",
  "params": {
    "blueprint_name": "MyActor",
    "variable_name": "CurrentStamina",
    "owner_blueprint": "AC_PlayerStatus",
    "node_position": [100, 460]
  }
}
```

### add_blueprint_variable_set_node

> Added in this fork.

Add a "Set \<Variable\>" node to a Blueprint's graph. Accepts the same parameters as [`add_blueprint_variable_get_node`](#add_blueprint_variable_get_node) above (`blueprint_name`, `variable_name`, `node_position`, `graph_name`, `owner_blueprint`).

**Example:**
```json
{
  "command": "add_blueprint_variable_set_node",
  "params": {
    "blueprint_name": "MyActor",
    "variable_name": "Health",
    "node_position": [100, 520]
  }
}
```

### create_input_mapping

Create an input mapping for the project.

**Parameters:**
- `action_name` (string) - Name of the input action
- `key` (string) - Key to bind (SpaceBar, LeftMouseButton, etc.)
- `input_type` (string, optional) - Type of input mapping (Action or Axis, default: "Action")

**Returns:**
- Response indicating success or failure

**Example:**
```json
{
  "command": "create_input_mapping",
  "params": {
    "action_name": "Jump",
    "key": "SpaceBar",
    "input_type": "Action"
  }
}
```

### add_blueprint_get_self_component_reference

Add a node that gets a reference to a component owned by the current Blueprint.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `component_name` (string) - Name of the component to get a reference to
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])
- `graph_name` (string, optional) - *Fork addition.* Name of a custom function graph to target (e.g. `"Enable"`). Defaults to the main EventGraph if omitted.

**Returns:**
- Response containing the node ID and success status

**Example:**
```json
{
  "command": "add_blueprint_get_self_component_reference",
  "params": {
    "blueprint_name": "MyActor",
    "component_name": "Mesh",
    "node_position": [400, 400],
    "graph_name": "Enable"
  }
}
```

### add_blueprint_self_reference

Add a 'Get Self' node to a Blueprint's graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `node_position` (array, optional) - [X, Y] position in the graph (default: [0, 0])
- `graph_name` (string, optional) - *Fork addition.* Name of a custom function graph to target (e.g. `"Enable"`). Defaults to the main EventGraph if omitted.

**Returns:**
- Response containing the node ID and success status

**Example:**
```json
{
  "command": "add_blueprint_self_reference",
  "params": {
    "blueprint_name": "MyActor",
    "node_position": [500, 500],
    "graph_name": "Enable"
  }
}
```

### find_blueprint_nodes

Find nodes in a Blueprint's graph.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `node_type` (string, optional) - Type of node to find (Event, Function, Variable, Component, Self, or `"All"`). `"Event"` requires `event_type`. `"All"` *(fork addition)* returns every node in the graph with full details (class, title, pins, and their linked connections) — use this to inspect an existing custom function before editing it.
- `event_type` (string, optional) - Specific event type to find (BeginPlay, Tick, etc.), required when `node_type="Event"`
- `graph_name` (string, optional) - *Fork addition.* Name of a custom function graph to target (e.g. `"Enable"`). Defaults to the main EventGraph if omitted.

**Returns:**
- Response containing array of found node IDs (or the full node/pin dump when `node_type="All"`) and success status

**Example:**
```json
{
  "command": "find_blueprint_nodes",
  "params": {
    "blueprint_name": "MyActor",
    "node_type": "Event",
    "event_type": "BeginPlay"
  }
}
```

Dumping every node/pin in a custom function graph:
```json
{
  "command": "find_blueprint_nodes",
  "params": {
    "blueprint_name": "MyActor",
    "node_type": "All",
    "graph_name": "Enable"
  }
}
```

### list_blueprint_graphs

> Added in this fork.

List the names of a Blueprint's event graphs and custom function graphs. Use this to confirm the exact name of a custom function (e.g. `"Enable"`) before targeting it with `graph_name` in other node tools.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint

**Returns:**
- Response containing `event_graphs` and `function_graphs` name arrays

**Example:**
```json
{
  "command": "list_blueprint_graphs",
  "params": {
    "blueprint_name": "MyActor"
  }
}
```

### break_blueprint_pin_links

> Added in this fork.

Break all links on a specific pin of a node in a Blueprint's graph. Use this before wiring a new source into an input pin (or a new target out of an output pin) that already has an existing connection, so the old link doesn't remain alongside the new one.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `node_id` (string) - ID of the node that owns the pin
- `pin_name` (string) - Name of the pin to disconnect
- `graph_name` (string, optional) - Name of a custom function graph to target (e.g. `"Enable"`). Defaults to the main EventGraph if omitted.
- `direction` (string, optional) - `"Input"` or `"Output"` to disambiguate identically-named pins

**Returns:**
- Response indicating success or failure

**Example:**
```json
{
  "command": "break_blueprint_pin_links",
  "params": {
    "blueprint_name": "MyActor",
    "node_id": "node_2",
    "pin_name": "then",
    "direction": "Output"
  }
}
```

### delete_blueprint_node

> Added in this fork.

Delete a node from a Blueprint's graph, breaking all of its pin links first.

**Parameters:**
- `blueprint_name` (string) - Name of the target Blueprint
- `node_id` (string) - ID of the node to delete
- `graph_name` (string, optional) - Name of a custom function graph to target (e.g. `"Enable"`). Defaults to the main EventGraph if omitted.

**Returns:**
- Response indicating success or failure

**Example:**
```json
{
  "command": "delete_blueprint_node",
  "params": {
    "blueprint_name": "MyActor",
    "node_id": "node_2"
  }
}
```

## Error Handling

All command responses include a "success" field indicating whether the operation succeeded, and an optional "message" field with details in case of failure.

```json
{
  "success": false,
  "message": "Blueprint 'MyActor' not found in the project",
  "command": "add_blueprint_event_node"
}
```

## Type Reference

### Node Types

Common node types for the `find_blueprint_nodes` command:

- `Event` - Event nodes (BeginPlay, Tick, etc.)
- `Function` - Function call nodes
- `Variable` - Variable nodes
- `Component` - Component reference nodes
- `Self` - Self reference nodes
- `All` - *(fork addition)* Every node in the target graph, with full pin/connection details

### Variable Types

Common variable types for the `add_blueprint_variable` command:

- `Boolean` - True/false values
- `Integer` - Whole numbers
- `Float` - Decimal numbers
- `Vector` - 3D vector values
- `String` - Text values
- `Object Reference` - References to other objects
- `Actor Reference` - References to actors
- `Component Reference` - References to components
