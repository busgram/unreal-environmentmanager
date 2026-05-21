// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/PostProcessSettingsCollection.h"
#include "GameFramework/Actor.h"
#include "Utility/MDRPostProcessUtility.h"
#include "Data/MDREnvironmentData.h"
#include "MDRGlobalEnvironmentManager.generated.h"

class UMDRSkyboxComponent;
class AMDRPostProcessVolume;
class UDirectionalLightComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UExponentialHeightFogComponent;
class UVolumetricCloudComponent;
class UPostProcessComponent;

UCLASS(meta=(PrioritizeCategories="MDR"))
class MDR_API AMDRGlobalEnvironmentManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMDRGlobalEnvironmentManager();

	// 인스턴스 타입 구분 (메인 파티션의 글로벌 인스턴스인지 여부)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR", meta=(DisplayPriority="0"))
	bool bIsGlobalInstance = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR", AdvancedDisplay, meta=(ClampMin = ".5", UIMax = "4", EditCondition = "bIsGlobalInstance == true",  EditConditionHides, DisplayPriority="0"))
	float LumenFinalGatherLightingUpdateSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR", AdvancedDisplay, meta=(ClampMin = ".5", UIMax = "4", EditCondition = "bIsGlobalInstance == true",  EditConditionHides, DisplayPriority="0"))
	float LumenSceneLightingUpdateSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR", meta=(ClampMin = "0.02", UIMax = "20.0",EditCondition = "bIsGlobalInstance == true",  EditConditionHides, DisplayPriority="0"))
	float AutoExposureSpeedUp = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR", meta=(ClampMin = "0.02", UIMax = "20.0", EditCondition = "bIsGlobalInstance == true",  EditConditionHides, DisplayPriority="0"))
	float AutoExposureSpeedDown = 8.0f;

	// 보간 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MDR", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float TransitionDuration = 2.0f;

	// Enum 이나 bool 값인경우 Transition 할 임계 시간 비율(Ratio) 값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MDR", meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float NonInterpolableThresholdTime = 0.9f;

	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | Data", meta=(DisplayPriority="0"))
	// TSoftObjectPtr<class UMDREnvironmentData> TargetEnvironmentData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | MPC Control", meta=(DisplayPriority="0"))
	TArray<FMDRControlMPC> MPCControlList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MDR | InterpolateCurveData")
	TObjectPtr<UCurveFloat> InterpPosCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | Default MPC Control", meta=(DisplayPriority="0"))
	TArray<FMDRControlMPC> DefaultMPCControlList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | Character Level Lighting Properties")
	FMDRLevelCharLightSetting LevelCharLightSetting;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | MPC Control", meta=(DisplayPriority="0"))
	bool bApplyForceMPC = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | Character Level Lighting Properties", meta=(DisplayPriority="0"))
	bool bApplyForceCharacterLight = false;

	// 현재 컴포넌트 값을 저장할 대상 애셋 (ExportCurrentValuesToDataAsset 전용)
	UPROPERTY(EditAnywhere, Category = "MDR | Data", meta=(DisplayPriority="1"))
	TObjectPtr<UMDREnvironmentData> ExportTargetAsset;

#endif

#if WITH_EDITOR
	// 현재 액터 컴포넌트 값을 ExportTargetAsset 애셋에 저장
	// Helper 용 추후 해당 액터의 값을 외부에 공유하거나 데이터로 저장할 필요가 다시 생길일을 위해서 추가
	UFUNCTION(CallInEditor, Category = "MDR | Data")
	void ExportCurrentValuesToDataAsset();

	void Editor_ApplyEnvironmentImmediately(AMDRPostProcessVolume* SourceVolume, bool bForce = false);
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | Environment")
	TObjectPtr<class USceneComponent> RootSceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | Environment")
	TObjectPtr<class UDirectionalLightComponent> DirectionalLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | Environment")
	TObjectPtr<class USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | Environment")
	TObjectPtr<class USkyLightComponent> SkyLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | Environment")
	TObjectPtr<class UExponentialHeightFogComponent> HeightFog;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | Environment")
	TObjectPtr<class UVolumetricCloudComponent> VolumetricCloud;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MDR | Environment")
	TObjectPtr<class UPostProcessComponent> PostProcess;

public:
	bool bTargetDirectionalLightVisible = false;
	bool bTargetSkyAtmosphereVisible = false;
	bool bTargetSkyLightVisible = false;
	bool bTargetHeightFogVisible = false;
	bool bTargetVolumetricCloudVisible = false;

	// DAta Asset 으로 Transition
	void TransitionToWithDataAsset(UMDREnvironmentData* DataAsset, float InTransitionDuration);
	// PostProcessVolume 의 설정값에서 Transition
	void TransitionToWithVolume(AMDRPostProcessVolume* SourceVolume, float InTransitionDuration);
	// 보간처리가 아닌 즉시 적용 처리 (Only With DataAsset)
	void ApplyEnvironmentImmediately(UMDREnvironmentData* DataAsset, bool bForce = false);

	// 현재 컴포넌트 상태를 UMDREnvironmentData 로 스냅샷
	UMDREnvironmentData* SnapshotCurrentState();

	// 보간 완료 이벤트
	void OnTransitionComplete();


public:
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

	static void TransitionGlobalEnvironment(const AMDRGlobalEnvironmentManager* LoadedData, const struct FMDRLevelSetupTable* TableData, float TransitionDuration = 1.0f);
	static void ApplyGlobalEnvironmentImmediately(const AMDRGlobalEnvironmentManager* LoadedData, const struct FMDRLevelSetupTable* TableData);
	static void ReadyToTransitionGlobalEnvironment();
	static AMDRGlobalEnvironmentManager* Get() { return GlobalEnvironmentManager; }

protected:
	static UMDREnvironmentData* ExportData(const AMDRGlobalEnvironmentManager* SourceData);
	// MainWP 참조용
	inline static TObjectPtr<class AMDRGlobalEnvironmentManager> GlobalEnvironmentManager{ nullptr };

protected:
	// 현재 보간 상태
	bool bIsTransitioning = false;
	bool bIsTransitioningPostProcess = false;
	float TransitionProgress = 0.0f;

	// 타겟 설정값을 저장할 데이터 구조체들
	UPROPERTY()
	FMDRDirectionalLightData TargetDirectionalLightData;

	UPROPERTY()
	FMDRSkyAtmosphereData TargetSkyAtmosphereData;

	UPROPERTY()
	FMDRSkyLightData TargetSkyLightData;

	UPROPERTY()
	FMDRHeightFogData TargetHeightFogData;

	UPROPERTY()
	FMDRVolumetricCloudData TargetVolumetricCloudData;

	UPROPERTY()
	FMDRPostProcessData TargetPostProcessData;

	// 보간 시작값을 저장할 데이터 구조체들
	UPROPERTY()
	FMDRDirectionalLightData StartDirectionalLightData;

	UPROPERTY()
	FMDRSkyAtmosphereData StartSkyAtmosphereData;

	UPROPERTY()
	FMDRSkyLightData StartSkyLightData;

	UPROPERTY()
	FMDRHeightFogData StartHeightFogData;

	UPROPERTY()
	FMDRVolumetricCloudData StartVolumetricCloudData;

	UPROPERTY()
	FMDRPostProcessData StartPostProcessData;

	UPROPERTY()
	TArray<FMDRMPCIterpolationItem> CachedMPCInterpolationList;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> RegisteredEnvironmentActors;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> LastRegisteredEnvironmentActors;

	UPROPERTY()
	TArray<TWeakObjectPtr<AMDRPostProcessVolume>> RegisteredPostProcessVolumes;

	UPROPERTY()
	TObjectPtr<UMDREnvironmentData> LastLoadedEnvironmentData;

	// 보간 시작 시간
	float TransitionStartTime = 0.0f;

	// 개별 컴포넌트 보간 함수들
	bool InterpolateDirectionalLight(float Alpha);
	bool InterpolateSkyAtmosphere(float Alpha);
	bool InterpolateSkyLight(float Alpha);
	bool InterpolateHeightFog(float Alpha);
	bool InterpolateVolumetricCloud(float Alpha);
	void InterpolateMPC(float Alpha);

	void CopySettingsToTargetComponents(const UMDREnvironmentData* EnvironmentData);
	void CopyCurrentSettingsToStartComponents();
	void UpdateAllMPCValues();
	void ReadyToTransition();

	// PostProcessVolume 컴포넌트 관련 처리 추가
	// Volume 에 설정한 Lighting 관련 컴포넌트 값 적용 처리
	void SetupTargetFromVolume(AMDRPostProcessVolume* SourceVolume);
	void CopyDirectionalLightToTarget(UDirectionalLightComponent* Comp);
	void CopySkyAtmosphereToTarget(USkyAtmosphereComponent* Comp);
	void CopySkyLightToTarget(USkyLightComponent* Comp);
	void CopyHeightFogToTarget(UExponentialHeightFogComponent* Comp);
	void CopyVolumetricCloudToTarget(UVolumetricCloudComponent* Comp);
	void CopyPostProcessToTarget(UPostProcessComponent* Comp);

	// 각각의 Component 의 스냅샷 처리
	static void SnapshotDirectionalLight(UDirectionalLightComponent* Comp, FMDRDirectionalLightData& Out);
	static void SnapshotSkyAtmosphere(USkyAtmosphereComponent* Comp, FMDRSkyAtmosphereData& Out);
	static void SnapshotSkyLight(USkyLightComponent* Comp, FMDRSkyLightData& Out);
	static void SnapshotHeightFog(UExponentialHeightFogComponent* Comp, FMDRHeightFogData& Out);
	static void SnapshotVolumetricCloud(UVolumetricCloudComponent* Comp, FMDRVolumetricCloudData& Out);


	void SyncTargetVisibilityFlags();
	void ApplyComponentVisibilities() const;
	void SetupPostProcessForTransition() const;
	void ApplyAllComponentsImmediately();
	void ProcessRegisteredEnvironmentActors();
	void ProcessRegisteredPostProcessVolumes(bool bEnable);
	void BeginTransitionState(float InDuration);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void RegisterEnvironmentActor(AActor* Actor);
	void UnregisterEnvironmentActor(AActor* Actor);

	void RegisterPostProcessVolume(AMDRPostProcessVolume* Volume);
	void UnregisterPostProcessVolume(AMDRPostProcessVolume* Volume);

	// 볼륨 이탈 시 복귀용으로 현재 레벨 데이터 노출
	UMDREnvironmentData* GetLastLoadedEnvironmentData() const { return LastLoadedEnvironmentData; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;
};