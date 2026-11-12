// The look of the film-slate screens (lane A8): colours, fonts and makers for C++-drawn UMG. No widget blueprints, no assets:
// a black screen, a slate-grey card with a clapper stripe, chalk text, the take-yellow of lane A5's HUD for focus and calls.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Brushes/SlateColorBrush.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"

namespace JamMenuStyle
{
	inline FLinearColor Black() { return FLinearColor(0.f, 0.f, 0.f, 1.f); }
	inline FLinearColor Slate() { return FLinearColor(0.16f, 0.17f, 0.19f, 1.f); }
	inline FLinearColor SlateDark() { return FLinearColor(0.09f, 0.095f, 0.11f, 1.f); }
	inline FLinearColor Chalk() { return FLinearColor(0.92f, 0.92f, 0.90f, 1.f); }
	inline FLinearColor Dim() { return FLinearColor(0.62f, 0.63f, 0.66f, 1.f); }
	inline FLinearColor Yellow() { return FLinearColor(1.f, 0.85f, 0.4f, 1.f); }
	inline FLinearColor Disabled() { return FLinearColor(0.35f, 0.35f, 0.37f, 1.f); }

	inline FSlateFontInfo Font(int32 Size, const FName Face = FName("Bold")) { return FSlateFontInfo(FCoreStyle::GetDefaultFontStyle(Face, Size)); }

	inline UTextBlock* Text(UWidgetTree* T, const FString& S, int32 Size, const FLinearColor& Colour, const FName Face = FName("Bold"))
	{
		UTextBlock* W = T->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		W->SetText(FText::FromString(S));
		W->SetFont(Font(Size, Face));
		W->SetColorAndOpacity(FSlateColor(Colour));
		W->SetShadowOffset(FVector2D(1, 1));
		W->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.6f));
		return W;
	}

	inline FButtonStyle ButtonStyle()
	{
		FButtonStyle S;
		S.SetNormal(FSlateColorBrush(SlateDark()));
		S.SetHovered(FSlateColorBrush(FLinearColor(0.24f, 0.25f, 0.28f, 1.f)));
		S.SetPressed(FSlateColorBrush(Yellow()));
		S.SetDisabled(FSlateColorBrush(FLinearColor(0.06f, 0.06f, 0.07f, 1.f)));
		S.SetNormalPadding(FMargin(18, 10));
		S.SetPressedPadding(FMargin(18, 12, 18, 8));
		return S;
	}

	// A button holding one centred text; the text comes back so the focus tint can colour it.
	inline UButton* Button(UWidgetTree* T, const FString& Label, int32 Size, UTextBlock*& OutText)
	{
		UButton* B = T->ConstructWidget<UButton>(UButton::StaticClass());
		B->SetStyle(ButtonStyle());
		OutText = Text(T, Label, Size, Chalk());
		OutText->SetJustification(ETextJustify::Center);
		B->AddChild(OutText);
		return B;
	}

	// Focus and hover tint, called every tick: yellow text and a lighter plate when reached by stick, keys or mouse.
	inline void Tint(UButton* B, UTextBlock* Txt)
	{
		if (!B || !Txt) return;
		const bool bEnabled = B->GetIsEnabled();
		const bool bHot = bEnabled && (B->HasAnyUserFocus() || B->IsHovered() || B->HasFocusedDescendants());
		Txt->SetColorAndOpacity(FSlateColor(!bEnabled ? Disabled() : (bHot ? Yellow() : Chalk())));
		B->SetBackgroundColor(bHot ? FLinearColor(1.8f, 1.8f, 1.8f, 1.f) : FLinearColor::White);
	}

	inline UVerticalBoxSlot* AddV(UVerticalBox* Box, UWidget* W, FMargin Pad = FMargin(0, 6), EHorizontalAlignment H = HAlign_Fill)
	{
		UVerticalBoxSlot* S = Box->AddChildToVerticalBox(W);
		S->SetPadding(Pad);
		S->SetHorizontalAlignment(H);
		return S;
	}

	inline UHorizontalBoxSlot* AddH(UHorizontalBox* Box, UWidget* W, FMargin Pad = FMargin(6, 0), bool bFill = false, EVerticalAlignment V = VAlign_Center)
	{
		UHorizontalBoxSlot* S = Box->AddChildToHorizontalBox(W);
		S->SetPadding(Pad);
		S->SetVerticalAlignment(V);
		if (bFill) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		return S;
	}

	// The clapper stripe: alternating chalk and black blocks across the top of a slate card.
	inline UHorizontalBox* Clapper(UWidgetTree* T, int32 Blocks = 12, float Height = 18.f)
	{
		UHorizontalBox* H = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		for (int32 i = 0; i < Blocks; ++i)
		{
			UBorder* B = T->ConstructWidget<UBorder>(UBorder::StaticClass());
			B->SetBrushColor(i % 2 ? FLinearColor(0.05f, 0.05f, 0.06f, 1.f) : Chalk());
			B->SetPadding(FMargin(0, Height * 0.5f));
			UHorizontalBoxSlot* S = H->AddChildToHorizontalBox(B);
			S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
		return H;
	}

	inline UBorder* Plate(UWidgetTree* T, const FLinearColor& Colour, FMargin Pad)
	{
		UBorder* B = T->ConstructWidget<UBorder>(UBorder::StaticClass());
		B->SetBrushColor(Colour);
		B->SetPadding(Pad);
		return B;
	}

	// Back is Esc, the gamepad's B (east face button) or Slate's virtual back.
	inline bool IsBackKey(const FKey& K)
	{
		return K == EKeys::Escape || K == EKeys::Gamepad_FaceButton_Right;
	}

	// Menu art (R-0008: Codex-generated T_UI_* textures under /Game/UI). Null when the texture is not there: every screen keeps
	// its text so the menu reads without the art.
	inline UTexture2D* Art(const TCHAR* Name)
	{
		const FString Path = FString::Printf(TEXT("/Game/UI/%s.%s"), Name, Name);
		return LoadObject<UTexture2D>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
	}

	// A texture drawn at a fixed size: a size box forces the geometry (a desired-size override alone drew the art at icon size
	// in PIE) and the brush stretches into it. HeightPx 0 keeps the texture's aspect through the width.
	inline UWidget* Picture(UWidgetTree* T, UTexture2D* Tex, float WidthPx, float HeightPx = 0.f, float Opacity = 1.f)
	{
		UImage* I = T->ConstructWidget<UImage>(UImage::StaticClass());
		I->SetBrushFromTexture(Tex, true);
		I->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity));
		I->SetVisibility(ESlateVisibility::HitTestInvisible);
		const float H = HeightPx > 0.f ? HeightPx : (Tex && Tex->GetSizeX() > 0 ? WidthPx * static_cast<float>(Tex->GetSizeY()) / static_cast<float>(Tex->GetSizeX()) : WidthPx * 9.f / 16.f);
		USizeBox* Box = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Box->SetWidthOverride(WidthPx);
		Box->SetHeightOverride(H);
		Box->AddChild(I);
		if (USizeBoxSlot* S = Cast<USizeBoxSlot>(I->Slot)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
		Box->SetVisibility(ESlateVisibility::HitTestInvisible);
		return Box;
	}
}
