
#include "CameraManager.h"


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


	float X, Y;
	FVector2D ViewportSize;

	GEngine->GameViewport->GetViewportSize(ViewportSize);

	if (!GetWorld()->GetFirstPlayerController()->GetMousePosition(X, Y))
		return;

	FVector EdgeMove = FVector::ZeroVector;

	if (X <= EdgeScrollThreshold)
		EdgeMove.Y = -1.f; // 화면 왼쪽
	else if (X >= ViewportSize.X - EdgeScrollThreshold)
		EdgeMove.Y = 1.f; // 화면 오른쪽

	if (Y <= EdgeScrollThreshold)
		EdgeMove.X = 1.f;  // 화면 위
	else if (Y >= ViewportSize.Y - EdgeScrollThreshold)
		EdgeMove.X = -1.f; // 화면 아래

	if (!EdgeMove.IsZero())
		AddActorWorldOffset(EdgeMove * MoveSpeed * DeltaTime);

	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength, TargetArmLength, DeltaTime, 5.f);

	float T = FMath::GetRangePct(MinZoom, MaxZoom, SpringArm->TargetArmLength);
	float Pitch = FMath::Lerp(MinPitch, MaxPitch, T);
	SpringArm->SetRelativeRotation(FRotator(Pitch, 0.f, 0.f));
}


void ACameraManager::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EIC)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACameraManager::Move);
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
	float Input = Value.Get<float>();
	TargetArmLength += Input * ZoomSpeed * GetWorld()->GetDeltaSeconds();
	TargetArmLength = FMath::Clamp(TargetArmLength, MinZoom, MaxZoom);
}

