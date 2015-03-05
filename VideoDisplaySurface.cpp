/*****************************
 Copyright 2015 (c) Leonardo Malave. All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are
 permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this list of
 conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright notice, this list
 of conditions and the following disclaimer in the documentation and/or other materials
 provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY Leonardo Malave ''AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 The views and conclusions contained in the software and documentation are those of the
 authors and should not be interpreted as representing official policies, either expressed
 or implied, of Leonardo Malave.
 ********************************/

#include "OculusARPOC.h"
#include "Engine.h"
#include "VideoDisplaySurface.h"


AVideoDisplaySurface::AVideoDisplaySurface(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	VideoSurfaceMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("VideoSurfaceMesh"));
	RootComponent = VideoSurfaceMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh>ShapePlaneMesh(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Plane.Shape_Plane'"));
	static ConstructorHelpers::FObjectFinder <UMaterialInterface>LeapVideoMaterial(TEXT("Material'/Game/Materials/VideoMaterial.VideoMaterial'"));
	VideoSurfaceMesh->SetStaticMesh(ShapePlaneMesh.Object);
	VideoSurfaceMesh->SetMaterial(0, LeapVideoMaterial.Object);
	PrimaryActorTick.bCanEverTick = true;

	PreferredDistanceInMeters = 10.0;
}

//////////////////////////////////////////////////////////////////////////
// Texture

void AVideoDisplaySurface::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (Texture && Texture->Resource)
	{

		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
							);
					}
				}
				if (bFreeData)
				{
					FMemory::Free(RegionData->Regions);
					FMemory::Free(RegionData->SrcData);
				}
				delete RegionData;

			});

	}

}

void AVideoDisplaySurface::UpdateVideoFrame()
{
	uint8* DestinationImageBuffer = (uint8*)VideoFrameData.GetData();
	VideoSource->GetFrameImage(DestinationImageBuffer);
	UpdateTextureRegions(VideoTexture, (int32)0, (uint32)1, VideoTextureRegion, (uint32)(4 * VideoSource->GetVideoWidth()), (uint32)4, DestinationImageBuffer, false);

}

FVector AVideoDisplaySurface::GetWorldLocationFromPixelCoordinates(FVector2D PixelCoordinates) {
	FVector2D NormalizedCoordinates = FVector2D(PixelCoordinates.X / VideoSource->GetVideoWidth() - 0.5, PixelCoordinates.Y / VideoSource->GetVideoHeight() - 0.5);
	return this->GetActorLocation() - NormalizedCoordinates.X * this->GetActorForwardVector() * this->GetActorScale3D().X * 50 - NormalizedCoordinates.Y * this->GetActorRightVector() * this->GetActorScale3D().Y * 50;
}

void AVideoDisplaySurface::BeginPlay()
{
}

void AVideoDisplaySurface::Init(IVideoSource* videoSource)
{
    this->VideoSource = videoSource;
	VideoFrameData.Init(FColor::MakeRandomColor(), VideoSource->GetVideoWidth() * VideoSource->GetVideoHeight()); // TODO: avoid hardcoding of image height/width
	InitVideoMaterialTexture();
}

void AVideoDisplaySurface::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	VideoSource->Close();
}

void AVideoDisplaySurface::Tick(float DeltaTime)
{
	UpdateVideoFrame();
}

void AVideoDisplaySurface::InitVideoMaterialTexture()
{
    // First find the dynamic material
	TArray<UPrimitiveComponent*> Primitives;
	this->GetComponents<UPrimitiveComponent>(Primitives);
	uint16 PrimitivesCount = Primitives.Num();
	for (uint16 i = 0; i != PrimitivesCount; ++i)
	{
		UPrimitiveComponent* Primitive = Primitives[i];
		uint16 MaterialsCount = Primitive->GetNumMaterials();
		for (uint16 m = 0; m != MaterialsCount; ++m)
		{
			UMaterialInterface* Material = Primitive->GetMaterial(m);
			UTexture* Texture = nullptr;
			if (!Material)
			{
				continue;
			}

			if (Material->GetTextureParameterValue("VideoTexture", Texture))
			{
				UMaterialInstanceDynamic* DynamicMaterialInstance = Cast<UMaterialInstanceDynamic>(Material);
				if (!DynamicMaterialInstance)
				{
					DynamicMaterialInstance = UMaterialInstanceDynamic::Create(Material, Primitive);
					Primitive->SetMaterial(m, DynamicMaterialInstance);
				}
                VideoMaterial = DynamicMaterialInstance;
                // now that we have dynamic video material, create transient texture to draw video to, and set the material VideoTexture parameter
                VideoTexture = UTexture2D::CreateTransient(VideoSource->GetVideoWidth(), VideoSource->GetVideoHeight());
                VideoTexture->UpdateResource();
                VideoMaterial->SetTextureParameterValue(FName("VideoTexture"), VideoTexture);
                VideoTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, VideoSource->GetVideoWidth(), VideoSource->GetVideoHeight());  // Note: This never gets freed
                break;
			}
		}
	}
}

uint16 AVideoDisplaySurface::GetWidthToDistanceRatio() {
    return VideoSource->GetWidthToDistanceRatio();
}

uint16 AVideoDisplaySurface::GetHeightToDistanceRatio() {
    return VideoSource->GetHeightToDistanceRatio();
}

