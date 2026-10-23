#include "FantasyUITheme.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

namespace
{
	const FLinearColor PanelColor(0.025f, 0.038f, 0.072f, 0.94f);
	const FLinearColor InsetColor(0.045f, 0.062f, 0.102f, 0.96f);
	const FLinearColor GoldColor(0.86f, 0.70f, 0.40f, 1.f);
	const FLinearColor CyanColor(0.36f, 0.78f, 0.86f, 1.f);
	const FLinearColor PrimaryTextColor(0.84f, 0.88f, 0.95f, 1.f);
	const FLinearColor MutedTextColor(0.56f, 0.64f, 0.75f, 1.f);

	void ApplyWidgetStyle(UWidget* Widget)
	{
		if (!IsValid(Widget))
		{
			return;
		}

		const FString Name = Widget->GetName();
		const FString LowerName = Name.ToLower();

		if (UBorder* Border = Cast<UBorder>(Widget))
		{
			if (LowerName.Contains(TEXT("divider")))
			{
				Border->SetBrushColor(FLinearColor(0.48f, 0.36f, 0.19f, 0.9f));
			}
			else if (LowerName.Contains(TEXT("badge")))
			{
				Border->SetBrushColor(FLinearColor(0.36f, 0.25f, 0.09f, 0.98f));
			}
			else
			{
				Border->SetBrushColor(LowerName.Contains(TEXT("card")) ? InsetColor : PanelColor);
			}
		}
		else if (UButton* Button = Cast<UButton>(Widget))
		{
			if (LowerName.Contains(TEXT("place")) || LowerName.Contains(TEXT("upgrade")) ||
				LowerName.Contains(TEXT("reload")) || LowerName.Contains(TEXT("heal")))
			{
				Button->SetBackgroundColor(FLinearColor(0.34f, 0.24f, 0.105f, 1.f));
			}
			else if (LowerName.Contains(TEXT("purge")) || LowerName.Contains(TEXT("skill")) ||
				LowerName.Contains(TEXT("autofire")))
			{
				Button->SetBackgroundColor(FLinearColor(0.075f, 0.22f, 0.28f, 1.f));
			}
			else
			{
				Button->SetBackgroundColor(InsetColor);
			}
		}
		else if (UProgressBar* ProgressBar = Cast<UProgressBar>(Widget))
		{
			if (LowerName.Contains(TEXT("hp")))
			{
				ProgressBar->SetFillColorAndOpacity(FLinearColor(0.27f, 0.72f, 0.46f, 1.f));
			}
			else if (LowerName.Contains(TEXT("mana")))
			{
				ProgressBar->SetFillColorAndOpacity(CyanColor);
			}
			else
			{
				ProgressBar->SetFillColorAndOpacity(GoldColor);
			}
		}
		else if (UTextBlock* Text = Cast<UTextBlock>(Widget))
		{
			FLinearColor TextColor = PrimaryTextColor;
			if (LowerName.StartsWith(TEXT("lbl_")) || LowerName.Contains(TEXT("effect")) ||
				LowerName.Contains(TEXT("meta")) || LowerName.Contains(TEXT("cooldown")) ||
				LowerName.Contains(TEXT("attacktype")) || LowerName.Contains(TEXT("info")))
			{
				TextColor = MutedTextColor;
			}
			else if (LowerName.Contains(TEXT("name")) || LowerName.Contains(TEXT("grade")) ||
				LowerName.Contains(TEXT("round")) || LowerName.Contains(TEXT("level")) ||
				LowerName.Contains(TEXT("gold")))
			{
				TextColor = GoldColor;
			}
			else if (LowerName.Contains(TEXT("count")) || LowerName.Contains(TEXT("token")) ||
				LowerName.Contains(TEXT("hp")) || LowerName.Contains(TEXT("mana")) ||
				LowerName.Contains(TEXT("exp")))
			{
				TextColor = CyanColor;
			}
			Text->SetColorAndOpacity(FSlateColor(TextColor));
		}
	}

	void ApplyRecursive(UUserWidget* Widget, TSet<UUserWidget*>& Visited)
	{
		if (!IsValid(Widget) || Visited.Contains(Widget) || !Widget->WidgetTree)
		{
			return;
		}
		Visited.Add(Widget);

		TArray<UWidget*> Children;
		Widget->WidgetTree->ForEachWidget([&Children](UWidget* Child)
		{
			if (IsValid(Child))
			{
				Children.Add(Child);
			}
		});

		for (UWidget* Child : Children)
		{
			ApplyWidgetStyle(Child);
			if (UUserWidget* NestedWidget = Cast<UUserWidget>(Child))
			{
				ApplyRecursive(NestedWidget, Visited);
			}
		}
	}
}

void FFantasyUITheme::ApplyToWidgetTree(UUserWidget* RootWidget)
{
	TSet<UUserWidget*> Visited;
	ApplyRecursive(RootWidget, Visited);
}

void FFantasyUITheme::ApplyToViewport(UWorld* World)
{
	if (!IsValid(World))
	{
		return;
	}

	TArray<UUserWidget*> TopLevelWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
		World,
		TopLevelWidgets,
		UUserWidget::StaticClass(),
		true);

	TSet<UUserWidget*> Visited;
	for (UUserWidget* Widget : TopLevelWidgets)
	{
		ApplyRecursive(Widget, Visited);
	}
}
