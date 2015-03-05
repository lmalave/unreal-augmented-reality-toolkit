// Fill out your copyright notice in the Description page of Project Settings.
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

#pragma once

#include "GameFramework/Actor.h"
#include "IVideoSource.h"
#include "VideoDisplaySurface.generated.h"

/**
 * This is a surface (ShapePlane mesh) that can display video from a camera
 */
UCLASS()
class AVideoDisplaySurface : public AActor
{
	GENERATED_BODY()


public:
	AVideoDisplaySurface(const class FPostConstructInitializeProperties& PCIP);

	// mesh to use
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = AugmentedReality)
    TSubobjectPtr<UStaticMeshComponent> VideoSurfaceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AugmentedReality)
	float PreferredDistanceInMeters;


    UFUNCTION(BlueprintCallable, Category = AugmentedReality)
    uint16 GetWidthToDistanceRatio();
    
    UFUNCTION(BlueprintCallable, Category = AugmentedReality)
    uint16 GetHeightToDistanceRatio();

    void Init(IVideoSource* VideoSource);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AugmentedReality)
    UTexture2D* VideoTexture;

	/** Update texture region from https://wiki.unrealengine.com/Dynamic_Textures */
	void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);

	//UFUNCTION(BlueprintCallable, Category = AugmentedReality)
    //void CreateVideoTexture();

	UFUNCTION(BlueprintCallable, Category = AugmentedReality)
    void UpdateVideoFrame();

	virtual void Tick(float DeltaTime) override;
    
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitVideoMaterialTexture();

	FVector GetWorldLocationFromPixelCoordinates(FVector2D PixelCoordinates);
		
	UPROPERTY()
	TArray<FColor> VideoFrameData;

	FUpdateTextureRegion2D *VideoTextureRegion;

	UMaterialInstanceDynamic *VideoMaterial;

    IVideoSource* VideoSource;

private:
	
	
};
