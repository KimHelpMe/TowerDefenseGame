#include "CameraManager.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/PlayerStart.h"
#include "GridTile.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

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

	SpringArm->bUsePawnControlRotation = false;
	// The boom can ignore parent yaw while its component still inherits it.
	// Keep its rendered rotation in world space so rotated Pawn spawns cannot
	// turn the movement basis by 90 degrees relative to the visible camera.
	const FRotator InitialViewRotation = SpringArm->GetTargetRotation();
	SpringArm->SetAbsolute(SpringArm->IsUsingAbsoluteLocation(), true, SpringArm->IsUsingAbsoluteScale());
	SpringArm->SetWorldRotation(InitialViewRotation);
	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;
	Camera->bUsePawnControlRotation = false;

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
			SpringArm->TargetArmLength = FMath::Max(Extent.X, Extent.Y) * 2.4f;
			SpringArm->SetWorldRotation(FRotator(-55.f, -90.f, 0.f));
		}
	}
	TargetArmLength = SpringArm->TargetArmLength;
}

void ACameraManager::BeginNavigation(APlayerController* PC)
{
	if (bNavigating || !PC) return;
	bNavigating = true;
	NavigationController = PC;
	bSavedShowMouseCursor = PC->bShowMouseCursor;
	bSavedMousePosition = PC->GetMousePosition(SavedMousePosition.X, SavedMousePosition.Y);
	SavedCursorOpacities.Reset();

	TArray<UUserWidget*> Widgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Widgets, UUserWidget::StaticClass(), false);
	for (UUserWidget* Widget : Widgets)
	{
		if (Widget->GetClass()->GetName().Contains(TEXT("WBP_Cursor")))
		{
			SavedCursorOpacities.Add(Widget, Widget->GetRenderOpacity());
			Widget->SetRenderOpacity(0.f);
		}
	}

	PC->bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(false);
	PC->SetInputMode(InputMode);
}

void ACameraManager::EndNavigation()
{
	if (!bNavigating) return;
	bNavigating = false;
	for (const auto& Entry : SavedCursorOpacities)
	{
		if (UUserWidget* Widget = Entry.Key.Get()) Widget->SetRenderOpacity(Entry.Value);
	}
	SavedCursorOpacities.Reset();

	if (APlayerController* PC = NavigationController.Get())
	{
		PC->bShowMouseCursor = bSavedShowMouseCursor;
		if (GetWorld() && !GetWorld()->bIsTearingDown)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			if (bSavedMousePosition)
			{
				PC->SetMouseLocation(FMath::RoundToInt(SavedMousePosition.X), FMath::RoundToInt(SavedMousePosition.Y));
			}
		}
	}
	NavigationController.Reset();
}

void ACameraManager::RotateView(float YawDelta, float PitchDelta)
{
	const FRotator OldRotation = SpringArm->GetTargetRotation();
	FRotator NewRotation = OldRotation;
	NewRotation.Yaw = FRotator::NormalizeAxis(NewRotation.Yaw + YawDelta);
	NewRotation.Pitch = FMath::Clamp(FRotator::NormalizeAxis(NewRotation.Pitch) + PitchDelta, -89.f, 89.f);
	NewRotation.Roll = 0.f;

	// Rotating a spring arm normally orbits its pivot. Offset the pivot so the
	// camera stays at the same world position, like the editor's flight camera.
	const FVector SocketOffset = FVector(-SpringArm->TargetArmLength, 0.f, 0.f)
		+ SpringArm->SocketOffset + Camera->GetRelativeLocation();
	AddActorWorldOffset(OldRotation.RotateVector(SocketOffset) - NewRotation.RotateVector(SocketOffset));
	SpringArm->SetWorldRotation(NewRotation);
}

void ACameraManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	APlayerController* PC = Cast<APlayerController>(GetController());
	UWorld* World = GetWorld();
	if (!PC || !World || !World->GetGameViewport() || World->IsPaused())
	{
		EndNavigation();
		return;
	}

	const bool bFly = PC->IsInputKeyDown(EKeys::RightMouseButton);
	const bool bPan = PC->IsInputKeyDown(EKeys::MiddleMouseButton);
	if (bFly || bPan) BeginNavigation(PC);
	else EndNavigation();

	const float MouseX = PC->PlayerInput ? PC->PlayerInput->GetRawKeyValue(EKeys::MouseX) : 0.f;
	const float MouseY = PC->PlayerInput ? PC->PlayerInput->GetRawKeyValue(EKeys::MouseY) : 0.f;
	if (bFly)
	{
		// Unreal's MouseY is positive when the mouse moves up.
		RotateView(MouseX * MouseRotationSpeed, MouseY * MouseRotationSpeed);
		const FRotationMatrix ViewRotation(SpringArm->GetTargetRotation());
		const float Forward = (PC->IsInputKeyDown(EKeys::W) ? 1.f : 0.f) - (PC->IsInputKeyDown(EKeys::S) ? 1.f : 0.f);
		const float Right = (PC->IsInputKeyDown(EKeys::D) ? 1.f : 0.f) - (PC->IsInputKeyDown(EKeys::A) ? 1.f : 0.f);
		const float Up = (PC->IsInputKeyDown(EKeys::E) ? 1.f : 0.f) - (PC->IsInputKeyDown(EKeys::Q) ? 1.f : 0.f);
		const bool bFast = PC->IsInputKeyDown(EKeys::LeftShift) || PC->IsInputKeyDown(EKeys::RightShift);
		const FVector Direction = ViewRotation.GetUnitAxis(EAxis::X) * Forward
			+ ViewRotation.GetUnitAxis(EAxis::Y) * Right + FVector::UpVector * Up;
		AddActorWorldOffset(Direction.GetClampedToMaxSize(1.f) * MoveSpeed * NavigationSpeedScale * (bFast ? 3.f : 1.f) * DeltaTime);
	}
	else if (bPan)
	{
		const FRotationMatrix ViewRotation(SpringArm->GetTargetRotation());
		AddActorWorldOffset((-ViewRotation.GetUnitAxis(EAxis::Y) * MouseX
			- ViewRotation.GetUnitAxis(EAxis::Z) * MouseY) * MoveSpeed * 0.002f);
	}
	else
	{
		FVector2D ViewportSize;
		World->GetGameViewport()->GetViewportSize(ViewportSize);
		float X = 0.f, Y = 0.f;
		if (PC->GetMousePosition(X, Y) && X >= 0.f && Y >= 0.f && X <= ViewportSize.X && Y <= ViewportSize.Y)
		{
			FVector2D Edge = FVector2D::ZeroVector;
			if (X <= EdgeScrollThreshold) Edge.X = -1.f;
			else if (X >= ViewportSize.X - EdgeScrollThreshold) Edge.X = 1.f;
			if (Y <= EdgeScrollThreshold) Edge.Y = 1.f;
			else if (Y >= ViewportSize.Y - EdgeScrollThreshold) Edge.Y = -1.f;
			const FRotationMatrix CameraYaw(FRotator(0.f, SpringArm->GetTargetRotation().Yaw, 0.f));
			const FVector Direction = CameraYaw.GetUnitAxis(EAxis::X) * Edge.Y + CameraYaw.GetUnitAxis(EAxis::Y) * Edge.X;
			AddActorWorldOffset(Direction.GetSafeNormal() * MoveSpeed * DeltaTime);
		}
	}
}

void ACameraManager::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (CameraIMC) Subsystem->AddMappingContext(CameraIMC, 0);
		}
	}
	// Bind wheel keys directly so navigation does not depend on an authored IA/IMC.
	PlayerInputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &ACameraManager::ZoomIn);
	PlayerInputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &ACameraManager::ZoomOut);
}

void ACameraManager::Move(const FInputActionValue& Value)
{
	if (!bNavigating) return;
	const FVector2D Input = Value.Get<FVector2D>();
	const FRotationMatrix ViewRotation(SpringArm->GetTargetRotation());
	AddActorWorldOffset((ViewRotation.GetUnitAxis(EAxis::X) * Input.Y
		+ ViewRotation.GetUnitAxis(EAxis::Y) * Input.X).GetClampedToMaxSize(1.f) * MoveSpeed * GetWorld()->GetDeltaSeconds());
}

void ACameraManager::Dolly(float Direction)
{
	if (GetWorld() && !GetWorld()->IsPaused())
	{
		AddActorWorldOffset(Camera->GetForwardVector() * Direction * ZoomSpeed);
	}
}

void ACameraManager::ZoomIn()
{
	if (bNavigating) NavigationSpeedScale = FMath::Clamp(NavigationSpeedScale * 1.1f, 0.1f, 20.f);
	else Dolly(1.f);
}

void ACameraManager::ZoomOut()
{
	if (bNavigating) NavigationSpeedScale = FMath::Clamp(NavigationSpeedScale / 1.1f, 0.1f, 20.f);
	else Dolly(-1.f);
}

void ACameraManager::Zoom(const FInputActionValue& Value)
{
	Dolly(-Value.Get<float>());
}

void ACameraManager::UnPossessed()
{
	EndNavigation();
	Super::UnPossessed();
}

void ACameraManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EndNavigation();
	Super::EndPlay(EndPlayReason);
}
