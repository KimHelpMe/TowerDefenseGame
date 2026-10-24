#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "GachaWidget.h"
#include "GachaManager.generated.h"

class ABaseTower;
class ATowerSlot;
class AUpgradeItem;

UCLASS()
class TOWERDEFENSE_API AGachaManager : public AActor
{
	GENERATED_BODY()
	
public:	

	AGachaManager();

	// 전체 가중치 합계 (자동 계산용)
	UPROPERTY(EditAnywhere)
	float TotalPoolRate;

	// 뽑기 풀(등급별)
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> CommonPool;
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> RarePool;
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> UniquePool;
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> EpicPool;


	// 업그레이드 풀
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> UpgradePool;

	UPROPERTY(EditAnywhere)
	float UpgradePoolRate;

	// 등급별 확률
	UPROPERTY(EditAnywhere)
	float CommonPoolRate;

	UPROPERTY(EditAnywhere)
	float RarePoolRate;

	UPROPERTY(EditAnywhere)
	float UniquePoolRate;

	UPROPERTY(EditAnywhere)
	float EpicPoolRate = 3.f;

	// 현재 열려 있는 슬롯 참조.
	// UPROPERTY로 GC가 추적하게 해 슬롯 파괴 후 dangling 포인터가 남지 않도록 함.
	UPROPERTY(Transient)
	TObjectPtr<ATowerSlot> CurrentSlot;

	// 이번 뽑기 결과 3장.
	UPROPERTY(Transient)
	TArray<TSubclassOf<AActor>> CurrentRollResult;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsGachaOpen;

	UFUNCTION()
	TArray<FGachaCardInfo> BuildCardInfos(const TArray<TSubclassOf<AActor>>& Items);

public:

	// 뽑기 풀 중복 관련.

	// 설치된 타워 클래스 목록.
	UPROPERTY(VisibleAnywhere)
	TArray<TSubclassOf<AActor>> InstalledTowerClasses;

	// 업그레이드 아이템 선택 회수.
	UPROPERTY(VisibleAnywhere)
	TMap<TSubclassOf<AActor>, int32> UpgradeItemCounts;

	// Epic Tower.
	UPROPERTY(VisibleAnywhere)
	TArray<TSubclassOf<AActor>> PendingFusionTowers;

public:

	// 어떤 UI 클래스를 생성할지 (에디터에서 지정함).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> GachaWidgetClass;

	// 생성된 UI 인스턴스 참조.
	UPROPERTY(VisibleAnywhere)
	UGachaWidget* GachaWidget;

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

public:

	UFUNCTION()
	void OpenGacha(ATowerSlot* Slot);

	UFUNCTION(BlueprintCallable)
	void CloseGacha();

	UFUNCTION()
	void Roll();

	UFUNCTION(BlueprintCallable)
	void SelectItem(int32 Index);

	UFUNCTION(BlueprintCallable)
	void Reload();

	UFUNCTION(BlueprintCallable)
	void AbandonGacha();
};
