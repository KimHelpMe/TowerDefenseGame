#include "MainMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildMenu();
}

APlayerController* UMainMenuWidget::GetMenuPlayerController() const
{
    if (APlayerController* PlayerController = GetOwningPlayer()) return PlayerController;
    return GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);

	if (StartButton)
	{
		StartButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandleStartClicked);
		StartButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleStartClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandleQuitClicked);
		QuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleQuitClicked);
	}

	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGamePaused(World, true);
	}

	APlayerController* PlayerController = GetMenuPlayerController();

	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
	}
}

void UMainMenuWidget::BuildMenu()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MenuRoot"));
	WidgetTree->RootWidget = Root;

	UImage* Backdrop = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ArcaneBackdrop"));
	Backdrop->SetColorAndOpacity(FLinearColor(0.008f, 0.012f, 0.026f, 0.88f));
	UCanvasPanelSlot* BackdropSlot = Root->AddChildToCanvas(Backdrop);
	BackdropSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	BackdropSlot->SetOffsets(FMargin(0.f));
	BackdropSlot->SetZOrder(0);

	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MenuFrame"));
	Frame->SetBrushColor(FLinearColor(0.60f, 0.43f, 0.18f, 1.f));
	Frame->SetPadding(FMargin(2.f));

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MenuPanel"));
	Panel->SetBrushColor(FLinearColor(0.025f, 0.035f, 0.065f, 0.98f));
	Panel->SetPadding(FMargin(42.f, 38.f, 42.f, 30.f));
	Frame->SetContent(Panel);

	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuContent"));
	Panel->SetContent(Content);

	UTextBlock* Brand = CreateMenuText(FText::FromString(TEXT("ARCANE FRONTIER")), 15, FLinearColor(0.35f, 0.78f, 0.84f, 1.f), ETextJustify::Center);
	Content->AddChildToVerticalBox(Brand);

	USizeBox* BrandSpace = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("BrandSpacing"));
	BrandSpace->SetHeightOverride(24.f);
	Content->AddChildToVerticalBox(BrandSpace);

	UTextBlock* Title = CreateMenuText(FText::FromString(TEXT("균열의 수호자")), 46, FLinearColor(0.92f, 0.82f, 0.60f, 1.f), ETextJustify::Center);
	Content->AddChildToVerticalBox(Title);

	USizeBox* TitleSpace = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("TitleSpacing"));
	TitleSpace->SetHeightOverride(12.f);
	Content->AddChildToVerticalBox(TitleSpace);

	UTextBlock* Subtitle = CreateMenuText(FText::FromString(TEXT("전략을 세우고, 몰려오는 적을 막아라")), 17, FLinearColor(0.68f, 0.72f, 0.82f, 1.f), ETextJustify::Center);
	Content->AddChildToVerticalBox(Subtitle);

	USizeBox* DividerSpace = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DividerSpacing"));
	DividerSpace->SetHeightOverride(34.f);
	Content->AddChildToVerticalBox(DividerSpace);

	UImage* Divider = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("GoldDivider"));
	Divider->SetColorAndOpacity(FLinearColor(0.48f, 0.36f, 0.17f, 0.9f));
	USizeBox* DividerSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DividerSize"));
	DividerSize->SetHeightOverride(2.f);
	DividerSize->SetContent(Divider);
	UVerticalBoxSlot* DividerSlot = Content->AddChildToVerticalBox(DividerSize);
	DividerSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 30.f));

	USizeBox* StartSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("StartButtonSize"));
	StartSize->SetWidthOverride(390.f);
	StartSize->SetHeightOverride(64.f);
	StartButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StartBattleButton"));
	StartButton->SetBackgroundColor(FLinearColor(0.30f, 0.21f, 0.095f, 1.f));
	StartButton->SetContent(CreateMenuText(FText::FromString(TEXT("전투 시작")), 22, FLinearColor(1.f, 0.91f, 0.72f, 1.f), ETextJustify::Center));
	StartSize->SetContent(StartButton);
	UVerticalBoxSlot* StartSlot = Content->AddChildToVerticalBox(StartSize);
	StartSlot->SetHorizontalAlignment(HAlign_Center);

	USizeBox* QuitSpace = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("QuitSpacing"));
	QuitSpace->SetHeightOverride(12.f);
	Content->AddChildToVerticalBox(QuitSpace);

	USizeBox* QuitSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("QuitButtonSize"));
	QuitSize->SetWidthOverride(390.f);
	QuitSize->SetHeightOverride(54.f);
	QuitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("QuitButton"));
	QuitButton->SetBackgroundColor(FLinearColor(0.075f, 0.09f, 0.14f, 1.f));
	QuitButton->SetContent(CreateMenuText(FText::FromString(TEXT("게임 종료")), 17, FLinearColor(0.77f, 0.79f, 0.84f, 1.f), ETextJustify::Center));
	QuitSize->SetContent(QuitButton);
	UVerticalBoxSlot* QuitSlot = Content->AddChildToVerticalBox(QuitSize);
	QuitSlot->SetHorizontalAlignment(HAlign_Center);

	USizeBox* FooterSpace = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("FooterSpacing"));
	FooterSpace->SetHeightOverride(28.f);
	Content->AddChildToVerticalBox(FooterSpace);

	UTextBlock* Footer = CreateMenuText(FText::FromString(TEXT("마우스로 방어시설을 배치하고 시너지를 완성하세요")), 13, FLinearColor(0.52f, 0.58f, 0.68f, 1.f), ETextJustify::Center);
	Content->AddChildToVerticalBox(Footer);

	USizeBox* FrameSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("FrameSize"));
	FrameSize->SetWidthOverride(520.f);
	FrameSize->SetHeightOverride(490.f);
	FrameSize->SetContent(Frame);
	UCanvasPanelSlot* FrameSlot = Root->AddChildToCanvas(FrameSize);
	FrameSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	FrameSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	FrameSlot->SetPosition(FVector2D::ZeroVector);
	FrameSlot->SetSize(FVector2D(520.f, 490.f));
	FrameSlot->SetZOrder(1);
}

UTextBlock* UMainMenuWidget::CreateMenuText(const FText& Text, int32 FontSize, const FLinearColor& Color, ETextJustify::Type Justification)
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TextBlock->SetText(Text);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetJustification(Justification);
	FSlateFontInfo Font = TextBlock->GetFont();
	Font.Size = FontSize;
	TextBlock->SetFont(Font);
	return TextBlock;
}

void UMainMenuWidget::HandleStartClicked()
{
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGamePaused(World, false);
	}

	if (APlayerController* PlayerController = GetMenuPlayerController())
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->SetInputMode(FInputModeGameAndUI());
	}

	RemoveFromParent();
}

void UMainMenuWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetMenuPlayerController(), EQuitPreference::Quit, false);
}
