// Fill out your copyright notice in the Description page of Project Settings.


#include "MDRGlobalEnvironmentManager.h"
#include "Character/MDRCharacterPlayer.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "DataTable/MDRLevelSetupTable.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utility/MDRPostProcessVolume.h"

// Sets default values
AMDRGlobalEnvironmentManager::AMDRGlobalEnvironmentManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootSceneComponent);

	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));
	DirectionalLight->SetupAttachment(RootSceneComponent);

	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(RootSceneComponent);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(RootSceneComponent);

	HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
	HeightFog->SetupAttachment(RootSceneComponent);

	VolumetricCloud = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricCloud"));
	VolumetricCloud->SetupAttachment(RootSceneComponent);

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(RootSceneComponent);
	PostProcess->Priority = 0.0f;
	PostProcess->bUnbound = true;

#if WITH_EDITOR
	SetIsSpatiallyLoaded(false);
#endif
}

// ── Snapshot helpers: 컴포넌트 → 데이터 struct (CopyCurrentSettings / CopyXxxToTarget / ExportData 공용) ──

void AMDRGlobalEnvironmentManager::SnapshotDirectionalLight(UDirectionalLightComponent* Comp, FMDRDirectionalLightData& Out)
{
	if (!Comp) return;
	Out.bVisible                              = Comp->IsVisible();
	Out.bAffectsWorld                         = Comp->bAffectsWorld;
	Out.Intensity                             = Comp->Intensity;
	Out.LightColor                            = Comp->LightColor;
	Out.IndirectLightingIntensity             = Comp->IndirectLightingIntensity;
	Out.VolumetricScatteringIntensity         = Comp->VolumetricScatteringIntensity;
	Out.Rotation                              = Comp->GetRelativeRotation();
	Out.CastShadows                           = Comp->CastShadows;
	Out.CastStaticShadows                     = Comp->CastStaticShadows;
	Out.CastDynamicShadows                    = Comp->CastDynamicShadows;
	Out.bAffectTranslucentLighting            = Comp->bAffectTranslucentLighting;
	Out.bTransmission                         = Comp->bTransmission;
	Out.bCastVolumetricShadow                 = Comp->bCastVolumetricShadow;
	Out.bCastDeepShadow                       = Comp->bCastDeepShadow;
	Out.bAffectReflection                     = Comp->bAffectReflection;
	Out.bAffectGlobalIllumination             = Comp->bAffectGlobalIllumination;
	Out.CastRaytracedShadow                   = Comp->CastRaytracedShadow;
	Out.DeepShadowLayerDistribution           = Comp->DeepShadowLayerDistribution;
	Out.SamplesPerPixel                       = Comp->SamplesPerPixel;
	Out.Temperature                           = Comp->Temperature;
	Out.bUseTemperature                       = Comp->bUseTemperature;
	Out.SpecularScale                         = Comp->SpecularScale;
	Out.DiffuseScale                          = Comp->DiffuseScale;
	Out.ShadowResolutionScale                 = Comp->ShadowResolutionScale;
	Out.ShadowBias                            = Comp->ShadowBias;
	Out.ShadowSlopeBias                       = Comp->ShadowSlopeBias;
	Out.ShadowSharpen                         = Comp->ShadowSharpen;
	Out.ContactShadowLength                   = Comp->ContactShadowLength;
	Out.ContactShadowLengthInWS               = Comp->ContactShadowLengthInWS;
	Out.ContactShadowCastingIntensity         = Comp->ContactShadowCastingIntensity;
	Out.ContactShadowNonCastingIntensity      = Comp->ContactShadowNonCastingIntensity;
	Out.CastTranslucentShadows                = Comp->CastTranslucentShadows;
	Out.bCastShadowsFromCinematicObjectsOnly  = Comp->bCastShadowsFromCinematicObjectsOnly;
	Out.bForceCachedShadowsForMovablePrimitives = Comp->bForceCachedShadowsForMovablePrimitives;
	Out.bAllowMegaLights                      = Comp->bAllowMegaLights;
	Out.MegaLightsShadowMethod                = Comp->MegaLightsShadowMethod;
	Out.LightFunctionMaterial                 = Comp->LightFunctionMaterial;
	Out.LightFunctionScale                    = Comp->LightFunctionScale;
	Out.IESTexture                            = Comp->IESTexture;
	Out.bUseIESBrightness                     = Comp->bUseIESBrightness;
	Out.IESBrightnessScale                    = Comp->IESBrightnessScale;
	Out.LightFunctionFadeDistance             = Comp->LightFunctionFadeDistance;
	Out.DisabledBrightness                    = Comp->DisabledBrightness;
	Out.bEnableLightShaftBloom                = Comp->bEnableLightShaftBloom;
	Out.BloomScale                            = Comp->BloomScale;
	Out.BloomThreshold                        = Comp->BloomThreshold;
	Out.BloomMaxBrightness                    = Comp->BloomMaxBrightness;
	Out.BloomTint                             = Comp->BloomTint;
	Out.bUseRayTracedDistanceFieldShadows     = Comp->bUseRayTracedDistanceFieldShadows;
	Out.RayStartOffsetDepthScale              = Comp->RayStartOffsetDepthScale;
	Out.ShadowCascadeBiasDistribution         = Comp->ShadowCascadeBiasDistribution;
	Out.bEnableLightShaftOcclusion            = Comp->bEnableLightShaftOcclusion;
	Out.OcclusionMaskDarkness                 = Comp->OcclusionMaskDarkness;
	Out.OcclusionDepthRange                   = Comp->OcclusionDepthRange;
	Out.LightShaftOverrideDirection           = Comp->LightShaftOverrideDirection;
	Out.DynamicShadowDistanceMovableLight     = Comp->DynamicShadowDistanceMovableLight;
	Out.DynamicShadowDistanceStationaryLight  = Comp->DynamicShadowDistanceStationaryLight;
	Out.DynamicShadowCascades                 = Comp->DynamicShadowCascades;
	Out.CascadeDistributionExponent           = Comp->CascadeDistributionExponent;
	Out.CascadeTransitionFraction             = Comp->CascadeTransitionFraction;
	Out.ShadowDistanceFadeoutFraction         = Comp->ShadowDistanceFadeoutFraction;
	Out.bUseInsetShadowsForMovableObjects     = Comp->bUseInsetShadowsForMovableObjects;
	Out.FarShadowCascadeCount                 = Comp->FarShadowCascadeCount;
	Out.FarShadowDistance                     = Comp->FarShadowDistance;
	Out.DistanceFieldShadowDistance           = Comp->DistanceFieldShadowDistance;
	Out.ForwardShadingPriority                = Comp->ForwardShadingPriority;
	Out.LightSourceAngle                      = Comp->LightSourceAngle;
	Out.LightSourceSoftAngle                  = Comp->LightSourceSoftAngle;
	Out.ShadowSourceAngleFactor               = Comp->ShadowSourceAngleFactor;
	Out.TraceDistance                         = Comp->TraceDistance;
	Out.bAtmosphereSunLight                   = Comp->bAtmosphereSunLight;
	Out.AtmosphereSunLightIndex               = Comp->AtmosphereSunLightIndex;
	Out.AtmosphereSunDiskColorScale           = Comp->AtmosphereSunDiskColorScale;
	Out.bPerPixelAtmosphereTransmittance      = Comp->bPerPixelAtmosphereTransmittance;
	Out.bCastShadowsOnClouds                  = Comp->bCastShadowsOnClouds;
	Out.bCastShadowsOnAtmosphere              = Comp->bCastShadowsOnAtmosphere;
	Out.bCastCloudShadows                     = Comp->bCastCloudShadows;
	Out.CloudShadowStrength                   = Comp->CloudShadowStrength;
	Out.CloudShadowOnAtmosphereStrength       = Comp->CloudShadowOnAtmosphereStrength;
	Out.CloudShadowOnSurfaceStrength          = Comp->CloudShadowOnSurfaceStrength;
	Out.CloudShadowDepthBias                  = Comp->CloudShadowDepthBias;
	Out.CloudShadowExtent                     = Comp->CloudShadowExtent;
	Out.CloudShadowMapResolutionScale         = Comp->CloudShadowMapResolutionScale;
	Out.CloudShadowRaySampleCountScale        = Comp->CloudShadowRaySampleCountScale;
	Out.CloudScatteredLuminanceScale          = Comp->CloudScatteredLuminanceScale;
	Out.bCastModulatedShadows                 = Comp->bCastModulatedShadows;
	Out.ModulatedShadowColor                  = Comp->ModulatedShadowColor;
	Out.ShadowAmount                          = Comp->ShadowAmount;
}

void AMDRGlobalEnvironmentManager::SnapshotSkyAtmosphere(USkyAtmosphereComponent* Comp, FMDRSkyAtmosphereData& Out)
{
	if (!Comp) return;
	Out.bVisible                                  = Comp->IsVisible();
	Out.TransformMode                             = Comp->TransformMode;
	Out.BottomRadius                              = Comp->BottomRadius;
	Out.GroundAlbedo                              = Comp->GroundAlbedo;
	Out.AtmosphereHeight                          = Comp->AtmosphereHeight;
	Out.MultiScatteringFactor                     = Comp->MultiScatteringFactor;
	Out.TraceSampleCountScale                     = Comp->TraceSampleCountScale;
	Out.RayleighScatteringScale                   = Comp->RayleighScatteringScale;
	Out.RayleighScattering                        = Comp->RayleighScattering;
	Out.RayleighExponentialDistribution           = Comp->RayleighExponentialDistribution;
	Out.MieScatteringScale                        = Comp->MieScatteringScale;
	Out.MieScattering                             = Comp->MieScattering;
	Out.MieAbsorptionScale                        = Comp->MieAbsorptionScale;
	Out.MieAbsorption                             = Comp->MieAbsorption;
	Out.MieAnisotropy                             = Comp->MieAnisotropy;
	Out.MieExponentialDistribution                = Comp->MieExponentialDistribution;
	Out.OtherAbsorptionScale                      = Comp->OtherAbsorptionScale;
	Out.OtherAbsorption                           = Comp->OtherAbsorption;
	Out.OtherTentDistribution                     = Comp->OtherTentDistribution;
	Out.SkyLuminanceFactor                        = Comp->SkyLuminanceFactor;
	Out.SkyAndAerialPerspectiveLuminanceFactor    = Comp->SkyAndAerialPerspectiveLuminanceFactor;
	Out.AerialPespectiveViewDistanceScale         = Comp->AerialPespectiveViewDistanceScale;
	Out.HeightFogContribution                     = Comp->HeightFogContribution;
	Out.TransmittanceMinLightElevationAngle       = Comp->TransmittanceMinLightElevationAngle;
	Out.AerialPerspectiveStartDepth               = Comp->AerialPerspectiveStartDepth;
	Out.bHoldout                                  = Comp->bHoldout;
	Out.bRenderInMainPass                         = Comp->bRenderInMainPass;
}

void AMDRGlobalEnvironmentManager::SnapshotSkyLight(USkyLightComponent* Comp, FMDRSkyLightData& Out)
{
	if (!Comp) return;
	Out.bVisible                                = Comp->IsVisible();
	Out.bAffectsWorld                           = Comp->bAffectsWorld;
	Out.Intensity                               = Comp->Intensity;
	Out.LightColor                              = Comp->LightColor;
	Out.IndirectLightingIntensity               = Comp->IndirectLightingIntensity;
	Out.VolumetricScatteringIntensity           = Comp->VolumetricScatteringIntensity;
	Out.CastShadows                             = Comp->CastShadows;
	Out.CastStaticShadows                       = Comp->CastStaticShadows;
	Out.CastDynamicShadows                      = Comp->CastDynamicShadows;
	Out.bAffectTranslucentLighting              = Comp->bAffectTranslucentLighting;
	Out.bTransmission                           = Comp->bTransmission;
	Out.bCastVolumetricShadow                   = Comp->bCastVolumetricShadow;
	Out.bCastDeepShadow                         = Comp->bCastDeepShadow;
	Out.CastRaytracedShadow                     = Comp->CastRaytracedShadow;
	Out.bAffectReflection                       = Comp->bAffectReflection;
	Out.bAffectGlobalIllumination               = Comp->bAffectGlobalIllumination;
	Out.DeepShadowLayerDistribution             = Comp->DeepShadowLayerDistribution;
	Out.Rotation                                = Comp->GetRelativeRotation();
	Out.SkyDistanceThreshold                    = Comp->SkyDistanceThreshold;
	Out.bCaptureEmissiveOnly                    = Comp->bCaptureEmissiveOnly;
	Out.bLowerHemisphereIsBlack                 = Comp->bLowerHemisphereIsBlack;
	Out.LowerHemisphereColor                    = Comp->LowerHemisphereColor;
	Out.OcclusionMaxDistance                    = Comp->OcclusionMaxDistance;
	Out.Contrast                                = Comp->Contrast;
	Out.OcclusionExponent                       = Comp->OcclusionExponent;
	Out.MinOcclusion                            = Comp->MinOcclusion;
	Out.OcclusionTint                           = Comp->OcclusionTint;
	Out.bCloudAmbientOcclusion                  = Comp->bCloudAmbientOcclusion;
	Out.CloudAmbientOcclusionStrength           = Comp->CloudAmbientOcclusionStrength;
	Out.CloudAmbientOcclusionExtent             = Comp->CloudAmbientOcclusionExtent;
	Out.CloudAmbientOcclusionMapResolutionScale = Comp->CloudAmbientOcclusionMapResolutionScale;
	Out.CloudAmbientOcclusionApertureScale      = Comp->CloudAmbientOcclusionApertureScale;
	Out.OcclusionCombineMode                    = Comp->OcclusionCombineMode;
}

void AMDRGlobalEnvironmentManager::SnapshotHeightFog(UExponentialHeightFogComponent* Comp, FMDRHeightFogData& Out)
{
	if (!Comp) return;
	Out.bVisible                                     = Comp->IsVisible();
	Out.FogDensity                                   = Comp->FogDensity;
	Out.FogHeightFalloff                             = Comp->FogHeightFalloff;
	Out.SecondFogDensity                             = Comp->SecondFogData.FogDensity;
	Out.SecondFogHeightFalloff                       = Comp->SecondFogData.FogHeightFalloff;
	Out.SecondFogHeightOffset                        = Comp->SecondFogData.FogHeightOffset;
	Out.FogInscatteringLuminance                     = Comp->FogInscatteringLuminance;
	Out.SkyAtmosphereAmbientContributionColorScale   = Comp->SkyAtmosphereAmbientContributionColorScale;
	Out.InscatteringColorCubemap                     = Comp->InscatteringColorCubemap;
	Out.InscatteringColorCubemapAngle                = Comp->InscatteringColorCubemapAngle;
	Out.InscatteringTextureTint                      = Comp->InscatteringTextureTint;
	Out.FullyDirectionalInscatteringColorDistance    = Comp->FullyDirectionalInscatteringColorDistance;
	Out.NonDirectionalInscatteringColorDistance      = Comp->NonDirectionalInscatteringColorDistance;
	Out.DirectionalInscatteringExponent              = Comp->DirectionalInscatteringExponent;
	Out.DirectionalInscatteringStartDistance         = Comp->DirectionalInscatteringStartDistance;
	Out.DirectionalInscatteringLuminance             = Comp->DirectionalInscatteringLuminance;
	Out.FogMaxOpacity                                = Comp->FogMaxOpacity;
	Out.StartDistance                                = Comp->StartDistance;
	Out.EndDistance                                  = Comp->EndDistance;
	Out.FogCutoffDistance                            = Comp->FogCutoffDistance;
	Out.bEnableVolumetricFog                         = Comp->bEnableVolumetricFog;
	Out.VolumetricFogScatteringDistribution          = Comp->VolumetricFogScatteringDistribution;
	Out.VolumetricFogAlbedo                          = Comp->VolumetricFogAlbedo;
	Out.VolumetricFogEmissive                        = Comp->VolumetricFogEmissive;
	Out.VolumetricFogExtinctionScale                 = Comp->VolumetricFogExtinctionScale;
	Out.VolumetricFogDistance                        = Comp->VolumetricFogDistance;
	Out.VolumetricFogStartDistance                   = Comp->VolumetricFogStartDistance;
	Out.VolumetricFogNearFadeInDistance              = Comp->VolumetricFogNearFadeInDistance;
	Out.VolumetricFogStaticLightingScatteringIntensity = Comp->VolumetricFogStaticLightingScatteringIntensity;
	Out.bOverrideLightColorsWithFogInscatteringColors = Comp->bOverrideLightColorsWithFogInscatteringColors;
	Out.bHoldout                                     = Comp->bHoldout;
	Out.bRenderInMainPass                            = Comp->bRenderInMainPass;
	Out.bVisibleInReflectionCaptures                 = Comp->bVisibleInReflectionCaptures;
	Out.bVisibleInRealTimeSkyCaptures                = Comp->bVisibleInRealTimeSkyCaptures;
}

void AMDRGlobalEnvironmentManager::SnapshotVolumetricCloud(UVolumetricCloudComponent* Comp, FMDRVolumetricCloudData& Out)
{
	if (!Comp) return;
	Out.bVisible                                          = Comp->IsVisible();
	Out.LayerBottomAltitude                               = Comp->LayerBottomAltitude;
	Out.LayerHeight                                       = Comp->LayerHeight;
	Out.TracingStartMaxDistance                           = Comp->TracingStartMaxDistance;
	Out.TracingStartDistanceFromCamera                    = Comp->TracingStartDistanceFromCamera;
	Out.TracingMaxDistanceMode                            = Comp->TracingMaxDistanceMode;
	Out.TracingMaxDistance                                = Comp->TracingMaxDistance;
	Out.PlanetRadius                                      = Comp->PlanetRadius;
	Out.GroundAlbedo                                      = Comp->GroundAlbedo;
	Out.Material                                          = Comp->Material;
	Out.bUsePerSampleAtmosphericLightTransmittance        = Comp->bUsePerSampleAtmosphericLightTransmittance;
	Out.SkyLightCloudBottomOcclusion                      = Comp->SkyLightCloudBottomOcclusion;
	Out.ViewSampleCountScale                              = Comp->ViewSampleCountScale;
	Out.ReflectionViewSampleCountScaleValue               = Comp->ReflectionViewSampleCountScaleValue;
	Out.ShadowViewSampleCountScale                        = Comp->ShadowViewSampleCountScale;
	Out.ShadowReflectionViewSampleCountScaleValue         = Comp->ShadowReflectionViewSampleCountScaleValue;
	Out.ShadowTracingDistance                             = Comp->ShadowTracingDistance;
	Out.StopTracingTransmittanceThreshold                 = Comp->StopTracingTransmittanceThreshold;
	Out.AerialPespectiveRayleighScatteringStartDistance   = Comp->AerialPespectiveRayleighScatteringStartDistance;
	Out.AerialPespectiveRayleighScatteringFadeDistance    = Comp->AerialPespectiveRayleighScatteringFadeDistance;
	Out.AerialPespectiveMieScatteringStartDistance        = Comp->AerialPespectiveMieScatteringStartDistance;
	Out.AerialPespectiveMieScatteringFadeDistance         = Comp->AerialPespectiveMieScatteringFadeDistance;
	Out.bHoldout                                          = Comp->bHoldout;
	Out.bRenderInMainPass                                 = Comp->bRenderInMainPass;
	Out.bVisibleInRealTimeSkyCaptures                     = Comp->bVisibleInRealTimeSkyCaptures;
}

// ─────────────────────────────────────────────────────────────────────────────

void AMDRGlobalEnvironmentManager::CopyCurrentSettingsToStartComponents()
{
	SnapshotDirectionalLight(DirectionalLight, StartDirectionalLightData);
	SnapshotSkyAtmosphere(SkyAtmosphere,       StartSkyAtmosphereData);
	SnapshotSkyLight(SkyLight,                 StartSkyLightData);
	SnapshotHeightFog(HeightFog,               StartHeightFogData);
	SnapshotVolumetricCloud(VolumetricCloud,   StartVolumetricCloudData);

	// PostProcess 가 꺼져있는 상태도 시작 상태로 기록해야 이전 전환 값이 남지 않음
	StartPostProcessData = FMDRPostProcessData();
	if (PostProcess)
	{
		StartPostProcessData.Settings = PostProcess->Settings;
		StartPostProcessData.bEnabled = PostProcess->bEnabled;
	}
}

void AMDRGlobalEnvironmentManager::CopySettingsToTargetComponents(const UMDREnvironmentData* EnvironmentData)
{
	// DirectionalLight 설정 복사
	TargetDirectionalLightData = EnvironmentData->DirectionalLightData;
	// SkyAtmosphere 설정 복사
	TargetSkyAtmosphereData = EnvironmentData->SkyAtmosphereData;
	// SkyLight 설정 복사
	TargetSkyLightData = EnvironmentData->SkyLightData;
	// HeightFog 설정 복사
	TargetHeightFogData = EnvironmentData->HeightFogData;
	// VolumetricCloud 설정 복사
	TargetVolumetricCloudData = EnvironmentData->VolumetricCloudData;
	// PostProcess 설정 복사 (Settings 구조체 전체)
	TargetPostProcessData = EnvironmentData->PostProcessData;
	// MPC 보간 캐시 생성
	CachedMPCInterpolationList.Empty();

	if (EnvironmentData && EnvironmentData->MPCControlList.Num() > 0)
	{
		// Target의 리스트를 기준으로 순회하여 보간 캐시 구성
		for (const FMDRControlMPC& TargetMPC : EnvironmentData->MPCControlList)
		{
			if (TargetMPC.TargetMPC)
			{
				float CurrentValue = UKismetMaterialLibrary::GetScalarParameterValue(GetWorld(), TargetMPC.TargetMPC, TargetMPC.ParameterName);
				float TargetValue = TargetMPC.ParameterValue;

				// 값이 다른 경우에만 보간 리스트에 추가
				if (!FMath::IsNearlyEqual(CurrentValue, TargetValue, 0.001f))
				{
					CachedMPCInterpolationList.Add(FMDRMPCIterpolationItem(
						TargetMPC.TargetMPC,
						TargetMPC.ParameterName,
						CurrentValue,
						TargetValue
					));
				}
			}
		}
	}
}

// Called when the game starts or when spawned
void AMDRGlobalEnvironmentManager::BeginPlay()
{
	Super::BeginPlay();

	if (bIsGlobalInstance)
	{
		check(GlobalEnvironmentManager == nullptr);
		GlobalEnvironmentManager = this;
	}
	else
	{
#if WITH_EDITOR
		if (bApplyForceMPC)
		{
			UpdateAllMPCValues();
		}

		if (bApplyForceCharacterLight)
		{
			if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
			{
				if (AMDRCharacterPlayer* Player = Cast<AMDRCharacterPlayer>(Character))
				{
					Player->ApplyPlayerLight(LevelCharLightSetting);
				}
			}
		}
#endif
	}
}

void AMDRGlobalEnvironmentManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (bIsGlobalInstance)
	{
		GlobalEnvironmentManager = nullptr;
	}
}

void AMDRGlobalEnvironmentManager::RegisterEnvironmentActor(AActor* Actor)
{
	if (Actor && !RegisteredEnvironmentActors.Contains(Actor))
	{
		RegisteredEnvironmentActors.Add(Actor);
		LastRegisteredEnvironmentActors.Add(Actor);

		if (LastLoadedEnvironmentData != nullptr)
		{
			Actor->SetActorHiddenInGame(true);
		}

		UE_LOG(LogTemp, Display, TEXT("MDRGlobalEnvironmentManager: RegisterEnvironmentActor %s"), *Actor->GetName());
	}
}

void AMDRGlobalEnvironmentManager::UnregisterEnvironmentActor(AActor* Actor)
{
	if (Actor && RegisteredEnvironmentActors.Contains(Actor))
	{
		Actor->SetActorHiddenInGame(true);
		RegisteredEnvironmentActors.Remove(Actor);

		UE_LOG(LogTemp, Display, TEXT("MDRGlobalEnvironmentManager: UnregisterEnvironmentActor %s"), *Actor->GetName());
	}
}

void AMDRGlobalEnvironmentManager::RegisterPostProcessVolume(AMDRPostProcessVolume* Volume)
{
	if (Volume && !RegisteredPostProcessVolumes.Contains(Volume))
	{
		// 등록 시 항상 비활성화: 타겟 레벨 활성화 후 캐릭터 이동(TransitionTo) 전에 등록되므로
		// 트랜지션 완료(OnTransitionComplete) 시점까지 비활성 유지
		if (LastLoadedEnvironmentData != nullptr)
		{
			Volume->bEnabled = false;
		}

		RegisteredPostProcessVolumes.Add(Volume);
		UE_LOG(LogTemp, Display, TEXT("MDRGlobalEnvironmentManager: RegisterPostProcessVolume %s"), *Volume->GetName());
	}
}

void AMDRGlobalEnvironmentManager::UnregisterPostProcessVolume(AMDRPostProcessVolume* Volume)
{
	if (Volume && RegisteredPostProcessVolumes.Contains(Volume))
	{
		Volume->bEnabled = false;
		RegisteredPostProcessVolumes.Remove(Volume);
		UE_LOG(LogTemp, Display, TEXT("MDRGlobalEnvironmentManager: UnregisterPostProcessVolume %s"), *Volume->GetName());
	}
}

bool AMDRGlobalEnvironmentManager::InterpolateDirectionalLight(float Alpha)
{
	if (!DirectionalLight)
	{
		return false;
	}

	if (!DirectionalLight->IsVisible() && bTargetDirectionalLightVisible)
	{
		DirectionalLight->SetVisibility(true);
	}

	bool bChanged = false;

	// 타겟이 꺼져있으면 페이드 아웃 (Intensity를 0으로)
	float TargetIntensity = bTargetDirectionalLightVisible ? TargetDirectionalLightData.Intensity : 0.0f;

	DirectionalLight->SetIntensity(FMath::Lerp(StartDirectionalLightData.Intensity, TargetIntensity, Alpha));
	DirectionalLight->SetLightColor(FMath::Lerp(FLinearColor(StartDirectionalLightData.LightColor), FLinearColor(TargetDirectionalLightData.LightColor), Alpha));

	// Rotation 보간
	FQuat StartQuat = StartDirectionalLightData.Rotation.Quaternion();
	FQuat TargetQuat = TargetDirectionalLightData.Rotation.Quaternion();
	FQuat NewQuat = FQuat::Slerp(StartQuat, TargetQuat, Alpha);
	DirectionalLight->SetRelativeRotation(NewQuat.Rotator());

	// Float 프로퍼티들 보간
	DirectionalLight->SetIndirectLightingIntensity(FMath::Lerp(StartDirectionalLightData.IndirectLightingIntensity,TargetDirectionalLightData.IndirectLightingIntensity,Alpha));
	DirectionalLight->SetVolumetricScatteringIntensity(FMath::Lerp(StartDirectionalLightData.VolumetricScatteringIntensity,TargetDirectionalLightData.VolumetricScatteringIntensity,Alpha));

	//DirectionalLight->DeepShadowLayerDistribution = FMath::Lerp(StartDirectionalLightData.DeepShadowLayerDistribution,TargetDirectionalLightData.DeepShadowLayerDistribution,Alpha);
	DirectionalLight->SetTemperature(FMath::Lerp(StartDirectionalLightData.Temperature,TargetDirectionalLightData.Temperature,Alpha));
	DirectionalLight->SetSpecularScale(FMath::Lerp(StartDirectionalLightData.SpecularScale,TargetDirectionalLightData.SpecularScale,Alpha));
	DirectionalLight->SetDiffuseScale(FMath::Lerp(StartDirectionalLightData.DiffuseScale,TargetDirectionalLightData.DiffuseScale,Alpha));

	//DirectionalLight->ShadowResolutionScale = FMath::Lerp(StartDirectionalLightData.ShadowResolutionScale,TargetDirectionalLightData.ShadowResolutionScale,Alpha);
	DirectionalLight->SetShadowBias(FMath::Lerp(StartDirectionalLightData.ShadowBias,TargetDirectionalLightData.ShadowBias,Alpha));
	DirectionalLight->SetShadowSlopeBias(FMath::Lerp(StartDirectionalLightData.ShadowSlopeBias,TargetDirectionalLightData.ShadowSlopeBias,Alpha));
	//DirectionalLight->ShadowSharpen = FMath::Lerp(StartDirectionalLightData.ShadowSharpen,TargetDirectionalLightData.ShadowSharpen,Alpha);
	//DirectionalLight->ContactShadowLength = FMath::Lerp(StartDirectionalLightData.ContactShadowLength,TargetDirectionalLightData.ContactShadowLength,Alpha);

	//DirectionalLight->ContactShadowNonCastingIntensity = FMath::Lerp(StartDirectionalLightData.ContactShadowNonCastingIntensity, TargetDirectionalLightData.ContactShadowNonCastingIntensity, Alpha);
	DirectionalLight->SetIESBrightnessScale(FMath::Lerp(StartDirectionalLightData.IESBrightnessScale, TargetDirectionalLightData.IESBrightnessScale, Alpha));
	DirectionalLight->SetLightFunctionFadeDistance(FMath::Lerp(StartDirectionalLightData.LightFunctionFadeDistance, TargetDirectionalLightData.LightFunctionFadeDistance, Alpha));
	DirectionalLight->SetLightFunctionDisabledBrightness(FMath::Lerp(StartDirectionalLightData.DisabledBrightness, TargetDirectionalLightData.DisabledBrightness, Alpha));
	DirectionalLight->SetBloomScale(FMath::Lerp(StartDirectionalLightData.BloomScale, TargetDirectionalLightData.BloomScale, Alpha));
	DirectionalLight->SetBloomThreshold(FMath::Lerp(StartDirectionalLightData.BloomThreshold, TargetDirectionalLightData.BloomThreshold, Alpha));
	DirectionalLight->SetBloomMaxBrightness(FMath::Lerp(StartDirectionalLightData.BloomMaxBrightness, TargetDirectionalLightData.BloomMaxBrightness, Alpha));

	//DirectionalLight->RayStartOffsetDepthScale = FMath::Lerp(StartDirectionalLightData.RayStartOffsetDepthScale, TargetDirectionalLightData.RayStartOffsetDepthScale, Alpha);

	DirectionalLight->SetShadowCascadeBiasDistribution(FMath::Lerp(StartDirectionalLightData.ShadowCascadeBiasDistribution, TargetDirectionalLightData.ShadowCascadeBiasDistribution, Alpha));
	DirectionalLight->SetOcclusionMaskDarkness(FMath::Lerp(StartDirectionalLightData.OcclusionMaskDarkness, TargetDirectionalLightData.OcclusionMaskDarkness, Alpha));
	DirectionalLight->SetOcclusionDepthRange(FMath::Lerp(StartDirectionalLightData.OcclusionDepthRange, TargetDirectionalLightData.OcclusionDepthRange, Alpha));
	DirectionalLight->SetDynamicShadowDistanceMovableLight(FMath::Lerp(StartDirectionalLightData.DynamicShadowDistanceMovableLight, TargetDirectionalLightData.DynamicShadowDistanceMovableLight, Alpha));
	DirectionalLight->SetDynamicShadowDistanceStationaryLight(FMath::Lerp(StartDirectionalLightData.DynamicShadowDistanceStationaryLight, TargetDirectionalLightData.DynamicShadowDistanceStationaryLight, Alpha));\
	DirectionalLight->SetShadowDistanceFadeoutFraction(FMath::Lerp(StartDirectionalLightData.ShadowDistanceFadeoutFraction, TargetDirectionalLightData.ShadowDistanceFadeoutFraction, Alpha));
	//DirectionalLight->DistanceFieldShadowDistance = FMath::Lerp(StartDirectionalLightData.DistanceFieldShadowDistance, TargetDirectionalLightData.DistanceFieldShadowDistance, Alpha);
	//DirectionalLight->FarShadowDistance = FMath::Lerp(StartDirectionalLightData.FarShadowDistance, TargetDirectionalLightData.FarShadowDistance, Alpha);

	DirectionalLight->SetCascadeDistributionExponent(FMath::Lerp(StartDirectionalLightData.CascadeDistributionExponent, TargetDirectionalLightData.CascadeDistributionExponent, Alpha));
	DirectionalLight->SetCascadeTransitionFraction(FMath::Lerp(StartDirectionalLightData.CascadeTransitionFraction, TargetDirectionalLightData.CascadeTransitionFraction, Alpha));


	DirectionalLight->SetLightSourceAngle(FMath::Lerp(StartDirectionalLightData.LightSourceAngle, TargetDirectionalLightData.LightSourceAngle, Alpha));
	DirectionalLight->SetLightSourceSoftAngle(FMath::Lerp(StartDirectionalLightData.LightSourceSoftAngle, TargetDirectionalLightData.LightSourceSoftAngle, Alpha));
	DirectionalLight->SetShadowSourceAngleFactor(FMath::Lerp(StartDirectionalLightData.ShadowSourceAngleFactor, TargetDirectionalLightData.ShadowSourceAngleFactor, Alpha));
	DirectionalLight->SetShadowAmount(FMath::Lerp(StartDirectionalLightData.ShadowAmount, TargetDirectionalLightData.ShadowAmount, Alpha));

	//DirectionalLight->TraceDistance = FMath::Lerp(StartDirectionalLightData.TraceDistance, TargetDirectionalLightData.TraceDistance, Alpha);

	// DirectionalLight->CloudShadowStrength = FMath::Lerp(StartDirectionalLightData.CloudShadowStrength, TargetDirectionalLightData.CloudShadowStrength, Alpha);
	// DirectionalLight->CloudShadowOnAtmosphereStrength = FMath::Lerp(StartDirectionalLightData.CloudShadowOnAtmosphereStrength, TargetDirectionalLightData.CloudShadowOnAtmosphereStrength, Alpha);
	// DirectionalLight->CloudShadowOnSurfaceStrength = FMath::Lerp(StartDirectionalLightData.CloudShadowOnSurfaceStrength, TargetDirectionalLightData.CloudShadowOnSurfaceStrength, Alpha);
	// DirectionalLight->CloudShadowDepthBias = FMath::Lerp(StartDirectionalLightData.CloudShadowDepthBias, TargetDirectionalLightData.CloudShadowDepthBias, Alpha);
	// DirectionalLight->CloudShadowExtent = FMath::Lerp(StartDirectionalLightData.CloudShadowExtent, TargetDirectionalLightData.CloudShadowExtent, Alpha);
	// DirectionalLight->CloudShadowMapResolutionScale = FMath::Lerp(StartDirectionalLightData.CloudShadowMapResolutionScale, TargetDirectionalLightData.CloudShadowMapResolutionScale, Alpha);
	// DirectionalLight->CloudShadowRaySampleCountScale = FMath::Lerp(StartDirectionalLightData.CloudShadowRaySampleCountScale, TargetDirectionalLightData.CloudShadowRaySampleCountScale, Alpha);

	DirectionalLight->SetLightFunctionScale(FMath::Lerp(StartDirectionalLightData.LightFunctionScale, TargetDirectionalLightData.LightFunctionScale, Alpha));
	DirectionalLight->SetLightShaftOverrideDirection(FMath::Lerp(StartDirectionalLightData.LightShaftOverrideDirection, TargetDirectionalLightData.LightShaftOverrideDirection, Alpha));
	DirectionalLight->SetAtmosphereSunDiskColorScale(FMath::Lerp(FLinearColor(StartDirectionalLightData.AtmosphereSunDiskColorScale), FLinearColor(TargetDirectionalLightData.AtmosphereSunDiskColorScale), Alpha));

	//DirectionalLight->CloudScatteredLuminanceScale = FLinearColor::LerpUsingHSV(StartDirectionalLightData.CloudScatteredLuminanceScale, TargetDirectionalLightData.CloudScatteredLuminanceScale, Alpha);

	DirectionalLight->SetBloomTint(FMath::Lerp(FLinearColor(StartDirectionalLightData.BloomTint), FLinearColor(TargetDirectionalLightData.BloomTint), Alpha).ToFColor(true));
	//DirectionalLight->ModulatedShadowColor = FLinearColor::LerpUsingHSV(FLinearColor(StartDirectionalLightData.ModulatedShadowColor), FLinearColor(TargetDirectionalLightData.ModulatedShadowColor), Alpha).ToFColor(true);

	// Boolean 프로퍼티들은 임계값 이후에 바로 교체
	if (Alpha > NonInterpolableThresholdTime)
	{
		if (DirectionalLight->bAffectsWorld != TargetDirectionalLightData.bAffectsWorld)
		{
			DirectionalLight->bAffectsWorld = TargetDirectionalLightData.bAffectsWorld;
			bChanged = true;
		}
		if (DirectionalLight->CastShadows != TargetDirectionalLightData.CastShadows)
		{
			DirectionalLight->CastShadows = TargetDirectionalLightData.CastShadows;
			bChanged = true;
		}
		if (DirectionalLight->CastStaticShadows != TargetDirectionalLightData.CastStaticShadows)
		{
			DirectionalLight->CastStaticShadows = TargetDirectionalLightData.CastStaticShadows;
			bChanged = true;
		}
		if (DirectionalLight->CastDynamicShadows != TargetDirectionalLightData.CastDynamicShadows)
		{
			DirectionalLight->CastDynamicShadows = TargetDirectionalLightData.CastDynamicShadows;
			bChanged = true;
		}
		if (DirectionalLight->bAffectTranslucentLighting != TargetDirectionalLightData.bAffectTranslucentLighting)
		{
			DirectionalLight->bAffectTranslucentLighting = TargetDirectionalLightData.bAffectTranslucentLighting;
			bChanged = true;
		}
		if (DirectionalLight->bTransmission != TargetDirectionalLightData.bTransmission)
		{
			DirectionalLight->bTransmission = TargetDirectionalLightData.bTransmission;
			bChanged = true;
		}
		if (DirectionalLight->bCastVolumetricShadow != TargetDirectionalLightData.bCastVolumetricShadow)
		{
			DirectionalLight->bCastVolumetricShadow = TargetDirectionalLightData.bCastVolumetricShadow;
			bChanged = true;
		}
		if (DirectionalLight->bCastDeepShadow != TargetDirectionalLightData.bCastDeepShadow)
		{
			DirectionalLight->bCastDeepShadow = TargetDirectionalLightData.bCastDeepShadow;
			bChanged = true;
		}
		if (DirectionalLight->bAffectReflection != TargetDirectionalLightData.bAffectReflection)
		{
			DirectionalLight->bAffectReflection = TargetDirectionalLightData.bAffectReflection;
			bChanged = true;
		}
		if (DirectionalLight->bAffectGlobalIllumination != TargetDirectionalLightData.bAffectGlobalIllumination)
		{
			DirectionalLight->bAffectGlobalIllumination = TargetDirectionalLightData.bAffectGlobalIllumination;
			bChanged = true;
		}
		if (DirectionalLight->CastRaytracedShadow != TargetDirectionalLightData.CastRaytracedShadow)
		{
			DirectionalLight->CastRaytracedShadow = TargetDirectionalLightData.CastRaytracedShadow;
			bChanged = true;
		}
		if (DirectionalLight->SamplesPerPixel != TargetDirectionalLightData.SamplesPerPixel)
		{
			DirectionalLight->SamplesPerPixel = TargetDirectionalLightData.SamplesPerPixel;
			bChanged = true;
		}

		// Missing Boolean Properties
		if (DirectionalLight->bUseTemperature != TargetDirectionalLightData.bUseTemperature)
		{
			DirectionalLight->bUseTemperature = TargetDirectionalLightData.bUseTemperature;
			bChanged = true;
		}
		if (DirectionalLight->ContactShadowLengthInWS != TargetDirectionalLightData.ContactShadowLengthInWS)
		{
			DirectionalLight->ContactShadowLengthInWS = TargetDirectionalLightData.ContactShadowLengthInWS;
			bChanged = true;
		}
		if (DirectionalLight->CastTranslucentShadows != TargetDirectionalLightData.CastTranslucentShadows)
		{
			DirectionalLight->CastTranslucentShadows = TargetDirectionalLightData.CastTranslucentShadows;
			bChanged = true;
		}
		if (DirectionalLight->bCastShadowsFromCinematicObjectsOnly != TargetDirectionalLightData.bCastShadowsFromCinematicObjectsOnly)
		{
			DirectionalLight->bCastShadowsFromCinematicObjectsOnly = TargetDirectionalLightData.bCastShadowsFromCinematicObjectsOnly;
			bChanged = true;
		}
		if (DirectionalLight->bForceCachedShadowsForMovablePrimitives != TargetDirectionalLightData.bForceCachedShadowsForMovablePrimitives)
		{
			DirectionalLight->bForceCachedShadowsForMovablePrimitives = TargetDirectionalLightData.bForceCachedShadowsForMovablePrimitives;
			bChanged = true;
		}
		if (DirectionalLight->bAllowMegaLights != TargetDirectionalLightData.bAllowMegaLights)
		{
			DirectionalLight->bAllowMegaLights = TargetDirectionalLightData.bAllowMegaLights;
			bChanged = true;
		}
		if (DirectionalLight->bUseIESBrightness != TargetDirectionalLightData.bUseIESBrightness)
		{
			DirectionalLight->bUseIESBrightness = TargetDirectionalLightData.bUseIESBrightness;
			bChanged = true;
		}
		if (DirectionalLight->bEnableLightShaftBloom != TargetDirectionalLightData.bEnableLightShaftBloom)
		{
			DirectionalLight->bEnableLightShaftBloom = TargetDirectionalLightData.bEnableLightShaftBloom;
			bChanged = true;
		}
		if (DirectionalLight->bUseRayTracedDistanceFieldShadows != TargetDirectionalLightData.bUseRayTracedDistanceFieldShadows)
		{
			DirectionalLight->bUseRayTracedDistanceFieldShadows = TargetDirectionalLightData.bUseRayTracedDistanceFieldShadows;
			bChanged = true;
		}
		if (DirectionalLight->bEnableLightShaftOcclusion != TargetDirectionalLightData.bEnableLightShaftOcclusion)
		{
			DirectionalLight->bEnableLightShaftOcclusion = TargetDirectionalLightData.bEnableLightShaftOcclusion;
			bChanged = true;
		}
		if (DirectionalLight->bUseInsetShadowsForMovableObjects != TargetDirectionalLightData.bUseInsetShadowsForMovableObjects)
		{
			DirectionalLight->bUseInsetShadowsForMovableObjects = TargetDirectionalLightData.bUseInsetShadowsForMovableObjects;
			bChanged = true;
		}
		if (DirectionalLight->bAtmosphereSunLight != TargetDirectionalLightData.bAtmosphereSunLight)
		{
			DirectionalLight->bAtmosphereSunLight = TargetDirectionalLightData.bAtmosphereSunLight;
			bChanged = true;
		}
		if (DirectionalLight->bPerPixelAtmosphereTransmittance != TargetDirectionalLightData.bPerPixelAtmosphereTransmittance)
		{
			DirectionalLight->bPerPixelAtmosphereTransmittance = TargetDirectionalLightData.bPerPixelAtmosphereTransmittance;
			bChanged = true;
		}
		if (DirectionalLight->bCastShadowsOnClouds != TargetDirectionalLightData.bCastShadowsOnClouds)
		{
			DirectionalLight->bCastShadowsOnClouds = TargetDirectionalLightData.bCastShadowsOnClouds;
			bChanged = true;
		}
		if (DirectionalLight->bCastShadowsOnAtmosphere != TargetDirectionalLightData.bCastShadowsOnAtmosphere)
		{
			DirectionalLight->bCastShadowsOnAtmosphere = TargetDirectionalLightData.bCastShadowsOnAtmosphere;
			bChanged = true;
		}
		if (DirectionalLight->bCastCloudShadows != TargetDirectionalLightData.bCastCloudShadows)
		{
			DirectionalLight->bCastCloudShadows = TargetDirectionalLightData.bCastCloudShadows;
			bChanged = true;
		}
		if (DirectionalLight->bCastModulatedShadows != TargetDirectionalLightData.bCastModulatedShadows)
		{
			DirectionalLight->bCastModulatedShadows = TargetDirectionalLightData.bCastModulatedShadows;
			bChanged = true;
		}

		// Missing Int32 Properties
		if (DirectionalLight->DynamicShadowCascades != TargetDirectionalLightData.DynamicShadowCascades)
		{
			DirectionalLight->DynamicShadowCascades = TargetDirectionalLightData.DynamicShadowCascades;
			bChanged = true;
		}
		if (DirectionalLight->FarShadowCascadeCount != TargetDirectionalLightData.FarShadowCascadeCount)
		{
			DirectionalLight->FarShadowCascadeCount = TargetDirectionalLightData.FarShadowCascadeCount;
			bChanged = true;
		}
		if (DirectionalLight->ForwardShadingPriority != TargetDirectionalLightData.ForwardShadingPriority)
		{
			DirectionalLight->ForwardShadingPriority = TargetDirectionalLightData.ForwardShadingPriority;
			bChanged = true;
		}
		if (DirectionalLight->AtmosphereSunLightIndex != TargetDirectionalLightData.AtmosphereSunLightIndex)
		{
			DirectionalLight->AtmosphereSunLightIndex = TargetDirectionalLightData.AtmosphereSunLightIndex;
			bChanged = true;
		}

		// Missing Enum Properties
		if (DirectionalLight->MegaLightsShadowMethod != TargetDirectionalLightData.MegaLightsShadowMethod)
		{
			DirectionalLight->MegaLightsShadowMethod = TargetDirectionalLightData.MegaLightsShadowMethod;
			bChanged = true;
		}

		// Missing Object Properties
		if (DirectionalLight->LightFunctionMaterial != TargetDirectionalLightData.LightFunctionMaterial)
		{
			DirectionalLight->SetLightFunctionMaterial(TargetDirectionalLightData.LightFunctionMaterial);
			bChanged = true;
		}
		if (DirectionalLight->IESTexture != TargetDirectionalLightData.IESTexture)
		{
			DirectionalLight->IESTexture = TargetDirectionalLightData.IESTexture;
			bChanged = true;
		}

		if (!bTargetDirectionalLightVisible)
		{
			DirectionalLight->SetVisibility(false);
			bChanged = true;
		}
	}

	return bChanged;
}

bool AMDRGlobalEnvironmentManager::InterpolateSkyAtmosphere(float Alpha)
{
	if (!SkyAtmosphere)
	{
		return false;
	}

	// Float 프로퍼티 보간
	SkyAtmosphere->SetBottomRadius(FMath::Lerp(StartSkyAtmosphereData.BottomRadius, TargetSkyAtmosphereData.BottomRadius, Alpha));
	SkyAtmosphere->SetAtmosphereHeight(FMath::Lerp(StartSkyAtmosphereData.AtmosphereHeight, TargetSkyAtmosphereData.AtmosphereHeight, Alpha));
	SkyAtmosphere->SetMultiScatteringFactor(FMath::Lerp(StartSkyAtmosphereData.MultiScatteringFactor, TargetSkyAtmosphereData.MultiScatteringFactor, Alpha));
	//SkyAtmosphere->TraceSampleCountScale = FMath::Lerp(StartSkyAtmosphereData.TraceSampleCountScale, TargetSkyAtmosphereData.TraceSampleCountScale, Alpha);
	SkyAtmosphere->SetRayleighScatteringScale(FMath::Lerp(StartSkyAtmosphereData.RayleighScatteringScale, TargetSkyAtmosphereData.RayleighScatteringScale, Alpha));
	SkyAtmosphere->SetRayleighExponentialDistribution(FMath::Lerp(StartSkyAtmosphereData.RayleighExponentialDistribution, TargetSkyAtmosphereData.RayleighExponentialDistribution, Alpha));
	SkyAtmosphere->SetMieScatteringScale(FMath::Lerp(StartSkyAtmosphereData.MieScatteringScale, TargetSkyAtmosphereData.MieScatteringScale, Alpha));
	SkyAtmosphere->SetMieAbsorptionScale(FMath::Lerp(StartSkyAtmosphereData.MieAbsorptionScale, TargetSkyAtmosphereData.MieAbsorptionScale, Alpha));
	SkyAtmosphere->SetMieAnisotropy(FMath::Lerp(StartSkyAtmosphereData.MieAnisotropy, TargetSkyAtmosphereData.MieAnisotropy, Alpha));
	SkyAtmosphere->SetMieExponentialDistribution(FMath::Lerp(StartSkyAtmosphereData.MieExponentialDistribution, TargetSkyAtmosphereData.MieExponentialDistribution, Alpha));
	SkyAtmosphere->SetOtherAbsorptionScale(FMath::Lerp(StartSkyAtmosphereData.OtherAbsorptionScale, TargetSkyAtmosphereData.OtherAbsorptionScale, Alpha));
	SkyAtmosphere->SetAerialPespectiveViewDistanceScale(FMath::Lerp(StartSkyAtmosphereData.AerialPespectiveViewDistanceScale, TargetSkyAtmosphereData.AerialPespectiveViewDistanceScale, Alpha));
	SkyAtmosphere->SetHeightFogContribution(FMath::Lerp(StartSkyAtmosphereData.HeightFogContribution, TargetSkyAtmosphereData.HeightFogContribution, Alpha));
	SkyAtmosphere->SetTransmittanceMinLightElevationAngle(FMath::Lerp(StartSkyAtmosphereData.TransmittanceMinLightElevationAngle, TargetSkyAtmosphereData.TransmittanceMinLightElevationAngle, Alpha));
	SkyAtmosphere->SetAerialPerspectiveStartDepth(FMath::Lerp(StartSkyAtmosphereData.AerialPerspectiveStartDepth, TargetSkyAtmosphereData.AerialPerspectiveStartDepth, Alpha));

	// Color 보간
	SkyAtmosphere->SetGroundAlbedo(FMath::Lerp(FLinearColor(StartSkyAtmosphereData.GroundAlbedo), FLinearColor(TargetSkyAtmosphereData.GroundAlbedo), Alpha).ToFColor(true));
	SkyAtmosphere->SetRayleighScattering(FMath::Lerp(StartSkyAtmosphereData.RayleighScattering, TargetSkyAtmosphereData.RayleighScattering, Alpha).ToFColor(true));
	SkyAtmosphere->SetMieScattering(FMath::Lerp(StartSkyAtmosphereData.MieScattering, TargetSkyAtmosphereData.MieScattering, Alpha).ToFColor(true));
	SkyAtmosphere->SetMieAbsorption(FMath::Lerp(StartSkyAtmosphereData.MieAbsorption, TargetSkyAtmosphereData.MieAbsorption, Alpha).ToFColor(true));
	SkyAtmosphere->SetOtherAbsorption(FMath::Lerp(StartSkyAtmosphereData.OtherAbsorption, TargetSkyAtmosphereData.OtherAbsorption, Alpha).ToFColor(true));

	SkyAtmosphere->SetSkyLuminanceFactor(FMath::Lerp(StartSkyAtmosphereData.SkyLuminanceFactor, TargetSkyAtmosphereData.SkyLuminanceFactor, Alpha));
	SkyAtmosphere->SetSkyAndAerialPerspectiveLuminanceFactor(FMath::Lerp(StartSkyAtmosphereData.SkyAndAerialPerspectiveLuminanceFactor, TargetSkyAtmosphereData.SkyAndAerialPerspectiveLuminanceFactor, Alpha));

	// Enum 프로퍼티는 임계시간 이후에 교체
	if (Alpha > NonInterpolableThresholdTime)
	{
		if (SkyAtmosphere->TransformMode != TargetSkyAtmosphereData.TransformMode)
		{
			SkyAtmosphere->TransformMode = TargetSkyAtmosphereData.TransformMode;
		}
		SkyAtmosphere->OtherTentDistribution = TargetSkyAtmosphereData.OtherTentDistribution;
		SkyAtmosphere->bHoldout = TargetSkyAtmosphereData.bHoldout;
		SkyAtmosphere->bRenderInMainPass = TargetSkyAtmosphereData.bRenderInMainPass;
	}

	if (Alpha > NonInterpolableThresholdTime && !bTargetSkyAtmosphereVisible)
	{
		SkyAtmosphere->SetVisibility(false);
		return true;
	}

	return true;
}

bool AMDRGlobalEnvironmentManager::InterpolateSkyLight(float Alpha)
{
	if (!SkyLight->IsVisible() && bTargetSkyLightVisible)
	{
		SkyLight->SetVisibility(true);
	}

	bool bChanged = false;
	// Color 보간
	SkyLight->SetLightColor(FMath::Lerp( FLinearColor(StartSkyLightData.LightColor), FLinearColor(TargetSkyLightData.LightColor), Alpha ).ToFColor(true));

	// Rotation (Quaternion 보간)
	FQuat StartQuatSky = StartSkyLightData.Rotation.Quaternion();
	FQuat TargetQuatSky = TargetSkyLightData.Rotation.Quaternion();
	FQuat NewQuatSky = FQuat::Slerp(StartQuatSky, TargetQuatSky, Alpha);
	SkyLight->SetRelativeRotation(NewQuatSky.Rotator());

	// Float 프로퍼티들
	SkyLight->SetIndirectLightingIntensity(FMath::Lerp(StartSkyLightData.IndirectLightingIntensity, TargetSkyLightData.IndirectLightingIntensity, Alpha));
	SkyLight->SetVolumetricScatteringIntensity(FMath::Lerp(StartSkyLightData.VolumetricScatteringIntensity, TargetSkyLightData.VolumetricScatteringIntensity, Alpha));
	//SkyLight->SkyDistanceThreshold = FMath::Lerp(StartSkyLightData.SkyDistanceThreshold, TargetSkyLightData.SkyDistanceThreshold, Alpha);
	//SkyLight->OcclusionMaxDistance = FMath::Lerp(StartSkyLightData.OcclusionMaxDistance, TargetSkyLightData.OcclusionMaxDistance, Alpha);
	SkyLight->SetOcclusionContrast(FMath::Lerp(StartSkyLightData.Contrast, TargetSkyLightData.Contrast, Alpha));
	SkyLight->SetOcclusionExponent(FMath::Lerp(StartSkyLightData.OcclusionExponent, TargetSkyLightData.OcclusionExponent, Alpha));
	SkyLight->SetMinOcclusion(FMath::Lerp(StartSkyLightData.MinOcclusion, TargetSkyLightData.MinOcclusion, Alpha));
	//SkyLight->CloudAmbientOcclusionStrength = FMath::Lerp(StartSkyLightData.CloudAmbientOcclusionStrength, TargetSkyLightData.CloudAmbientOcclusionStrength, Alpha);
	//SkyLight->CloudAmbientOcclusionExtent = FMath::Lerp(StartSkyLightData.CloudAmbientOcclusionExtent, TargetSkyLightData.CloudAmbientOcclusionExtent, Alpha);
	//SkyLight->CloudAmbientOcclusionMapResolutionScale = FMath::Lerp(StartSkyLightData.CloudAmbientOcclusionMapResolutionScale, TargetSkyLightData.CloudAmbientOcclusionMapResolutionScale, Alpha);
	//SkyLight->CloudAmbientOcclusionApertureScale = FMath::Lerp(StartSkyLightData.CloudAmbientOcclusionApertureScale, TargetSkyLightData.CloudAmbientOcclusionApertureScale, Alpha);

	// Color 프로퍼티들
	SkyLight->SetLowerHemisphereColor(FMath::Lerp(StartSkyLightData.LowerHemisphereColor, TargetSkyLightData.LowerHemisphereColor, Alpha));
	SkyLight->SetOcclusionTint(FMath::Lerp(FLinearColor(StartSkyLightData.OcclusionTint), FLinearColor(TargetSkyLightData.OcclusionTint), Alpha).ToFColor(true));

	float TargetIntensity = bTargetSkyLightVisible ? TargetSkyLightData.Intensity : 0.0f;

	// Intensity 보간
	float NewIntensity = FMath::Lerp(StartSkyLightData.Intensity, TargetIntensity, Alpha);
	if (!FMath::IsNearlyEqual(SkyLight->Intensity, NewIntensity))
	{
		SkyLight->SetIntensity(NewIntensity);
		bChanged = true;
	}

	// Enum/Boolean 프로퍼티는 임계시간 이후에 교체되게
	if (Alpha > NonInterpolableThresholdTime)
	{
		if (SkyLight->bCaptureEmissiveOnly != TargetSkyLightData.bCaptureEmissiveOnly)
		{
			SkyLight->bCaptureEmissiveOnly = TargetSkyLightData.bCaptureEmissiveOnly;
			bChanged = true;
		}
		if (SkyLight->bLowerHemisphereIsBlack != TargetSkyLightData.bLowerHemisphereIsBlack)
		{
			SkyLight->bLowerHemisphereIsBlack = TargetSkyLightData.bLowerHemisphereIsBlack;
			bChanged = true;
		}
		if (SkyLight->bCloudAmbientOcclusion != TargetSkyLightData.bCloudAmbientOcclusion)
		{
			SkyLight->bCloudAmbientOcclusion = TargetSkyLightData.bCloudAmbientOcclusion;
			bChanged = true;
		}
		if (SkyLight->OcclusionCombineMode != TargetSkyLightData.OcclusionCombineMode)
		{
			SkyLight->OcclusionCombineMode = TargetSkyLightData.OcclusionCombineMode;
			bChanged = true;
		}
		if (!bTargetSkyLightVisible)
		{
			SkyLight->SetVisibility(false);
			bChanged = true;
		}
	}


	return bChanged;
}

bool AMDRGlobalEnvironmentManager::InterpolateHeightFog(float Alpha)
{
	if (!HeightFog)
	{
		return false;
	}

	// Float 프로퍼티 보간
	HeightFog->SetFogDensity(FMath::Lerp(StartHeightFogData.FogDensity, TargetHeightFogData.FogDensity, Alpha));
	HeightFog->SetFogHeightFalloff(FMath::Lerp(StartHeightFogData.FogHeightFalloff, TargetHeightFogData.FogHeightFalloff, Alpha));

	// SecondFogData 보간
	HeightFog->SetSecondFogDensity(FMath::Lerp(StartHeightFogData.SecondFogDensity, TargetHeightFogData.SecondFogDensity, Alpha));
	HeightFog->SetSecondFogHeightFalloff(FMath::Lerp(StartHeightFogData.SecondFogHeightFalloff, TargetHeightFogData.SecondFogHeightFalloff, Alpha));
	HeightFog->SetSecondFogHeightOffset(FMath::Lerp(StartHeightFogData.SecondFogHeightOffset, TargetHeightFogData.SecondFogHeightOffset, Alpha));

	HeightFog->SetFogMaxOpacity(FMath::Lerp(StartHeightFogData.FogMaxOpacity, TargetHeightFogData.FogMaxOpacity, Alpha));
	HeightFog->SetStartDistance(FMath::Lerp(StartHeightFogData.StartDistance, TargetHeightFogData.StartDistance, Alpha));
	HeightFog->SetEndDistance(FMath::Lerp(StartHeightFogData.EndDistance, TargetHeightFogData.EndDistance, Alpha));
	HeightFog->SetFogCutoffDistance(FMath::Lerp(StartHeightFogData.FogCutoffDistance, TargetHeightFogData.FogCutoffDistance, Alpha));

	HeightFog->SetDirectionalInscatteringExponent(FMath::Lerp(StartHeightFogData.DirectionalInscatteringExponent, TargetHeightFogData.DirectionalInscatteringExponent, Alpha));
	HeightFog->SetDirectionalInscatteringStartDistance(FMath::Lerp(StartHeightFogData.DirectionalInscatteringStartDistance, TargetHeightFogData.DirectionalInscatteringStartDistance, Alpha));

	HeightFog->SetFullyDirectionalInscatteringColorDistance(FMath::Lerp(StartHeightFogData.FullyDirectionalInscatteringColorDistance, TargetHeightFogData.FullyDirectionalInscatteringColorDistance, Alpha));
	HeightFog->SetNonDirectionalInscatteringColorDistance(FMath::Lerp(StartHeightFogData.NonDirectionalInscatteringColorDistance, TargetHeightFogData.NonDirectionalInscatteringColorDistance, Alpha));
	HeightFog->SetInscatteringColorCubemapAngle(FMath::Lerp(StartHeightFogData.InscatteringColorCubemapAngle, TargetHeightFogData.InscatteringColorCubemapAngle, Alpha));

	HeightFog->SetVolumetricFogScatteringDistribution(FMath::Lerp(StartHeightFogData.VolumetricFogScatteringDistribution, TargetHeightFogData.VolumetricFogScatteringDistribution, Alpha));
	HeightFog->SetVolumetricFogExtinctionScale(FMath::Lerp(StartHeightFogData.VolumetricFogExtinctionScale, TargetHeightFogData.VolumetricFogExtinctionScale, Alpha));
	HeightFog->SetVolumetricFogDistance(FMath::Lerp(StartHeightFogData.VolumetricFogDistance, TargetHeightFogData.VolumetricFogDistance, Alpha));
	HeightFog->SetVolumetricFogStartDistance(FMath::Lerp(StartHeightFogData.VolumetricFogStartDistance, TargetHeightFogData.VolumetricFogStartDistance, Alpha));
	HeightFog->SetVolumetricFogNearFadeInDistance(FMath::Lerp(StartHeightFogData.VolumetricFogNearFadeInDistance, TargetHeightFogData.VolumetricFogNearFadeInDistance, Alpha));
	//HeightFog->VolumetricFogStaticLightingScatteringIntensity = FMath::Lerp(StartHeightFogData.VolumetricFogStaticLightingScatteringIntensity, TargetHeightFogData.VolumetricFogStaticLightingScatteringIntensity, Alpha);


	// Color 보간
	HeightFog->SetFogInscatteringColor(FMath::Lerp(StartHeightFogData.FogInscatteringLuminance, TargetHeightFogData.FogInscatteringLuminance, Alpha));
	HeightFog->SetSkyAtmosphereAmbientContributionColorScale(FMath::Lerp(StartHeightFogData.SkyAtmosphereAmbientContributionColorScale, TargetHeightFogData.SkyAtmosphereAmbientContributionColorScale, Alpha));
	HeightFog->SetInscatteringTextureTint(FMath::Lerp(StartHeightFogData.InscatteringTextureTint, TargetHeightFogData.InscatteringTextureTint, Alpha));
	HeightFog->SetDirectionalInscatteringColor(FMath::Lerp(StartHeightFogData.DirectionalInscatteringLuminance, TargetHeightFogData.DirectionalInscatteringLuminance, Alpha));

	// FColor 변환이 필요한 프로퍼티들
	HeightFog->SetVolumetricFogAlbedo(FMath::Lerp(FLinearColor(StartHeightFogData.VolumetricFogAlbedo), FLinearColor(TargetHeightFogData.VolumetricFogAlbedo), Alpha).ToFColor(true));
	HeightFog->SetVolumetricFogEmissive(FMath::Lerp(StartHeightFogData.VolumetricFogEmissive, TargetHeightFogData.VolumetricFogEmissive, Alpha).ToFColor(true));

	// Cubemap은 임계값 이후에 교체
	if (Alpha > NonInterpolableThresholdTime)
	{
		if (HeightFog->InscatteringColorCubemap != TargetHeightFogData.InscatteringColorCubemap)
		{
			HeightFog->InscatteringColorCubemap = TargetHeightFogData.InscatteringColorCubemap;
		}
		if (HeightFog->bEnableVolumetricFog != TargetHeightFogData.bEnableVolumetricFog)
		{
			HeightFog->bEnableVolumetricFog = TargetHeightFogData.bEnableVolumetricFog;
		}
		if (HeightFog->bOverrideLightColorsWithFogInscatteringColors != TargetHeightFogData.bOverrideLightColorsWithFogInscatteringColors)
		{
			HeightFog->bOverrideLightColorsWithFogInscatteringColors = TargetHeightFogData.bOverrideLightColorsWithFogInscatteringColors;
		}
		if (HeightFog->bHoldout != TargetHeightFogData.bHoldout)
		{
			HeightFog->bHoldout = TargetHeightFogData.bHoldout;
		}
		if (HeightFog->bRenderInMainPass != TargetHeightFogData.bRenderInMainPass)
		{
			HeightFog->bRenderInMainPass = TargetHeightFogData.bRenderInMainPass;
		}
		if (HeightFog->bVisibleInReflectionCaptures != TargetHeightFogData.bVisibleInReflectionCaptures)
		{
			HeightFog->bVisibleInReflectionCaptures = TargetHeightFogData.bVisibleInReflectionCaptures;
		}
		if (HeightFog->bVisibleInRealTimeSkyCaptures != TargetHeightFogData.bVisibleInRealTimeSkyCaptures)
		{
			HeightFog->bVisibleInRealTimeSkyCaptures = TargetHeightFogData.bVisibleInRealTimeSkyCaptures;
		}
	}

	// 마찬가지로 임계시간후에 이동 레벨의 타켓이 꺼져있는 경우에는 꺼주기
	if (Alpha > NonInterpolableThresholdTime && !bTargetHeightFogVisible)
	{
		HeightFog->SetVisibility(false);
		return true;
	}

	return true;
}

bool AMDRGlobalEnvironmentManager::InterpolateVolumetricCloud(float Alpha)
{
	if (!VolumetricCloud)
	{
		return false;
	}

	if (!VolumetricCloud->IsVisible())
	{
		return false;
	}

	// Float 프로퍼티 보간
	VolumetricCloud->SetLayerBottomAltitude(FMath::Lerp(StartVolumetricCloudData.LayerBottomAltitude, TargetVolumetricCloudData.LayerBottomAltitude, Alpha));
	VolumetricCloud->SetLayerHeight(FMath::Lerp(StartVolumetricCloudData.LayerHeight, TargetVolumetricCloudData.LayerHeight, Alpha));
	VolumetricCloud->SetTracingStartMaxDistance(FMath::Lerp(StartVolumetricCloudData.TracingStartMaxDistance, TargetVolumetricCloudData.TracingStartMaxDistance, Alpha));
	VolumetricCloud->SetTracingStartDistanceFromCamera(FMath::Lerp(StartVolumetricCloudData.TracingStartDistanceFromCamera, TargetVolumetricCloudData.TracingStartDistanceFromCamera, Alpha));
	VolumetricCloud->SetTracingMaxDistance(FMath::Lerp(StartVolumetricCloudData.TracingMaxDistance, TargetVolumetricCloudData.TracingMaxDistance, Alpha));
	VolumetricCloud->SetPlanetRadius(FMath::Lerp(StartVolumetricCloudData.PlanetRadius, TargetVolumetricCloudData.PlanetRadius, Alpha));
	VolumetricCloud->SetSkyLightCloudBottomOcclusion(FMath::Lerp(StartVolumetricCloudData.SkyLightCloudBottomOcclusion, TargetVolumetricCloudData.SkyLightCloudBottomOcclusion, Alpha));
	VolumetricCloud->SetViewSampleCountScale(FMath::Lerp(StartVolumetricCloudData.ViewSampleCountScale, TargetVolumetricCloudData.ViewSampleCountScale, Alpha));
	VolumetricCloud->SetReflectionViewSampleCountScale(FMath::Lerp(StartVolumetricCloudData.ReflectionViewSampleCountScaleValue, TargetVolumetricCloudData.ReflectionViewSampleCountScaleValue, Alpha));
	VolumetricCloud->SetShadowViewSampleCountScale(FMath::Lerp(StartVolumetricCloudData.ShadowViewSampleCountScale, TargetVolumetricCloudData.ShadowViewSampleCountScale, Alpha));
	VolumetricCloud->SetShadowReflectionViewSampleCountScale(FMath::Lerp(StartVolumetricCloudData.ShadowReflectionViewSampleCountScaleValue, TargetVolumetricCloudData.ShadowReflectionViewSampleCountScaleValue, Alpha));
	VolumetricCloud->SetShadowTracingDistance(FMath::Lerp(StartVolumetricCloudData.ShadowTracingDistance, TargetVolumetricCloudData.ShadowTracingDistance, Alpha));
	VolumetricCloud->SetStopTracingTransmittanceThreshold(FMath::Lerp(StartVolumetricCloudData.StopTracingTransmittanceThreshold, TargetVolumetricCloudData.StopTracingTransmittanceThreshold, Alpha));

	// VolumetricCloud->AerialPespectiveRayleighScatteringStartDistance = FMath::Lerp(StartVolumetricCloudData.AerialPespectiveRayleighScatteringStartDistance, TargetVolumetricCloudData.AerialPespectiveRayleighScatteringStartDistance, Alpha);
	// VolumetricCloud->AerialPespectiveRayleighScatteringFadeDistance = FMath::Lerp(StartVolumetricCloudData.AerialPespectiveRayleighScatteringFadeDistance, TargetVolumetricCloudData.AerialPespectiveRayleighScatteringFadeDistance, Alpha);
	// VolumetricCloud->AerialPespectiveMieScatteringStartDistance = FMath::Lerp(StartVolumetricCloudData.AerialPespectiveMieScatteringStartDistance, TargetVolumetricCloudData.AerialPespectiveMieScatteringStartDistance, Alpha);
	// VolumetricCloud->AerialPespectiveMieScatteringFadeDistance = FMath::Lerp(StartVolumetricCloudData.AerialPespectiveMieScatteringFadeDistance, TargetVolumetricCloudData.AerialPespectiveMieScatteringFadeDistance, Alpha);

	// Color 보간
	VolumetricCloud->SetGroundAlbedo(FMath::Lerp(FLinearColor(StartVolumetricCloudData.GroundAlbedo),FLinearColor(TargetVolumetricCloudData.GroundAlbedo),Alpha).ToFColor(true));

	// Enum/Boolean 프로퍼티는 임계시간 이후에 교체
	if (Alpha > NonInterpolableThresholdTime)
	{
		if (VolumetricCloud->TracingMaxDistanceMode != TargetVolumetricCloudData.TracingMaxDistanceMode)
		{
			VolumetricCloud->TracingMaxDistanceMode = TargetVolumetricCloudData.TracingMaxDistanceMode;
		}
		if (VolumetricCloud->Material != TargetVolumetricCloudData.Material)
		{
			VolumetricCloud->Material = TargetVolumetricCloudData.Material;
		}
		if (VolumetricCloud->bUsePerSampleAtmosphericLightTransmittance != TargetVolumetricCloudData.bUsePerSampleAtmosphericLightTransmittance)
		{
			VolumetricCloud->bUsePerSampleAtmosphericLightTransmittance = TargetVolumetricCloudData.bUsePerSampleAtmosphericLightTransmittance;
		}
		if (VolumetricCloud->bHoldout != TargetVolumetricCloudData.bHoldout)
		{
			VolumetricCloud->bHoldout = TargetVolumetricCloudData.bHoldout;
		}
		if (VolumetricCloud->bRenderInMainPass != TargetVolumetricCloudData.bRenderInMainPass)
		{
			VolumetricCloud->bRenderInMainPass = TargetVolumetricCloudData.bRenderInMainPass;
		}
		if (VolumetricCloud->bVisibleInRealTimeSkyCaptures != TargetVolumetricCloudData.bVisibleInRealTimeSkyCaptures)
		{
			VolumetricCloud->bVisibleInRealTimeSkyCaptures = TargetVolumetricCloudData.bVisibleInRealTimeSkyCaptures;
		}
	}

	if (Alpha > NonInterpolableThresholdTime && !bTargetVolumetricCloudVisible)
	{
		VolumetricCloud->SetVisibility(false);
		return true;
	}

	return true;
}

void AMDRGlobalEnvironmentManager::InterpolateMPC(float Alpha)
{
	if (CachedMPCInterpolationList.Num() == 0)
	{
		return;
	}

	for (const FMDRMPCIterpolationItem& Item : CachedMPCInterpolationList)
	{
		if (Item.TargetMPC)
		{
			float InterpolatedValue = FMath::Lerp(Item.StartValue, Item.TargetValue, Alpha);
			UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), Item.TargetMPC, Item.ParameterName, InterpolatedValue);
		}
	}
}

void AMDRGlobalEnvironmentManager::UpdateAllMPCValues()
{
	for (const FMDRControlMPC& ControlItem : MPCControlList)
	{
		if (ControlItem.TargetMPC && !ControlItem.ParameterName.IsNone())
		{
			UKismetMaterialLibrary::SetScalarParameterValue(
				GetWorld(),
				ControlItem.TargetMPC,
				ControlItem.ParameterName,
				ControlItem.ParameterValue
			);
		}
	}
}

void AMDRGlobalEnvironmentManager::ReadyToTransition()
{
	bIsTransitioning = false;
	LastRegisteredEnvironmentActors.Empty();
}

void AMDRGlobalEnvironmentManager::TransitionToWithDataAsset(UMDREnvironmentData* DataAsset, float InTransitionDuration = 1.0f)
{
	if (!DataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("MDRGlobalEnvironmentManager: DataAsset is null"));
		return;
	}
	if (!bIsGlobalInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("MDRGlobalEnvironmentManager: StartTransitionTo called on non-global instance"));
		return;
	}

	bIsTransitioningPostProcess = true;

	ProcessRegisteredEnvironmentActors();

	CopyCurrentSettingsToStartComponents();
	CopySettingsToTargetComponents(DataAsset);

	SyncTargetVisibilityFlags();
	ApplyComponentVisibilities();
	SetupPostProcessForTransition();

	// 트랜지션 중 레벨 내 MDRPostProcessVolume 이 글로벌 PP 를 덮어쓰지 않도록 비활성화
	ProcessRegisteredPostProcessVolumes(false);

	BeginTransitionState(InTransitionDuration);
	LastLoadedEnvironmentData = DataAsset;
}

void AMDRGlobalEnvironmentManager::TransitionToWithVolume(AMDRPostProcessVolume* SourceVolume, float InTransitionDuration)
{
	if (!SourceVolume || !bIsGlobalInstance) return;

	bIsTransitioningPostProcess = false;

	CopyCurrentSettingsToStartComponents();
	SetupTargetFromVolume(SourceVolume);

	SyncTargetVisibilityFlags();
	ApplyComponentVisibilities();
	// Volume 에서 처리되는건 PostProcess 관련 처리는 해주지 않는다.
	// 마찬가지로 레벨내 MDRPostProcessVolume 관련 Enable 처리도 해주지 않는다.

	BeginTransitionState(InTransitionDuration);
	// LastLoadedEnvironmentData는 DataAsset 으로 인한 처리에서만 — 볼륨 오버라이드는 변경 안 함
}

void AMDRGlobalEnvironmentManager::ApplyEnvironmentImmediately(UMDREnvironmentData* DataAsset, bool bForce)
{
	if (!DataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("MDRGlobalEnvironmentManager: ApplyEnvironmentImmediate - DataAsset is null"));
		return;
	}
	if (!bIsGlobalInstance && !bForce)
	{
		UE_LOG(LogTemp, Warning, TEXT("MDRGlobalEnvironmentManager: ApplyEnvironmentImmediate - called on non-global instance"));
		return;
	}

	ProcessRegisteredEnvironmentActors();

	// DataAsset 경로는 항상 PP 포함
    bIsTransitioningPostProcess = true;
	
	CopyCurrentSettingsToStartComponents();
	CopySettingsToTargetComponents(DataAsset);

	// ApplyAllComponentsImmediately() 내부의 OnTransitionComplete()에서 LastLoadedEnvironmentData를 사용하므로
	// 즉시 적용 전에 먼저 갱신해야 캐릭터 라이트도 새 레벨 데이터로 적용됨
	LastLoadedEnvironmentData = DataAsset;

	SyncTargetVisibilityFlags();
	ApplyComponentVisibilities();
	ApplyAllComponentsImmediately();
}

// Called every frame
void AMDRGlobalEnvironmentManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsGlobalInstance || (!bIsTransitioning))
		return;

	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (bIsTransitioning)
	{
		float ElapsedTime = CurrentTime - TransitionStartTime;
		TransitionProgress = FMath::Clamp(ElapsedTime / TransitionDuration, 0.0f, 1.0f);

		float Alpha = 0.0f;
		if (InterpPosCurve)
		{
			Alpha = InterpPosCurve->GetFloatValue(TransitionProgress);
		}
		else
		{
			Alpha = FMath::SmoothStep(0.1f, 1.0f, TransitionProgress);
		}

		if (InterpolateDirectionalLight(Alpha))
			DirectionalLight->MarkRenderStateDirty();

		if (InterpolateSkyAtmosphere(Alpha))
			SkyAtmosphere->MarkRenderStateDirty();

		if (InterpolateHeightFog(Alpha))
			HeightFog->MarkRenderStateDirty();

		if (InterpolateVolumetricCloud(Alpha))
			VolumetricCloud->MarkRenderStateDirty();

		if (InterpolateSkyLight(Alpha))
			SkyLight->MarkRenderStateDirty();

		// MPC Control Interpolate
		InterpolateMPC(Alpha);

		if (bIsTransitioningPostProcess && PostProcess)
		{
			// 엔진 빌트인 PP Lerp - 비선형 결합 없이 정의된 알고리즘으로 보간
			FPostProcessSettingsCollection Collection;
			Collection.OverrideChanged(StartPostProcessData.Settings);
			Collection.LerpAll(TargetPostProcessData.Settings, Alpha);

			PostProcess->Settings = Collection.Get();

			// LerpAll 결과 위에 매니저 강제 오버라이드 재주입
			PostProcess->Settings.bOverride_AutoExposureSpeedUp                 = true;
			PostProcess->Settings.bOverride_AutoExposureSpeedDown               = true;
			PostProcess->Settings.AutoExposureSpeedUp                           = AutoExposureSpeedUp;
			PostProcess->Settings.AutoExposureSpeedDown                         = AutoExposureSpeedDown;
			PostProcess->Settings.bOverride_LumenFinalGatherLightingUpdateSpeed = true;
			PostProcess->Settings.bOverride_LumenSceneLightingUpdateSpeed       = true;
			PostProcess->Settings.LumenFinalGatherLightingUpdateSpeed           = LumenFinalGatherLightingUpdateSpeed;
			PostProcess->Settings.LumenSceneLightingUpdateSpeed                 = LumenSceneLightingUpdateSpeed;

			// BlendWeight 페이드 (한쪽이 disabled여도 자연스럽게 처리)
			const float BWStart  = StartPostProcessData.bEnabled  ? 1.0f : 0.0f;
			const float BWTarget = TargetPostProcessData.bEnabled ? 1.0f : 0.0f;
			PostProcess->BlendWeight = FMath::Lerp(BWStart, BWTarget, Alpha);

			PostProcess->MarkRenderStateDirty();
		}

		// Transition Complete
		if (TransitionProgress >= 1.0f)
		{
			bIsTransitioning = false;
			OnTransitionComplete();
		}
	}
}

void AMDRGlobalEnvironmentManager::OnTransitionComplete()
{
	// DirectionalLight 최종값 적용
	if (DirectionalLight)
	{
		if (bTargetDirectionalLightVisible)
		{
			DirectionalLight->SetVisibility(true);
			DirectionalLight->MarkRenderStateDirty();
		}
		else
		{
			DirectionalLight->SetVisibility(false);
		}
	}

	// SkyAtmosphere 최종값 적용
	if (SkyAtmosphere)
	{
		if (bTargetSkyAtmosphereVisible)
		{
			SkyAtmosphere->SetVisibility(true);
			SkyAtmosphere->MarkRenderStateDirty();
		}
		else
		{
			SkyAtmosphere->SetVisibility(false);
		}
	}

	// HeightFog 최종값 적용
	if (HeightFog)
	{
		if (bTargetHeightFogVisible)
		{
			HeightFog->SetVisibility(true);
			HeightFog->MarkRenderStateDirty();
		}
		else
		{
			HeightFog->SetVisibility(false);
		}
	}

	// VolumetricCloud 최종값 적용
	if (VolumetricCloud)
	{
		if (bTargetVolumetricCloudVisible)
		{
			VolumetricCloud->SetVisibility(true);
			VolumetricCloud->MarkRenderStateDirty();
		}
		else
		{
			VolumetricCloud->SetVisibility(false);
		}
	}

	// SkyLight 최종값 적용
	if (SkyLight)
	{
		if (bTargetSkyLightVisible)
		{
			// RealtimeCapoture 는 true 로 됨을 강제로 조정
			if (!SkyLight->bRealTimeCapture)
			{
				SkyLight->bRealTimeCapture = true;
			}
			SkyLight->SetVisibility(true);
			SkyLight->MarkRenderStateDirty();
		}
		else
		{
			SkyLight->SetVisibility(false);
		}
	}

	if (LastLoadedEnvironmentData != nullptr)
	{
		if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
		{
			if (AMDRCharacterPlayer* Player = Cast<AMDRCharacterPlayer>(Character))
			{
				Player->ApplyPlayerLight(LastLoadedEnvironmentData->LevelCharLightSettingData);
			}
		}
	}

	if (bIsTransitioningPostProcess)
	{
		PostProcess->Settings = TargetPostProcessData.Settings;

		// 매니저 강제 오버라이드 (Lumen 속도 추가 포함 - 기존엔 OnTransitionComplete에서 누락돼있었음)
		PostProcess->Settings.bOverride_AutoExposureSpeedUp                 = true;
		PostProcess->Settings.bOverride_AutoExposureSpeedDown               = true;
		PostProcess->Settings.AutoExposureSpeedUp                           = AutoExposureSpeedUp;
		PostProcess->Settings.AutoExposureSpeedDown                         = AutoExposureSpeedDown;
		PostProcess->Settings.bOverride_LumenFinalGatherLightingUpdateSpeed = true;
		PostProcess->Settings.bOverride_LumenSceneLightingUpdateSpeed       = true;
		PostProcess->Settings.LumenFinalGatherLightingUpdateSpeed           = LumenFinalGatherLightingUpdateSpeed;
		PostProcess->Settings.LumenSceneLightingUpdateSpeed                 = LumenSceneLightingUpdateSpeed;

		PostProcess->bEnabled    = TargetPostProcessData.bEnabled;
		PostProcess->BlendWeight = TargetPostProcessData.bEnabled ? 1.0f : 0.0f;
		PostProcess->MarkRenderStateDirty();
	}

	// 트랜지션 완료 후 MDRPostProcessVolume 다시 활성화
	ProcessRegisteredPostProcessVolumes(true);

	UKismetSystemLibrary::PrintString(this, TEXT("OnTransitionComplete"), true, true, FLinearColor::Green, 2.0f);
}

void AMDRGlobalEnvironmentManager::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(InThis, Collector);

	if (GlobalEnvironmentManager)
	{
		Collector.AddReferencedObject(GlobalEnvironmentManager);
	}
}

void AMDRGlobalEnvironmentManager::TransitionGlobalEnvironment(const AMDRGlobalEnvironmentManager* LoadedData, const FMDRLevelSetupTable* TableData, float TransitionDuration/* = 1.0f*/)
{
	if (GlobalEnvironmentManager == nullptr)
	{
		return;
	}

	UMDREnvironmentData* EnvironmentData = nullptr;
	extern bool GUseEnvironmentDataAsset;
	if (GUseEnvironmentDataAsset)
	{
		if (TableData)
		{
			EnvironmentData = TableData->GlobalEnvironmentData.LoadSynchronous();
		}
	}
	else
	{
		EnvironmentData = ExportData(LoadedData);
	}

	if (EnvironmentData)
	{
		if (GlobalEnvironmentManager)
		{
			GlobalEnvironmentManager->TransitionToWithDataAsset(EnvironmentData, TransitionDuration);
		}
	}
}

void AMDRGlobalEnvironmentManager::ApplyGlobalEnvironmentImmediately(const AMDRGlobalEnvironmentManager* LoadedData, const struct FMDRLevelSetupTable* TableData)
{
	if (GlobalEnvironmentManager == nullptr)
	{
		return;
	}

	UMDREnvironmentData* EnvironmentData = nullptr;

	extern bool GUseEnvironmentDataAsset;
	if (GUseEnvironmentDataAsset)
	{
		if (TableData)
		{
			EnvironmentData = TableData->GlobalEnvironmentData.LoadSynchronous();
		}
	}
	else
	{
		EnvironmentData = ExportData(LoadedData);
	}

	if (EnvironmentData)
	{
		if (GlobalEnvironmentManager)
		{
			GlobalEnvironmentManager->ApplyEnvironmentImmediately(EnvironmentData);
		}
	}
}

void AMDRGlobalEnvironmentManager::ReadyToTransitionGlobalEnvironment()
{
	if (GlobalEnvironmentManager == nullptr)
	{
		return;
	}

	GlobalEnvironmentManager->ReadyToTransition();
}

UMDREnvironmentData* AMDRGlobalEnvironmentManager::SnapshotCurrentState()
{
	return ExportData(this);
}

void AMDRGlobalEnvironmentManager::CopyDirectionalLightToTarget(UDirectionalLightComponent* Comp)
{
	SnapshotDirectionalLight(Comp, TargetDirectionalLightData);
	TargetDirectionalLightData.bVisible      = true; // 볼륨이 이 컴포넌트를 명시적으로 오버라이드
	TargetDirectionalLightData.bAffectsWorld = true;
}

void AMDRGlobalEnvironmentManager::CopySkyAtmosphereToTarget(USkyAtmosphereComponent* Comp)
{
	SnapshotSkyAtmosphere(Comp, TargetSkyAtmosphereData);
	TargetSkyAtmosphereData.bVisible = true;
}

void AMDRGlobalEnvironmentManager::CopySkyLightToTarget(USkyLightComponent* Comp)
{
	SnapshotSkyLight(Comp, TargetSkyLightData);
	TargetSkyLightData.bVisible      = true;
	TargetSkyLightData.bAffectsWorld = true;
}

void AMDRGlobalEnvironmentManager::CopyHeightFogToTarget(UExponentialHeightFogComponent* Comp)
{
	SnapshotHeightFog(Comp, TargetHeightFogData);
	TargetHeightFogData.bVisible = true;
}

void AMDRGlobalEnvironmentManager::CopyVolumetricCloudToTarget(UVolumetricCloudComponent* Comp)
{
	SnapshotVolumetricCloud(Comp, TargetVolumetricCloudData);
	TargetVolumetricCloudData.bVisible = true;
}

void AMDRGlobalEnvironmentManager::CopyPostProcessToTarget(UPostProcessComponent* Comp)
{
	if (!Comp) return;
	TargetPostProcessData.Settings = Comp->Settings;
	TargetPostProcessData.bEnabled = true;
}

void AMDRGlobalEnvironmentManager::SyncTargetVisibilityFlags()
{
	bTargetDirectionalLightVisible = TargetDirectionalLightData.bVisible;
	bTargetSkyAtmosphereVisible    = TargetSkyAtmosphereData.bVisible;
	bTargetSkyLightVisible         = TargetSkyLightData.bVisible;
	bTargetHeightFogVisible        = TargetHeightFogData.bVisible;
	bTargetVolumetricCloudVisible  = TargetVolumetricCloudData.bVisible;
}

void AMDRGlobalEnvironmentManager::ApplyComponentVisibilities() const
{
	// 트랜지션 시작 시에는 타겟에서 켜져야 하는 컴포넌트만 미리 켠다.\
	// DirectionLight
	if (DirectionalLight && bTargetDirectionalLightVisible)
	{
		DirectionalLight->SetVisibility(true);
	}

	//
	if (SkyAtmosphere && bTargetSkyAtmosphereVisible)
	{
		SkyAtmosphere->SetVisibility(true);
	}

	// SkyLight
	if (SkyLight && bTargetSkyLightVisible)
	{
		SkyLight->SetVisibility(true);
		if (!SkyLight->bRealTimeCapture)
		{
			SkyLight->bRealTimeCapture = true;
		}
		SkyLight->MarkRenderStateDirty();
	}

	// Height Fog
	if (HeightFog && bTargetHeightFogVisible)
	{
		HeightFog->SetVisibility(true);
	}

	// VolumetricCloud
	if (VolumetricCloud && bTargetVolumetricCloudVisible)
	{
		VolumetricCloud->SetVisibility(true);
	}
}

void AMDRGlobalEnvironmentManager::SetupPostProcessForTransition() const
{
	if (!PostProcess) return;

    // 트랜지션 동안 PP 컴포넌트는 항상 활성.
    // 한쪽만 bEnabled인 경우 BlendWeight 보간으로 페이드 처리.
    PostProcess->bEnabled = StartPostProcessData.bEnabled || TargetPostProcessData.bEnabled;

    // 시작 시점 세팅 적재
    PostProcess->Settings = StartPostProcessData.Settings;

    // 매니저 강제 오버라이드 (데이터 애셋의 값 무시하고 매니저 멤버로 강제)
    PostProcess->Settings.bOverride_AutoExposureSpeedUp                 = true;
    PostProcess->Settings.bOverride_AutoExposureSpeedDown               = true;
    PostProcess->Settings.AutoExposureSpeedUp                           = AutoExposureSpeedUp;
    PostProcess->Settings.AutoExposureSpeedDown                         = AutoExposureSpeedDown;
    PostProcess->Settings.bOverride_LumenFinalGatherLightingUpdateSpeed = true;
    PostProcess->Settings.bOverride_LumenSceneLightingUpdateSpeed       = true;
    PostProcess->Settings.LumenFinalGatherLightingUpdateSpeed           = LumenFinalGatherLightingUpdateSpeed;
    PostProcess->Settings.LumenSceneLightingUpdateSpeed                 = LumenSceneLightingUpdateSpeed;

    // BlendWeight 시작값
    PostProcess->BlendWeight = StartPostProcessData.bEnabled ? 1.0f : 0.0f;

    PostProcess->MarkRenderStateDirty();
}

void AMDRGlobalEnvironmentManager::ApplyAllComponentsImmediately()
{
	if (InterpolateDirectionalLight(1.0f)) DirectionalLight->MarkRenderStateDirty();
	if (InterpolateSkyAtmosphere(1.0f))   SkyAtmosphere->MarkRenderStateDirty();
	if (InterpolateHeightFog(1.0f))       HeightFog->MarkRenderStateDirty();
	if (InterpolateVolumetricCloud(1.0f)) VolumetricCloud->MarkRenderStateDirty();
	if (InterpolateSkyLight(1.0f))        SkyLight->MarkRenderStateDirty();

	InterpolateMPC(1.0f);

	//
	OnTransitionComplete();
}

void AMDRGlobalEnvironmentManager::ProcessRegisteredEnvironmentActors()
{
	if (LastLoadedEnvironmentData == nullptr) return;

	for (int32 Index = RegisteredEnvironmentActors.Num() - 1; Index >= 0; --Index)
	{
		auto EnvironmentActor = RegisteredEnvironmentActors[Index];
		if (!EnvironmentActor.IsValid())
		{
			RegisteredEnvironmentActors.RemoveAt(Index);
			continue;
		}
		const bool bShouldShow = LastRegisteredEnvironmentActors.Contains(EnvironmentActor);
		EnvironmentActor->SetActorHiddenInGame(!bShouldShow);

		UE_LOG(LogTemp, Display, TEXT("MDRGlobalEnvironmentManager: %s environment Actor %s"),
			bShouldShow ? TEXT("show") : TEXT("Hidden"),
			*EnvironmentActor->GetName());
	}
}

void AMDRGlobalEnvironmentManager::ProcessRegisteredPostProcessVolumes(bool bEnable)
{
	for (int32 Index = RegisteredPostProcessVolumes.Num() - 1; Index >= 0; --Index)
	{
		auto PPVolume = RegisteredPostProcessVolumes[Index];
		if (!PPVolume.IsValid())
		{
			RegisteredPostProcessVolumes.RemoveAt(Index);
			continue;
		}
		PPVolume->bEnabled = bEnable;
	}
}

void AMDRGlobalEnvironmentManager::BeginTransitionState(float InDuration)
{
	bIsTransitioning    = true;
	TransitionStartTime = GetWorld()->GetTimeSeconds();
	TransitionProgress  = 0.0f;
	TransitionDuration  = FMath::Max(InDuration, KINDA_SMALL_NUMBER);
}

// ── TransitionTo(Volume) / ApplyEnvironmentImmediately(Volume) ───────────────

void AMDRGlobalEnvironmentManager::SetupTargetFromVolume(AMDRPostProcessVolume* SourceVolume)
{
	TargetDirectionalLightData  = StartDirectionalLightData;
	TargetSkyAtmosphereData     = StartSkyAtmosphereData;
	TargetSkyLightData          = StartSkyLightData;
	TargetHeightFogData         = StartHeightFogData;
	TargetVolumetricCloudData   = StartVolumetricCloudData;

	if (SourceVolume->bApplyDirectionalLight) CopyDirectionalLightToTarget(SourceVolume->OverrideDirectionalLight);
	if (SourceVolume->bApplySkyAtmosphere)    CopySkyAtmosphereToTarget(SourceVolume->OverrideSkyAtmosphere);
	if (SourceVolume->bApplySkyLight)         CopySkyLightToTarget(SourceVolume->OverrideSkyLight);
	if (SourceVolume->bApplyHeightFog)        CopyHeightFogToTarget(SourceVolume->OverrideHeightFog);
	if (SourceVolume->bApplyVolumetricCloud)  CopyVolumetricCloudToTarget(SourceVolume->OverrideVolumetricCloud);
}

#pragma region MDREnvironmentData

UMDREnvironmentData* AMDRGlobalEnvironmentManager::ExportData(const AMDRGlobalEnvironmentManager* SourceData)
{
	if (!SourceData) return nullptr;

	UMDREnvironmentData* EnvData = NewObject<UMDREnvironmentData>();

	SnapshotDirectionalLight(SourceData->DirectionalLight, EnvData->DirectionalLightData);
	SnapshotSkyAtmosphere(SourceData->SkyAtmosphere,       EnvData->SkyAtmosphereData);
	SnapshotSkyLight(SourceData->SkyLight,                 EnvData->SkyLightData);
	SnapshotHeightFog(SourceData->HeightFog,               EnvData->HeightFogData);
	SnapshotVolumetricCloud(SourceData->VolumetricCloud,   EnvData->VolumetricCloudData);

	if (auto* PostProcess = SourceData->PostProcess.Get())
	{
		EnvData->PostProcessData.Settings = PostProcess->Settings;
		EnvData->PostProcessData.bEnabled = PostProcess->bEnabled;
	}

	if (SourceData->MPCControlList.Num() > 0)
		EnvData->MPCControlList = SourceData->MPCControlList;

	EnvData->LevelCharLightSettingData = SourceData->LevelCharLightSetting;
	return EnvData;
}

#if WITH_EDITOR
void AMDRGlobalEnvironmentManager::Editor_ApplyEnvironmentImmediately(AMDRPostProcessVolume* SourceVolume, bool bForce)
{
	if (!bForce && (!SourceVolume || !bIsGlobalInstance)) return;

	CopyCurrentSettingsToStartComponents();
	SetupTargetFromVolume(SourceVolume);

	SyncTargetVisibilityFlags();
	ApplyComponentVisibilities();
	ApplyAllComponentsImmediately();
}

void AMDRGlobalEnvironmentManager::ExportCurrentValuesToDataAsset()
{
	if (!ExportTargetAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("MDRGlobalEnvironmentManager: ExportCurrentValuesToDataAsset - ExportTargetAsset 이 설정되지 않았습니다."));
		return;
	}

	UMDREnvironmentData* Asset = ExportTargetAsset;

	UMDREnvironmentData* Exported = ExportData(this);
	if (!Exported) return;

	// 현재 컴포넌트 값을 애셋에 리플렉션으로 복사
	for (TFieldIterator<FProperty> It(UMDREnvironmentData::StaticClass()); It; ++It)
	{
		FProperty* Prop = *It;
		const void* SrcPtr = Prop->ContainerPtrToValuePtr<void>(Exported);
		void* DstPtr = Prop->ContainerPtrToValuePtr<void>(Asset);
		Prop->CopyCompleteValue(DstPtr, SrcPtr);
	}

	Asset->MarkPackageDirty();
	UE_LOG(LogTemp, Display, TEXT("MDRGlobalEnvironmentManager: 현재 환경 값을 '%s' 애셋에 저장했습니다."), *Asset->GetName());
}
#endif

#pragma endregion
