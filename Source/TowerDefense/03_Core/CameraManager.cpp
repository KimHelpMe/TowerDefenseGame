
#include "CameraManager.h"
#include "Engine/GameViewportClient.h"
#include "GridTile.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "InputCoreTypes.h"


ACameraManager::ACameraManager()
{
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootScene;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootScene);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = false;
	SpringArm->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 1500.f;

	TargetArmLength = 1500.f;

	MoveSpeed = 1000.f;
	MouseRotationSpeed = 0.15f;
	MousePitchOffset = 0.f;
	EdgeScrollThreshold = 10.f;
	ZoomSpeed = 200.f;
	MinZoom = 500.f;
	MaxZoom = 4000.f;
	MinPitch = -30.f;
	MaxPitch = -60.f;
	
}


void ACameraManager::BeginPlay()
{
	Super::BeginPlay();

    // Frame the placement area when the level has no authored player start.
    if (!UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass()))
    {
        TArray<AActor*> Tiles;
        UGameplayStatics::GetAllActorsOfClass(this, AGridTile::StaticClass(), Tiles);
        FBox Bounds(ForceInit);
        for (AActor* Tile : Tiles) Bounds += Tile->GetActorLocation();
        if (Bounds.IsValid)
        {
            SetActorLocation(Bounds.GetCenter());
            const FVector Extent = Bounds.GetExtent();
            TargetArmLength = FMath::Max(Extent.X, Extent.Y) * 2.4f;
            MaxZoom = FMath::Max(MaxZoom, TargetArmLength * 1.4f);
            SpringArm->TargetArmLength = TargetArmLength;
            SpringArm->SetRelativeRotation(FRotator(-55.f, -90.f, 0.f));
        }
    }

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->AddMappingContext(CameraIMC, 0);
		}
	}
}


void ACameraManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* PC = Cast<APlayerController>(GetController());
	UWorld* World = GetWorld();
	if (!PC || !World || !World->GetGameViewport() || World->IsPaused())
	{
		return;
	}

	FVector2D ViewportSize;
	World->GetGameViewport()->GetViewportSize(ViewportSize);

	float MouseX = 0.f;
	float MouseY = 0.f;
	const bool bRotateCamera = PC->IsInputKeyDown(EKeys::RightMouseButton);
	if (bRotateCamera)
	{
		PC->GetInputMouseDelta(MouseX, MouseY);
	}

	FRotator ArmRotation = SpringArm->GetRelativeRotation();
	if (bRotateCamera)
	{
		ArmRotation.Yaw = FRotator::NormalizeAxis(ArmRotation.Yaw + MouseX * MouseRotationSpeed);
		MousePitchOffset = FMath::Clamp(MousePitchOffset - MouseY * MouseRotationSpeed, -30.f, 30.f);
	}

	float ForwardInput = (PC->IsInputKeyDown(EKeys::W) ? 1.f : 0.f)
		- (PC->IsInputKeyDown(EKeys::S) ? 1.f : 0.f);
	float RightInput = (PC->IsInputKeyDown(EKeys::D) ? 1.f : 0.f)
		- (PC->IsInputKeyDown(EKeys::A) ? 1.f : 0.f);

	FVector2D EdgeInput = FVector2D::ZeroVector;
	float MouseXPosition = 0.f;
	float MouseYPosition = 0.f;
	const bool bMouseInViewport = PC->GetMousePosition(MouseXPosition, MouseYPosition)
		&& MouseXPosition >= 0.f && MouseYPosition >= 0.f
		&& MouseXPosition <= ViewportSize.X && MouseYPosition <= ViewportSize.Y;
	if (bMouseInViewport && !bRotateCamera)
	{
		if (MouseXPosition <= EdgeScrollThreshold)
		{
			EdgeInput.X = -1.f;
		}
		else if (MouseXPosition >= ViewportSize.X - EdgeScrollThreshold)
		{
			EdgeInput.X = 1.f;
		}

		if (MouseYPosition <= EdgeScrollThreshold)
		{
			EdgeInput.Y = 1.f;
		}
		else if (MouseYPosition >= ViewportSize.Y - EdgeScrollThreshold)
		{
			EdgeInput.Y = -1.f;
		}
	}

	const FVector2D MoveInput(RightInput + EdgeInput.X, ForwardInput + EdgeInput.Y);
	if (!MoveInput.IsNearlyZero())
	{
		const FRotationMatrix CameraYaw(FRotator(0.f, ArmRotation.Yaw, 0.f));
		const FVector MoveDirection =
			(CameraYaw.GetUnitAxis(EAxis::X) * MoveInput.Y
				+ CameraYaw.GetUnitAxis(EAxis::Y) * MoveInput.X).GetSafeNormal();
		AddActorWorldOffset(MoveDirection * MoveSpeed * DeltaTime);
	}

	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength, TargetArmLength, DeltaTime, 5.f);

	const float ZoomAlpha = FMath::GetRangePct(MinZoom, MaxZoom, SpringArm->TargetArmLength);
	const float ZoomPitch = FMath::Lerp(MinPitch, MaxPitch, ZoomAlpha);
	ArmRotation.Pitch = FMath::Clamp(ZoomPitch + MousePitchOffset, -85.f, -15.f);
	ArmRotation.Roll = 0.f;
	SpringArm->SetRelativeRotation(ArmRotation);
}


void ACameraManager::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
            if (CameraIMC) Subsystem->AddMappingContext(CameraIMC, 0);
    }
    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EIC)
	{
		// Camera panning uses the mouse at the viewport edges.
		EIC->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ACameraManager::Zoom);
	}

}

void ACameraManager::Move(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
    FVector MoveDirection = FVector(Input.X, Input.Y, 0.f) * MoveSpeed * GetWorld()->GetDeltaSeconds();
	AddActorWorldOffset(MoveDirection);

}

void ACameraManager::Zoom(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();
	TargetArmLength += Input * ZoomSpeed;
	TargetArmLength = FMath::Clamp(TargetArmLength, MinZoom, MaxZoom);
}

