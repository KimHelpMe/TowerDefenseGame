
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CameraManager.generated.h"


class UInputMappingContext;
class UInputAction;

UCLASS()
class TOWERDEFENSE_API ACameraManager : public APawn
{
	GENERATED_BODY()

public:

	ACameraManager();

protected:
	
	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;


	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:

	UPROPERTY(EditAnywhere)
	UInputMappingContext* CameraIMC;

	UPROPERTY(EditAnywhere)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere)
	UInputAction* ZoomAction;

	void Move(const FInputActionValue& Value);

	void Zoom(const FInputActionValue& Value);

public:

	UPROPERTY(EditAnywhere)
	USceneComponent* RootScene;

	UPROPERTY(EditAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera;

public:

	UPROPERTY(EditAnywhere)
	float MoveSpeed;

	UPROPERTY(EditAnywhere)
	float EdgeScrollThreshold;

	UPROPERTY(EditAnywhere)
	float ZoomSpeed;

	UPROPERTY(EditAnywhere)
	float MinZoom;

	UPROPERTY(EditAnywhere)
	float MaxZoom;

	UPROPERTY(EditAnywhere)
	float MinPitch;

	UPROPERTY(EditAnywhere)
	float MaxPitch;

	UPROPERTY(VisibleAnywhere)
	float TargetArmLength;
};
