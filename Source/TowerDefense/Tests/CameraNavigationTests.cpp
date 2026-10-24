#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "InputCoreTypes.h"
#include "CameraManager.h"
#include "Camera/PlayerCameraManager.h"
#include "MainMenuWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

namespace
{
	class FViewportNavigationCheck : public IAutomationLatentCommand
	{
	public:
		explicit FViewportNavigationCheck(FAutomationTestBase* InTest) : Test(InTest) {}

		virtual bool Update() override
		{
			const double Now = FPlatformTime::Seconds();
			if (Now < NextStepTime) return false;
			if (Step == 0)
			{
				for (const FWorldContext& Context : GEngine->GetWorldContexts())
				{
					if (Context.WorldType == EWorldType::PIE) World = Context.World();
				}
				PC = World.IsValid() ? World->GetFirstPlayerController() : nullptr;
				Pawn = PC.IsValid() ? Cast<ACameraManager>(PC->GetPawn()) : nullptr;
				if (!Pawn.IsValid())
				{
					Test->AddError(TEXT("Start PIE with the project's CameraManager pawn before this test."));
					return true;
				}
				TArray<UUserWidget*> Menus;
				UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World.Get(), Menus, UMainMenuWidget::StaticClass(), false);
				for (UUserWidget* Menu : Menus) Menu->RemoveFromParent();
				PC->SetPause(false);
				PC->bShowMouseCursor = true;
				FInputModeGameAndUI Mode;
				Mode.SetHideCursorDuringCapture(false);
				Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(Mode);
				SavedTransform = Pawn->GetActorTransform();
				SavedArmRotation = Pawn->SpringArm->GetComponentRotation();
				// Reproduce camera/Pawn rotation disagreement from rotated player spawns.
                Pawn->SetActorRotation(FRotator(0.f, 90.f, 0.f));
                SavedEdgeThreshold = Pawn->EdgeScrollThreshold;
				Pawn->EdgeScrollThreshold = -1.f;
			}
			else if (!Pawn.IsValid() || !PC.IsValid())
			{
				Test->AddError(TEXT("PIE ended during the camera navigation test."));
				return true;
			}
			else
			{
				switch (Step)
				{
				case 1:
					Key(EKeys::RightMouseButton, true);
					break;
				case 2:
					Test->TestFalse(TEXT("Right mouse captures and hides the cursor"), PC->bShowMouseCursor);
					Before = PC->PlayerCameraManager->GetCameraLocation();
					RotationBefore = PC->PlayerCameraManager->GetCameraRotation();
					Axis(EKeys::MouseX, 60.f);
					Axis(EKeys::MouseY, 35.f);
					break;
				case 3:
				{
					const FRotator Rotation = PC->PlayerCameraManager->GetCameraRotation();
					Test->TestTrue(TEXT("Mouse X changes yaw"), FMath::Abs(FMath::FindDeltaAngleDegrees(RotationBefore.Yaw, Rotation.Yaw)) > 5.f);
					Test->TestTrue(TEXT("Mouse up looks up"), Rotation.Pitch > RotationBefore.Pitch + 2.f);
					Test->TestTrue(TEXT("Looking rotates in place instead of orbiting"), FVector::Dist(Before, PC->PlayerCameraManager->GetCameraLocation()) < 1.f);
					Before = PC->PlayerCameraManager->GetCameraLocation();
					Key(EKeys::W, true);
					break;
				}
				case 4:
					Key(EKeys::W, false);
					CheckDirection(TEXT("W moves forward in the rendered view with Pawn yaw 90"), PC->PlayerCameraManager->GetCameraRotation().Vector());
					Before = PC->PlayerCameraManager->GetCameraLocation();
					Key(EKeys::S, true);
					break;
				case 5:
					Key(EKeys::S, false);
					CheckDirection(TEXT("S moves backward in the rendered view with Pawn yaw 90"), -PC->PlayerCameraManager->GetCameraRotation().Vector());
					Before = PC->PlayerCameraManager->GetCameraLocation();
					Key(EKeys::D, true);
					break;
				case 6:
					Key(EKeys::D, false);
					CheckDirection(TEXT("D moves right in the rendered view with Pawn yaw 90"), FRotationMatrix(PC->PlayerCameraManager->GetCameraRotation()).GetUnitAxis(EAxis::Y));
					Before = PC->PlayerCameraManager->GetCameraLocation();
					Key(EKeys::A, true);
					break;
				case 7:
					Key(EKeys::A, false);
					CheckDirection(TEXT("A moves left in the rendered view with Pawn yaw 90"), -FRotationMatrix(PC->PlayerCameraManager->GetCameraRotation()).GetUnitAxis(EAxis::Y));
					Before = PC->PlayerCameraManager->GetCameraLocation();
					Key(EKeys::E, true);
					break;
				case 8:
					Key(EKeys::E, false);
					CheckDirection(TEXT("E moves up"), FVector::UpVector);
					Before = PC->PlayerCameraManager->GetCameraLocation();
					Key(EKeys::Q, true);
					break;
				case 9:
					Key(EKeys::Q, false);
					CheckDirection(TEXT("Q moves down"), -FVector::UpVector);
					Key(EKeys::RightMouseButton, false);
					break;
				case 10:
					Test->TestTrue(TEXT("Releasing right mouse restores the cursor"), PC->bShowMouseCursor);
					Before = PC->PlayerCameraManager->GetCameraLocation();
					Key(EKeys::W, true);
					break;
				case 11:
					Key(EKeys::W, false);
					Test->TestTrue(TEXT("W outside flight mode does not move the camera"), FVector::Dist(Before, PC->PlayerCameraManager->GetCameraLocation()) < 1.f);
					Before = PC->PlayerCameraManager->GetCameraLocation();
					Key(EKeys::MouseScrollUp, true);
					Key(EKeys::MouseScrollUp, false);
					break;
				case 12:
					CheckDirection(TEXT("Wheel up dollies forward"), PC->PlayerCameraManager->GetCameraRotation().Vector());
					Before = PC->PlayerCameraManager->GetCameraLocation();
					Key(EKeys::MouseScrollDown, true);
					Key(EKeys::MouseScrollDown, false);
					break;
				case 13:
					CheckDirection(TEXT("Wheel down dollies backward"), -PC->PlayerCameraManager->GetCameraRotation().Vector());
					Pawn->SetActorTransform(SavedTransform);
					Pawn->SpringArm->SetWorldRotation(SavedArmRotation);
					Pawn->EdgeScrollThreshold = SavedEdgeThreshold;
					Test->AddInfo(TEXT("Verified all WASD directions against PlayerCameraManager's rendered view at Pawn yaw 90, plus mouse look, Q/E, cursor release, and wheel dolly."));
					return true;
				}
			}
			++Step;
			NextStepTime = Now + 0.2;
			return false;
		}

	private:
		void Key(const FKey& Key, bool bPressed)
		{
			PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1.f : 0.f));
		}
		void Axis(const FKey& Key, float Amount)
		{
			PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Axis, Amount, 1));
		}
		void CheckDirection(const TCHAR* Label, const FVector& Expected)
		{
			const FVector Delta = PC->PlayerCameraManager->GetCameraLocation() - Before;
			Test->TestTrue(Label, Delta.Size() > 5.f && FVector::DotProduct(Delta.GetSafeNormal(), Expected.GetSafeNormal()) > 0.98f);
		}
		FAutomationTestBase* Test;
		TWeakObjectPtr<UWorld> World;
		TWeakObjectPtr<APlayerController> PC;
		TWeakObjectPtr<ACameraManager> Pawn;
		FTransform SavedTransform;
		FRotator SavedArmRotation;
		FRotator RotationBefore;
		FVector Before;
		float SavedEdgeThreshold = 0.f;
		int32 Step = 0;
		double NextStepTime = 0.;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FViewportCameraNavigationTest,
	"TowerDefense.Camera.ViewportNavigation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FViewportCameraNavigationTest::RunTest(const FString& Parameters)
{
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
    ADD_LATENT_AUTOMATION_COMMAND(FViewportNavigationCheck(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif