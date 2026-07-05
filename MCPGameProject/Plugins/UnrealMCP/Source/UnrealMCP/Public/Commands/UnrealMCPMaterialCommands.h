#pragma once

#include "CoreMinimal.h"
#include "Json.h"

class UMaterial;
class UMaterialExpression;

/**
 * Handles Material graph editing MCP commands.
 * Responsible for building UMaterialExpression node graphs (create/connect nodes,
 * wire to the final material outputs, tweak properties, and recompile/save) on
 * existing Material assets.
 */
class UNREALMCP_API FUnrealMCPMaterialCommands
{
public:
    FUnrealMCPMaterialCommands();

    /**
     * Handle Material-related commands
     * @param CommandType - The type of command to handle
     * @param Params - JSON parameters for the command
     * @return JSON response with results or error
     */
    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    /**
     * Create a new UMaterialExpression node in a Material's graph.
     * @param Params - Must include:
     *                "material_path" - Content path of the target Material asset
     *                "expression_type" - Short name from the supported whitelist (e.g. "Add", "TextureSampleParameter2D")
     *                "node_name" - Unique name used to reference this node in later commands (stored in Expression->Desc)
     *                "pos_x" / "pos_y" - Optional graph position (default 0,0)
     *                "properties" - Optional JSON object of property_name -> value applied immediately after creation
     */
    TSharedPtr<FJsonObject> HandleCreateMaterialExpression(const TSharedPtr<FJsonObject>& Params);

    /**
     * Connect one expression's output pin to another expression's input pin.
     * @param Params - Must include:
     *                "material_path", "from_node", "to_node"
     *                "from_output" - Output pin name (optional, "" = default output)
     *                "to_input" - Input pin name (optional, "" = default input)
     */
    TSharedPtr<FJsonObject> HandleConnectMaterialExpressions(const TSharedPtr<FJsonObject>& Params);

    /**
     * Connect an expression's output pin directly to one of the Material's final outputs.
     * @param Params - Must include:
     *                "material_path", "from_node", "property" (e.g. "EmissiveColor", "Opacity", "BaseColor")
     *                "from_output" - Output pin name (optional, "" = default output)
     */
    TSharedPtr<FJsonObject> HandleConnectMaterialProperty(const TSharedPtr<FJsonObject>& Params);

    /**
     * Set a property on a previously created expression node (found by node_name).
     * @param Params - Must include: "material_path", "node_name", "property_name", "value"
     */
    TSharedPtr<FJsonObject> HandleSetMaterialExpressionProperty(const TSharedPtr<FJsonObject>& Params);

    /**
     * Set a property on the Material asset itself (e.g. MaterialDomain, BlendMode).
     * @param Params - Must include: "material_path", "property_name", "value"
     */
    TSharedPtr<FJsonObject> HandleSetMaterialProperty(const TSharedPtr<FJsonObject>& Params);

    /**
     * Recompile and save a Material asset after graph edits.
     * @param Params - Must include: "material_path"
     */
    TSharedPtr<FJsonObject> HandleCompileMaterial(const TSharedPtr<FJsonObject>& Params);

    // --- Helpers ---
    static UMaterial* LoadMaterialByPath(const FString& MaterialPath, FString& OutError);
    static UMaterialExpression* FindExpressionByNodeName(UMaterial* Material, const FString& NodeName);
    static UClass* ResolveExpressionClass(const FString& ExpressionType);
};
