#include "Commands/UnrealMCPMaterialCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "EditorAssetLibrary.h"
#include "MaterialEditingLibrary.h"
#include "SceneTypes.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionAbs.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Materials/MaterialExpressionMax.h"
#include "Materials/MaterialExpressionMin.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    // Maps the short, user-facing property names accepted by connect_material_property
    // to the engine's EMaterialProperty enum used for the final material output pins.
    bool ResolveMaterialPropertyName(const FString& PropertyName, EMaterialProperty& OutProperty)
    {
        static const TMap<FString, EMaterialProperty> Lookup = {
            { TEXT("BaseColor"), MP_BaseColor },
            { TEXT("Metallic"), MP_Metallic },
            { TEXT("Specular"), MP_Specular },
            { TEXT("Roughness"), MP_Roughness },
            { TEXT("Anisotropy"), MP_Anisotropy },
            { TEXT("EmissiveColor"), MP_EmissiveColor },
            { TEXT("Opacity"), MP_Opacity },
            { TEXT("OpacityMask"), MP_OpacityMask },
            { TEXT("Normal"), MP_Normal },
            { TEXT("Tangent"), MP_Tangent },
            { TEXT("WorldPositionOffset"), MP_WorldPositionOffset },
            { TEXT("AmbientOcclusion"), MP_AmbientOcclusion },
            { TEXT("Refraction"), MP_Refraction },
            { TEXT("SubsurfaceColor"), MP_SubsurfaceColor },
            { TEXT("PixelDepthOffset"), MP_PixelDepthOffset },
        };
        const EMaterialProperty* Found = Lookup.Find(PropertyName);
        if (Found)
        {
            OutProperty = *Found;
            return true;
        }
        return false;
    }
}

FUnrealMCPMaterialCommands::FUnrealMCPMaterialCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("create_material_expression"))
    {
        return HandleCreateMaterialExpression(Params);
    }
    else if (CommandType == TEXT("connect_material_expressions"))
    {
        return HandleConnectMaterialExpressions(Params);
    }
    else if (CommandType == TEXT("connect_material_property"))
    {
        return HandleConnectMaterialProperty(Params);
    }
    else if (CommandType == TEXT("set_material_expression_property"))
    {
        return HandleSetMaterialExpressionProperty(Params);
    }
    else if (CommandType == TEXT("set_material_property"))
    {
        return HandleSetMaterialProperty(Params);
    }
    else if (CommandType == TEXT("compile_material"))
    {
        return HandleCompileMaterial(Params);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown Material command: %s"), *CommandType));
}

UMaterial* FUnrealMCPMaterialCommands::LoadMaterialByPath(const FString& MaterialPath, FString& OutError)
{
    UObject* Asset = UEditorAssetLibrary::LoadAsset(MaterialPath);
    UMaterial* Material = Cast<UMaterial>(Asset);
    if (!Material)
    {
        OutError = FString::Printf(TEXT("Could not load Material asset at '%s'"), *MaterialPath);
    }
    return Material;
}

UMaterialExpression* FUnrealMCPMaterialCommands::FindExpressionByNodeName(UMaterial* Material, const FString& NodeName)
{
    if (!Material)
    {
        return nullptr;
    }
    // CreateMaterialExpression parents the new expression object directly under the Material,
    // and HandleCreateMaterialExpression renames it to NodeName, so a direct outer-scoped
    // object lookup is a reliable way to re-find it in later commands (no separate registry needed).
    return FindObject<UMaterialExpression>(Material, *NodeName);
}

UClass* FUnrealMCPMaterialCommands::ResolveExpressionClass(const FString& ExpressionType)
{
    static const TMap<FString, UClass*> Lookup = {
        { TEXT("TextureSampleParameter2D"), UMaterialExpressionTextureSampleParameter2D::StaticClass() },
        { TEXT("VectorParameter"), UMaterialExpressionVectorParameter::StaticClass() },
        { TEXT("ScalarParameter"), UMaterialExpressionScalarParameter::StaticClass() },
        { TEXT("TextureCoordinate"), UMaterialExpressionTextureCoordinate::StaticClass() },
        { TEXT("ComponentMask"), UMaterialExpressionComponentMask::StaticClass() },
        { TEXT("Add"), UMaterialExpressionAdd::StaticClass() },
        { TEXT("Subtract"), UMaterialExpressionSubtract::StaticClass() },
        { TEXT("Multiply"), UMaterialExpressionMultiply::StaticClass() },
        { TEXT("Divide"), UMaterialExpressionDivide::StaticClass() },
        { TEXT("Abs"), UMaterialExpressionAbs::StaticClass() },
        { TEXT("Saturate"), UMaterialExpressionSaturate::StaticClass() },
        { TEXT("Max"), UMaterialExpressionMax::StaticClass() },
        { TEXT("Min"), UMaterialExpressionMin::StaticClass() },
        { TEXT("If"), UMaterialExpressionIf::StaticClass() },
        { TEXT("Constant"), UMaterialExpressionConstant::StaticClass() },
        { TEXT("Constant2Vector"), UMaterialExpressionConstant2Vector::StaticClass() },
        { TEXT("Constant3Vector"), UMaterialExpressionConstant3Vector::StaticClass() },
        { TEXT("Constant4Vector"), UMaterialExpressionConstant4Vector::StaticClass() },
    };
    UClass* const* Found = Lookup.Find(ExpressionType);
    return Found ? *Found : nullptr;
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleCreateMaterialExpression(const TSharedPtr<FJsonObject>& Params)
{
    FString MaterialPath, ExpressionType, NodeName;
    if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath) ||
        !Params->TryGetStringField(TEXT("expression_type"), ExpressionType) ||
        !Params->TryGetStringField(TEXT("node_name"), NodeName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_path', 'expression_type', or 'node_name' parameter"));
    }

    FString LoadError;
    UMaterial* Material = LoadMaterialByPath(MaterialPath, LoadError);
    if (!Material)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(LoadError);
    }

    UClass* ExpressionClass = ResolveExpressionClass(ExpressionType);
    if (!ExpressionClass)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unsupported expression_type: '%s'"), *ExpressionType));
    }

    double PosX = 0.0, PosY = 0.0;
    Params->TryGetNumberField(TEXT("pos_x"), PosX);
    Params->TryGetNumberField(TEXT("pos_y"), PosY);

    if (FindExpressionByNodeName(Material, NodeName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("A node named '%s' already exists on this material"), *NodeName));
    }

    UMaterialExpression* Expression = UMaterialEditingLibrary::CreateMaterialExpression(Material, ExpressionClass, static_cast<int32>(PosX), static_cast<int32>(PosY));
    if (!Expression)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create expression of type '%s'"), *ExpressionType));
    }
    // Desc shows up as an in-graph comment (nice for humans); the Rename below is what
    // FindExpressionByNodeName actually relies on to re-find this node later.
    Expression->Desc = NodeName;
    if (!Expression->Rename(*NodeName, nullptr, REN_DontCreateRedirectors | REN_NonTransactional))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Created expression but failed to name it '%s'"), *NodeName));
    }

    TArray<TSharedPtr<FJsonValue>> PropertyErrors;
    const TSharedPtr<FJsonObject>* PropertiesObj;
    if (Params->TryGetObjectField(TEXT("properties"), PropertiesObj))
    {
        for (const auto& Pair : (*PropertiesObj)->Values)
        {
            FString PropError;
            if (!FUnrealMCPCommonUtils::SetObjectProperty(Expression, Pair.Key, Pair.Value, PropError))
            {
                TSharedPtr<FJsonObject> ErrEntry = MakeShareable(new FJsonObject);
                ErrEntry->SetStringField(TEXT("property"), Pair.Key);
                ErrEntry->SetStringField(TEXT("error"), PropError);
                PropertyErrors.Add(MakeShareable(new FJsonValueObject(ErrEntry)));
            }
        }
    }

    Material->MarkPackageDirty();

    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("node_name"), NodeName);
    Result->SetStringField(TEXT("expression_type"), ExpressionType);
    Result->SetArrayField(TEXT("property_errors"), PropertyErrors);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleConnectMaterialExpressions(const TSharedPtr<FJsonObject>& Params)
{
    FString MaterialPath, FromNode, ToNode;
    if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath) ||
        !Params->TryGetStringField(TEXT("from_node"), FromNode) ||
        !Params->TryGetStringField(TEXT("to_node"), ToNode))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_path', 'from_node', or 'to_node' parameter"));
    }
    FString FromOutput, ToInput;
    Params->TryGetStringField(TEXT("from_output"), FromOutput);
    Params->TryGetStringField(TEXT("to_input"), ToInput);

    FString LoadError;
    UMaterial* Material = LoadMaterialByPath(MaterialPath, LoadError);
    if (!Material)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(LoadError);
    }

    UMaterialExpression* FromExpr = FindExpressionByNodeName(Material, FromNode);
    UMaterialExpression* ToExpr = FindExpressionByNodeName(Material, ToNode);
    if (!FromExpr || !ToExpr)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Could not find node(s): from='%s' (%s), to='%s' (%s)"),
            *FromNode, FromExpr ? TEXT("found") : TEXT("missing"),
            *ToNode, ToExpr ? TEXT("found") : TEXT("missing")));
    }

    bool bConnected = UMaterialEditingLibrary::ConnectMaterialExpressions(FromExpr, FromOutput, ToExpr, ToInput);
    if (!bConnected)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to connect '%s'(%s) -> '%s'(%s). Check pin names."),
            *FromNode, *FromOutput, *ToNode, *ToInput));
    }

    Material->MarkPackageDirty();
    return FUnrealMCPCommonUtils::CreateSuccessResponse();
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleConnectMaterialProperty(const TSharedPtr<FJsonObject>& Params)
{
    FString MaterialPath, FromNode, PropertyName;
    if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath) ||
        !Params->TryGetStringField(TEXT("from_node"), FromNode) ||
        !Params->TryGetStringField(TEXT("property"), PropertyName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_path', 'from_node', or 'property' parameter"));
    }
    FString FromOutput;
    Params->TryGetStringField(TEXT("from_output"), FromOutput);

    EMaterialProperty MaterialProperty;
    if (!ResolveMaterialPropertyName(PropertyName, MaterialProperty))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unsupported material property: '%s'"), *PropertyName));
    }

    FString LoadError;
    UMaterial* Material = LoadMaterialByPath(MaterialPath, LoadError);
    if (!Material)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(LoadError);
    }

    UMaterialExpression* FromExpr = FindExpressionByNodeName(Material, FromNode);
    if (!FromExpr)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Could not find node '%s'"), *FromNode));
    }

    bool bConnected = UMaterialEditingLibrary::ConnectMaterialProperty(FromExpr, FromOutput, MaterialProperty);
    if (!bConnected)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to connect '%s'(%s) -> %s"), *FromNode, *FromOutput, *PropertyName));
    }

    Material->MarkPackageDirty();
    return FUnrealMCPCommonUtils::CreateSuccessResponse();
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleSetMaterialExpressionProperty(const TSharedPtr<FJsonObject>& Params)
{
    FString MaterialPath, NodeName, PropertyName;
    if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath) ||
        !Params->TryGetStringField(TEXT("node_name"), NodeName) ||
        !Params->TryGetStringField(TEXT("property_name"), PropertyName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_path', 'node_name', or 'property_name' parameter"));
    }
    TSharedPtr<FJsonValue> Value = Params->TryGetField(TEXT("value"));
    if (!Value.IsValid())
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'value' parameter"));
    }

    FString LoadError;
    UMaterial* Material = LoadMaterialByPath(MaterialPath, LoadError);
    if (!Material)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(LoadError);
    }

    UMaterialExpression* Expr = FindExpressionByNodeName(Material, NodeName);
    if (!Expr)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Could not find node '%s'"), *NodeName));
    }

    FString PropError;
    if (!FUnrealMCPCommonUtils::SetObjectProperty(Expr, PropertyName, Value, PropError))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(PropError);
    }

    Material->MarkPackageDirty();
    return FUnrealMCPCommonUtils::CreateSuccessResponse();
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleSetMaterialProperty(const TSharedPtr<FJsonObject>& Params)
{
    FString MaterialPath, PropertyName;
    if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath) ||
        !Params->TryGetStringField(TEXT("property_name"), PropertyName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_path' or 'property_name' parameter"));
    }
    TSharedPtr<FJsonValue> Value = Params->TryGetField(TEXT("value"));
    if (!Value.IsValid())
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'value' parameter"));
    }

    FString LoadError;
    UMaterial* Material = LoadMaterialByPath(MaterialPath, LoadError);
    if (!Material)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(LoadError);
    }

    FString PropError;
    if (!FUnrealMCPCommonUtils::SetObjectProperty(Material, PropertyName, Value, PropError))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(PropError);
    }

    Material->PostEditChange();
    Material->MarkPackageDirty();
    return FUnrealMCPCommonUtils::CreateSuccessResponse();
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleCompileMaterial(const TSharedPtr<FJsonObject>& Params)
{
    FString MaterialPath;
    if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_path' parameter"));
    }

    FString LoadError;
    UMaterial* Material = LoadMaterialByPath(MaterialPath, LoadError);
    if (!Material)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(LoadError);
    }

    UMaterialEditingLibrary::RecompileMaterial(Material);
    bool bSaved = UEditorAssetLibrary::SaveLoadedAsset(Material);

    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    Result->SetBoolField(TEXT("success"), true);
    Result->SetBoolField(TEXT("saved"), bSaved);
    return Result;
}
