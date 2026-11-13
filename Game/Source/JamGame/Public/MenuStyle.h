// The look of every HOKEJ screen (lane A18, from lane A8's slate): one FJamUiStyle with the type scale, the colours, the spacing and the
// slate kit (R-0008 Codex-generated T_UI_* textures under /Game/UI: 9-slice panel, score plate, buttons, card frames, countdown ring, icons),
// plus makers for C++-drawn UMG. Every maker reads without the art: a texture that is not there falls back to a rounded-box brush of the
// same colour, so a screen never loses its text. The JamMenuStyle namespace at the end keeps lane A8's names as thin forwards.
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
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Engine/Texture2D.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"

struct FJamUiStyle
{
	// ---------------------------------------------------------------- colours (sRGB hex in the comment, linear here)
	static FLinearColor Ink() { return FLinearColor::FromSRGBColor(FColor(0x0E, 0x10, 0x14)); }          // #0E1014 near-black
	static FLinearColor Paper() { return FLinearColor::FromSRGBColor(FColor(0xF4, 0xF4, 0xF0)); }        // #F4F4F0 chalk paper
	static FLinearColor Chalk() { return Paper(); }
	static FLinearColor Blue() { return FLinearColor::FromSRGBColor(FColor(0x1D, 0x4E, 0xD8)); }         // #1D4ED8 home jersey
	static FLinearColor Orange() { return FLinearColor::FromSRGBColor(FColor(0xE8, 0x65, 0x1A)); }       // #E8651A away jersey
	static FLinearColor Yellow() { return FLinearColor::FromSRGBColor(FColor(0xFF, 0xD9, 0x66)); }       // #FFD966 take-yellow (focus, calls)
	static FLinearColor Muted() { return FLinearColor::FromSRGBColor(FColor(0x9A, 0xA0, 0xA8)); }        // #9AA0A8 captions
	static FLinearColor Slate() { return FLinearColor::FromSRGBColor(FColor(0x1C, 0x1F, 0x26)); }        // #1C1F26 panel fill
	static FLinearColor SlateDark() { return FLinearColor::FromSRGBColor(FColor(0x12, 0x14, 0x1A)); }    // #12141A button fill
	static FLinearColor Disabled() { return FLinearColor::FromSRGBColor(FColor(0x5A, 0x5E, 0x66)); }     // #5A5E66
	static FLinearColor Black() { return FLinearColor(0.f, 0.f, 0.f, 1.f); }
	static FLinearColor Dim() { return Muted(); }

	// ---------------------------------------------------------------- type scale (Slate units = px at 1080p; the DPI curve scales 720p by 2/3)
	static constexpr int32 Caption = 14;
	static constexpr int32 Body = 18;
	static constexpr int32 Label = 24;
	static constexpr int32 Title = 36;
	static constexpr int32 Display = 64;
	// Tracking in 1/1000 em: wide on the capitals of labels and titles, none on body copy.
	static constexpr int32 TrackLabel = 40;
	static constexpr int32 TrackTitle = 60;
	static constexpr int32 OutlinePx = 2;

	// Weights are the engine's Roboto faces: Regular, Medium, Bold, Black, BoldCondensed (scores), Italic.
	static FSlateFontInfo Font(int32 Size, const FName Face = FName("Bold"), int32 Tracking = 0, bool bOutline = false)
	{
		FSlateFontInfo F(FCoreStyle::GetDefaultFontStyle(Face, Size));
		F.LetterSpacing = Tracking;
		if (bOutline) F.OutlineSettings = FFontOutlineSettings(OutlinePx, FLinearColor(0.f, 0.f, 0.f, 0.9f));
		return F;
	}
	static FSlateFontInfo CaptionFont(bool bOutline = false) { return Font(Caption, FName("Medium"), 20, bOutline); }
	static FSlateFontInfo BodyFont(bool bOutline = false) { return Font(Body, FName("Regular"), 0, bOutline); }
	static FSlateFontInfo LabelFont(bool bOutline = false) { return Font(Label, FName("Bold"), TrackLabel, bOutline); }
	static FSlateFontInfo TitleFont(bool bOutline = false) { return Font(Title, FName("Black"), TrackTitle, bOutline); }
	static FSlateFontInfo DisplayFont(bool bOutline = false) { return Font(Display, FName("Black"), TrackTitle, bOutline); }
	static FSlateFontInfo ScoreFont(int32 Size, bool bOutline = true) { return Font(Size, FName("BoldCondensed"), 0, bOutline); }

	// ---------------------------------------------------------------- spacing and corners
	static constexpr float Gap = 8.f;
	static constexpr float Pad = 16.f;
	static constexpr float PadWide = 24.f;
	static constexpr float Radius = 10.f;
	static constexpr float EdgePx = 24.f;        // HUD safe margin from the screen edge

	// ---------------------------------------------------------------- the kit (textures under /Game/UI; null when absent)
	static UTexture2D* Art(const TCHAR* Name)
	{
		const FString Path = FString::Printf(TEXT("/Game/UI/%s.%s"), Name, Name);
		// Lane A114: an absent texture (T_UI_ScorePlate today) is asked for once per process, not at every match start (a synchronous lookup each time).
		static TSet<FString> Absent;   // game thread only
		if (Absent.Contains(Path)) return nullptr;
		UTexture2D* T = LoadObject<UTexture2D>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
		if (!T) Absent.Add(Path);
		return T;
	}

	// A texture drawn as a 9-slice box; MarginFrac is the fraction of each edge that stays unscaled.
	static FSlateBrush BoxBrush(UTexture2D* Tex, const FMargin& MarginFrac, const FLinearColor& InTint = FLinearColor::White)
	{
		FSlateBrush B;
		B.SetResourceObject(Tex);
		B.DrawAs = ESlateBrushDrawType::Box;
		B.Margin = MarginFrac;
		B.TintColor = FSlateColor(InTint);
		B.ImageSize = Tex ? FVector2D(Tex->GetSizeX(), Tex->GetSizeY()) : FVector2D(64, 64);
		return B;
	}
	static FSlateBrush ImageBrush(UTexture2D* Tex, FVector2D Size, const FLinearColor& InTint = FLinearColor::White)
	{
		FSlateBrush B;
		B.SetResourceObject(Tex);
		B.DrawAs = ESlateBrushDrawType::Image;
		B.ImageSize = Size;
		B.TintColor = FSlateColor(InTint);
		return B;
	}
	static FSlateBrush Rounded(const FLinearColor& Fill, float R = Radius, const FLinearColor& Outline = FLinearColor::Transparent, float OutlineW = 0.f)
	{
		return OutlineW > 0.f ? FSlateBrush(FSlateRoundedBoxBrush(Fill, R, Outline, OutlineW)) : FSlateBrush(FSlateRoundedBoxBrush(Fill, R));
	}

	// The dark slate panel (T_UI_Panel: 9-slice, chalk edge). Opacity scales the fill; the fallback is a rounded box with a chalk edge.
	static FSlateBrush Panel(float Opacity = 0.92f)
	{
		if (UTexture2D* T = Art(TEXT("T_UI_Panel")))
		{
			// Drawn at a quarter of its pixels: 28 px corner tiles and a 2.5 px chalk edge, so a 60 px panel keeps its shape.
			FSlateBrush B = BoxBrush(T, FMargin(0.22f), FLinearColor(1, 1, 1, Opacity));
			B.ImageSize = B.ImageSize * 0.25f;
			return B;
		}
		return Rounded(FLinearColor(Slate().R, Slate().G, Slate().B, Opacity), Radius, FLinearColor(Paper().R, Paper().G, Paper().B, 0.85f), 2.f);
	}
	// A plain dark pill with no edge: strips, hints, the bark strip.
	static FSlateBrush Pill(float Opacity = 0.78f, float R = Radius)
	{
		return Rounded(FLinearColor(Ink().R, Ink().G, Ink().B, Opacity), R);
	}
	// The scoreboard digit plate (T_UI_ScorePlate, light; tinted with the team colour by the caller).
	static FSlateBrush ScorePlate(const FLinearColor& Team)
	{
		if (UTexture2D* T = Art(TEXT("T_UI_ScorePlate"))) return BoxBrush(T, FMargin(0.28f, 0.4f), Team);
		return Rounded(Team, 8.f, Ink(), 2.f);
	}
	// The Director's Call card and the pause card (T_UI_CallCard: clapper band on top).
	static FSlateBrush CallCard()
	{
		if (UTexture2D* T = Art(TEXT("T_UI_CallCard"))) return BoxBrush(T, FMargin(0.12f, 0.2f, 0.12f, 0.12f));
		return Rounded(Slate(), Radius, Paper(), 2.f);
	}
	// The film-strip frame around a cut card (T_UI_CutFrame: transparent centre).
	static FSlateBrush CutFrame()
	{
		if (UTexture2D* T = Art(TEXT("T_UI_CutFrame"))) return BoxBrush(T, FMargin(0.08f, 0.22f));
		return Rounded(FLinearColor::Transparent, 6.f, Paper(), 2.f);
	}
	// The countdown ring (T_UI_Ring: transparent centre), drawn at Size.
	static FSlateBrush Ring(float Size)
	{
		if (UTexture2D* T = Art(TEXT("T_UI_Ring"))) return ImageBrush(T, FVector2D(Size, Size));
		return Rounded(FLinearColor::Transparent, Size * 0.5f, Paper(), 6.f);
	}
	// Icons: T_UI_Icon_<Name> (cut-regular, cut-raw, cut-boost, cut-large, cut-wide, cut-directors, pickup-speed, pickup-precision,
	// pickup-heavy, signal-defend, signal-space, signal-pass, signal-praise, settings, voice, barks, upscale). Null brush when absent.
	static bool IconBrush(const FString& Name, float Size, FSlateBrush& Out)
	{
		UTexture2D* T = Art(*FString::Printf(TEXT("T_UI_Icon_%s"), *Name.Replace(TEXT("-"), TEXT("_"))));
		if (!T) return false;
		Out = ImageBrush(T, FVector2D(Size, Size));
		return true;
	}

	// ---------------------------------------------------------------- makers
	static UTextBlock* Text(UWidgetTree* T, const FString& S, const FSlateFontInfo& F, const FLinearColor& Colour, bool bShadow = true)
	{
		UTextBlock* W = T->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		W->SetText(FText::FromString(S));
		W->SetFont(F);
		W->SetColorAndOpacity(FSlateColor(Colour));
		if (bShadow)
		{
			W->SetShadowOffset(FVector2D(1, 1));
			W->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.6f));
		}
		return W;
	}
	// Lane A8's signature: size and face; tracking follows the size class.
	static UTextBlock* Text(UWidgetTree* T, const FString& S, int32 Size, const FLinearColor& Colour, const FName Face = FName("Bold"))
	{
		const int32 Track = Size >= Title ? TrackTitle : (Size >= Label && Face != FName("Regular") ? TrackLabel : 0);
		return Text(T, S, Font(Size, Face, Track), Colour);
	}
	// HUD text over ice: outline instead of a shadow.
	static UTextBlock* IceText(UWidgetTree* T, const FString& S, const FSlateFontInfo& F, const FLinearColor& Colour)
	{
		UTextBlock* W = Text(T, S, F, Colour, false);
		W->SetVisibility(ESlateVisibility::HitTestInvisible);
		return W;
	}

	static UBorder* Plate(UWidgetTree* T, const FSlateBrush& Brush, FMargin InPad)
	{
		UBorder* B = T->ConstructWidget<UBorder>(UBorder::StaticClass());
		B->SetBrush(Brush);
		B->SetPadding(InPad);
		return B;
	}
	static UBorder* Plate(UWidgetTree* T, const FLinearColor& Colour, FMargin InPad)
	{
		UBorder* B = T->ConstructWidget<UBorder>(UBorder::StaticClass());
		B->SetBrushColor(Colour);
		B->SetPadding(InPad);
		return B;
	}
	static UImage* Icon(UWidgetTree* T, const FString& Name, float Size, const FLinearColor& InTint = FLinearColor::White)
	{
		UImage* I = T->ConstructWidget<UImage>(UImage::StaticClass());
		FSlateBrush B;
		if (IconBrush(Name, Size, B)) I->SetBrush(B);
		else { I->SetBrush(Rounded(FLinearColor::Transparent, Size * 0.5f, Paper(), 2.f)); I->SetDesiredSizeOverride(FVector2D(Size, Size)); }
		I->SetColorAndOpacity(InTint);
		I->SetVisibility(ESlateVisibility::HitTestInvisible);
		return I;
	}

	// Buttons: the three generated plates (T_UI_Button_Normal / _Hover / _Pressed) as 9-slices; dark, blue and orange boxes without them.
	static FButtonStyle ButtonStyle()
	{
		FButtonStyle S;
		UTexture2D* N = Art(TEXT("T_UI_Button_Normal"));
		UTexture2D* H = Art(TEXT("T_UI_Button_Hover"));
		UTexture2D* P = Art(TEXT("T_UI_Button_Pressed"));
		const FMargin M(0.2f, 0.42f);
		S.SetNormal(N ? BoxBrush(N, M) : Rounded(SlateDark(), Radius, FLinearColor(Paper().R, Paper().G, Paper().B, 0.5f), 1.5f));
		S.SetHovered(H ? BoxBrush(H, M) : Rounded(Blue(), Radius, Paper(), 2.f));
		S.SetPressed(P ? BoxBrush(P, M) : Rounded(Orange(), Radius, Ink(), 2.f));
		S.SetDisabled(N ? BoxBrush(N, M, FLinearColor(0.5f, 0.5f, 0.5f, 0.6f)) : Rounded(FLinearColor(SlateDark().R, SlateDark().G, SlateDark().B, 0.6f), Radius));
		S.SetNormalPadding(FMargin(22, 10));
		S.SetPressedPadding(FMargin(22, 12, 22, 8));
		return S;
	}
	static UButton* Button(UWidgetTree* T, const FString& LabelText, int32 Size, UTextBlock*& OutText)
	{
		UButton* B = T->ConstructWidget<UButton>(UButton::StaticClass());
		B->SetStyle(ButtonStyle());
		OutText = Text(T, LabelText, Size, Chalk());
		OutText->SetJustification(ETextJustify::Center);
		B->AddChild(OutText);
		return B;
	}
	// Focus and hover tint, called every tick: the label goes white over the blue hover plate, the yellow marks the focused one.
	static void Tint(UButton* B, UTextBlock* Txt)
	{
		if (!B || !Txt) return;
		const bool bEnabled = B->GetIsEnabled();
		const bool bHot = bEnabled && (B->HasAnyUserFocus() || B->IsHovered() || B->HasFocusedDescendants());
		Txt->SetColorAndOpacity(FSlateColor(!bEnabled ? Disabled() : (bHot ? Yellow() : Chalk())));
		B->SetBackgroundColor(FLinearColor::White);
	}
	static FSliderStyle SliderStyle()
	{
		FSliderStyle S = FCoreStyle::Get().GetWidgetStyle<FSliderStyle>("Slider");
		S.SetNormalBarImage(Rounded(FLinearColor(Muted().R, Muted().G, Muted().B, 0.6f), 3.f));
		S.SetHoveredBarImage(Rounded(Muted(), 3.f));
		S.SetDisabledBarImage(Rounded(FLinearColor(Disabled().R, Disabled().G, Disabled().B, 0.4f), 3.f));
		S.SetNormalThumbImage(Rounded(Chalk(), 7.f, Ink(), 1.5f));
		S.SetHoveredThumbImage(Rounded(Yellow(), 7.f, Ink(), 1.5f));
		S.SetDisabledThumbImage(Rounded(Disabled(), 7.f));
		S.SetBarThickness(6.f);
		return S;
	}

	static UVerticalBoxSlot* AddV(UVerticalBox* Box, UWidget* W, FMargin InPad = FMargin(0, 6), EHorizontalAlignment H = HAlign_Fill)
	{
		UVerticalBoxSlot* S = Box->AddChildToVerticalBox(W);
		S->SetPadding(InPad);
		S->SetHorizontalAlignment(H);
		return S;
	}
	static UHorizontalBoxSlot* AddH(UHorizontalBox* Box, UWidget* W, FMargin InPad = FMargin(6, 0), bool bFill = false, EVerticalAlignment V = VAlign_Center)
	{
		UHorizontalBoxSlot* S = Box->AddChildToHorizontalBox(W);
		S->SetPadding(InPad);
		S->SetVerticalAlignment(V);
		if (bFill) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		return S;
	}
	static UOverlaySlot* AddO(UOverlay* O, UWidget* W, EHorizontalAlignment H = HAlign_Fill, EVerticalAlignment V = VAlign_Fill, FMargin InPad = FMargin(0))
	{
		UOverlaySlot* S = O->AddChildToOverlay(W);
		S->SetHorizontalAlignment(H);
		S->SetVerticalAlignment(V);
		S->SetPadding(InPad);
		return S;
	}

	// The clapper stripe: alternating chalk and ink blocks across the top of a slate card.
	static UHorizontalBox* Clapper(UWidgetTree* T, int32 Blocks = 12, float Height = 18.f)
	{
		UHorizontalBox* H = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		for (int32 i = 0; i < Blocks; ++i)
		{
			UBorder* B = T->ConstructWidget<UBorder>(UBorder::StaticClass());
			B->SetBrushColor(i % 2 ? Ink() : Chalk());
			B->SetPadding(FMargin(0, Height * 0.5f));
			UHorizontalBoxSlot* S = H->AddChildToHorizontalBox(B);
			S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
		return H;
	}

	// A section title with a thin rule under it (menu screens).
	static UWidget* Section(UWidgetTree* T, const FString& S)
	{
		UVerticalBox* V = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		AddV(V, Text(T, S, LabelFont(), Yellow()), FMargin(0));
		UBorder* Rule = T->ConstructWidget<UBorder>(UBorder::StaticClass());
		Rule->SetBrushColor(FLinearColor(Yellow().R, Yellow().G, Yellow().B, 0.5f));
		Rule->SetPadding(FMargin(0, 1));
		AddV(V, Rule, FMargin(0, 4, 0, 0));
		return V;
	}

	static bool IsBackKey(const FKey& K) { return K == EKeys::Escape || K == EKeys::Gamepad_FaceButton_Right; }

	// A texture drawn at a fixed size (a size box forces the geometry; the brush stretches into it). HeightPx 0 keeps the aspect.
	static UWidget* Picture(UWidgetTree* T, UTexture2D* Tex, float WidthPx, float HeightPx = 0.f, float Opacity = 1.f)
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
};

// Lane A8's names, kept so its call sites read unchanged; everything resolves to FJamUiStyle.
namespace JamMenuStyle
{
	inline FLinearColor Black() { return FJamUiStyle::Black(); }
	inline FLinearColor Slate() { return FJamUiStyle::Slate(); }
	inline FLinearColor SlateDark() { return FJamUiStyle::SlateDark(); }
	inline FLinearColor Chalk() { return FJamUiStyle::Chalk(); }
	inline FLinearColor Dim() { return FJamUiStyle::Muted(); }
	inline FLinearColor Yellow() { return FJamUiStyle::Yellow(); }
	inline FLinearColor Disabled() { return FJamUiStyle::Disabled(); }
	inline FSlateFontInfo Font(int32 Size, const FName Face = FName("Bold")) { return FJamUiStyle::Font(Size, Face); }
	inline UTextBlock* Text(UWidgetTree* T, const FString& S, int32 Size, const FLinearColor& Colour, const FName Face = FName("Bold")) { return FJamUiStyle::Text(T, S, Size, Colour, Face); }
	inline FButtonStyle ButtonStyle() { return FJamUiStyle::ButtonStyle(); }
	inline UButton* Button(UWidgetTree* T, const FString& Label, int32 Size, UTextBlock*& OutText) { return FJamUiStyle::Button(T, Label, Size, OutText); }
	inline void Tint(UButton* B, UTextBlock* Txt) { FJamUiStyle::Tint(B, Txt); }
	inline UVerticalBoxSlot* AddV(UVerticalBox* Box, UWidget* W, FMargin InPad = FMargin(0, 6), EHorizontalAlignment H = HAlign_Fill) { return FJamUiStyle::AddV(Box, W, InPad, H); }
	inline UHorizontalBoxSlot* AddH(UHorizontalBox* Box, UWidget* W, FMargin InPad = FMargin(6, 0), bool bFill = false, EVerticalAlignment V = VAlign_Center) { return FJamUiStyle::AddH(Box, W, InPad, bFill, V); }
	inline UHorizontalBox* Clapper(UWidgetTree* T, int32 Blocks = 12, float Height = 18.f) { return FJamUiStyle::Clapper(T, Blocks, Height); }
	inline UBorder* Plate(UWidgetTree* T, const FLinearColor& Colour, FMargin InPad) { return FJamUiStyle::Plate(T, Colour, InPad); }
	inline bool IsBackKey(const FKey& K) { return FJamUiStyle::IsBackKey(K); }
	inline UTexture2D* Art(const TCHAR* Name) { return FJamUiStyle::Art(Name); }
	inline UWidget* Picture(UWidgetTree* T, UTexture2D* Tex, float WidthPx, float HeightPx = 0.f, float Opacity = 1.f) { return FJamUiStyle::Picture(T, Tex, WidthPx, HeightPx, Opacity); }
}
