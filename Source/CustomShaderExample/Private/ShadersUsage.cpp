#include "ShadersUsage.h"
#include "RHIStaticStates.h"
#include "Runtime/RHI/Public/RHIUtilities.h"
#include "Engine/Texture2D.h"
#include "PipelineStateCache.h"
#include "Components.h"
#include "StaticMeshVertexData.h"

//It seems to be the convention to expose all vertex declarations as globals, and then reference them as externs in the headers where they are needed.
//It kind of makes sense since they do not contain any parameters that change and are purely used as their names suggest, as declarations :)
TGlobalResource<FVertexDeclarationExample> GTextureVertexDeclaration;


class FPositionVertexData :
	public TStaticMeshVertexData<FTextureVertex>
{
public:
	FPositionVertexData(bool InNeedsCPUAccess = false)
		: TStaticMeshVertexData<FTextureVertex>(InNeedsCPUAccess)
	{
	}
};


FPositionVertexData* data;


AShaderUsageExample::AShaderUsageExample()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	
	FeatureLevel = ERHIFeatureLevel::SM5;
	FColor StartColor = FColor::Red;


	ConstantParameters = FPixelShaderConstantParameters();
	ConstantParameters.StartColor = FVector4(StartColor.R / 255.0, StartColor.G / 255.0, StartColor.B / 255.0, StartColor.A / 255.0);

	VariableParameters = FPixelShaderVariableParameters();

	bMustRegenerateSRV = false;
	bIsPixelShaderExecuting = false;
	bIsUnloading = false;

	CurrentTexture = NULL;
	CurrentRenderTarget = NULL;
	TextureParameterSRV = NULL;
}

AShaderUsageExample::~AShaderUsageExample()
{
	bIsUnloading = true;
}

void AShaderUsageExample::ExecutePixelShader(UTextureRenderTarget2D* RenderTarget, UTexture2D* texture, FColor EndColor, float TextureParameterBlendFactor)
{
	check(IsInGameThread());

	if (bIsUnloading || bIsPixelShaderExecuting) //Skip this execution round if we are already executing
		return;

	if (!RenderTarget)
		return;

	bIsPixelShaderExecuting = true;


	auto InputTexture = static_cast <FTexture2DResource*> (texture->Resource)->GetTexture2DRHI();

	if (TextureParameter != InputTexture)
		bMustRegenerateSRV = true;

	//Now set our runtime parameters!
	VariableParameters.EndColor = FVector4(EndColor.R / 255.0, EndColor.G / 255.0, EndColor.B / 255.0, EndColor.A / 255.0);
	VariableParameters.TextureParameterBlendFactor = TextureParameterBlendFactor;

	CurrentRenderTarget = RenderTarget;
	TextureParameter = InputTexture;


	//This macro sends the function we declare inside to be run on the render thread. What we do is essentially just send this class and tell the render thread to run the internal render function as soon as it can.
	//I am still not 100% Certain on the thread safety of this, if you are getting crashes, depending on how advanced code you have in the start of the ExecutePixelShader function, you might have to use a lock :)
	ENQUEUE_RENDER_COMMAND(FPixelShaderRunner)(
		[&] (FRHICommandListImmediate& RHICmdList)
		{
			ExecutePixelShaderInternal(RHICmdList);
		}
	);
}

void AShaderUsageExample::ExecutePixelShaderInternal(FRHICommandListImmediate& RHICmdList)
{
	check(IsInRenderingThread());
	

	if (!CurrentRenderTarget)
		return;
	if (!CurrentRenderTarget->GetRenderTargetResource())
		return;

	if (bIsUnloading) //If we are about to unload, so just clean up the SRV :)
	{
		if (NULL != TextureParameterSRV)
		{
			TextureParameterSRV.SafeRelease();
			TextureParameterSRV = NULL;
		}

		return;
	}


	if (!vertBuf.IsValid()) {
		// Draw a fullscreen quad that we can run our pixel shader on
		TArray<FTextureVertex> Vertices;

		FTextureVertex vert;
		vert.Position = FVector(-1.0f, 1.0f, 0);
		vert.Color = FColor::Red;
		Vertices.Add(vert);

		FTextureVertex vert1;
		vert1.Position = FVector4(1.0f, 1.0f, 0, 1.0f);
		vert1.Color = FColor::Green;
		Vertices.Add(vert1);

		FTextureVertex vert2;
		vert2.Position = FVector4(-1.0f, -1.0f, 0, 1.0f);
		vert2.Color = FColor::Blue;
		Vertices.Add(vert2);


		data = new FPositionVertexData();
		data->ResizeBuffer(Vertices.Num());

		memcpy(data->GetDataPointer(), Vertices.GetData(), sizeof(FTextureVertex) * Vertices.Num());

		//vertBuf->Init(Vertices, false);
		//vertBuf->InitRHI();
		
		FResourceArrayInterface* RESTRICT ResourceArray = data->GetResourceArray();
		FRHIResourceCreateInfo info (ResourceArray);
		info.bWithoutNativeResource = false;

		//auto t = info.ResourceArray->GetResourceDataSize();

		vertBuf = RHICreateVertexBuffer(sizeof(FTextureVertex) * Vertices.Num(), BUF_Static, info);
	}

	//FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();

	//If our input texture reference has changed, we need to recreate our SRV
	if (bMustRegenerateSRV)
	{
		bMustRegenerateSRV = false;

		if (NULL != TextureParameterSRV)
		{
			TextureParameterSRV.SafeRelease();
			TextureParameterSRV = NULL;
		}

		TextureParameterSRV = RHICreateShaderResourceView(TextureParameter, 0);
	}

	// This is where the magic happens
	TShaderMapRef<FVertexShaderExample> VertexShader(GetGlobalShaderMap(FeatureLevel));
	TShaderMapRef<FPixelShaderExample> PixelShader(GetGlobalShaderMap(FeatureLevel));



	CurrentTexture = CurrentRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();

	FRHIRenderPassInfo RPInfo(CurrentTexture, ERenderTargetActions::DontLoad_Store, FTextureRHIRef());
	RHICmdList.BeginRenderPass(RPInfo, TEXT("TestTestTest")); 
	{ 
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.BlendState			= TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState		= TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState	= TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTextureVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI	= GETSAFERHISHADER_VERTEX(*VertexShader);
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI		= GETSAFERHISHADER_PIXEL(*PixelShader);
		

		PixelShader->SetOutputTexture(RHICmdList, TextureParameterSRV);
		PixelShader->SetUniformBuffers(RHICmdList, ConstantParameters, VariableParameters);

		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		RHICmdList.SetViewport(
			0, 0, 0.f,
			CurrentTexture->GetSizeX(), CurrentTexture->GetSizeY(), 1.f);

		
		RHICmdList.SetStreamSource(0, vertBuf, 0);

		RHICmdList.DrawPrimitive(0, 1, 0);
	} 
	RHICmdList.EndRenderPass();


	//DrawPrimitiveUP(RHICmdList, PT_TriangleStrip, 2, Vertices, sizeof(Vertices[0]));

	PixelShader->UnbindBuffers(RHICmdList);

	// Resolve render target.
	//RHICmdList.CopyToResolveTarget(
	//	CurrentRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture(),
	//	CurrentRenderTarget->GetRenderTargetResource()->TextureRHI,
	//	FResolveParams());


	//if (bSave) //Save to disk if we have a save request!
	//{
	//	bSave = false;
	//
	//	SaveScreenshot(RHICmdList);
	//}

	bIsPixelShaderExecuting = false;
}

void AShaderUsageExample::SaveScreenshot(FRHICommandListImmediate& RHICmdList)
{
	check(IsInRenderingThread());

	TArray<FColor> Bitmap;

	FReadSurfaceDataFlags ReadDataFlags;
	ReadDataFlags.SetLinearToGamma(false);
	ReadDataFlags.SetOutputStencil(false);
	ReadDataFlags.SetMip(0); //No mip supported ofc!

	//This is pretty straight forward. Since we are using a standard format, we can use this convenience function instead of having to lock rect.
	RHICmdList.ReadSurfaceData(CurrentTexture, FIntRect(0, 0, CurrentTexture->GetSizeX(), CurrentTexture->GetSizeY()), Bitmap, ReadDataFlags);

	// if the format and texture type is supported
	if (Bitmap.Num())
	{
		// Create screenshot folder if not already present.
		//IFileManager::Get().MakeDirectory(*FPaths::ScreenShotDir(), true);

		const FString ScreenFileName(FPaths::ScreenShotDir() / TEXT("VisualizeTexture"));

		uint32 ExtendXWithMSAA = Bitmap.Num() / CurrentTexture->GetSizeY();

		// Save the contents of the array to a bitmap file. (24bit only so alpha channel is dropped)
		//FFileHelper::CreateBitmap(*ScreenFileName, ExtendXWithMSAA, CurrentTexture->GetSizeY(), Bitmap.GetData());

		UE_LOG(LogConsoleResponse, Display, TEXT("Content was saved to \"%s\""), *FPaths::ScreenShotDir());
	}
	else
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Failed to save BMP, format or texture type is not supported"));
	}
}