#include "GachaWidget.h"
#include "FantasyUITheme.h"

#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Widget.h"
#include "Blueprint/WidgetTree.h"
#include "HAL/PlatformTime.h"
#include "UObject/UnrealType.h"

namespace
{
	void GatherGachaCards(UUserWidget* Parent, TArray<UUserWidget*>& OutCards, TSet<UUserWidget*>& Visited)
	{
		if (!IsValid(Parent) || Visited.Contains(Parent) || !Parent->WidgetTree) return;
		Visited.Add(Parent);

		TArray<UWidget*> Children;
		Parent->WidgetTree->ForEachWidget([&Children](UWidget* Child)
		{
			if (IsValid(Child)) Children.Add(Child);
		});

		for (UWidget* Child : Children)
		{
			UUserWidget* Nested = Cast<UUserWidget>(Child);
			if (!Nested) continue;
			if (Nested->GetClass()->GetName().Contains(TEXT("GachaCard"))) OutCards.Add(Nested);
			GatherGachaCards(Nested, OutCards, Visited);
		}
	}

	UCanvasPanel* EnsureRarityCanvas(UUserWidget* Card, UBorder* Frame)
	{
		if (!Card || !Card->WidgetTree || !Frame) return nullptr;

		UOverlay* Overlay = Cast<UOverlay>(Frame->GetContent());
		if (!Overlay || !Overlay->GetName().StartsWith(TEXT("TD_RarityOverlay")))
		{
			UWidget* ExistingContent = Frame->GetContent();
			Overlay = Card->WidgetTree->ConstructWidget<UOverlay>(
				UOverlay::StaticClass(), MakeUniqueObjectName(Card->WidgetTree, UOverlay::StaticClass(), TEXT("TD_RarityOverlay")));
			Frame->SetContent(Overlay);
			if (ExistingContent)
			{
				UOverlaySlot* BodySlot = Overlay->AddChildToOverlay(ExistingContent);
				BodySlot->SetHorizontalAlignment(HAlign_Fill);
				BodySlot->SetVerticalAlignment(VAlign_Fill);
			}
		}

		UCanvasPanel* Canvas = Cast<UCanvasPanel>(Card->GetWidgetFromName(TEXT("TD_RaritySparkCanvas")));
		if (!Canvas)
		{
			Canvas = Card->WidgetTree->ConstructWidget<UCanvasPanel>(
				UCanvasPanel::StaticClass(), FName(TEXT("TD_RaritySparkCanvas")));
			UOverlaySlot* CanvasOverlaySlot = Overlay->AddChildToOverlay(Canvas);
			CanvasOverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			CanvasOverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}
		Canvas->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (Canvas->GetChildrenCount() == 0)
		{
			constexpr int32 SparkCount = 24;
			for (int32 Index = 0; Index < SparkCount; ++Index)
			{
				UBorder* Spark = Card->WidgetTree->ConstructWidget<UBorder>(
					UBorder::StaticClass(), MakeUniqueObjectName(Card->WidgetTree, UBorder::StaticClass(), TEXT("TD_GachaSpark")));
				Spark->SetBrushColor(FLinearColor::White);
				Spark->SetVisibility(ESlateVisibility::HitTestInvisible);
				Spark->SetRenderTransformPivot(FVector2D(.5f, .5f));
				UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Spark);
				Slot->SetAnchors(FAnchors(0.f, 0.f));
				Slot->SetAlignment(FVector2D(.5f, .5f));
				Slot->SetPosition(FVector2D(200.f, 170.f));
				Slot->SetSize(FVector2D(5.f, 5.f));
			}
		}
		return Canvas;
	}
}

void UGachaWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FFantasyUITheme::ApplyToWidgetTree(this);
}

void UGachaWidget::ResetRarityReveals()
{
	for (FGachaRarityRevealState& Reveal : ActiveRarityReveals)
	{
		if (UUserWidget* Card = Reveal.Card.Get())
		{
			Card->SetRenderTransform(FWidgetTransform());
			Card->SetRenderOpacity(1.f);
		}
		if (UBorder* Frame = Reveal.Frame.Get()) Frame->SetBrushColor(Reveal.OriginalFrameColor);
		if (UCanvasPanel* Canvas = Reveal.SparkCanvas.Get()) Canvas->SetVisibility(ESlateVisibility::Collapsed);
	}
	ActiveRarityReveals.Empty();
}

void UGachaWidget::PlayRarityReveal(const TArray<FGachaCardInfo>& CardInfos)
{
	ResetRarityReveals();

	TArray<UUserWidget*> Cards;
	TSet<UUserWidget*> Visited;
	GatherGachaCards(this, Cards, Visited);

	for (UUserWidget* Card : Cards)
	{
		FIntProperty* IndexProperty = FindFProperty<FIntProperty>(Card->GetClass(), TEXT("CardIndex"));
		if (!IndexProperty) continue;
		const int32 Index = IndexProperty->GetPropertyValue_InContainer(Card);
		if (!CardInfos.IsValidIndex(Index)) continue;

		const FString& Grade = CardInfos[Index].Grade;
		const bool bEpic = Grade == TEXT("Epic");
		if (!bEpic && Grade != TEXT("Unique")) continue;

		UBorder* Frame = Cast<UBorder>(Card->GetWidgetFromName(TEXT("CardBorderGoldFrame")));
		if (!Frame) continue;
		UCanvasPanel* SparkCanvas = EnsureRarityCanvas(Card, Frame);
		if (!SparkCanvas) continue;

		const int32 SparkCount = bEpic ? 24 : 16;
		for (int32 SparkIndex = 0; SparkIndex < SparkCanvas->GetChildrenCount(); ++SparkIndex)
		{
			if (UBorder* Spark = Cast<UBorder>(SparkCanvas->GetChildAt(SparkIndex)))
				Spark->SetVisibility(ESlateVisibility::Collapsed);
		}
		TArray<TWeakObjectPtr<UBorder>> Sparks;
		Sparks.Reserve(SparkCount);
		const FLinearColor Accent = bEpic
			? FLinearColor(.98f, .67f, .18f, 1.f)
			: FLinearColor(.96f, .20f, .30f, 1.f);
		const FLinearColor SparkAlt = bEpic
			? FLinearColor(1.f, .93f, .60f, 1.f)
			: FLinearColor(1.f, .65f, .76f, 1.f);

		for (int32 SparkIndex = 0; SparkIndex < SparkCount; ++SparkIndex)
		{
			UBorder* Spark = Cast<UBorder>(SparkCanvas->GetChildAt(SparkIndex));
			if (!Spark) continue;
			Spark->SetBrushColor(SparkIndex % 3 == 0 ? SparkAlt : Accent);
			Spark->SetVisibility(ESlateVisibility::Visible);
			Sparks.Add(Spark);
		}

		Card->SetRenderTransformPivot(FVector2D(.5f, .5f));
		SparkCanvas->SetVisibility(ESlateVisibility::Visible);

		FGachaRarityRevealState& Reveal = ActiveRarityReveals.AddDefaulted_GetRef();
		Reveal.Card = Card;
		Reveal.Frame = Frame;
		Reveal.SparkCanvas = SparkCanvas;
		Reveal.Sparks = MoveTemp(Sparks);
		Reveal.AccentColor = Accent;
		Reveal.OriginalFrameColor = Frame->GetBrushColor();
		Reveal.StartTime = FPlatformTime::Seconds();
		Reveal.StartDelay = static_cast<float>(Index) * .16f;
		Reveal.Duration = bEpic ? 1.65f : 1.35f;
		Reveal.bEpic = bEpic;
	}
}

void UGachaWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);
	if (ActiveRarityReveals.IsEmpty()) return;

	const double Now = FPlatformTime::Seconds();
	for (int32 Index = ActiveRarityReveals.Num() - 1; Index >= 0; --Index)
	{
		FGachaRarityRevealState& Reveal = ActiveRarityReveals[Index];
		UUserWidget* Card = Reveal.Card.Get();
		UBorder* Frame = Reveal.Frame.Get();
		UCanvasPanel* SparkCanvas = Reveal.SparkCanvas.Get();
		if (!IsValid(Card) || !IsValid(Frame) || !IsValid(SparkCanvas))
		{
			ActiveRarityReveals.RemoveAtSwap(Index);
			continue;
		}

		const float Age = static_cast<float>(Now - Reveal.StartTime) - Reveal.StartDelay;
		if (Age < 0.f) continue;
		if (Age >= Reveal.Duration)
		{
			Card->SetRenderTransform(FWidgetTransform());
			Card->SetRenderOpacity(1.f);
			Frame->SetBrushColor(Reveal.OriginalFrameColor);
			SparkCanvas->SetVisibility(ESlateVisibility::Collapsed);
			ActiveRarityReveals.RemoveAtSwap(Index);
			continue;
		}

		const float Intro = FMath::Clamp(Age / .42f, 0.f, 1.f);
		const float Scale = FMath::InterpEaseOut(.78f, 1.f, Intro, 3.f);
		Card->SetRenderTransform(FWidgetTransform(FVector2D::ZeroVector, FVector2D(Scale, Scale), FVector2D::ZeroVector, 0.f));
		Card->SetRenderOpacity(FMath::Lerp(.62f, 1.f, FMath::InterpEaseOut(0.f, 1.f, Intro, 2.f)));

		const float Pulse = .18f + .38f * (.5f + .5f * FMath::Sin(Age * (Reveal.bEpic ? 13.f : 10.f)));
		Frame->SetBrushColor(FLinearColor::LerpUsingHSV(Reveal.OriginalFrameColor, Reveal.AccentColor, Pulse));

		FVector2D SparkCenter(200.f, 170.f);
		if (UImage* Icon = Cast<UImage>(Card->GetWidgetFromName(TEXT("IMG_TowerIcon"))))
		{
			const FGeometry IconGeometry = Icon->GetCachedGeometry();
			if (IconGeometry.GetLocalSize().X > 0.f && IconGeometry.GetLocalSize().Y > 0.f)
			{
				SparkCenter = SparkCanvas->GetCachedGeometry().AbsoluteToLocal(
					IconGeometry.GetAbsolutePosition() + IconGeometry.GetLocalSize() * .5f);
			}
		}

		const float Progress = FMath::Clamp(Age / Reveal.Duration, 0.f, 1.f);
		for (int32 SparkIndex = 0; SparkIndex < Reveal.Sparks.Num(); ++SparkIndex)
		{
			UBorder* Spark = Reveal.Sparks[SparkIndex].Get();
			if (!IsValid(Spark)) continue;

			const float Angle = (2.f * PI * SparkIndex / Reveal.Sparks.Num()) + Age * (Reveal.bEpic ? 1.9f : 1.25f);
			const float Ring = Reveal.bEpic && SparkIndex >= Reveal.Sparks.Num() / 2 ? 1.35f : 1.f;
			const float Radius = Ring * (10.f + 118.f * FMath::InterpEaseOut(0.f, 1.f, Progress, 1.35f));
			const float Twinkle = .35f + .65f * (.5f + .5f * FMath::Sin(Age * 17.f + SparkIndex * 1.7f));
			const float SparkSize = (3.f + 5.f * Twinkle) * (Reveal.bEpic ? 1.15f : 1.f);

			if (UCanvasPanelSlot* SparkSlot = Cast<UCanvasPanelSlot>(Spark->Slot))
			{
				SparkSlot->SetPosition(SparkCenter + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
				SparkSlot->SetSize(FVector2D(SparkSize, SparkSize));
			}
			Spark->SetRenderOpacity((1.f - Progress) * Twinkle);
			FWidgetTransform Transform;
			Transform.Angle = FMath::RadiansToDegrees(Angle);
			Transform.Scale = FVector2D(1.f + Twinkle * .5f);
			Spark->SetRenderTransform(Transform);
		}
	}
}
