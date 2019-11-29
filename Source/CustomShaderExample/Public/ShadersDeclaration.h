#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"


//This buffer should contain variables that never, or rarely change
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderConstantParameters, )
SHADER_PARAMETER(FVector4, StartColor)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

//This buffer is for variables that change very often (each frame for example)
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderVariableParameters, )
SHADER_PARAMETER(FMatrix, WorldViewProj)
SHADER_PARAMETER(float, TextureParameterBlendFactor)
END_GLOBAL_SHADER_PARAMETER_STRUCT()


typedef TUniformBufferRef<FPixelShaderConstantParameters> FPixelShaderConstantParametersRef;
typedef TUniformBufferRef<FPixelShaderVariableParameters> FPixelShaderVariableParametersRef;

/************************************************************************/
/* This is the type we use as vertices for our fullscreen quad.         */
/************************************************************************/
struct FTextureVertex
{
	FVector4 Position;
	FColor Color;

	friend FArchive& operator<<(FArchive& Ar, FTextureVertex& V)
	{
		Ar << V.Position;
		Ar << V.Color;
		return Ar;
	}
};


/************************************************************************/
/* We define our vertex declaration to let us get our UV coords into    */
/* the shader                                                           */
/************************************************************************/
class FVertexDeclarationExample : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FTextureVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, Position), VET_Float3, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, Color), VET_Color, 1, Stride));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};


/************************************************************************/
/* A simple passthrough vertexshader that we will use.                  */
/************************************************************************/
class FVertexShaderExample : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FVertexShaderExample, Global);
public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) {
		return true;
	};

	FVertexShaderExample(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FGlobalShader(Initializer)
	{}
	FVertexShaderExample() {}
};


/***************************************************************************/
/* This class is what encapsulates the shader in the engine.               */
/* It is the main bridge between the HLSL located in the engine directory  */
/* and the engine itself.                                                  */
/***************************************************************************/
class FPixelShaderExample : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FPixelShaderExample, Global);

public:

	FPixelShaderExample() {}

	explicit FPixelShaderExample(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) {
		return true;
	};

	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParams = FGlobalShader::Serialize(Ar);

		Ar << TextureParameter;

		return bShaderHasOutdatedParams;
	}

	//This function is required to let us bind our runtime surface to the shader using an SRV.
	void SetOutputTexture(FRHICommandList& RHICmdList, FShaderResourceViewRHIRef TextureParameterSRV);
	//This function is required to bind our constant / uniform buffers to the shader.
	void SetUniformBuffers(FRHICommandList& RHICmdList, FPixelShaderConstantParameters& ConstantParameters, FPixelShaderVariableParameters& VariableParameters);
	//This is used to clean up the buffer binds after each invocation to let them be changed and used elsewhere if needed.
	void UnbindBuffers(FRHICommandList& RHICmdList);

private:
	//This is how you declare resources that are going to be made available in the HLSL
	FShaderResourceParameter TextureParameter;
};
