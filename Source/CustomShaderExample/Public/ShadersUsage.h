#pragma once

#include "CoreMinimal.h"
#include "ShadersDeclaration.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PositionVertexBuffer.h"
#include "ShadersUsage.generated.h"


UCLASS()
class AShaderUsageExample : public AActor
{
	GENERATED_BODY()

public:
	AShaderUsageExample();
	~AShaderUsageExample();

	/********************************************************************************************************/
	/* Let the user change rendertarget during runtime if they want to :D                                   */
	/* @param RenderTarget - This is the output rendertarget!                                               */
	/* @param InputTexture - This is a rendertarget that's used as a texture parameter to the shader :)     */
	/* @param EndColor - This will be set to the dynamic parameter buffer each frame                        */
	/* @param TextureParameterBlendFactor - The scalar weight that decides how much of the texture to blend */
	/********************************************************************************************************/
	UFUNCTION(BlueprintCallable)
	void ExecutePixelShader(UTextureRenderTarget2D* RenderTarget, UTexture2D* InputTexture, FColor EndColor, float TextureParameterBlendFactor);

	/************************************************************************/
	/* Only execute this from the render thread!!!                          */
	/************************************************************************/
	void ExecutePixelShaderInternal(FRHICommandListImmediate& RHICmdList);

private:
	bool bIsPixelShaderExecuting;
	bool bMustRegenerateSRV;
	bool bIsUnloading;

	FPixelShaderConstantParameters ConstantParameters;
	FPixelShaderVariableParameters VariableParameters;
	ERHIFeatureLevel::Type FeatureLevel;

	/** Main texture */
	FTexture2DRHIRef CurrentTexture;
	FTexture2DRHIRef TextureParameter;
	UTextureRenderTarget2D* CurrentRenderTarget = nullptr;

	/** Since we are only reading from the resource, we do not need a UAV; an SRV is sufficient */
	FShaderResourceViewRHIRef TextureParameterSRV;

	//TShaderMapRef<FVertexShaderExample> MyVS(ShaderMap);
	//TShaderMapRef<FPixelShaderExample> MyPS(ShaderMap);

	FVertexBufferRHIRef vertBuf;

	void SaveScreenshot(FRHICommandListImmediate& RHICmdList);
};