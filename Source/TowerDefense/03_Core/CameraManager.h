#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "CameraManager.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;

UCLASS()
class TOWERDEFENSE_API ACameraManager : public APawn
{
	GENERATED_BODY()

public:
	ACameraManager();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void UnPossessed() override;

	UPROPERTY(EditAnywhere)
	UInputMappingContext* CameraIMC;

	UPROPERTY(EditAnywhere)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere)
	UInputAction* ZoomAction;

	void Move(const FInputActionValue& Value);
	void Zoom(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere)
	USceneComponent* RootScene;

	UPROPERTY(EditAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere)
	float MoveSpeed;

	UPROPERTY(EditAnywhere, Category="Camera|Input", meta=(ClampMin="0.0"))
	float MouseRotationSpeed;

	UPROPERTY(EditAnywhere)
	float EdgeScrollThreshold;

	UPROPERTY(EditAnywhere)
	float ZoomSpeed;

	UPROPERTY(VisibleAnywhere)
	float TargetArmLength;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BeginNavigation(APlayerController* PlayerController);
	void EndNavigation();
	void RotateView(float YawDelta, float PitchDelta);
	void ZoomIn();
	void ZoomOut();
	void Dolly(float Direction);

	bool bNavigating = false;
	bool bSavedShowMouseCursor = true;
	bool bSavedMousePosition = false;
	FVector2D SavedMousePosition = FVector2D::ZeroVector;
	float NavigationSpeedScale = 1.f;
	TWeakObjectPtr<APlayerController> NavigationController;
	TMap<TWeakObjectPtr<UUserWidget>, float> SavedCursorOpacities;
};
