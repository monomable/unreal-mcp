"""
Material Tools for Unreal MCP.

This module provides tools for building UMaterialExpression node graphs on
existing Material assets (create/connect nodes, wire to the final material
outputs, tweak properties, and recompile/save).
"""

import logging
from typing import Any, Dict

from mcp.server.fastmcp import FastMCP, Context

# Get logger
logger = logging.getLogger("UnrealMCP")


def register_material_tools(mcp: FastMCP):
    """Register Material tools with the MCP server."""

    @mcp.tool()
    def create_material_expression(
        ctx: Context,
        material_path: str,
        expression_type: str,
        node_name: str,
        pos_x: float = 0.0,
        pos_y: float = 0.0,
        properties: Dict[str, Any] = {}
    ) -> Dict[str, Any]:
        """
        Create a new node in a Material's graph.

        Args:
            material_path: Content path of the target Material asset (e.g. "/Game/.../M_Reproject")
            expression_type: Short type name, e.g. "TextureSampleParameter2D", "VectorParameter",
                "ScalarParameter", "TextureCoordinate", "ComponentMask", "Add", "Subtract",
                "Multiply", "Divide", "Abs", "Saturate", "Max", "Min", "If", "Constant",
                "Constant2Vector", "Constant3Vector", "Constant4Vector"
            node_name: Unique name to reference this node in later calls (must be unique per material)
            pos_x, pos_y: Graph position (cosmetic only)
            properties: Property name -> value to apply right after creation
                (e.g. {"ParameterName": "Source"} or {"R": true, "G": true})

        Returns:
            Response with node_name and any per-property errors
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("create_material_expression", {
                "material_path": material_path,
                "expression_type": expression_type,
                "node_name": node_name,
                "pos_x": pos_x,
                "pos_y": pos_y,
                "properties": properties or {}
            })

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            return response

        except Exception as e:
            error_msg = f"Error creating material expression: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def connect_material_expressions(
        ctx: Context,
        material_path: str,
        from_node: str,
        to_node: str,
        from_output: str = "",
        to_input: str = ""
    ) -> Dict[str, Any]:
        """
        Connect one node's output pin to another node's input pin.

        Args:
            material_path: Content path of the target Material asset
            from_node: node_name of the source node
            to_node: node_name of the destination node
            from_output: Output pin name (e.g. "" for default, "R"/"G"/"B"/"A"/"RGB" for multi-output nodes)
            to_input: Input pin name (e.g. "" for default, "A"/"B" for math nodes)

        Returns:
            Response indicating success or failure
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("connect_material_expressions", {
                "material_path": material_path,
                "from_node": from_node,
                "to_node": to_node,
                "from_output": from_output,
                "to_input": to_input
            })

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            return response

        except Exception as e:
            error_msg = f"Error connecting material expressions: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def connect_material_property(
        ctx: Context,
        material_path: str,
        from_node: str,
        property: str,
        from_output: str = ""
    ) -> Dict[str, Any]:
        """
        Connect a node's output directly to one of the Material's final output pins.

        Args:
            material_path: Content path of the target Material asset
            from_node: node_name of the source node
            property: One of "BaseColor", "Metallic", "Specular", "Roughness", "Anisotropy",
                "EmissiveColor", "Opacity", "OpacityMask", "Normal", "Tangent",
                "WorldPositionOffset", "AmbientOcclusion", "Refraction", "SubsurfaceColor",
                "PixelDepthOffset"
            from_output: Output pin name (optional, "" = default output)

        Returns:
            Response indicating success or failure
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("connect_material_property", {
                "material_path": material_path,
                "from_node": from_node,
                "property": property,
                "from_output": from_output
            })

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            return response

        except Exception as e:
            error_msg = f"Error connecting material property: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def set_material_expression_property(
        ctx: Context,
        material_path: str,
        node_name: str,
        property_name: str,
        value: Any
    ) -> Dict[str, Any]:
        """
        Set a property on a previously created node.

        Args:
            material_path: Content path of the target Material asset
            node_name: node_name of the target node
            property_name: Reflected property name on the expression
                (e.g. "ParameterName", "DefaultValue", "R", "G", "B", "A", "CoordinateIndex",
                "ConstA", "ConstB", "Texture")
            value: New value. Bool/Int/Float/String pass through directly. FName properties
                (e.g. ParameterName) take a string. FLinearColor/FVector2D/FVector properties
                take either an array [x,y,z,w] or an object {"r":..,"g":..,"b":..,"a":..} /
                {"x":..,"y":..,"z":..}. Texture/object-reference properties take an asset path
                string (e.g. "/Engine/EngineResources/DefaultTexture").

        Returns:
            Response indicating success or failure
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("set_material_expression_property", {
                "material_path": material_path,
                "node_name": node_name,
                "property_name": property_name,
                "value": value
            })

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            return response

        except Exception as e:
            error_msg = f"Error setting material expression property: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def set_material_property(
        ctx: Context,
        material_path: str,
        property_name: str,
        value: Any
    ) -> Dict[str, Any]:
        """
        Set a property on the Material asset itself (not a graph node).

        Args:
            material_path: Content path of the target Material asset
            property_name: e.g. "MaterialDomain" (value "MD_UI"/"MD_Surface"/...),
                "BlendMode" (value "BLEND_Translucent"/"BLEND_Opaque"/...)
            value: New value (string enum name or number works for byte/enum properties)

        Returns:
            Response indicating success or failure
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("set_material_property", {
                "material_path": material_path,
                "property_name": property_name,
                "value": value
            })

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            return response

        except Exception as e:
            error_msg = f"Error setting material property: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def compile_material(
        ctx: Context,
        material_path: str
    ) -> Dict[str, Any]:
        """
        Recompile and save a Material asset after graph edits.

        Args:
            material_path: Content path of the target Material asset

        Returns:
            Response with a "saved" flag; check the Unreal Output Log for compile errors/warnings
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("compile_material", {
                "material_path": material_path
            })

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            return response

        except Exception as e:
            error_msg = f"Error compiling material: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
