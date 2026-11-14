#include "MenuWidget.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "MenuStyle.h"
#include "SettingsWidget.h"
#include "InputMapping.h"
#include "JamCartridge.h"
#include "Progression.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WidgetSwitcherSlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/ButtonSlot.h"
#include "Components/BorderSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/Spacer.h"
#include "Components/Slider.h"
#include "Components/Image.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Input/HittestGrid.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

// Lane A187: the controls lane (A183) builds the rebinding screen in SettingsWidget.*: UJamSettingsWidget::MakeControlsScreen(UObject* Outer). The
// menu asks the compiler whether that function exists on this tree (a C++20 requires-expression on a dependent type, so nothing names it when it
// is absent): when it does, CONTROLS opens that screen; until then today's controls card.
namespace JamMenuControlsHook
{
	template <typename TSettings>
	UUserWidget* Make(UJamMenuWidget* Menu, TFunction<UWidget*()>& OutFirst)
	{
		if constexpr (requires(UObject* O) { TSettings::MakeControlsScreen(O); })
		{
			auto* C = TSettings::MakeControlsScreen(Menu);
			if (!C) return nullptr;
			C->OnBack.AddDynamic(Menu, &UJamMenuWidget::OnControlsBack);
			TWeakObjectPtr<std::remove_pointer_t<decltype(C)>> Weak(C);
			OutFirst = [Weak]() -> UWidget* { return Weak.IsValid() ? Weak->GetFirstFocusable() : nullptr; };
			return C;
		}
		else
		{
			return nullptr;
		}
	}
}

using namespace JamMenuStyle;
using S = FJamUiStyle;
using L = FJamMenuLook;

DEFINE_LOG_CATEGORY_STATIC(LogJamMenuUi, Log, All);

// ---------------------------------------------------------------- the style table (lane A187)

FLinearColor FJamMenuLook::Home() { return FLinearColor::FromSRGBColor(FColor(0x2F, 0x6B, 0xFF)); }
FLinearColor FJamMenuLook::Away() { return FLinearColor::FromSRGBColor(FColor(0xF2, 0x6A, 0x1B)); }
FLinearColor FJamMenuLook::Ink() { return FLinearColor::FromSRGBColor(FColor(0x0A, 0x0C, 0x10)); }
FLinearColor FJamMenuLook::Panel() { FLinearColor C = FLinearColor::FromSRGBColor(FColor(0x14, 0x18, 0x21)); C.A = 0.90f; return C; }
FLinearColor FJamMenuLook::PanelEdge() { return FLinearColor(1.f, 1.f, 1.f, 0.12f); }
FLinearColor FJamMenuLook::Text() { return FLinearColor::FromSRGBColor(FColor(0xF5, 0xF6, 0xF8)); }
FLinearColor FJamMenuLook::TextDim() { return FLinearColor::FromSRGBColor(FColor(0xA3, 0xAB, 0xB8)); }
FLinearColor FJamMenuLook::TextOnFocus() { return FLinearColor::White; }
FLinearColor FJamMenuLook::Disabled() { return FLinearColor::FromSRGBColor(FColor(0x4A, 0x50, 0x5C)); }
FLinearColor FJamMenuLook::Scrim() { return FLinearColor(0.f, 0.f, 0.f, 0.62f); }

TArray<float> FJamMenuLook::GridValues()
{
	return { HeaderH, FooterH, SideMargin, FocusBarW, ItemPadX, ItemPadY, Gap, CardPad, MainColumnW, FeatureW, ModesW, ModePictureH, ModeSlotPad,
		ModeCardPadX, SeasonW, PanelW, WidePanelW, TransitionSlide };
}

namespace
{
	const TCHAR* const KitRoot = TEXT("/JamAssets/UI/");
	const TCHAR* const KitFontPath = TEXT("/JamAssets/UI/Fonts/F_Hokej_Display.F_Hokej_Display");

	UObject* KitFontObject()
	{
		static TWeakObjectPtr<UObject> Font;
		static bool bAsked = false;   // game thread only
		if (!bAsked)
		{
			bAsked = true;
			Font = LoadObject<UFont>(nullptr, KitFontPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
		}
		return Font.Get();
	}
}

FSlateFontInfo FJamMenuLook::Font(int32 Size, FName Face, int32 Tracking)
{
	if (UObject* F = KitFontObject())
	{
		FSlateFontInfo Info(F, static_cast<float>(Size), Face);
		Info.LetterSpacing = Tracking;
		return Info;
	}
	FSlateFontInfo Info(FCoreStyle::GetDefaultFontStyle(Face, Size));
	Info.LetterSpacing = Tracking;
	return Info;
}

bool FJamMenuLook::HasKitFont() { return KitFontObject() != nullptr; }

UTexture2D* FJamMenuLook::Kit(const FString& Name)
{
	FString Leaf = Name;
	int32 Slash = INDEX_NONE;
	if (Name.FindLastChar(TEXT('/'), Slash)) Leaf = Name.Mid(Slash + 1);
	const FString Path = FString::Printf(TEXT("%s%s.%s"), KitRoot, *Name, *Leaf);
	static TMap<FString, TWeakObjectPtr<UTexture2D>> Found;   // game thread only
	static TSet<FString> Absent;
	if (Absent.Contains(Path)) return nullptr;
	if (const TWeakObjectPtr<UTexture2D>* F = Found.Find(Path)) if (F->IsValid()) return F->Get();
	UTexture2D* T = LoadObject<UTexture2D>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (T) Found.Add(Path, T); else Absent.Add(Path);
	return T;
}

TArray<FString> FJamMenuLook::KitNames()
{
	TArray<FString> Out = { TEXT("T_UI_MenuBackdrop"), TEXT("T_UI_Logo_Hokej"), TEXT("T_UI_Panel"), TEXT("T_UI_Button_Normal"), TEXT("T_UI_Button_Focused"),
		TEXT("T_UI_Button_Pressed"), TEXT("T_UI_Button_Disabled"), TEXT("T_UI_Segment_Lit"), TEXT("T_UI_Segment_Unlit"), TEXT("T_UI_Card_Mode"), TEXT("T_UI_HeaderBar") };
	for (const TCHAR* Slug : { TEXT("pond"), TEXT("cage"), TEXT("sniper"), TEXT("marathon"), TEXT("power_play"), TEXT("shotgun"), TEXT("sudden_death"),
		TEXT("outnumbered"), TEXT("glass_house"), TEXT("arsenal"), TEXT("bruisers"), TEXT("fog_of_war"), TEXT("rinks"), TEXT("make_your_own") })
	{
		Out.Add(FString::Printf(TEXT("Modes/T_UI_Mode_%s"), Slug));
	}
	Out.Add(TEXT("Fonts/F_Hokej_Display"));
	return Out;
}

// ---------------------------------------------------------------- lane A195: the makers every screen uses

FSlateBrush FJamMenuLook::FillBrush(const FLinearColor& C, bool bEdge)
{
	return bEdge ? FSlateBrush(FSlateRoundedBoxBrush(C, 4.f, PanelEdge(), 1.f)) : FSlateBrush(FSlateRoundedBoxBrush(C, 4.f));
}

FSlateBrush FJamMenuLook::KitOr(const TCHAR* Name, const FSlateBrush& Fallback)
{
	if (UTexture2D* T = Kit(Name))
	{
		FSlateBrush B = FJamUiStyle::BoxBrush(T, FMargin(0.2f, 0.35f));
		B.ImageSize = B.ImageSize * 0.5f;
		return B;
	}
	return Fallback;
}

FSlateBrush FJamMenuLook::PanelBrush() { return KitOr(TEXT("T_UI_Panel"), FillBrush(Panel(), true)); }

FButtonStyle FJamMenuLook::ItemStyle(int32 State, bool bSegment, const FMargin& Pad)
{
	// The item look: a flat fill in the panel colour at rest, a solid accent fill (and a white bar at its left edge) on the focus, the home blue
	// for the chosen level segment. Device B's button plates replace the flat fills by name when they are in the plugin.
	FButtonStyle St;
	FSlateBrush B;
	switch (State)
	{
	case 0: B = KitOr(TEXT("T_UI_Button_Disabled"), FillBrush(FLinearColor(Ink().R, Ink().G, Ink().B, 0.55f), false)); break;
	case 2: B = KitOr(TEXT("T_UI_Button_Focused"), FillBrush(Focus(), false)); break;
	case 3: B = KitOr(TEXT("T_UI_Segment_Lit"), FillBrush(Chosen(), false)); break;
	default: B = KitOr(bSegment ? TEXT("T_UI_Segment_Unlit") : TEXT("T_UI_Button_Normal"), FillBrush(Panel(), true)); break;
	}
	St.SetNormal(B);
	St.SetHovered(B);
	FSlateBrush P = State == 2 ? KitOr(TEXT("T_UI_Button_Pressed"), FillBrush(FLinearColor::FromSRGBColor(FColor(0xC9, 0x52, 0x0E)), false)) : B;
	St.SetPressed(P);
	St.SetDisabled(B);
	St.SetNormalPadding(Pad);
	St.SetPressedPadding(Pad);
	return St;
}

UTextBlock* FJamMenuLook::MakeText(UWidgetTree* T, const FString& Str, const FSlateFontInfo& F, const FLinearColor& C)
{
	UTextBlock* W = T->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	W->SetText(FText::FromString(Str));
	W->SetFont(F);
	W->SetColorAndOpacity(FSlateColor(C));
	return W;
}

UButton* FJamMenuLook::MakeItem(UWidgetTree* T, const FString& Label, int32 Size, UTextBlock*& OutText, UWidget*& OutBar, bool bCentre)
{
	UButton* B = T->ConstructWidget<UButton>(UButton::StaticClass());
	B->SetStyle(ItemStyle(1));
	UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UBorder* Bar = T->ConstructWidget<UBorder>(UBorder::StaticClass());
	Bar->SetBrushColor(FLinearColor::Transparent);
	USizeBox* BarBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	BarBox->SetWidthOverride(FocusBarW);
	BarBox->AddChild(Bar);
	if (USizeBoxSlot* Sl = Cast<USizeBoxSlot>(Bar->Slot)) { Sl->SetHorizontalAlignment(HAlign_Fill); Sl->SetVerticalAlignment(VAlign_Fill); }
	UHorizontalBoxSlot* BS = Row->AddChildToHorizontalBox(BarBox);
	BS->SetVerticalAlignment(VAlign_Fill);
	OutText = MakeText(T, Label, Font(Size, FName("Bold"), TrackLabel), Text());
	if (bCentre) OutText->SetJustification(ETextJustify::Center);
	UHorizontalBoxSlot* TS = Row->AddChildToHorizontalBox(OutText);
	TS->SetPadding(FMargin(ItemPadX, Unit + 4.f, ItemPadX + FocusBarW, Unit + 4.f));
	TS->SetVerticalAlignment(VAlign_Center);
	if (bCentre) TS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	B->AddChild(Row);
	if (UButtonSlot* Sl = Cast<UButtonSlot>(Row->Slot)) { Sl->SetHorizontalAlignment(HAlign_Fill); Sl->SetVerticalAlignment(VAlign_Fill); Sl->SetPadding(FMargin(0)); }
	OutBar = Bar;
	return B;
}

UWidget* FJamMenuLook::MakeTeamRule(UWidgetTree* T, float Height, float Width)
{
	UHorizontalBox* H = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	for (const FLinearColor& C : { Home(), Away() })
	{
		UBorder* Piece = T->ConstructWidget<UBorder>(UBorder::StaticClass());
		Piece->SetBrushColor(C);
		USizeBox* SB = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		if (Width > 0.f) SB->SetWidthOverride(Width * 0.5f);
		SB->SetHeightOverride(Height);
		SB->AddChild(Piece);
		UHorizontalBoxSlot* Sl = H->AddChildToHorizontalBox(SB);
		if (Width <= 0.f) Sl->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	return H;
}

UWidget* FJamMenuLook::MakePanel(UWidgetTree* T, UWidget* Content, float Width, const FMargin& Pad)
{
	UVerticalBox* V = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	UVerticalBoxSlot* RS = V->AddChildToVerticalBox(MakeTeamRule(T, 4.f));
	RS->SetHorizontalAlignment(HAlign_Fill);
	UBorder* P = T->ConstructWidget<UBorder>(UBorder::StaticClass());
	P->SetBrush(PanelBrush());
	P->SetPadding(Pad);
	P->SetContent(Content);
	UVerticalBoxSlot* PS = V->AddChildToVerticalBox(P);
	PS->SetHorizontalAlignment(HAlign_Fill);
	if (Width <= 0.f) return V;
	USizeBox* SB = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SB->SetWidthOverride(Width);
	SB->AddChild(V);
	return SB;
}

void FJamMenuLook::FillHints(UWidgetTree* T, UHorizontalBox* Box, const TArray<TPair<FString, FString>>& Hints)
{
	if (!T || !Box) return;
	Box->ClearChildren();
	for (const TPair<FString, FString>& H : Hints)
	{
		UBorder* Cap = T->ConstructWidget<UBorder>(UBorder::StaticClass());
		Cap->SetBrush(FillBrush(FLinearColor(1, 1, 1, 0.14f), false));
		Cap->SetPadding(FMargin(Unit, 3.f));
		Cap->SetContent(MakeText(T, H.Key, Font(Caption, FName("Black"), 40), Text()));
		UHorizontalBoxSlot* CS = Box->AddChildToHorizontalBox(Cap);
		CS->SetPadding(FMargin(0, 0, Unit, 0));
		CS->SetVerticalAlignment(VAlign_Center);
		UHorizontalBoxSlot* AS = Box->AddChildToHorizontalBox(MakeText(T, H.Value, Font(Caption, FName("Bold"), TrackLabel), TextDim()));
		AS->SetPadding(FMargin(0, 0, &H == &Hints.Last() ? 0.f : Unit * 4.f, 0));   // no gap after the last: the row is as wide as its words
		AS->SetVerticalAlignment(VAlign_Center);
	}
}

FString FJamMenuLook::HintsLine(const TArray<TPair<FString, FString>>& Hints)
{
	FString Line;
	for (const TPair<FString, FString>& H : Hints) Line += FString::Printf(TEXT("%s %s   "), *H.Key, *H.Value);
	Line.TrimEndInline();
	return Line;
}

FString FJamMenuLook::RestartKeyCap()
{
	const FJamVerbBinding& Bi = UJamInputMap::Get()->GetBindings()[EJamVerb::Restart];
	FString Cap = Bi.Keys.Num() ? UJamInputMap::KeyName(Bi.Keys[0]) : FString(TEXT("-"));
	if (Bi.Pads.Num()) Cap += TEXT(" / ") + UJamInputMap::KeyName(Bi.Pads[0]);
	return Cap;
}

// ---- the painter

void FJamMenuPainter::Add(UButton* B, UTextBlock* T, UWidget* Bar, const FMargin& Pad)
{
	if (!B) return;
	FEntry E;
	E.Button = B;
	E.Text = T;
	E.Bar = Bar;
	E.Pad = Pad;
	Entries.Add(E);
	B->SetStyle(FJamMenuLook::ItemStyle(1, false, Pad));
	B->SetBackgroundColor(FLinearColor::White);
}

int32 FJamMenuPainter::StateOf(const UButton* B) const
{
	for (const FEntry& E : Entries) if (E.Button.Get() == B) return E.State;
	return -1;
}

void FJamMenuPainter::Paint(APlayerController* PC, bool bHoverFocus)
{
	// The pointer moved: the item under it takes the focus (so Enter presses what the pointer is on, and one item is lit).
	if (bHoverFocus && FSlateApplication::IsInitialized())
	{
		const FVector2D Pointer = FSlateApplication::Get().GetCursorPos();
		if (!Pointer.Equals(LastCursor, 0.5))
		{
			const bool bFirst = LastCursor.X < 0.f;
			LastCursor = Pointer;
			const TSharedPtr<SWidget> KF = FSlateApplication::Get().GetKeyboardFocusedWidget();
			const bool bTyping = KF.IsValid() && KF->GetType().ToString().Contains(TEXT("EditableText"));   // a key being typed keeps its box
			if (!bFirst && !bTyping && PC)
				for (const FEntry& E : Entries)
				{
					UButton* B = E.Button.Get();
					if (B && B->IsHovered() && B->GetIsEnabled() && B->IsVisible() && !B->HasUserFocus(PC)) { B->SetFocus(); break; }
				}
		}
	}
	for (FEntry& E : Entries)
	{
		UButton* B = E.Button.Get();
		if (!B) continue;
		const bool bEnabled = B->GetIsEnabled();
		const bool bHot = bEnabled && ((PC && B->HasUserFocus(PC)) || B->HasFocusedDescendants() || (!bHoverFocus && B->IsHovered()));
		const int32 State = !bEnabled ? 0 : (bHot ? 2 : 1);
		if (State == E.State) continue;
		E.State = State;
		B->SetStyle(FJamMenuLook::ItemStyle(State, false, E.Pad));
		B->SetBackgroundColor(FLinearColor::White);
		if (UTextBlock* Tb = E.Text.Get()) Tb->SetColorAndOpacity(FSlateColor(State == 0 ? FJamMenuLook::Disabled() : (State == 2 ? FJamMenuLook::TextOnFocus() : FJamMenuLook::Text())));
		if (UBorder* Bar = Cast<UBorder>(E.Bar.Get())) Bar->SetBrushColor(State == 2 ? FJamMenuLook::TextOnFocus() : FLinearColor::Transparent);
	}
}

namespace
{
	// ---- widths in slate units (lane A144's measures, re-laid on lane A187's panels)
	constexpr float ScreenPadX = L::CardPad;     // inside a panel
	constexpr float ModeTextW = L::ModesW / L::ModesPerRow - 2.f * L::ModeSlotPad - 2.f * L::ModeCardPadX;   // 288
	constexpr int32 CutsPerRow = 3;
	constexpr float CutIcon = 36.f;
	constexpr float CutPadX = 12.f;
	constexpr float CutSlotPad = 4.f;
	// The cut cards inside the scroll box: its bar and the bar's padding take 30 units; 6 more are kept free (lane A144).
	constexpr float CutTextW = (L::WidePanelW - 2.f * ScreenPadX - 30.f) / CutsPerRow - 2.f * CutSlotPad - 2.f * CutPadX - CutIcon - 10.f - 6.f;   // 311
	constexpr float CutBodyH = 78.f;             // a cut card's words: the name and two lines
	constexpr float CutListH = 3.f * (CutBodyH + 14.f + 2.f * CutSlotPad) + 4.f;   // 304: three rows of cut cards
	constexpr float BodyTextW = L::PanelW - 2.f * ScreenPadX;   // 816: a paragraph on an 880 panel
	constexpr float ControlsW = L::WidePanelW;
	constexpr float FeatureTextW = L::FeatureW - 2.f * L::CardPad;   // 640
	constexpr float MainTextW = L::MainColumnW - 2.f * L::ItemPadX - L::FocusBarW;   // 584

	// The item look: a flat fill in the panel colour at rest, a solid accent fill (and a white bar at its left edge) on the focus, the home blue
	// for the chosen level segment. Device B's button plates replace the flat fills by name when they are in the plugin.
	// Lane A195: the bodies moved onto FJamMenuLook (every screen outside the menu uses them too); these names stay for the menu's call sites.
	FSlateBrush Fill(const FLinearColor& C, bool bEdge) { return L::FillBrush(C, bEdge); }
	FSlateBrush KitOr(const TCHAR* Name, const FSlateBrush& Fallback) { return L::KitOr(Name, Fallback); }
	FButtonStyle ItemStyle(int32 State, bool bSegment) { return L::ItemStyle(State, bSegment); }

	UTextBlock* Txt(UWidgetTree* T, const FString& Str, const FSlateFontInfo& F, const FLinearColor& C)
	{
		UTextBlock* W = T->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		W->SetText(FText::FromString(Str));
		W->SetFont(F);
		W->SetColorAndOpacity(FSlateColor(C));
		return W;
	}
	// A text block that wraps at a fixed width: its desired size is the same before and after the first layout pass (auto-wrap is not), which is
	// what Jam.Menu.FitsTheScreen measures.
	UTextBlock* Wrapped(UTextBlock* Text, float Width)
	{
		Text->SetAutoWrapText(false);
		Text->SetWrapTextAt(Width);
		return Text;
	}
	UBorder* Box(UWidgetTree* T, const FSlateBrush& Brush, FMargin Pad)
	{
		UBorder* B = T->ConstructWidget<UBorder>(UBorder::StaticClass());
		B->SetBrush(Brush);
		B->SetPadding(Pad);
		return B;
	}
	UBorder* Solid(UWidgetTree* T, const FLinearColor& C, FMargin Pad = FMargin(0))
	{
		UBorder* B = T->ConstructWidget<UBorder>(UBorder::StaticClass());
		B->SetBrushColor(C);
		B->SetPadding(Pad);
		return B;
	}
	USizeBox* Sized(UWidgetTree* T, UWidget* Child, float W, float H = 0.f)
	{
		USizeBox* SB = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		if (W > 0.f) SB->SetWidthOverride(W);
		if (H > 0.f) SB->SetHeightOverride(H);
		if (Child)
		{
			SB->AddChild(Child);
			if (USizeBoxSlot* Sl = Cast<USizeBoxSlot>(Child->Slot)) { Sl->SetHorizontalAlignment(HAlign_Fill); Sl->SetVerticalAlignment(VAlign_Fill); }
		}
		return SB;
	}
	// The two team colours side by side: under the header, on a panel's top edge.
	UHorizontalBox* TeamRule(UWidgetTree* T, float Height, float Width = 0.f)
	{
		UHorizontalBox* H = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		for (const FLinearColor& C : { L::Home(), L::Away() })
		{
			UWidget* Piece = Width > 0.f ? static_cast<UWidget*>(Sized(T, Solid(T, C), Width * 0.5f, Height)) : static_cast<UWidget*>(Sized(T, Solid(T, C), 0.f, Height));
			UHorizontalBoxSlot* Sl = H->AddChildToHorizontalBox(Piece);
			if (Width <= 0.f) Sl->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
		return H;
	}
	void ToTop(UWidget* Child)
	{
		if (UButtonSlot* BS = Cast<UButtonSlot>(Child->Slot)) { BS->SetHorizontalAlignment(HAlign_Fill); BS->SetVerticalAlignment(VAlign_Fill); BS->SetPadding(FMargin(0)); }
	}

	// The kit icon of a cut row.
	FString IconFor(const FJamMenuCutRow& R)
	{
		if (R.bSaved) return TEXT("cut-directors");
		if (R.Name == TEXT("hockey.raw")) return TEXT("cut-raw");
		if (R.Name == TEXT("hockey.boost")) return TEXT("cut-boost");
		if (R.Name == TEXT("hockey.arena.large")) return TEXT("cut-large");
		if (R.Name == TEXT("hockey.arena.wide")) return TEXT("cut-wide");
		return TEXT("cut-regular");
	}
	FString ModeSlug(const FString& CutName)
	{
		FString Slug = CutName;
		int32 At = Slug.Find(TEXT("hockey.mode."));
		if (At != INDEX_NONE) Slug = Slug.Mid(At + 12);
		return Slug.Replace(TEXT("-"), TEXT("_"));
	}

	// Lane A187: a match started from this menu in this process: the next menu opens on MAIN (a return from a match is not a new start).
	bool GJamMenuStartedMatch = false;

	const EJamMenuScreen AllScreens[] = { EJamMenuScreen::Main, EJamMenuScreen::Cuts, EJamMenuScreen::Direct, EJamMenuScreen::Settings, EJamMenuScreen::Credits,
		EJamMenuScreen::Controls, EJamMenuScreen::Landing, EJamMenuScreen::Modes, EJamMenuScreen::Season };
	constexpr int32 ScreenCount = UE_ARRAY_COUNT(AllScreens);
}

// ---------------------------------------------------------------- row button

void UJamMenuRowButton::Bind(UJamMenuWidget* InOwner, EJamMenuRowKind InKind, const FString& InPayload)
{
	Owner = InOwner;
	Kind = InKind;
	Payload = InPayload;
	OnClicked.AddUniqueDynamic(this, &UJamMenuRowButton::HandleClicked);
}

void UJamMenuRowButton::HandleClicked()
{
	if (Owner) Owner->OnRowClicked(Kind, Payload);
}

// ---------------------------------------------------------------- screens by name (lane A187)

FString UJamMenuWidget::ScreenName(EJamMenuScreen InScreen)
{
	switch (InScreen)
	{
	case EJamMenuScreen::Main: return TEXT("main");
	case EJamMenuScreen::Cuts: return TEXT("cuts");
	case EJamMenuScreen::Direct: return TEXT("direct");
	case EJamMenuScreen::Settings: return TEXT("settings");
	case EJamMenuScreen::Credits: return TEXT("credits");
	case EJamMenuScreen::Controls: return TEXT("controls");
	case EJamMenuScreen::Landing: return TEXT("landing");
	case EJamMenuScreen::Modes: return TEXT("modes");
	case EJamMenuScreen::Season: return TEXT("season");
	}
	return TEXT("main");
}

bool UJamMenuWidget::ParseScreen(const FString& Name, EJamMenuScreen& Out)
{
	const FString N = Name.ToLower();
	if (N == TEXT("rinks")) { Out = EJamMenuScreen::Cuts; return true; }
	if (N == TEXT("make")) { Out = EJamMenuScreen::Direct; return true; }
	for (const EJamMenuScreen Sc : AllScreens) if (ScreenName(Sc) == N) { Out = Sc; return true; }
	return false;
}

EJamMenuScreen UJamMenuWidget::ParentOf(EJamMenuScreen InScreen)
{
	switch (InScreen)
	{
	case EJamMenuScreen::Landing: return EJamMenuScreen::Landing;
	case EJamMenuScreen::Main: return EJamMenuScreen::Landing;
	case EJamMenuScreen::Cuts:
	case EJamMenuScreen::Direct: return EJamMenuScreen::Modes;
	default: return EJamMenuScreen::Main;
	}
}

EJamMenuScreen UJamMenuWidget::OpeningScreen()
{
	FString LaunchCut;
	if (FParse::Value(FCommandLine::Get(), TEXT("-cut="), LaunchCut) && !LaunchCut.IsEmpty()) return EJamMenuScreen::Main;
	return GJamMenuStartedMatch ? EJamMenuScreen::Main : EJamMenuScreen::Landing;
}

// ---------------------------------------------------------------- menu

UJamMenuWidget* UJamMenuWidget::CreateAndAdd(APlayerController* PC)
{
	if (!PC) return nullptr;
	UJamMenuWidget* W = CreateWidget<UJamMenuWidget>(PC, UJamMenuWidget::StaticClass());
	if (W)
	{
		W->SetIsFocusable(true);
		W->AddToViewport(10);
		W->ShowScreen(OpeningScreen());
		UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU opens on %s"), *ScreenName(W->GetScreen()));
	}
	return W;
}

UJamDesignerSubsystem* UJamMenuWidget::Designer() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UJamDesignerSubsystem>() : nullptr;
}

TSharedRef<SWidget> UJamMenuWidget::RebuildWidget()
{
	BuildTree();
	return Super::RebuildWidget();
}

void UJamMenuWidget::AddItem(UButton* B, UTextBlock* T, UWidget* Bar, bool bSegment)
{
	if (!B) return;
	FItem I;
	I.Button = B;
	I.Text = T;
	I.Bar = Bar;
	I.bSegment = bSegment;
	Items.Add(I);
	B->SetStyle(ItemStyle(1, bSegment));
}

void UJamMenuWidget::AddSub(UButton* B, UTextBlock* T)
{
	for (FItem& I : Items) if (I.Button.Get() == B) { I.Subs.Add(T); I.State = -1; return; }
}

UButton* UJamMenuWidget::MakeBack(EJamMenuScreen Of)
{
	UWidgetTree* T = WidgetTree;
	UButton* B = T->ConstructWidget<UButton>(UButton::StaticClass());
	UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UBorder* Bar = Solid(T, FLinearColor::Transparent);
	AddH(Row, Sized(T, Bar, L::FocusBarW), FMargin(0), false, VAlign_Fill);
	UTextBlock* Label = Txt(T, TEXT("BACK"), L::Font(L::ItemSmall, FName("Bold"), L::TrackLabel), L::Text());
	AddH(Row, Label, FMargin(L::ItemPadX, L::Unit + 4.f, L::ItemPadX + L::FocusBarW, L::Unit + 4.f));
	B->AddChild(Row);
	ToTop(Row);
	B->OnClicked.AddDynamic(this, &UJamMenuWidget::OnBackClicked);
	AddItem(B, Label, Bar);
	const int32 I = static_cast<int32>(Of);
	if (BackButtons.Num() < ScreenCount) BackButtons.SetNum(ScreenCount);
	BackButtons[I] = B;
	return B;
}

UWidget* UJamMenuWidget::OnPanel(UWidget* Body, float Width)
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* V = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	AddV(V, TeamRule(T, 4.f), FMargin(0));
	UBorder* P = Box(T, KitOr(TEXT("T_UI_Panel"), Fill(L::Panel(), true)), FMargin(ScreenPadX, L::Unit * 3.f));
	P->SetContent(Body);
	AddV(V, P, FMargin(0));
	return Sized(T, V, Width);
}

void UJamMenuWidget::BuildTree()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	if (WidgetTree->RootWidget) return;
	UWidgetTree* T = WidgetTree;
	BackButtons.SetNum(ScreenCount);

	UOverlay* Root = T->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("MenuRoot"));
	T->RootWidget = Root;

	// The arena behind the menu: device B's still of the barn under a scrim, or the ink ground with two broad team-colour strokes.
	S::AddO(Root, Solid(T, L::Ink()));
	if (UTexture2D* Back = L::Kit(TEXT("T_UI_MenuBackdrop")))
	{
		UImage* I = T->ConstructWidget<UImage>(UImage::StaticClass());
		FSlateBrush B;
		B.SetResourceObject(Back);
		B.ImageSize = FVector2D(16, 9);   // fills the screen; asks for no size of its own
		I->SetBrush(B);
		I->SetVisibility(ESlateVisibility::HitTestInvisible);
		S::AddO(Root, I);
		S::AddO(Root, Solid(T, L::Scrim()));
	}
	else
	{
		// Lane A195 (U2 evolution 6, item 3): until device B's still, the best without an asset: a gradient from a cold arena blue at the top into the
		// ink (18 bands, a step of one or two levels each: no banding an eye finds), and the two team strokes drifting slowly (NativeTick, DriftSeconds).
		UVerticalBox* Grad = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BackdropGradient"));
		const FLinearColor Top = FLinearColor::FromSRGBColor(FColor(0x17, 0x20, 0x34));
		constexpr int32 Bands = 18;
		for (int32 i = 0; i < Bands; ++i)
		{
			const float A = static_cast<float>(i) / static_cast<float>(Bands - 1);
			UVerticalBoxSlot* Sl = Grad->AddChildToVerticalBox(Solid(T, FMath::Lerp(Top, L::Ink(), FMath::SmoothStep(0.f, 1.f, A))));
			Sl->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
		Grad->SetVisibility(ESlateVisibility::HitTestInvisible);
		S::AddO(Root, Grad);
		int32 k = 0;
		for (const FLinearColor& C : { L::Home(), L::Away() })
		{
			UBorder* Stroke = Solid(T, FLinearColor(C.R, C.G, C.B, 0.12f));
			USizeBox* SB = Sized(T, Stroke, 200.f, 240.f);   // drawn 6x taller by its render scale: a decoration asks the layout for no height
			SB->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
			SB->SetRenderTransformAngle(18.f);
			SB->SetRenderScale(FVector2D(1.f, 6.f));
			SB->SetVisibility(ESlateVisibility::HitTestInvisible);
			S::AddO(Root, SB, HAlign_Right, VAlign_Center, FMargin(0, 0, k == 0 ? 520.f : 260.f, 0));
			BackdropStrokes.Add(SB);
			++k;
		}
	}

	// The chrome: header, the team rule, the screens, the footer.
	UVerticalBox* Chrome = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuChrome"));
	S::AddO(Root, Chrome);
	HeaderBox = BuildHeader();
	AddV(Chrome, HeaderBox, FMargin(0));

	Switcher = T->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("Screens"));
	UVerticalBoxSlot* SwSlot = AddV(Chrome, Switcher, FMargin(0));
	SwSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	// In the enum's order: the switcher's index is the screen's value.
	auto Put = [&](UWidget* W, EHorizontalAlignment H, EVerticalAlignment V, FMargin Pad)
	{
		if (UWidgetSwitcherSlot* Sl = Cast<UWidgetSwitcherSlot>(Switcher->AddChild(W)))
		{
			Sl->SetHorizontalAlignment(H);
			Sl->SetVerticalAlignment(V);
			Sl->SetPadding(Pad);
		}
	};
	const FMargin Below(0, L::Unit * 3.f, 0, 0);
	Put(BuildMain(), HAlign_Fill, VAlign_Fill, FMargin(0, L::Unit * 5.f, 0, 0));
	Put(OnPanel(BuildCuts(), L::WidePanelW), HAlign_Center, VAlign_Top, Below);
	Put(OnPanel(BuildDirect(), L::PanelW), HAlign_Center, VAlign_Top, Below);
	Put(OnPanel(BuildSettings(), L::PanelW), HAlign_Center, VAlign_Top, Below);
	Put(OnPanel(BuildCredits(), L::PanelW), HAlign_Center, VAlign_Top, Below);
	{
		// Lane A195: the rebinding screen is 724 wide: on the 880 panel (the wide one left half of it empty); today's card keeps the wide one.
		UWidget* Ctl = BuildControls();
		Put(OnPanel(Ctl, ControlsScreen ? L::PanelW : ControlsW), HAlign_Center, VAlign_Top, Below);
	}
	Put(BuildLanding(), HAlign_Fill, VAlign_Fill, FMargin(0));
	Put(BuildModes(), HAlign_Center, VAlign_Top, Below);
	Put(BuildSeason(), HAlign_Center, VAlign_Top, Below);
	Switcher->SetActiveWidgetIndex(0);

	AddV(Chrome, BuildFooter(), FMargin(0));
}

UWidget* UJamMenuWidget::BuildHeader()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* V = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Header"));
	UBorder* Bar = Box(T, KitOr(TEXT("T_UI_HeaderBar"), FSlateBrush(FSlateColorBrush(FLinearColor(L::Ink().R, L::Ink().G, L::Ink().B, 0.94f)))), FMargin(L::SideMargin, 0));
	UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	if (UTexture2D* Logo = L::Kit(TEXT("T_UI_Logo_Hokej"))) AddH(Row, S::Picture(T, Logo, 168.f, 44.f), FMargin(0, 0, L::Unit * 3.f, 0));
	else AddH(Row, Txt(T, TEXT("HOKEJ"), L::Font(L::Title, FName("Black"), 160), L::Text()), FMargin(0, 0, L::Unit * 3.f, 0));
	AddH(Row, Sized(T, Solid(T, FLinearColor(1, 1, 1, 0.25f)), 2.f, 40.f), FMargin(0, 0, L::Unit * 3.f, 0));
	HeaderTitle = Txt(T, TEXT("HOME"), L::Font(L::Item - 4, FName("Bold"), L::TrackLabel), L::TextDim());
	AddH(Row, HeaderTitle, FMargin(0), true);
	AddH(Row, Txt(T, TEXT("LEVEL"), L::Font(L::Caption, FName("Bold"), L::TrackLabel), L::TextDim()), FMargin(0, 0, L::Unit * 2.f, 0));
	UBorder* Chip = Box(T, Fill(L::Chosen(), false), FMargin(L::Unit * 2.f, L::Unit * 0.5f));
	HeaderLevel = Txt(T, TEXT("ROOKIE"), L::Font(L::Body - 2, FName("Black"), L::TrackLabel), L::TextOnFocus());
	Chip->SetContent(HeaderLevel);
	AddH(Row, Chip, FMargin(0));
	Bar->SetContent(Row);
	Bar->SetVerticalAlignment(VAlign_Center);
	AddV(V, Sized(T, Bar, 0.f, L::HeaderH), FMargin(0));
	AddV(V, TeamRule(T, 4.f), FMargin(0));
	return V;
}

UWidget* UJamMenuWidget::BuildFooter()
{
	UWidgetTree* T = WidgetTree;
	UBorder* Bar = Solid(T, FLinearColor(L::Ink().R, L::Ink().G, L::Ink().B, 0.94f), FMargin(L::SideMargin, 0));
	FooterHints = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("FooterHints"));
	Bar->SetContent(FooterHints);
	Bar->SetVerticalAlignment(VAlign_Center);
	return Sized(T, Bar, 0.f, L::FooterH);
}

UWidget* UJamMenuWidget::BuildLanding()
{
	UWidgetTree* T = WidgetTree;
	// The whole screen is one button: Enter or A on it, a click anywhere on it, is the press; any other key reaches NativeOnKeyDown.
	LandingButton = T->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LandingButton"));
	FButtonStyle Clear;
	const FSlateBrush None = FSlateNoResource();
	Clear.SetNormal(None);
	Clear.SetHovered(None);
	Clear.SetPressed(None);
	Clear.SetDisabled(None);
	Clear.SetNormalPadding(FMargin(0));
	Clear.SetPressedPadding(FMargin(0));
	LandingButton->SetStyle(Clear);
	LandingButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnLandingClicked);

	UVerticalBox* V = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LandingScreen"));
	if (UTexture2D* Logo = L::Kit(TEXT("T_UI_Logo_Hokej")))
	{
		TitlePicture = S::Picture(T, Logo, 1120.f);
		AddV(V, TitlePicture, FMargin(0), HAlign_Center);
	}
	else
	{
		UTextBlock* Title = Txt(T, TEXT("HOKEJ"), L::Font(L::Hero, FName("Black"), 120), L::Text());
		Title->SetShadowOffset(FVector2D(0, 6));
		Title->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.55f));
		TitlePicture = Title;
		AddV(V, Title, FMargin(0), HAlign_Center);
	}
	AddV(V, TeamRule(T, 6.f, 480.f), FMargin(0, L::Unit, 0, L::Unit * 3.f), HAlign_Center);
	UTextBlock* Line = Txt(T, UJamMenuFlow::BrandLine().ToUpper(), L::Font(L::Body + 6, FName("Bold"), 160), L::Text());
	AddV(V, Line, FMargin(0, 0, 0, L::Unit * 14.f), HAlign_Center);
	UTextBlock* Prompt = Txt(T, TEXT("PRESS ENTER"), L::Font(L::Item, FName("Black"), L::TrackWide), L::Text());
	LandingPrompt = Prompt;
	AddV(V, Prompt, FMargin(0), HAlign_Center);
	AddV(V, Txt(T, TEXT("OR A, OR A CLICK"), L::Font(L::Caption, FName("Bold"), 160), L::TextDim()), FMargin(0, L::Unit, 0, 0), HAlign_Center);
	LandingButton->AddChild(V);
	if (UButtonSlot* BS = Cast<UButtonSlot>(V->Slot)) { BS->SetHorizontalAlignment(HAlign_Center); BS->SetVerticalAlignment(VAlign_Center); BS->SetPadding(FMargin(0)); }
	return LandingButton;
}

UWidget* UJamMenuWidget::BuildMain()
{
	UWidgetTree* T = WidgetTree;
	UHorizontalBox* Screen0 = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MainScreen"));
	UVerticalBox* Col = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainColumn"));
	AddH(Screen0, Sized(T, Col, L::MainColumnW), FMargin(L::SideMargin, 0, 0, 0), false, VAlign_Top);
	MainLabels.Reset();

	// An entry of the main column: the focus bar, the label, a sub line.
	auto Entry = [&](const FString& Label, int32 Size, const FString& SubText, UTextBlock*& OutLabel, UTextBlock*& OutSub, const TCHAR* Name) -> UButton*
	{
		UButton* B = T->ConstructWidget<UButton>(UButton::StaticClass(), Name);
		UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UBorder* Bar = Solid(T, FLinearColor::Transparent);
		AddH(Row, Sized(T, Bar, L::FocusBarW), FMargin(0), false, VAlign_Fill);
		UVerticalBox* Words = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		OutLabel = Txt(T, Label, L::Font(Size, FName("Black"), L::TrackLabel), L::Text());
		AddV(Words, OutLabel, FMargin(0));
		OutSub = Wrapped(Txt(T, SubText, L::Font(L::Caption, FName("Bold"), L::TrackLabel), L::TextDim()), MainTextW);
		AddV(Words, OutSub, FMargin(0, 2, 0, 0));
		AddH(Row, Words, FMargin(L::ItemPadX, L::ItemPadY - 4.f, L::ItemPadX, L::ItemPadY - 2.f), true);
		B->AddChild(Row);
		ToTop(Row);
		AddItem(B, OutLabel, Bar);
		AddSub(B, OutSub);
		MainLabels.Add(OutLabel);
		return B;
	};

	// PLAY NOW: the first press. One match on the regular cut at the lit level; MAIN opens on it, so Enter / A starts it.
	{
		UTextBlock* Sub = nullptr;
		UTextBlock* Head = nullptr;
		PlayNowButton = Entry(TEXT("PLAY NOW"), L::Display, UJamMenuFlow::FirstMatchLine(TEXT("rookie")), Head, Sub, TEXT("PlayNowButton"));
		PlayNowText = Head;
		PlayNowSub = Sub;
		PlayNowButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnPlayNowClicked);
		AddV(Col, PlayNowButton, FMargin(0));
		// Evolution 3: left and right on PLAY NOW step the level (the focus stays on the button).
		FCustomWidgetNavigationDelegate Nav;
		Nav.BindDynamic(this, &UJamMenuWidget::OnPlayNowNavigate);
		PlayNowButton->SetNavigationRuleCustom(EUINavigation::Left, Nav);
		PlayNowButton->SetNavigationRuleCustom(EUINavigation::Right, Nav);
	}

	// Lane A144 (evolution 3): the level switch under PLAY NOW: three segments, the level in force lit in the home blue; a click picks one; the keys
	// and the pad step it from PLAY NOW itself. The segments take a click and no focus.
	{
		UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("LevelSwitch"));
		LevelButtons.Reset();
		LevelTexts.Reset();
		for (const EJamMenuDifficulty D : { EJamMenuDifficulty::Rookie, EJamMenuDifficulty::Pro, EJamMenuDifficulty::AllStar })
		{
			UButton* B = T->ConstructWidget<UButton>(UButton::StaticClass());
			B->IsFocusable = false;
			UTextBlock* Label = Txt(T, FJamSettings::DifficultyLabel(D), L::Font(L::Body - 2, FName("Black"), 100), L::TextDim());
			Label->SetJustification(ETextJustify::Center);
			UBorder* Pad = Solid(T, FLinearColor::Transparent, FMargin(L::Unit, L::Unit + 2.f));
			Pad->SetContent(Label);
			Pad->SetHorizontalAlignment(HAlign_Center);
			B->AddChild(Pad);
			ToTop(Pad);
			switch (D)
			{
			case EJamMenuDifficulty::Rookie: B->OnClicked.AddDynamic(this, &UJamMenuWidget::OnLevelRookieClicked); break;
			case EJamMenuDifficulty::Pro: B->OnClicked.AddDynamic(this, &UJamMenuWidget::OnLevelProClicked); break;
			default: B->OnClicked.AddDynamic(this, &UJamMenuWidget::OnLevelAllStarClicked); break;
			}
			LevelButtons.Add(B);
			LevelTexts.Add(Label);
			AddItem(B, Label, nullptr, true);
			AddH(Row, B, FMargin(D == EJamMenuDifficulty::Rookie ? 0.f : L::Gap * 0.5f, 0, D == EJamMenuDifficulty::AllStar ? 0.f : L::Gap * 0.5f, 0), true, VAlign_Fill);
		}
		AddV(Col, Row, FMargin(0, L::Gap, 0, 0));
		LevelMeaningText = Wrapped(Txt(T, UJamMenuFlow::LevelMeaning(TEXT("rookie")), L::Font(L::Caption + 1, FName("Medium")), L::TextDim()), L::MainColumnW);
		AddV(Col, LevelMeaningText, FMargin(0, L::Gap, 0, L::Unit * 3.f));
	}

	UTextBlock* Label = nullptr;
	UTextBlock* Sub = nullptr;
	SeasonButton = Entry(TEXT("SEASON"), L::Item, TEXT("EIGHT TAKES"), Label, Sub, TEXT("SeasonButton"));
	SeasonText = Sub;
	SeasonButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnSeasonClicked);
	AddV(Col, SeasonButton, FMargin(0, 0, 0, L::Gap));
	ModesButton = Entry(TEXT("MODES"), L::Item, TEXT("TWELVE WAYS TO PLAY, ALL RINKS, MAKE YOUR OWN"), Label, Sub, TEXT("ModesButton"));
	ModesButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnModesClicked);
	AddV(Col, ModesButton, FMargin(0, 0, 0, L::Gap));
	// Lane A9's lobby: one entry that runs its console command (JamGame does not depend on JamNet).
	FriendButton = Entry(TEXT("PLAY WITH A FRIEND"), L::Item, TEXT("ONE HOSTS, ONE JOINS BY CODE"), Label, Sub, TEXT("FriendButton"));
	FriendButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnFriendClicked);
	AddV(Col, FriendButton, FMargin(0, 0, 0, L::Unit * 3.f));

	// The four that are not a way to play share one row; CONTROLS first.
	UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MainSmallRow"));
	auto AddSmall = [&](TObjectPtr<UButton>& Target, const FString& Text, bool bLast)
	{
		UButton* B = T->ConstructWidget<UButton>(UButton::StaticClass());
		UVerticalBox* Stack = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* Words = Txt(T, Text, L::Font(L::ItemSmall - 2, FName("Bold"), L::TrackLabel), L::Text());
		Words->SetJustification(ETextJustify::Center);
		AddV(Stack, Words, FMargin(L::Unit, L::ItemPadY - 2.f, L::Unit, L::ItemPadY - 6.f), HAlign_Center);
		// The focus bar of a small item runs along its foot.
		UBorder* Bar = Solid(T, FLinearColor::Transparent);
		AddV(Stack, Sized(T, Bar, 0.f, 4.f), FMargin(0));
		B->AddChild(Stack);
		ToTop(Stack);
		Target = B;
		AddItem(B, Words, Bar);
		MainLabels.Add(Words);
		AddH(Row, B, FMargin(0, 0, bLast ? 0.f : L::Gap, 0), true, VAlign_Fill);
	};
	AddSmall(ControlsButton, TEXT("CONTROLS"), false);
	ControlsButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnControlsClicked);
	AddSmall(SettingsButton, TEXT("SETTINGS"), false);
	SettingsButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnSettingsClicked);
	AddSmall(CreditsButton, TEXT("CREDITS"), false);
	CreditsButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnCreditsClicked);
	AddSmall(QuitButton, TEXT("QUIT"), true);
	QuitButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnQuitClicked);
	AddV(Col, Row, FMargin(0));

	MainStatus = Wrapped(Txt(T, TEXT(""), L::Font(L::Body, FName("Regular")), L::Away()), L::MainColumnW);
	AddV(Col, MainStatus, FMargin(0, L::Unit * 2.f, 0, 0));

	// The feature panel on the right: what the focused entry is, in two lines.
	UVerticalBox* F = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Feature"));
	AddV(F, TeamRule(T, 4.f), FMargin(0));
	UBorder* P = Box(T, KitOr(TEXT("T_UI_Panel"), Fill(L::Panel(), true)), FMargin(L::CardPad, L::CardPad - 4.f));
	UVerticalBox* PV = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	FeatureTitle = Txt(T, TEXT("PLAY NOW"), L::Font(L::Title, FName("Black"), L::TrackLabel), L::Text());
	AddV(PV, FeatureTitle, FMargin(0, 0, 0, L::Unit * 2.f));
	FeatureBody = Wrapped(Txt(T, TEXT(""), L::Font(L::Body, FName("Regular")), L::Text()), FeatureTextW);
	FeatureBody->SetLineHeightPercentage(1.15f);
	AddV(PV, FeatureBody, FMargin(0));
	P->SetContent(PV);
	AddV(F, P, FMargin(0));
	UHorizontalBoxSlot* Gap = AddH(Screen0, T->ConstructWidget<USpacer>(USpacer::StaticClass()), FMargin(0), true);
	(void)Gap;
	AddH(Screen0, Sized(T, F, L::FeatureW), FMargin(0, 0, L::SideMargin, 0), false, VAlign_Top);
	return Screen0;
}

UWidget* UJamMenuWidget::BuildModes()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* V = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ModesScreen"));
	ModeGrid = T->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("ModeGrid"));
	ModeGrid->SetSlotPadding(FMargin(L::ModeSlotPad));
	AddV(V, ModeGrid, FMargin(0));
	// The focused card's facts, one line under the grid.
	UBorder* DetailBar = Box(T, Fill(FLinearColor(L::Ink().R, L::Ink().G, L::Ink().B, 0.80f), false), FMargin(L::ItemPadX, L::Unit + 4.f));
	ModeDetail = Wrapped(Txt(T, TEXT(""), L::Font(L::Body - 1, FName("Bold"), 20), L::Text()), ModeDetailWidth());
	DetailBar->SetContent(ModeDetail);
	AddV(V, DetailBar, FMargin(L::ModeSlotPad, L::Unit, L::ModeSlotPad, L::Unit * 2.f));
	AddV(V, MakeBack(EJamMenuScreen::Modes), FMargin(L::ModeSlotPad, 0), HAlign_Left);
	return Sized(T, V, L::ModesW);
}

UWidget* UJamMenuWidget::BuildSeason()
{
	UWidgetTree* T = WidgetTree;
	UHorizontalBox* H = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("SeasonScreen"));
	UVerticalBox* Left = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	// CONTINUE: the next take, the season's one press.
	ContinueButton = T->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ContinueButton"));
	{
		UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UBorder* Bar = Solid(T, FLinearColor::Transparent);
		AddH(Row, Sized(T, Bar, L::FocusBarW), FMargin(0), false, VAlign_Fill);
		ContinueText = Txt(T, TEXT("CONTINUE"), L::Font(L::Item, FName("Black"), L::TrackLabel), L::Text());
		AddH(Row, ContinueText, FMargin(L::ItemPadX, L::ItemPadY, L::ItemPadX, L::ItemPadY), true);
		ContinueButton->AddChild(Row);
		ToTop(Row);
		ContinueButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnContinueSeasonClicked);
		AddItem(ContinueButton, ContinueText, Bar);
	}
	AddV(Left, ContinueButton, FMargin(0, 0, 0, L::Unit * 2.f));
	SeasonList = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SeasonList"));
	AddV(Left, SeasonList, FMargin(0, 0, 0, L::Unit * 2.f));
	AddV(Left, MakeBack(EJamMenuScreen::Season), FMargin(0), HAlign_Left);
	AddH(H, Sized(T, Left, 720.f), FMargin(0), false, VAlign_Top);

	UVerticalBox* R = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	AddV(R, TeamRule(T, 4.f), FMargin(0));
	UBorder* P = Box(T, KitOr(TEXT("T_UI_Panel"), Fill(L::Panel(), true)), FMargin(L::CardPad, L::CardPad - 4.f));
	UVerticalBox* PV = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	SeasonSummary = Wrapped(Txt(T, TEXT(""), L::Font(L::Title - 4, FName("Black"), L::TrackLabel), L::Text()), 512.f - 2.f * L::CardPad);
	AddV(PV, SeasonSummary, FMargin(0, 0, 0, L::Unit * 2.f));
	SeasonDetail = Wrapped(Txt(T, TEXT(""), L::Font(L::Body, FName("Regular")), L::Text()), 512.f - 2.f * L::CardPad);
	SeasonDetail->SetLineHeightPercentage(1.15f);
	AddV(PV, SeasonDetail, FMargin(0));
	P->SetContent(PV);
	AddV(R, P, FMargin(0));
	AddH(H, Sized(T, R, 512.f), FMargin(L::Unit * 6.f, 0, 0, 0), false, VAlign_Top);
	return Sized(T, H, L::SeasonW);
}

UWidget* UJamMenuWidget::BuildCuts()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box0 = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CutsScreen"));
	AddV(Box0, Wrapped(Txt(T, TEXT("The shipped rinks, a rink you made and saved, and the takes of the season you have played."), L::Font(L::Body, FName("Regular")), L::TextDim()),
		L::WidePanelW - 2.f * ScreenPadX), FMargin(0, 0, 0, L::Unit * 2.f));
	// Lane A144: the cuts are compact cards, three a row.
	CutList = T->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("CutList"));
	CutGrid = T->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("CutGrid"));
	CutGrid->SetSlotPadding(FMargin(CutSlotPad));
	CutList->AddChild(CutGrid);
	USizeBox* ListBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ListBox->SetMaxDesiredHeight(CutListH);
	ListBox->AddChild(CutList);
	AddV(Box0, ListBox, FMargin(0));
	CutsStatus = Wrapped(Txt(T, TEXT(""), L::Font(L::Body, FName("Regular")), L::TextDim()), L::WidePanelW - 2.f * ScreenPadX);
	CutsStatus->SetVisibility(ESlateVisibility::Collapsed);   // an error line only
	AddV(Box0, CutsStatus, FMargin(0, 6, 0, 0));
	AddV(Box0, MakeBack(EJamMenuScreen::Cuts), FMargin(0, L::Unit * 2.f, 0, 0), HAlign_Left);
	return Box0;
}

UWidget* UJamMenuWidget::BuildDirect()
{
	// MAKE YOUR OWN: the Designer's screen as it was (lane A8), under the front end's header.
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box0 = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DirectScreen"));
	UTextBlock* Intro = Txt(T, TEXT("Tell the Designer in one sentence how you want this game. It rewrites the rules, the ice, the hits, the look."), L::Font(L::Body, FName("Regular")), L::TextDim());
	Wrapped(Intro, BodyTextW);
	AddV(Box0, Intro, FMargin(0, 0, 0, 8));

	UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	SentenceBox = T->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("SentenceBox"));
	SentenceBox->SetHintText(FText::FromString(TEXT("make it faster and meaner")));
	SentenceBox->SetMinDesiredWidth(520.f);
	SentenceBox->WidgetStyle.SetFont(L::Font(L::Body, FName("Regular")));
	SentenceBox->WidgetStyle.SetPadding(FMargin(12, 8));
	SentenceBox->WidgetStyle.SetBackgroundColor(FLinearColor(0.16f, 0.17f, 0.20f, 1.f));
	SentenceBox->WidgetStyle.SetForegroundColor(L::Text());
	SentenceBox->WidgetStyle.SetFocusedForegroundColor(L::Text());
	SentenceBox->OnTextCommitted.AddDynamic(this, &UJamMenuWidget::OnSentenceCommitted);
	AddH(Row, SentenceBox, FMargin(0, 0, 8, 0), true);
	auto Small = [&](const FString& Text, UTextBlock*& OutText) -> UButton*
	{
		UButton* B = T->ConstructWidget<UButton>(UButton::StaticClass());
		OutText = Txt(T, Text, L::Font(L::ItemSmall - 2, FName("Bold"), L::TrackLabel), L::Text());
		UBorder* Pad = Solid(T, FLinearColor::Transparent, FMargin(L::Unit * 2.f, L::Unit + 2.f));
		Pad->SetContent(OutText);
		B->AddChild(Pad);
		ToTop(Pad);
		AddItem(B, OutText);
		return B;
	};
	UTextBlock* Label = nullptr;
	RollButton = Small(TEXT("ROLL"), Label);
	RollButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnRollClicked);
	AddH(Row, RollButton, FMargin(0));
	AddV(Box0, Row, FMargin(0, 0, 0, 4));

	DirectStatus = Wrapped(Txt(T, TEXT(""), L::Font(L::Body, FName("Regular")), L::Away()), BodyTextW);
	AddV(Box0, DirectStatus, FMargin(0, 2, 0, 2));
	DirectHint = Wrapped(Txt(T, TEXT(""), L::Font(L::Caption, FName("Medium")), L::TextDim()), BodyTextW);
	AddV(Box0, DirectHint, FMargin(0, 0, 0, 4));

	// The result: the game's name, its three biggest changes, and what to do with it.
	ResultBox = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ResultBox"));
	UBorder* ResultPlate = Box(T, Fill(L::Ink(), true), FMargin(16, 12));
	ResultPlate->SetContent(ResultBox);
	ResultName = Txt(T, TEXT(""), L::Font(L::Item - 4, FName("Black"), L::TrackLabel), L::Text());
	AddV(ResultBox, ResultName, FMargin(0, 0, 0, 2));
	ResultChanges = Wrapped(Txt(T, TEXT(""), L::Font(L::Body, FName("Regular")), L::Text()), BodyTextW - 32.f);
	AddV(ResultBox, ResultChanges, FMargin(0, 0, 0, 10));
	UHorizontalBox* Actions = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	PlayResultButton = Small(TEXT("PLAY"), Label);
	PlayResultButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnPlayResultClicked);
	AddH(Actions, PlayResultButton, FMargin(0, 0, 8, 0));
	UTextBlock* SaveTxt = nullptr;
	SaveResultButton = Small(TEXT("SAVE AS MY DIRECTOR'S CUT"), SaveTxt);
	SaveResultText = SaveTxt;
	SaveResultButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnSaveResultClicked);
	AddH(Actions, SaveResultButton, FMargin(0, 0, 8, 0));
	TryAgainButton = Small(TEXT("TRY AGAIN"), Label);
	TryAgainButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnTryAgainClicked);
	AddH(Actions, TryAgainButton, FMargin(0));
	AddV(ResultBox, Actions, FMargin(0));
	AddV(Box0, ResultPlate, FMargin(0, 4, 0, 4));
	ResultPlate->SetVisibility(ESlateVisibility::Collapsed);
	ResultBox->SetVisibility(ESlateVisibility::Collapsed);

	AddV(Box0, Txt(T, TEXT("OR PICK A SENTENCE THE DESIGNER ALREADY SHOT"), L::Font(L::Caption, FName("Bold"), L::TrackLabel), L::TextDim()), FMargin(0, 8, 0, 2));
	SentenceList = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SentenceList"));
	UScrollBox* SentenceScroll = T->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("SentenceScroll"));
	SentenceScroll->AddChild(SentenceList);
	USizeBox* SentenceClip = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SentenceClip->SetMaxDesiredHeight(190.f);
	SentenceClip->AddChild(SentenceScroll);
	AddV(Box0, SentenceClip, FMargin(0));
	AddV(Box0, MakeBack(EJamMenuScreen::Direct), FMargin(0, L::Unit * 2.f, 0, 0), HAlign_Left);
	return Box0;
}

UWidget* UJamMenuWidget::BuildSettings()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box0 = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsScreen"));
	SettingsPanel = CreateWidget<UJamSettingsWidget>(this, UJamSettingsWidget::StaticClass());
	if (SettingsPanel) AddV(Box0, SettingsPanel, FMargin(0));
	AddV(Box0, MakeBack(EJamMenuScreen::Settings), FMargin(0, L::Unit * 2.f, 0, 0), HAlign_Left);
	return Box0;
}

UWidget* UJamMenuWidget::BuildCredits()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box0 = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CreditsScreen"));
	UTextBlock* Body = Txt(T,
		TEXT("HOKEJ. Team Hokej, 2026 Epic MegaJam, theme Director's Cut.\n\n")
		TEXT("Built with Unreal Engine.\n\n")
		TEXT("One human directed two machines of agents; the agents wrote the game, its modes and this menu. ")
		TEXT("The Designer (MAKE YOUR OWN) turns your sentence into a game through the Claude API with a key you type yourself; without a key it answers from games it made earlier.\n\n")
		TEXT("Tools used: Unreal Engine 5.9, Claude Code and the Claude API, Codex image generation for the menu and HUD art, Node.js, Git with LFS, Meshy for generated figures, PowerShell.\n\n")
		TEXT("No goalies harmed. Skaters flattened. No logos."),
		L::Font(L::Body, FName("Regular")), L::Text());
	Wrapped(Body, BodyTextW);
	AddV(Box0, Body, FMargin(0, 0, 0, 6));
	AddV(Box0, MakeBack(EJamMenuScreen::Credits), FMargin(0, L::Unit * 2.f, 0, 0), HAlign_Left);
	return Box0;
}

// Lane A144 (track U2): the controls, one screen from MAIN: every verb with its mouse and keys, its pad button and one line on what it does
// (UJamMenuFlow::ControlRows; Jam.Menu.ControlsComplete holds it against UJamInputMap's bindings). Lane A187: when the controls lane's rebinding
// screen is in the tree (MakeControlsScreen exists), CONTROLS opens that instead.
UWidget* UJamMenuWidget::BuildControls()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box0 = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ControlsScreen"));
	ControlsScreen = JamMenuControlsHook::Make<UJamSettingsWidget>(this, ControlsFirst);
	if (ControlsScreen)
	{
		// Its own BACK and Esc fire OnBack; the menu adds no second BACK.
		AddV(Box0, ControlsScreen, FMargin(0));
		UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU controls: the rebinding screen (MakeControlsScreen)"));
		return Box0;
	}
	const float VerbW = 250.f, KeysW = 320.f, PadW = 190.f;
	const float NoteW = ControlsW - 2.f * ScreenPadX - VerbW - KeysW - PadW;   // 456
	auto Cell = [&](UHorizontalBox* Row, UTextBlock* Text, float Width)
	{
		USizeBox* Size = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(Width);
		Size->AddChild(Wrapped(Text, Width - 8.f));
		AddH(Row, Size, FMargin(0), false, VAlign_Top);
	};
	UHorizontalBox* Head = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	const FSlateFontInfo HeadFont = L::Font(L::Caption, FName("Bold"), L::TrackLabel);
	Cell(Head, Txt(T, TEXT(""), HeadFont, L::TextDim()), VerbW);
	Cell(Head, Txt(T, TEXT("MOUSE AND KEYS"), HeadFont, L::TextDim()), KeysW);
	Cell(Head, Txt(T, TEXT("PAD"), HeadFont, L::TextDim()), PadW);
	Cell(Head, Txt(T, TEXT("WHAT IT DOES"), HeadFont, L::TextDim()), NoteW);
	AddV(Box0, Head, FMargin(0, 0, 0, 2));
	for (const FJamControlRow& R : UJamMenuFlow::ControlRows())
	{
		UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		Cell(Row, Txt(T, R.Verb, L::Font(L::Body, FName("Black")), L::Away()), VerbW);
		Cell(Row, Txt(T, R.Keys, L::Font(L::Body, FName("Bold")), L::Text()), KeysW);
		Cell(Row, Txt(T, R.Pad, L::Font(L::Body, FName("Medium")), L::Text()), PadW);
		Cell(Row, Txt(T, R.Note, L::Font(L::Body - 2, FName("Regular")), L::TextDim()), NoteW);
		AddV(Box0, Row, FMargin(0, 3));
	}
	AddV(Box0, Wrapped(Txt(T, TEXT("In a match Esc shows this list again. The pad is mapped; no person has play-tested it yet."), L::Font(L::Caption, FName("Medium")), L::TextDim()),
		ControlsW - 2.f * ScreenPadX), FMargin(0, 8, 0, 0));
	AddV(Box0, MakeBack(EJamMenuScreen::Controls), FMargin(0, L::Unit * 2.f, 0, 0), HAlign_Left);
	return Box0;
}

// ---------------------------------------------------------------- screens

FString UJamMenuWidget::GetHeaderTitle() const { return HeaderTitle ? HeaderTitle->GetText().ToString() : FString(); }

void UJamMenuWidget::ShowScreen(EJamMenuScreen InScreen)
{
	Screen = InScreen;
	if (!Switcher) return;
	Switcher->SetActiveWidgetIndex(static_cast<int32>(Screen));
	if (HeaderBox) HeaderBox->SetVisibility(Screen == EJamMenuScreen::Landing ? ESlateVisibility::Hidden : ESlateVisibility::SelfHitTestInvisible);
	static const TMap<EJamMenuScreen, FString> Titles = {
		{ EJamMenuScreen::Main, TEXT("HOME") }, { EJamMenuScreen::Cuts, TEXT("MODES  /  ALL RINKS") }, { EJamMenuScreen::Direct, TEXT("MODES  /  MAKE YOUR OWN") },
		{ EJamMenuScreen::Settings, TEXT("SETTINGS") }, { EJamMenuScreen::Credits, TEXT("CREDITS") }, { EJamMenuScreen::Controls, TEXT("CONTROLS") },
		{ EJamMenuScreen::Landing, TEXT("") }, { EJamMenuScreen::Modes, TEXT("MODES") }, { EJamMenuScreen::Season, TEXT("SEASON") } };
	if (HeaderTitle) HeaderTitle->SetText(FText::FromString(Titles.FindRef(Screen)));
	// A screen comes in: a fade and a short slide, under 200 ms (FJamMenuLook::TransitionSeconds).
	TransitionT = 0.f;
	Switcher->SetRenderOpacity(0.f);
	switch (Screen)
	{
	case EJamMenuScreen::Main:
		RefreshSeason();
		break;
	case EJamMenuScreen::Cuts:
		if (CutsStatus) { CutsStatus->SetText(FText::GetEmpty()); CutsStatus->SetVisibility(ESlateVisibility::Collapsed); }
		RefreshCuts();
		break;
	case EJamMenuScreen::Modes:
		RefreshModes();
		break;
	case EJamMenuScreen::Season:
		RefreshSeasonScreen();
		break;
	case EJamMenuScreen::Direct:
		RefreshDirect();
		break;
	case EJamMenuScreen::Settings:
		if (SettingsPanel) SettingsPanel->Refresh();
		break;
	default:
		break;
	}
	RefreshLevelSwitch();
	PendingFocus = GetHomeFocus(Screen);
	FeatureFor = nullptr;
	RefreshFooter();
}

UWidget* UJamMenuWidget::GetHomeFocus(EJamMenuScreen InScreen) const
{
	auto Back = [&]() -> UWidget* { const int32 I = static_cast<int32>(InScreen); return BackButtons.IsValidIndex(I) ? BackButtons[I].Get() : nullptr; };
	switch (InScreen)
	{
	case EJamMenuScreen::Landing: return LandingButton;
	case EJamMenuScreen::Main: return PlayNowButton;
	case EJamMenuScreen::Cuts: return CutGrid && CutGrid->GetChildrenCount() ? CutGrid->GetChildAt(0) : Back();
	case EJamMenuScreen::Modes: return ModeGrid && ModeGrid->GetChildrenCount() ? ModeGrid->GetChildAt(0) : Back();
	case EJamMenuScreen::Season:
		if (ContinueButton && ContinueButton->GetIsEnabled()) return ContinueButton;
		return Back();
	case EJamMenuScreen::Direct: return RollButton;
	case EJamMenuScreen::Settings: return SettingsPanel && SettingsPanel->GetFirstFocusable() ? SettingsPanel->GetFirstFocusable() : Back();
	case EJamMenuScreen::Controls:
		if (ControlsScreen && ControlsFirst) if (UWidget* F = ControlsFirst()) return F;
		return Back();
	default: return Back();
	}
}

void UJamMenuWidget::GoBack()
{
	const EJamMenuScreen To = ParentOf(Screen);
	if (To == Screen) return;
	LastBackFrame = GFrameCounter;
	const EJamMenuScreen From = Screen;
	ShowScreen(To);
	// Back lands on the entry it left from: MODES' card from a card's screen, the main entry from a main screen.
	if (To == EJamMenuScreen::Main)
	{
		UWidget* Entry = From == EJamMenuScreen::Season ? static_cast<UWidget*>(SeasonButton.Get()) : From == EJamMenuScreen::Modes ? static_cast<UWidget*>(ModesButton.Get())
			: From == EJamMenuScreen::Controls ? static_cast<UWidget*>(ControlsButton.Get()) : From == EJamMenuScreen::Settings ? static_cast<UWidget*>(SettingsButton.Get())
			: From == EJamMenuScreen::Credits ? static_cast<UWidget*>(CreditsButton.Get()) : nullptr;
		if (Entry) PendingFocus = Entry;
	}
	else if (To == EJamMenuScreen::Modes && ModeGrid)
	{
		for (UWidget* W : ModeGrid->GetAllChildren())
			if (const UJamMenuRowButton* R = Cast<UJamMenuRowButton>(W)) if (R->Kind == EJamMenuRowKind::Open && R->Payload == ScreenName(From)) PendingFocus = W;
	}
	UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU back from=%s to=%s"), *ScreenName(From), *ScreenName(To));
}

void UJamMenuWidget::PressStart()
{
	if (Screen != EJamMenuScreen::Landing) return;
	ShowScreen(EJamMenuScreen::Main);
	UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU landing press: main, focus=play_now"));
}

void UJamMenuWidget::OnLandingClicked() { PressStart(); }
void UJamMenuWidget::OnModesClicked() { ShowScreen(EJamMenuScreen::Modes); }

// ---- the footer's live key hints

namespace
{
	struct FHint { const TCHAR* Key; const TCHAR* Action; };
}

void UJamMenuWidget::RefreshFooter()
{
	if (!FooterHints || !WidgetTree) return;
	TArray<FHint> Hints;
	const UWidget* Focus = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		for (const FItem& I : Items) if (I.Button.IsValid() && I.Button->HasUserFocus(PC)) { Focus = I.Button.Get(); break; }
	}
	if (!Focus) Focus = PendingFocus.Get();
	const UJamMenuRowButton* Row = Cast<UJamMenuRowButton>(Focus);
	switch (Screen)
	{
	case EJamMenuScreen::Landing:
		Hints = { { TEXT("ENTER / A"), TEXT("START") }, { TEXT("CLICK"), TEXT("START") } };
		break;
	case EJamMenuScreen::Main:
		if (Focus == PlayNowButton.Get()) Hints = { { TEXT("ENTER / A"), TEXT("PLAY") }, { TEXT("LEFT / RIGHT"), TEXT("LEVEL") }, { TEXT("UP / DOWN"), TEXT("MOVE") }, { TEXT("ESC / B"), TEXT("TITLE") } };
		else Hints = { { TEXT("ENTER / A"), TEXT("SELECT") }, { TEXT("ARROWS / STICK"), TEXT("MOVE") }, { TEXT("ESC / B"), TEXT("TITLE") } };
		break;
	case EJamMenuScreen::Modes:
	case EJamMenuScreen::Cuts:
		Hints = { { TEXT("ENTER / A"), Row && Row->Kind == EJamMenuRowKind::Open ? TEXT("OPEN") : TEXT("PLAY") }, { TEXT("ARROWS / STICK"), TEXT("MOVE") }, { TEXT("ESC / B"), TEXT("BACK") } };
		break;
	case EJamMenuScreen::Season:
		Hints = { { TEXT("ENTER / A"), TEXT("PLAY") }, { TEXT("UP / DOWN"), TEXT("MOVE") }, { TEXT("ESC / B"), TEXT("BACK") } };
		break;
	case EJamMenuScreen::Direct:
		Hints = { { TEXT("ENTER"), TEXT("ROLL / SELECT") }, { TEXT("ARROWS / STICK"), TEXT("MOVE") }, { TEXT("ESC / B"), TEXT("BACK") } };
		break;
	default:
		Hints = { { TEXT("ENTER / A"), TEXT("SELECT") }, { TEXT("ARROWS / STICK"), TEXT("MOVE") }, { TEXT("ESC / B"), TEXT("BACK") } };
		break;
	}
	FString Key;
	for (const FHint& H : Hints) Key += FString::Printf(TEXT("%s %s   "), H.Key, H.Action);
	Key.TrimEndInline();
	if (Key == FooterKey) return;
	FooterKey = Key;
	FooterHints->ClearChildren();
	UWidgetTree* T = WidgetTree;
	for (const FHint& H : Hints)
	{
		UBorder* Cap = Box(T, Fill(FLinearColor(1, 1, 1, 0.14f), false), FMargin(L::Unit, 3.f));
		Cap->SetContent(Txt(T, H.Key, L::Font(L::Caption, FName("Black"), 40), L::Text()));
		AddH(FooterHints, Cap, FMargin(0, 0, L::Unit, 0));
		AddH(FooterHints, Txt(T, H.Action, L::Font(L::Caption, FName("Bold"), L::TrackLabel), L::TextDim()), FMargin(0, 0, L::Unit * 4.f, 0));
	}
}

FString UJamMenuWidget::GetHintLine() const { return FooterKey; }

// ---- the main screen's feature panel

void UJamMenuWidget::RefreshFeature()
{
	if (!FeatureTitle || !FeatureBody || Screen != EJamMenuScreen::Main) return;
	UWidget* Focus = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		for (UButton* B : { PlayNowButton.Get(), SeasonButton.Get(), ModesButton.Get(), FriendButton.Get(), ControlsButton.Get(), SettingsButton.Get(), CreditsButton.Get(), QuitButton.Get() })
			if (B && B->HasUserFocus(PC)) { Focus = B; break; }
	}
	if (!Focus) Focus = PlayNowButton;
	if (FeatureFor.Get() == Focus && !FeatureBody->GetText().IsEmpty()) return;
	FeatureFor = Focus;
	FString Title, Body;
	if (Focus == PlayNowButton.Get())
	{
		Title = TEXT("ONE MATCH");
		Body = FString::Printf(TEXT("%s\n%s\n\nThe level is lit under PLAY NOW: left and right change it, and it is kept for the next time."), *UJamMenuFlow::WhatItIsLine(), *UJamMenuFlow::BrandLine());
	}
	else if (Focus == SeasonButton.Get())
	{
		const UJamProgression* P = UJamProgression::Get(this);
		const FJamSeasonState St = P ? P->GetSeasonState() : FJamSeasonState();
		Title = TEXT("THE SEASON");
		Body = FString::Printf(TEXT("Eight takes, each with its own twist. Win one and the next opens.\n\n%s"), St.bLoaded ? *St.MenuLine : TEXT("The season is not loaded."));
	}
	else if (Focus == ModesButton.Get())
	{
		Title = TEXT("MODES");
		Body = FString::Printf(TEXT("%d ways to play: the pond, the cage, the fog and the rest, each its own rules.\n\nAll the rinks, and MAKE YOUR OWN: one sentence and the Designer builds a game."),
			UJamMenuFlow::AvailableModeCards().Num());
	}
	else if (Focus == FriendButton.Get()) { Title = TEXT("PLAY WITH A FRIEND"); Body = TEXT("One of you hosts and reads out the code, the other joins with it. Same rink, two people on the ice."); }
	else if (Focus == ControlsButton.Get()) { Title = TEXT("CONTROLS"); Body = TEXT("Every move, its keys and its pad button, and what it does."); }
	else if (Focus == SettingsButton.Get()) { Title = TEXT("SETTINGS"); Body = TEXT("Level, look, volume, the voice, and your key for the Designer."); }
	else if (Focus == CreditsButton.Get()) { Title = TEXT("CREDITS"); Body = TEXT("Who made HOKEJ, and with what."); }
	else if (Focus == QuitButton.Get()) { Title = TEXT("QUIT"); Body = TEXT("Back to the desktop. The season keeps your results."); }
	FeatureTitle->SetText(FText::FromString(Title));
	FeatureBody->SetText(FText::FromString(Body));
}

void UJamMenuWidget::RefreshModeDetail()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	if (Screen == EJamMenuScreen::Modes && ModeDetail && ModeGrid)
	{
		for (UWidget* W : ModeGrid->GetAllChildren())
		{
			const UJamMenuRowButton* R = Cast<UJamMenuRowButton>(W);
			if (R && R->HasUserFocus(PC))
			{
				if (ModeDetail->GetText().ToString() != R->Detail) ModeDetail->SetText(FText::FromString(R->Detail));
				return;
			}
		}
	}
	if (Screen == EJamMenuScreen::Season && SeasonDetail)
	{
		for (int32 i = 0; i < SeasonRowButtons.Num(); ++i)
		{
			const UJamMenuRowButton* R = SeasonRowButtons[i];
			if (R && R->HasUserFocus(PC) && SeasonNotes.IsValidIndex(i))
			{
				if (SeasonDetail->GetText().ToString() != SeasonNotes[i]) SeasonDetail->SetText(FText::FromString(SeasonNotes[i]));
				return;
			}
		}
	}
}

// ---- the cards

void UJamMenuWidget::RefreshCuts()
{
	if (!CutGrid) return;
	CutGrid->ClearChildren();
	Items.RemoveAll([](const FItem& I) { return !I.Button.IsValid() || (I.Button->IsA<UJamMenuRowButton>() && Cast<UJamMenuRowButton>(I.Button.Get())->Kind == EJamMenuRowKind::Cut && !I.Button->GetParent()); });
	UJamDesignerSubsystem* D = Designer();
	Rows = D ? UJamMenuFlow::BuildCutRows(ListCutsWithSeason()) : TArray<FJamMenuCutRow>();
	UWidgetTree* T = WidgetTree;
	for (int32 i = 0; i < Rows.Num(); ++i)
	{
		const FJamMenuCutRow& R = Rows[i];
		UJamMenuRowButton* B = T->ConstructWidget<UJamMenuRowButton>(UJamMenuRowButton::StaticClass());
		B->Bind(this, EJamMenuRowKind::Cut, R.Name);
		UHorizontalBox* Line = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		AddH(Line, S::Icon(T, IconFor(R), CutIcon), FMargin(CutPadX, 9, 10, 0), false, VAlign_Top);
		UVerticalBox* Body = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		// Lane A187: a shipped rink by its own name ("REGULAR", "LARGE RINK"): the front end no longer sells cuts. A saved game keeps its name.
		FString Shown = R.Display.ToUpper();
		if (!R.bSaved && Shown.EndsWith(TEXT(" CUT"))) Shown.LeftChopInline(4);
		UTextBlock* Name = Txt(T, Shown, CutNameFont(), L::Text());
		B->TintText = Name;
		AddV(Body, Name, FMargin(0, 0, 0, 1));
		UTextBlock* Sub = Wrapped(Txt(T, UJamMenuFlow::CutCardLine(R), CutLineFont(), L::TextDim()), CutTextW);
		AddV(Body, Sub, FMargin(0));
		USizeBox* BodyBox = Sized(T, Body, 0.f, CutBodyH);
		BodyBox->SetClipping(EWidgetClipping::ClipToBounds);
		if (USizeBoxSlot* BS2 = Cast<USizeBoxSlot>(Body->Slot)) { BS2->SetHorizontalAlignment(HAlign_Fill); BS2->SetVerticalAlignment(VAlign_Top); }
		AddH(Line, BodyBox, FMargin(0, 7, CutPadX, 7), true, VAlign_Top);
		B->AddChild(Line);
		ToTop(Line);
		CutGrid->AddChildToUniformGrid(B, i / CutsPerRow, i % CutsPerRow);
		if (UUniformGridSlot* GS = Cast<UUniformGridSlot>(B->Slot)) { GS->SetHorizontalAlignment(HAlign_Fill); GS->SetVerticalAlignment(VAlign_Fill); }
		AddItem(B, Name);
		AddSub(B, Sub);
	}
	if (!Rows.Num() && CutsStatus)
	{
		CutsStatus->SetText(FText::FromString(TEXT("No rinks found in Content/Cartridges.")));
		CutsStatus->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UJamMenuWidget::RefreshModes()
{
	if (!ModeGrid) return;
	ModeGrid->ClearChildren();
	ModePitchTexts.Reset();
	Items.RemoveAll([](const FItem& I) { return !I.Button.IsValid() || (I.Button->IsA<UJamMenuRowButton>() && !I.Button->GetParent()); });
	UWidgetTree* T = WidgetTree;
	struct FCard { FString Name; FString Display; FString Pitch; FString Side; FString Detail; EJamMenuRowKind Kind; FString Picture; FString Band; };
	TArray<FCard> Cards;
	for (const FJamModeCard& M : UJamMenuFlow::AvailableModeCards())
	{
		FCard C;
		C.Name = M.Name;
		C.Display = M.Display.ToUpper();
		C.Pitch = M.Pitch;
		const int32 H = M.HomeSkaters, A = M.AwaySkaters;
		C.Band = H > 0 ? FString::Printf(TEXT("%d v %d"), H, A > 0 ? A : H) : FString(TEXT("HOKEJ"));
		C.Side = H > 0 ? (A > 0 && A != H ? FString::Printf(TEXT("%d AGAINST %d"), H, A) : FString::Printf(TEXT("%d A SIDE"), H)) : FString();
		C.Detail = FString::Printf(TEXT("%s   /   %s"), *C.Display, *FString::Join(M.Facts, TEXT("   /   ")));
		C.Kind = EJamMenuRowKind::Cut;
		C.Picture = FString::Printf(TEXT("Modes/T_UI_Mode_%s"), *ModeSlug(M.Name));
		Cards.Add(C);
	}
	Cards.Add({ TEXT("cuts"), TEXT("ALL RINKS"), TEXT("Every shipped rink, a rink you saved, the takes you played."), TEXT("5 A SIDE"),
		TEXT("ALL RINKS   /   the regular rink and its variants   /   your saved game   /   the season's takes you played"), EJamMenuRowKind::Open, TEXT("Modes/T_UI_Mode_rinks"), TEXT("RINKS") });
	Cards.Add({ TEXT("direct"), TEXT("MAKE YOUR OWN"), TEXT("One sentence, and the Designer rewrites the game."), TEXT("THE DESIGNER"),
		TEXT("MAKE YOUR OWN   /   type a sentence, or pick one the Designer already made   /   play it, save it"), EJamMenuRowKind::Open, TEXT("Modes/T_UI_Mode_make_your_own"), TEXT("+") });
	for (int32 i = 0; i < Cards.Num(); ++i)
	{
		const FCard& M = Cards[i];
		UJamMenuRowButton* B = T->ConstructWidget<UJamMenuRowButton>(UJamMenuRowButton::StaticClass());
		B->Bind(this, M.Kind, M.Name);
		B->Detail = M.Detail;
		UVerticalBox* Body = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		// The picture slot: device B's picture of the mode by name, a band in a team colour with the sides until it is there.
		UWidget* Pic = nullptr;
		if (UTexture2D* Tex = L::Kit(M.Picture))
		{
			UImage* I = T->ConstructWidget<UImage>(UImage::StaticClass());
			I->SetBrushFromTexture(Tex, false);
			I->SetVisibility(ESlateVisibility::HitTestInvisible);
			Pic = I;
		}
		else
		{
			// Opaque (a see-through band took the focus fill's orange under it).
			const FLinearColor Team = i % 2 ? L::Away() : L::Home();
			const FLinearColor C = M.Kind == EJamMenuRowKind::Open ? FLinearColor(0.035f, 0.040f, 0.050f, 1.f) : FLinearColor(Team.R * 0.55f, Team.G * 0.55f, Team.B * 0.55f, 1.f);
			UBorder* Band = Box(T, FSlateBrush(FSlateRoundedBoxBrush(C, FVector4(4.f, 4.f, 0.f, 0.f))), FMargin(0));
			Band->SetHorizontalAlignment(HAlign_Center);
			Band->SetVerticalAlignment(VAlign_Center);
			Band->SetContent(Txt(T, M.Band, L::Font(L::Title + 4, FName("Black"), 40), FLinearColor(1, 1, 1, 0.92f)));
			Band->SetVisibility(ESlateVisibility::HitTestInvisible);
			Pic = Band;
		}
		AddV(Body, Sized(T, Pic, 0.f, L::ModePictureH), FMargin(0));
		UVerticalBox* Words = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* Name = Txt(T, M.Display, ModeNameFont(), L::Text());
		B->TintText = Name;
		AddV(Words, Name, FMargin(0, 0, 0, 2));
		UTextBlock* Pitch = Wrapped(Txt(T, M.Pitch, ModePitchFont(), L::TextDim()), ModeTextW - 4.f);   // 4 units kept: a wrapped line measured 290 at 288
		ModePitchTexts.Add(Pitch);
		USizeBox* PitchBox = Sized(T, Pitch, 0.f, ModePitchBoxHeight());   // two full lines of the pitch (Jam.Menu.FitsTheScreen measures it), never a third
		PitchBox->SetClipping(EWidgetClipping::ClipToBounds);
		AddV(Words, PitchBox, FMargin(0, 0, 0, 6));
		UTextBlock* Side = Txt(T, M.Side, ModeFactFont(), L::TextDim());
		AddV(Words, Side, FMargin(0));
		AddV(Body, Words, FMargin(L::ModeCardPadX, L::Unit + 2.f, L::ModeCardPadX, L::Unit + 4.f));
		B->AddChild(Body);
		ToTop(Body);
		ModeGrid->AddChildToUniformGrid(B, i / L::ModesPerRow, i % L::ModesPerRow);
		if (UUniformGridSlot* GS = Cast<UUniformGridSlot>(B->Slot)) { GS->SetHorizontalAlignment(HAlign_Fill); GS->SetVerticalAlignment(VAlign_Fill); }
		AddItem(B, Name);
		AddSub(B, Pitch);
		AddSub(B, Side);
	}
	if (ModeDetail) ModeDetail->SetText(FText::FromString(Cards.Num() ? Cards[0].Detail : FString()));
}

void UJamMenuWidget::RefreshSeasonScreen()
{
	if (!SeasonList || !WidgetTree) return;
	SeasonList->ClearChildren();
	Items.RemoveAll([](const FItem& I) { return !I.Button.IsValid() || (I.Button->IsA<UJamMenuRowButton>() && Cast<UJamMenuRowButton>(I.Button.Get())->Kind == EJamMenuRowKind::Take); });
	SeasonRowButtons.Reset();
	SeasonRowTexts.Reset();
	SeasonNotes.Reset();
	UWidgetTree* T = WidgetTree;
	UJamProgression* P = UJamProgression::Get(this);
	const FJamSeasonState St = P ? P->GetSeasonState() : FJamSeasonState();
	if (!St.bLoaded)
	{
		if (ContinueText) ContinueText->SetText(FText::FromString(TEXT("NO SEASON")));
		if (ContinueButton) ContinueButton->SetIsEnabled(false);
		if (SeasonSummary) SeasonSummary->SetText(FText::FromString(TEXT("THE SEASON IS NOT LOADED")));
		if (SeasonDetail) SeasonDetail->SetText(FText::FromString(P ? St.Error : FString(TEXT("The season subsystem is missing."))));
		return;
	}
	if (ContinueButton) ContinueButton->SetIsEnabled(true);
	if (ContinueText) ContinueText->SetText(FText::FromString(St.bComplete ? FString(TEXT("PLAY THE LAST TAKE AGAIN"))
		: FString::Printf(TEXT("CONTINUE: TAKE %d OF %d"), St.CurrentTake + 1, St.TakeCount)));
	if (SeasonSummary) SeasonSummary->SetText(FText::FromString(St.bComplete ? FString::Printf(TEXT("WRAPPED: %d OF %d WON"), St.TakesWon, St.TakeCount)
		: FString::Printf(TEXT("%d OF %d WON"), St.TakesWon, St.TakeCount)));
	for (int32 i = 0; i < St.Takes.Num(); ++i)
	{
		const FJamSeasonTakeRow& R = St.Takes[i];
		const FString State = R.bWon ? TEXT("WON") : (i == St.CurrentTake && R.bUnlocked ? TEXT("NEXT") : (R.bUnlocked ? TEXT("OPEN") : TEXT("LOCKED")));
		UJamMenuRowButton* B = T->ConstructWidget<UJamMenuRowButton>(UJamMenuRowButton::StaticClass());
		B->Bind(this, EJamMenuRowKind::Take, FString::FromInt(i));
		UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UBorder* Bar = Solid(T, FLinearColor::Transparent);
		AddH(Row, Sized(T, Bar, L::FocusBarW), FMargin(0), false, VAlign_Fill);
		UTextBlock* Num = Txt(T, FString::FromInt(i + 1), L::Font(L::Body + 2, FName("Black")), L::TextDim());
		AddH(Row, Sized(T, Num, 40.f), FMargin(L::Unit * 2.f, L::Unit + 2.f, 0, L::Unit + 2.f));
		UTextBlock* Title = Txt(T, R.Title.ToUpper(), L::Font(L::Body + 2, FName("Black"), 40), L::Text());
		AddH(Row, Title, FMargin(0, L::Unit + 2.f, 0, L::Unit + 2.f), true);
		const FLinearColor StateColour = R.bWon ? L::Chosen() : (State == TEXT("NEXT") ? L::Away() : L::TextDim());
		UTextBlock* StateText = Txt(T, State, L::Font(L::Caption, FName("Black"), L::TrackLabel), StateColour);
		AddH(Row, StateText, FMargin(0, 0, L::Unit * 2.f, 0));
		B->AddChild(Row);
		ToTop(Row);
		B->SetIsEnabled(R.bUnlocked);
		AddV(SeasonList, B, FMargin(0, 0, 0, 4));
		AddItem(B, Title, Bar);
		AddSub(B, Num);
		SeasonRowButtons.Add(B);
		SeasonRowTexts.Add(Title);
		FString Note = FString::Printf(TEXT("TAKE %d: %s\n\n%s"), i + 1, *R.Title.ToUpper(), *R.Note);
		if (R.Attempts > 0) Note += FString::Printf(TEXT("\n\nPlayed %d time%s%s."), R.Attempts, R.Attempts == 1 ? TEXT("") : TEXT("s"), R.bWon ? TEXT(", won") : TEXT(""));
		if (!R.bUnlocked) Note += TEXT("\n\nWin the take before it to open this one.");
		SeasonNotes.Add(Note);
	}
	if (SeasonDetail) SeasonDetail->SetText(FText::FromString(SeasonNotes.IsValidIndex(St.CurrentTake) ? SeasonNotes[St.CurrentTake] : St.MenuLine));
}

TArray<FString> UJamMenuWidget::GetSeasonRows() const
{
	TArray<FString> Out;
	for (int32 i = 0; i < SeasonRowButtons.Num(); ++i)
	{
		const UJamMenuRowButton* B = SeasonRowButtons[i];
		if (!B) continue;
		FString Line;
		TArray<const UWidget*> Stack = { B->GetChildAt(0) };
		TArray<FString> Words;
		while (Stack.Num())
		{
			const UWidget* C = Stack.Pop();
			if (const UTextBlock* Tb = Cast<UTextBlock>(C)) Words.Add(Tb->GetText().ToString());
			if (const UPanelWidget* P = Cast<UPanelWidget>(C)) for (int32 k = P->GetChildrenCount() - 1; k >= 0; --k) Stack.Add(P->GetChildAt(k));
		}
		Out.Add(FString::Join(Words, TEXT("  ")));
	}
	return Out;
}

TArray<FString> UJamMenuWidget::GetModeCardNames() const
{
	TArray<FString> Out;
	if (ModeGrid) for (UWidget* W : ModeGrid->GetAllChildren()) if (const UJamMenuRowButton* B = Cast<UJamMenuRowButton>(W)) Out.Add(B->TintText ? B->TintText->GetText().ToString() : B->Payload);
	return Out;
}

// ---------------------------------------------------------------- the measures the tests read (lane A144)

float UJamMenuWidget::ModeTextWidth() { return ModeTextW; }
float UJamMenuWidget::CutTextWidth() { return CutTextW; }
float UJamMenuWidget::CutListHeight() { return CutListH; }
float UJamMenuWidget::ModePitchBoxHeight() { return 56.f; }
TArray<FVector2D> UJamMenuWidget::GetModePitchSizes() const
{
	TArray<FVector2D> Out;
	for (const TWeakObjectPtr<UTextBlock>& Tb : ModePitchTexts)
	{
		const TSharedPtr<SWidget> W = Tb.IsValid() ? TSharedPtr<SWidget>(const_cast<UTextBlock*>(Tb.Get())->TakeWidget()) : nullptr;
		if (W.IsValid()) W->SlatePrepass(1.f);   // the text's own layout at its fixed wrap width
		Out.Add(W.IsValid() ? FVector2D(W->GetDesiredSize()) : FVector2D::ZeroVector);
	}
	return Out;
}
float UJamMenuWidget::ModeDetailWidth() { return L::ModesW - 2.f * L::ModeSlotPad - 2.f * L::ItemPadX; }
FSlateFontInfo UJamMenuWidget::ModeNameFont() { return L::Font(L::Body + 4, FName("Black"), 40); }
FSlateFontInfo UJamMenuWidget::ModePitchFont() { return L::Font(L::Body - 2, FName("Italic")); }
FSlateFontInfo UJamMenuWidget::ModeFactFont() { return L::Font(L::Caption, FName("Bold"), L::TrackLabel); }
FSlateFontInfo UJamMenuWidget::CutNameFont() { return L::Font(L::Body + 2, FName("Black")); }
FSlateFontInfo UJamMenuWidget::CutLineFont() { return L::Font(L::Body - 3, FName("Regular")); }

TArray<FString> UJamMenuWidget::GetMainLabels() const
{
	TArray<FString> Out;
	for (const TObjectPtr<UTextBlock>& Lb : MainLabels) if (Lb) Out.Add(Lb->GetText().ToString());
	return Out;
}

TArray<UJamMenuRowButton*> UJamMenuWidget::GetModeButtons() const
{
	TArray<UJamMenuRowButton*> Out;
	if (ModeGrid) for (UWidget* W : ModeGrid->GetAllChildren()) if (UJamMenuRowButton* B = Cast<UJamMenuRowButton>(W)) if (B->Kind == EJamMenuRowKind::Cut) Out.Add(B);
	return Out;
}

TArray<UJamMenuRowButton*> UJamMenuWidget::GetCutButtons() const
{
	TArray<UJamMenuRowButton*> Out;
	if (CutGrid) for (UWidget* W : CutGrid->GetAllChildren()) if (UJamMenuRowButton* B = Cast<UJamMenuRowButton>(W)) Out.Add(B);
	return Out;
}

FVector2D UJamMenuWidget::MeasureScreen(EJamMenuScreen InScreen)
{
	TSharedRef<SWidget> W = TakeWidget();
	ShowScreen(InScreen);
	W->SlatePrepass(1.f);
	return FVector2D(W->GetDesiredSize());
}

void UJamMenuWidget::RefreshDirect()
{
	UJamDesignerSubsystem* D = Designer();
	const bool bLive = D && D->IsLiveAvailable();
	DirectHint->SetText(FText::FromString(bLive
		? TEXT("Live: your sentence goes to the Designer with your key. A roll takes a few seconds.")
		: TEXT("No API key: the Designer answers from the games it already made (nearest sentence). Add a key under SETTINGS to make one live.")));
	if (!bRolling && !bHasResult) DirectStatus->SetText(FText::GetEmpty());
	// Cached sentences as buttons (the gamepad's way to type).
	if (SentenceList)
	{
		SentenceList->ClearChildren();
		Items.RemoveAll([](const FItem& I)
		{
			return !I.Button.IsValid() || (I.Button->IsA<UJamMenuRowButton>() && Cast<UJamMenuRowButton>(I.Button.Get())->Kind == EJamMenuRowKind::Sentence);
		});
		if (D)
		{
			UWidgetTree* T = WidgetTree;
			TArray<FString> Sentences = D->GetCachedSentences();
			for (const FJamCutInfo& I : D->ListCuts()) if (I.bSaved && !I.Sentence.IsEmpty()) Sentences.AddUnique(I.Sentence);
			for (const FString& Sentence : Sentences)
			{
				UJamMenuRowButton* B = T->ConstructWidget<UJamMenuRowButton>(UJamMenuRowButton::StaticClass());
				B->Bind(this, EJamMenuRowKind::Sentence, Sentence);
				UTextBlock* Text = Txt(T, FString::Printf(TEXT("\"%s\""), *Sentence), L::Font(L::Body, FName("Italic")), L::Text());
				UBorder* Pad = Solid(T, FLinearColor::Transparent, FMargin(14, 5));
				Pad->SetContent(Text);
				B->TintText = Text;
				B->AddChild(Pad);
				ToTop(Pad);
				AddV(SentenceList, B, FMargin(0, 2), HAlign_Left);
				AddItem(B, Text);
			}
		}
	}
}

// ---------------------------------------------------------------- actions

bool UJamMenuWidget::ChooseCut(const FString& Name)
{
	UJamDesignerSubsystem* D = Designer();
	if (!D) { if (CutsStatus) { CutsStatus->SetText(FText::FromString(TEXT("The Designer subsystem is missing."))); CutsStatus->SetVisibility(ESlateVisibility::HitTestInvisible); } return false; }
	if (!Rows.Num()) Rows = UJamMenuFlow::BuildCutRows(ListCutsWithSeason());
	const FJamMenuCutRow* Row = Rows.FindByPredicate([&](const FJamMenuCutRow& R) { return R.Name == Name; });
	FString Err;
	if (!D->LoadCut(Row ? (Row->bSaved ? Row->Path : Row->Name) : Name, Err))
	{
		if (CutsStatus) { CutsStatus->SetText(FText::FromString(FString::Printf(TEXT("%s was rejected: %s"), *Name, *Err))); CutsStatus->SetVisibility(ESlateVisibility::HitTestInvisible); }
		if (ModeDetail && Screen == EJamMenuScreen::Modes) ModeDetail->SetText(FText::FromString(FString::Printf(TEXT("%s was rejected: %s"), *Name, *Err)));
		UE_LOG(LogJamMenuUi, Warning, TEXT("JAMMENU cut %s rejected: %s"), *Name, *Err);
		return false;
	}
	GJamMenuStartedMatch = true;
	// Lane A144 (the level wire): a regular cut plays the player's level; a mode, a take and a saved cut play their own (UJamMenuFlow::PlayerLevelFor).
	UJamMenuFlow::StartMatchAtLevel(this, Row && Row->bSaved ? Row->Path : Name, UJamMenuFlow::PlayerLevelFor(Name, Row && Row->bSaved, FJamSettings::Load()));
	return true;
}

void UJamMenuWidget::OnRowClicked(EJamMenuRowKind Kind, const FString& Payload)
{
	switch (Kind)
	{
	case EJamMenuRowKind::Cut: ChooseCut(Payload); break;
	case EJamMenuRowKind::Open:
	{
		EJamMenuScreen To;
		if (ParseScreen(Payload, To)) ShowScreen(To);
		break;
	}
	case EJamMenuRowKind::Take:
	{
		UJamProgression* P = UJamProgression::Get(this);
		const int32 Index = FCString::Atoi(*Payload);
		if (P && P->StartTake(this, Index)) GJamMenuStartedMatch = true;
		else if (SeasonDetail) SeasonDetail->SetText(FText::FromString(FString::Printf(TEXT("Take %d cannot start: %s"), Index + 1, P ? *P->GetSeasonState().Error : TEXT("no season"))));
		break;
	}
	default:
		if (SentenceBox) SentenceBox->SetText(FText::FromString(Payload));
		Direct(Payload);
		break;
	}
}

void UJamMenuWidget::Direct(const FString& Sentence)
{
	UJamDesignerSubsystem* D = Designer();
	const FString Trimmed = Sentence.TrimStartAndEnd();
	if (!D || Trimmed.IsEmpty()) { DirectStatus->SetText(FText::FromString(TEXT("Type one sentence first."))); return; }
	if (bRolling) return;
	bRolling = true;
	bHasResult = false;
	bResultSaved = false;
	SavedResultPath.Empty();
	if (ResultBox) { ResultBox->SetVisibility(ESlateVisibility::Collapsed); if (UWidget* P = ResultBox->GetParent()) P->SetVisibility(ESlateVisibility::Collapsed); }
	DirectStatus->SetText(FText::FromString(TEXT("rolling...")));
	RollButton->SetIsEnabled(false);
	TWeakObjectPtr<UJamMenuWidget> Weak(this);
	D->RequestCut(Trimmed, FString(), [Weak](const FJamDesignerResult& R) { if (Weak.IsValid()) Weak->OnResult(R); });
}

void UJamMenuWidget::OnResult(const FJamDesignerResult& R)
{
	bRolling = false;
	Last = R;
	if (RollButton) RollButton->SetIsEnabled(true);
	bHasResult = R.HasCartridge() && !R.CartridgeJson.IsEmpty();
	FString Status = R.Message;
	switch (R.Outcome)
	{
	case EJamDesignerOutcome::FirstTry: Status = FString::Printf(TEXT("Made. %lld in / %lld out tokens, %.1f s."), R.TokensIn, R.TokensOut, R.Ms / 1000.0); break;
	case EJamDesignerOutcome::Repaired: Status = FString::Printf(TEXT("Made after one repair. %lld in / %lld out tokens, %.1f s."), R.TokensIn, R.TokensOut, R.Ms / 1000.0); break;
	case EJamDesignerOutcome::Offline: Status = TEXT("From the games already made (no key): the nearest sentence."); break;
	case EJamDesignerOutcome::Refused: Status = FString::Printf(TEXT("The Designer declined: %s"), *R.Refusal); break;
	case EJamDesignerOutcome::Failed: Status = R.Errors.Num() ? FString::Printf(TEXT("No valid game: %s"), *R.Errors[0]) : (R.Message.IsEmpty() ? TEXT("No valid game.") : R.Message); break;
	default: break;
	}
	DirectStatus->SetText(FText::FromString(Status));
	if (bHasResult)
	{
		ResultName->SetText(FText::FromString(R.CutName.IsEmpty() ? TEXT("Untitled Cut") : R.CutName.ToUpper()));
		const TArray<FString> Changes = UJamMenuFlow::BiggestChanges(UJamMenuFlow::BaseCartridgeJson(Designer()), R.CartridgeJson, 3);
		ResultChanges->SetText(FText::FromString(Changes.Num() ? FString::Join(Changes, TEXT("\n")) : TEXT("no change from the current game")));
		SaveResultText->SetText(FText::FromString(TEXT("SAVE AS MY DIRECTOR'S CUT")));
		SaveResultButton->SetIsEnabled(true);
		ResultBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (UWidget* P = ResultBox->GetParent()) P->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		PendingFocus = PlayResultButton.Get();
	}
	else
	{
		PendingFocus = RollButton.Get();
	}
}

void UJamMenuWidget::OnPlayResultClicked()
{
	if (!bHasResult) return;
	FString Path = bResultSaved ? SavedResultPath : UJamMenuFlow::WritePreviewCut(Last.CartridgeJson);
	if (Path.IsEmpty()) { DirectStatus->SetText(FText::FromString(TEXT("Could not write the game to play it."))); return; }
	if (UJamDesignerSubsystem* D = Designer())
	{
		FString Err;
		if (!D->LoadCut(Path, Err)) { DirectStatus->SetText(FText::FromString(FString::Printf(TEXT("The game was rejected: %s"), *Err))); return; }
	}
	GJamMenuStartedMatch = true;
	UJamMenuFlow::StartMatch(this, Path);
}

void UJamMenuWidget::OnSaveResultClicked()
{
	UJamDesignerSubsystem* D = Designer();
	if (!bHasResult || !D) return;
	FString Err;
	if (!D->ApplyResult(Last, Err)) { DirectStatus->SetText(FText::FromString(FString::Printf(TEXT("Not saved: %s"), *Err))); return; }
	bResultSaved = true;
	for (const FJamCutInfo& I : D->ListCuts()) if (I.bSaved && I.Name == UJamDesignerSubsystem::MyDirectorsCutName) SavedResultPath = I.Path;
	DirectStatus->SetText(FText::FromString(TEXT("Saved. It is in MODES, ALL RINKS now.")));
	SaveResultText->SetText(FText::FromString(TEXT("SAVED")));
	SaveResultButton->SetIsEnabled(false);
	PendingFocus = PlayResultButton.Get();
}

void UJamMenuWidget::OnTryAgainClicked()
{
	bHasResult = false;
	ResultBox->SetVisibility(ESlateVisibility::Collapsed);
	if (UWidget* P = ResultBox->GetParent()) P->SetVisibility(ESlateVisibility::Collapsed);
	DirectStatus->SetText(FText::GetEmpty());
	if (SentenceBox) { SentenceBox->SetText(FText::GetEmpty()); PendingFocus = SentenceBox.Get(); }
}

void UJamMenuWidget::OnChooseClicked() { ShowScreen(EJamMenuScreen::Cuts); }

// Lane A144 (track U2): the first press. One match on the regular cut at the settings' difficulty; a rejected cut says so on MAIN.
bool UJamMenuWidget::PlayNow()
{
	// Director (14 Nov, the H27 release): the -cut=<name or path> launch argument wins, as it does in AHokejGameMode::InitGame. The acceptance run starts the
	// packaged game with its own cut and presses Return here; PLAY NOW loaded hockey.regular through the Designer instead (first to 3), and the run ended
	// after its third shot. A player never passes -cut.
	FString LaunchCut;
	if (FParse::Value(FCommandLine::Get(), TEXT("-cut="), LaunchCut) && !LaunchCut.IsEmpty() && ChooseCut(LaunchCut)) return true;
	// Lane A144 (the level wire): rookie on a first run, the player's level once picked: ChooseCut gives every regular cut the player's level
	// (UJamMenuFlow::PlayerLevelFor, ?level=<x>, AHokejGameMode, the brain).
	if (ChooseCut(UJamMenuFlow::FirstCut)) return true;
	if (MainStatus) MainStatus->SetText(FText::FromString(FString::Printf(TEXT("The match cannot start: %s"), CutsStatus ? *CutsStatus->GetText().ToString() : TEXT("no cut"))));
	return false;
}

FString UJamMenuWidget::GetFirstPressLine() const { return PlayNowSub ? PlayNowSub->GetText().ToString() : FString(); }

// ---------------------------------------------------------------- the level switch (lane A144, evolution 3)

void UJamMenuWidget::RefreshLevelSwitch()
{
	// The level in force is the saved one (rookie until the player has picked): the switch, PLAY NOW's line, the meaning line and the header read it.
	const FJamSettings Settings = FJamSettings::Load();
	const EJamMenuDifficulty Before = SwitchLevel;
	SwitchLevel = Settings.EffectiveDifficulty();
	const FString Level = FJamSettings::DifficultyString(SwitchLevel);
	if (PlayNowSub) PlayNowSub->SetText(FText::FromString(UJamMenuFlow::FirstMatchLine(Level)));
	if (LevelMeaningText) LevelMeaningText->SetText(FText::FromString(UJamMenuFlow::LevelMeaning(Level)));
	if (HeaderLevel) HeaderLevel->SetText(FText::FromString(FJamSettings::DifficultyLabel(SwitchLevel)));
	for (FItem& I : Items) if (I.bSegment) I.State = -1;   // the lit segment is painted again
	if (Before != SwitchLevel) FeatureFor = nullptr;
}

void UJamMenuWidget::PickLevel(EJamMenuDifficulty Level)
{
	FJamSettings Settings = FJamSettings::Load();
	Settings.Difficulty = Level;
	Settings.bDifficultyChosen = true;   // the player's pick from now on, as a press of the SETTINGS button is
	Settings.Save();
	RefreshLevelSwitch();
	UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU level picked=%s"), *FJamSettings::DifficultyString(Level));
}

void UJamMenuWidget::StepLevel(int32 Delta)
{
	const int32 Now = static_cast<int32>(FJamSettings::Load().EffectiveDifficulty());
	const int32 Next = FMath::Clamp(Now + (Delta < 0 ? -1 : 1), 0, 2);
	if (Next != Now) PickLevel(static_cast<EJamMenuDifficulty>(Next));
}

void UJamMenuWidget::OnLevelRookieClicked() { OnSegmentClicked(EJamMenuDifficulty::Rookie); }
void UJamMenuWidget::OnLevelProClicked() { OnSegmentClicked(EJamMenuDifficulty::Pro); }
void UJamMenuWidget::OnLevelAllStarClicked() { OnSegmentClicked(EJamMenuDifficulty::AllStar); }

void UJamMenuWidget::OnSegmentClicked(EJamMenuDifficulty Level)
{
	PickLevel(Level);
	// Lane A164 (evolution 4): "click PRO, press Return" plays pro. The segments take no focus, so on the mouse-down Slate gave the keyboard focus
	// to the leaf-most focusable widget under the pointer: the menu widget itself, where Return started nothing. The focus goes back to PLAY NOW now,
	// and once more on the next tick should the mouse-up move it.
	if (Screen == EJamMenuScreen::Main && PlayNowButton)
	{
		PlayNowButton->SetFocus();
		PendingFocus = PlayNowButton.Get();
	}
	UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU segment clicked level=%s focus=play_now"), *FJamSettings::DifficultyString(Level));
}

bool UJamMenuWidget::IsFocusOnPlayNow() const
{
	APlayerController* PC = GetOwningPlayer();
	return PlayNowButton && PC && PlayNowButton->HasUserFocus(PC);
}

UWidget* UJamMenuWidget::OnPlayNowNavigate(EUINavigation Direction)
{
	if (Direction == EUINavigation::Left) StepLevel(-1);
	else if (Direction == EUINavigation::Right) StepLevel(1);
	return PlayNowButton;   // the focus stays: Enter / A is still the match
}

TArray<FString> UJamMenuWidget::GetSwitchLabels() const
{
	TArray<FString> Out;
	for (const TObjectPtr<UTextBlock>& Tb : LevelTexts) if (Tb) Out.Add(Tb->GetText().ToString());
	return Out;
}

FString UJamMenuWidget::GetLevelMeaningLine() const { return LevelMeaningText ? LevelMeaningText->GetText().ToString() : FString(); }

void UJamMenuWidget::OnPlayNowClicked() { PlayNow(); }
void UJamMenuWidget::OnControlsClicked() { ShowScreen(EJamMenuScreen::Controls); }

TArray<FJamCutInfo> UJamMenuWidget::ListCutsWithSeason() const
{
	UJamDesignerSubsystem* D = Designer();
	TArray<FJamCutInfo> Cuts = D ? D->ListCuts() : TArray<FJamCutInfo>();
	// Lane A17: a played season take is listed as a cut of its own.
	if (const UJamProgression* P = UJamProgression::Get(this)) Cuts = P->AppendPlayedTakes(Cuts);
	return Cuts;
}

void UJamMenuWidget::RefreshSeason()
{
	// MAIN's SEASON entry: where the player is in it.
	RefreshLevelSwitch();
	if (!SeasonText) return;
	UJamProgression* P = UJamProgression::Get(this);
	const FJamSeasonState St = P ? P->GetSeasonState() : FJamSeasonState();
	FString Line = TEXT("EIGHT TAKES, EACH WITH A TWIST");
	if (St.bLoaded && St.bComplete) Line = FString::Printf(TEXT("WRAPPED: %d OF %d WON"), St.TakesWon, St.TakeCount);
	else if (St.bLoaded) Line = FString::Printf(TEXT("TAKE %d OF %d NEXT  /  %d WON"), St.CurrentTake + 1, St.TakeCount, St.TakesWon);
	SeasonText->SetText(FText::FromString(Line));
}

bool UJamMenuWidget::PlaySeason()
{
	UJamProgression* P = UJamProgression::Get(this);
	if (!P || !P->StartSeason(this))
	{
		const FString Why = P ? FString::Printf(TEXT("The season cannot start: %s"), *P->GetSeasonState().Error) : FString(TEXT("The season subsystem is missing."));
		if (MainStatus) MainStatus->SetText(FText::FromString(Why));
		if (SeasonDetail) SeasonDetail->SetText(FText::FromString(Why));
		return false;
	}
	GJamMenuStartedMatch = true;
	return true;
}

void UJamMenuWidget::OnSeasonClicked() { ShowScreen(EJamMenuScreen::Season); }
void UJamMenuWidget::OnContinueSeasonClicked() { PlaySeason(); }
void UJamMenuWidget::OnDirectClicked() { ShowScreen(EJamMenuScreen::Direct); }

void UJamMenuWidget::OnFriendClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->ConsoleCommand(TEXT("jam.lobby"));
		if (MainStatus) MainStatus->SetText(FText::FromString(TEXT("Opening the lobby...")));
	}
}
void UJamMenuWidget::OnSettingsClicked() { ShowScreen(EJamMenuScreen::Settings); }
void UJamMenuWidget::OnCreditsClicked() { ShowScreen(EJamMenuScreen::Credits); }
void UJamMenuWidget::OnBackClicked() { GoBack(); }
void UJamMenuWidget::OnControlsBack() { if (Screen == EJamMenuScreen::Controls) GoBack(); }
void UJamMenuWidget::OnRollClicked() { if (SentenceBox) Direct(SentenceBox->GetText().ToString()); }
void UJamMenuWidget::OnSentenceCommitted(const FText& InText, ETextCommit::Type Commit) { if (Commit == ETextCommit::OnEnter) Direct(InText.ToString()); }

void UJamMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

// ---------------------------------------------------------------- input and tick

void UJamMenuWidget::PaintItems()
{
	APlayerController* PC = GetOwningPlayer();
	for (FItem& I : Items)
	{
		UButton* B = I.Button.Get();
		if (!B) continue;
		const bool bEnabled = B->GetIsEnabled();
		// Lit is focused (the mouse moves the focus with it, NativeTick); a segment takes no focus, so it lights under the pointer.
		const bool bHot = bEnabled && ((PC && B->HasUserFocus(PC)) || B->HasFocusedDescendants() || (I.bSegment && B->IsHovered()));
		int32 State = !bEnabled ? 0 : (bHot ? 2 : 1);
		if (I.bSegment && State == 1)
		{
			const int32 Idx = LevelButtons.IndexOfByKey(B);
			if (Idx == static_cast<int32>(SwitchLevel)) State = 3;
		}
		if (State == I.State) continue;
		I.State = State;
		B->SetStyle(ItemStyle(State, I.bSegment));
		B->SetBackgroundColor(FLinearColor::White);
		const FLinearColor Main = State == 0 ? L::Disabled() : (State >= 2 ? L::TextOnFocus() : (I.bSegment ? L::TextDim() : L::Text()));
		if (UTextBlock* Tb = I.Text.Get()) Tb->SetColorAndOpacity(FSlateColor(Main));
		for (const TWeakObjectPtr<UTextBlock>& Sb : I.Subs)
			if (UTextBlock* Tb = Sb.Get()) Tb->SetColorAndOpacity(FSlateColor(State == 0 ? L::Disabled() : (State >= 2 ? FLinearColor(1, 1, 1, 0.88f) : L::TextDim())));
		if (UBorder* Bar = Cast<UBorder>(I.Bar.Get())) Bar->SetBrushColor(State == 2 ? L::TextOnFocus() : FLinearColor::Transparent);
	}
}

void UJamMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	APlayerController* PC = GetOwningPlayer();
	// Lane A164 (evolution 4), lane A187 on every screen: the keyboard focus never rests on the menu widget itself (a click on a segment, on a title or on
	// empty cloth puts it there, and Return then started nothing): it goes home to the screen's opening place. A focus anywhere else is left.
	if (WalkScreen < 0 && !PendingFocus.IsValid() && PC && HasUserFocus(PC))
	{
		if (UWidget* Home = GetHomeFocus(Screen))
		{
			if (Home->GetIsEnabled())
			{
				PendingFocus = Home;
				UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU focus home screen=%s from=menu"), *ScreenName(Screen));
			}
		}
	}
	if (PendingFocus.IsValid())
	{
		PendingFocus->SetFocus();
		PendingFocus = nullptr;
	}
	// The mouse moves the focus with it: the hovered item is the focused one, so one item is lit and Enter presses what the pointer is on.
	if (WalkScreen < 0 && FSlateApplication::IsInitialized())
	{
		const FVector2D Pointer = FSlateApplication::Get().GetCursorPos();
		if (!Pointer.Equals(LastCursor, 0.5))
		{
			LastCursor = Pointer;
			const TSharedPtr<SWidget> KF = FSlateApplication::Get().GetKeyboardFocusedWidget();
			const bool bTyping = KF.IsValid() && KF->GetType().ToString().Contains(TEXT("EditableText"));
			if (!bTyping && PC)
			{
				for (const FItem& I : Items)
				{
					UButton* B = I.Button.Get();
					if (B && !I.bSegment && B->IsHovered() && B->GetIsEnabled() && !B->HasUserFocus(PC) && B->IsVisible()) { B->SetFocus(); break; }
				}
			}
		}
	}
	if (WalkScreen >= 0) TickPadWalk();
	// The screen's way in: under 200 ms.
	if (TransitionT < 1.f && Switcher)
	{
		TransitionT = FMath::Min(1.f, TransitionT + InDeltaTime / L::TransitionSeconds);
		const float A = 1.f - FMath::Pow(1.f - TransitionT, 3.f);
		Switcher->SetRenderOpacity(A);
		Switcher->SetRenderTranslation(FVector2D((1.f - A) * L::TransitionSlide, 0.f));
	}
	// Lane A195: the backdrop's strokes drift, slowly and in opposite phase.
	DriftT += InDeltaTime;
	for (int32 k = 0; k < BackdropStrokes.Num(); ++k)
		if (UWidget* St = BackdropStrokes[k].Get()) St->SetRenderTranslation(FVector2D(L::DriftUnits * FMath::Sin(DriftT * 2.f * PI / L::DriftSeconds + k * PI), 0.f));
	if (Screen == EJamMenuScreen::Landing && LandingPrompt)
	{
		PulseT += InDeltaTime;
		LandingPrompt->SetRenderOpacity(0.55f + 0.45f * (0.5f + 0.5f * FMath::Cos(PulseT * 2.f * PI / L::PromptPulseSeconds)));
	}
	PaintItems();
	RefreshFooter();
	RefreshFeature();
	RefreshModeDetail();
}

FReply UJamMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey K = InKeyEvent.GetKey();
	// Lane A187: the landing takes any key (not a held one, not a modifier alone, not Esc) as its press.
	if (Screen == EJamMenuScreen::Landing && WalkScreen < 0)
	{
		if (!InKeyEvent.IsRepeat() && !K.IsModifierKey() && K != EKeys::Escape && !K.IsMouseButton())
		{
			UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU landing key %s"), *K.ToString());
			PressStart();
		}
		return FReply::Handled();
	}
	if (IsBackKey(K))
	{
		// One Back a frame: a child screen that fired its own Back for this key (the controls lane's screen) is not followed by a second.
		if (LastBackFrame != GFrameCounter) GoBack();
		return FReply::Handled();
	}
	// Lane A164 (evolution 4): Return and the pad's A on MAIN always start the match. A focused button takes the key itself (a click on that button);
	// a key that reaches the menu widget (nothing of MAIN had the focus) presses PLAY NOW.
	if (Screen == EJamMenuScreen::Main && WalkScreen < 0 && PlayNowButton && PlayNowButton->GetIsEnabled() && !InKeyEvent.IsRepeat()
		&& (K == EKeys::Enter || K == EKeys::Gamepad_FaceButton_Bottom))
	{
		UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU start key %s reached the menu: PLAY NOW"), *K.ToString());
		PlayNow();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UJamMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Lane A187: the landing button covers the middle of the screen; a click on the bands above and below it reaches the menu here and is the press too.
	if (Screen == EJamMenuScreen::Landing && WalkScreen < 0)
	{
		UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU landing click"));
		PressStart();
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

// ---------------------------------------------------------------- the pad walk (lane A144, evolution 2; jam.menuscreen walk)

namespace
{
	// Lane A187: every screen but the landing (one place, any key leaves it), parents checked by B.
	const EJamMenuScreen WalkOrder[] = { EJamMenuScreen::Main, EJamMenuScreen::Season, EJamMenuScreen::Modes, EJamMenuScreen::Cuts, EJamMenuScreen::Direct,
		EJamMenuScreen::Settings, EJamMenuScreen::Credits, EJamMenuScreen::Controls };
	const int32 WalkUser = 0;

	FString WalkLabel(const UWidget* W)
	{
		if (const UJamMenuRowButton* R = Cast<UJamMenuRowButton>(W)) return R->TintText ? R->TintText->GetText().ToString() : R->Payload;
		if (const UButton* B = Cast<UButton>(W))
		{
			// The label is the first text under the button, depth first.
			TArray<const UWidget*> Stack = { B->GetChildAt(0) };
			while (Stack.Num())
			{
				const UWidget* C = Stack.Pop();
				if (const UTextBlock* T = Cast<UTextBlock>(C)) return T->GetText().ToString();
				if (const UPanelWidget* P = Cast<UPanelWidget>(C)) for (int32 i = P->GetChildrenCount() - 1; i >= 0; --i) Stack.Add(P->GetChildAt(i));
			}
		}
		return W ? W->GetName() : FString(TEXT("none"));
	}

	void WalkPress(const FKey& Key)
	{
		const FKeyEvent Ev(Key, FModifierKeysState(), WalkUser, false, 0, 0);
		FSlateApplication::Get().ProcessKeyDownEvent(Ev);
		FSlateApplication::Get().ProcessKeyUpEvent(Ev);
	}
}

TArray<UWidget*> UJamMenuWidget::FocusablesOf(EJamMenuScreen InScreen) const
{
	TArray<UWidget*> All;
	auto Back = [&]() { const int32 I = static_cast<int32>(InScreen); if (BackButtons.IsValidIndex(I) && BackButtons[I]) All.Add(BackButtons[I]); };
	switch (InScreen)
	{
	case EJamMenuScreen::Landing:
		All.Add(LandingButton.Get());
		break;
	case EJamMenuScreen::Main:
		for (UButton* B : { PlayNowButton.Get(), SeasonButton.Get(), ModesButton.Get(), FriendButton.Get(), ControlsButton.Get(), SettingsButton.Get(), CreditsButton.Get(), QuitButton.Get() }) All.Add(B);
		break;
	case EJamMenuScreen::Season:
		All.Add(ContinueButton.Get());
		for (UJamMenuRowButton* B : SeasonRowButtons) All.Add(B);
		Back();
		break;
	case EJamMenuScreen::Modes:
		if (ModeGrid) for (UWidget* W : ModeGrid->GetAllChildren()) All.Add(W);
		Back();
		break;
	case EJamMenuScreen::Cuts:
		for (UJamMenuRowButton* B : GetCutButtons()) All.Add(B);
		Back();
		break;
	case EJamMenuScreen::Direct:
		All.Add(SentenceBox.Get());
		All.Add(RollButton.Get());
		if (SentenceList) for (UWidget* W : SentenceList->GetAllChildren()) All.Add(W);
		Back();
		break;
	case EJamMenuScreen::Settings:
		if (SettingsPanel) All = SettingsPanel->GetFocusables();
		Back();
		break;
	case EJamMenuScreen::Controls:
		if (ControlsScreen && ControlsFirst) if (UWidget* F = ControlsFirst()) All.Add(F);
		Back();
		break;
	default: Back(); break;
	}
	// A disabled or hidden thing is no place to stand (a locked take, CLEAR without a key).
	All.RemoveAll([](const UWidget* W) { return !W || !W->GetIsEnabled() || !W->IsVisible(); });
	return All;
}

int32 UJamMenuWidget::WalkFocusedIndex() const
{
	// The focused Slate widget or the place it is inside (a text box's and a slider's focus sits on an inner widget).
	for (TSharedPtr<SWidget> F = FSlateApplication::Get().GetUserFocusedWidget(WalkUser); F.IsValid(); F = F->GetParentWidget())
	{
		for (int32 i = 0; i < WalkNodes.Num(); ++i)
		{
			if (WalkNodes[i].IsValid() && WalkNodes[i]->GetCachedWidget() == F) return i;
		}
	}
	return -1;
}

void UJamMenuWidget::PressKeyForQa(const FKey& Key)
{
	if (!FSlateApplication::IsInitialized() || !Key.IsValid()) return;
	const TSharedPtr<SWidget> Before = FSlateApplication::Get().GetUserFocusedWidget(WalkUser);
	const EJamMenuScreen Was = Screen;
	WalkPress(Key);
	const TSharedPtr<SWidget> After = FSlateApplication::Get().GetUserFocusedWidget(WalkUser);
	UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU key %s screen=%s->%s focus_on_play_now=%d focus_moved=%d level=%s"), *Key.ToString(), *ScreenName(Was), *ScreenName(Screen),
		PlayNowButton && PlayNowButton->GetCachedWidget() == After ? 1 : 0, Before != After ? 1 : 0, *FJamSettings::DifficultyString(SwitchLevel));
}

TArray<FString> UJamMenuWidget::GetPlacesForQa() const
{
	// Lane A164 (evolution 4): the places of the shown screen from the widgets' own painted geometry. Paint space is the game window's client
	// area in pixels (the viewport fills it); the 1920 x 1080 figures scale that by the viewport's size.
	TArray<FString> Out;
	TArray<UWidget*> Places = FocusablesOf(Screen);
	if (Screen == EJamMenuScreen::Main)
	{
		// The level segments take a click and no focus: listed after PLAY NOW, where they sit.
		int32 At = Places.IndexOfByKey(PlayNowButton.Get()) + 1;
		for (const TObjectPtr<UButton>& B : LevelButtons) if (B) Places.Insert(B.Get(), At++);
	}
	FVector2D Vp(1920.0, 1080.0);
	if (GEngine && GEngine->GameViewport) { FVector2D Sz; GEngine->GameViewport->GetViewportSize(Sz); if (Sz.X > 1.0 && Sz.Y > 1.0) Vp = Sz; }
	for (UWidget* W : Places)
	{
		const TSharedPtr<SWidget> Slate = W ? W->GetCachedWidget() : nullptr;
		if (!Slate.IsValid()) continue;
		const FGeometry G = Slate->GetPaintSpaceGeometry();
		const FVector2D Size = FVector2D(G.GetAbsoluteSize());
		const FVector2D C = FVector2D(G.GetAbsolutePosition()) + Size * 0.5;
		const bool bPainted = Size.X > 0.5 && Size.Y > 0.5;
		Out.Add(FString::Printf(TEXT("screen=%s label=\"%s\" x=%d y=%d w=%d h=%d at1080=%d,%d painted=%d"), *ScreenName(Screen),
			*WalkLabel(W).Replace(TEXT("\n"), TEXT(" ")), FMath::RoundToInt32(C.X), FMath::RoundToInt32(C.Y), FMath::RoundToInt32(Size.X), FMath::RoundToInt32(Size.Y),
			FMath::RoundToInt32(C.X * 1920.0 / Vp.X), FMath::RoundToInt32(C.Y * 1080.0 / Vp.Y), bPainted ? 1 : 0));
	}
	return Out;
}

bool UJamMenuWidget::ClickForQa(UWidget* W, FString& Why)
{
	bLastClickDownOnMenu = false;
	if (!FSlateApplication::IsInitialized()) { Why = TEXT("no Slate application"); return false; }
	const TSharedPtr<SWidget> Slate = W ? W->GetCachedWidget() : nullptr;
	if (!Slate.IsValid()) { Why = TEXT("the widget has no Slate widget"); return false; }
	FWidgetPath Full;
	if (!FSlateApplication::Get().GeneratePathToWidgetUnchecked(Slate.ToSharedRef(), Full) || !Full.IsValid()) { Why = TEXT("no widget path to it"); return false; }
	// The whole path from the window to the widget, each step with no virtual pointer (no 3D widget on it).
	TArray<FWidgetAndPointer> Steps;
	for (int32 i = 0; i < Full.Widgets.Num(); ++i) Steps.Add(FWidgetAndPointer(Full.Widgets[i], TOptional<FVirtualPointerPosition>()));
	const FWidgetPath Path(Steps);
	if (!Path.IsValid()) { Why = TEXT("the click's path did not build"); return false; }
	const FGeometry G = Path.Widgets.Last().Geometry;
	const FVector2D At = FVector2D(G.GetAbsolutePosition()) + FVector2D(G.GetAbsoluteSize()) * 0.5;
	TSet<FKey> Down;
	Down.Add(EKeys::LeftMouseButton);
	const uint32 Pointer = FSlateApplicationBase::CursorPointerIndex;
	FSlateApplication::Get().RoutePointerMoveEvent(Path, FPointerEvent(WalkUser, Pointer, At, At, TSet<FKey>(), FKey(), 0.f, FModifierKeysState()), false);
	FSlateApplication::Get().RoutePointerDownEvent(Path, FPointerEvent(WalkUser, Pointer, At, At, Down, EKeys::LeftMouseButton, 0.f, FModifierKeysState()));
	APlayerController* PC = GetOwningPlayer();
	bLastClickDownOnMenu = PC && HasUserFocus(PC);
	FSlateApplication::Get().RoutePointerUpEvent(Path, FPointerEvent(WalkUser, Pointer, At, At, TSet<FKey>(), EKeys::LeftMouseButton, 0.f, FModifierKeysState()));
	UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU click label=\"%s\" at=%d,%d down_focus=%s focus_on_play_now=%d"), *WalkLabel(W), FMath::RoundToInt32(At.X), FMath::RoundToInt32(At.Y),
		bLastClickDownOnMenu ? TEXT("menu_itself") : TEXT("other"), IsFocusOnPlayNow() ? 1 : 0);
	return true;
}

bool UJamMenuWidget::ClickPlaceForQa(const FString& Name)
{
	UWidget* W = nullptr;
	EJamMenuDifficulty D;
	if (FJamSettings::ParseDifficulty(Name, D)) W = GetLevelButton(D);
	else if (Name == TEXT("play")) W = PlayNowButton.Get();
	else if (Name == TEXT("start")) W = LandingButton.Get();
	else if (Name == TEXT("modes")) W = ModesButton.Get();
	else if (Name == TEXT("season")) W = SeasonButton.Get();
	else if (Name == TEXT("continue")) W = ContinueButton.Get();
	else if (Name == TEXT("back")) { const int32 I = static_cast<int32>(Screen); W = BackButtons.IsValidIndex(I) ? BackButtons[I].Get() : nullptr; }
	else if (Name == TEXT("make") || Name == TEXT("rinks"))
	{
		const FString Payload = Name == TEXT("make") ? TEXT("direct") : TEXT("cuts");
		if (ModeGrid) for (UWidget* C : ModeGrid->GetAllChildren()) if (const UJamMenuRowButton* R = Cast<UJamMenuRowButton>(C)) if (R->Kind == EJamMenuRowKind::Open && R->Payload == Payload) W = C;
	}
	else if (Name == TEXT("settingslevel") && SettingsPanel) W = SettingsPanel->GetFirstFocusable();
	FString Why = TEXT("no such place");
	const bool bOk = W && ClickForQa(W, Why);
	if (!bOk) UE_LOG(LogJamMenuUi, Warning, TEXT("JAMMENU click %s failed: %s"), *Name, *Why);
	return bOk;
}

void UJamMenuWidget::PrintPlacesForQa() const
{
	FVector2D Vp(0.0, 0.0);
	if (GEngine && GEngine->GameViewport) GEngine->GameViewport->GetViewportSize(Vp);
	const TArray<FString> Places = GetPlacesForQa();
	UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU where screen=%s viewport=%dx%d places=%d"), *ScreenName(Screen), FMath::RoundToInt32(Vp.X), FMath::RoundToInt32(Vp.Y), Places.Num());
	for (const FString& P : Places) UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU place %s"), *P);
}

void UJamMenuWidget::StartPadWalk()
{
	WalkScreen = 0;
	WalkNode = -2;
	WalkPhase = 0;
	for (int32& T : WalkTotals) T = 0;
	WalkBlindScreens = 0;
	UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU walk begin screens=%d"), static_cast<int32>(UE_ARRAY_COUNT(WalkOrder)));
}

void UJamMenuWidget::TickPadWalk()
{
	if (!FSlateApplication::IsInitialized()) { WalkScreen = -1; return; }
	if (WalkScreen >= static_cast<int32>(UE_ARRAY_COUNT(WalkOrder)))
	{
		// blind: the directions could not be judged in this process (its hit-test grid does not hold the menu): run it in a windowed game.
		const TCHAR* Verdict = WalkTotals[3] > 0 ? TEXT("fail") : (WalkBlindScreens > 0 ? TEXT("blind")
			: (WalkTotals[0] > 0 && WalkTotals[0] == WalkTotals[1] && WalkTotals[2] == 0 ? TEXT("pass") : TEXT("fail")));
		UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU walk done focusables=%d reached=%d traps=%d back_failed=%d blind_screens=%d verdict=%s"), WalkTotals[0], WalkTotals[1], WalkTotals[2],
			WalkTotals[3], WalkBlindScreens, Verdict);
		WalkScreen = -1;
		ShowScreen(EJamMenuScreen::Main);
		return;
	}
	++WalkPhase;
	static const FKey Dirs[] = { EKeys::Gamepad_DPad_Up, EKeys::Gamepad_DPad_Down, EKeys::Gamepad_DPad_Left, EKeys::Gamepad_DPad_Right };
	const EJamMenuScreen Walked = WalkOrder[WalkScreen];

	if (WalkNode == -2)
	{
		// Entering a screen: show it, let it take its focus and paint (navigation reads the painted screen's hit-test grid).
		if (WalkPhase == 1) ShowScreen(Walked);
		if (WalkPhase < 10) return;
		WalkNodes.Reset();
		for (UWidget* W : FocusablesOf(Walked)) WalkNodes.Add(W);
		WalkReached.Reset();
		WalkQueue.Reset();
		WalkOut.Init(0, WalkNodes.Num());
		WalkEdges = 0;
		WalkStart = WalkFocusedIndex();
		const int32 Seed = WalkStart >= 0 ? WalkStart : (WalkNodes.Num() ? 0 : -1);
		// Lane A164 (evolution 4): the walk judges what Slate's own navigation did. A screen is blind only when no press from any place moved the
		// focus AND the grid has no menu under the opening place (grid_hit, a diagnostic); nothing moving although the grid holds the menu is a fail.
		bWalkGridHit = false;
		if (Seed >= 0 && WalkNodes[Seed].IsValid() && WalkNodes[Seed]->GetCachedWidget().IsValid())
		{
			const TSharedRef<SWidget> Slate = WalkNodes[Seed]->GetCachedWidget().ToSharedRef();
			const TSharedPtr<SWindow> Win = FSlateApplication::Get().FindWidgetWindow(Slate);
			const FGeometry G = Slate->GetTickSpaceGeometry();
			const FVector2D Centre = FVector2D(G.GetAbsolutePosition()) + FVector2D(G.GetAbsoluteSize()) * 0.5;
			if (Win.IsValid())
			{
				for (const FWidgetAndPointer& W : Win->GetHittestGrid().GetBubblePath(Centre, 0.f, true, WalkUser)) if (W.Widget == Slate) bWalkGridHit = true;
			}
		}
		bWalkBlind = false;
		if (Seed >= 0) { WalkReached.Add(Seed); WalkQueue.Add(Seed); }
		WalkNode = -1;
		WalkPhase = 0;
		return;
	}
	if (WalkNode == -1)
	{
		if (WalkQueue.Num())
		{
			WalkNode = WalkQueue.Pop();
			WalkDir = 0;
			WalkPhase = 0;
			return;
		}
		WalkNode = -3;
		WalkPhase = 0;
		return;
	}
	if (WalkNode == -3)
	{
		// B from the place the screen opened on: the screen's parent is back (lane A187: MAIN's parent is the landing; MAIN is not B-pressed here).
		const bool bMain = Walked == EJamMenuScreen::Main;
		if (WalkPhase == 1 && !bMain)
		{
			const int32 From = WalkStart >= 0 ? WalkStart : 0;
			if (WalkNodes.IsValidIndex(From) && WalkNodes[From].IsValid() && WalkNodes[From]->GetCachedWidget().IsValid())
				FSlateApplication::Get().SetUserFocus(WalkUser, WalkNodes[From]->GetCachedWidget(), EFocusCause::SetDirectly);
		}
		if (WalkPhase == 3 && !bMain) WalkPress(EKeys::Gamepad_FaceButton_Right);
		if (WalkPhase < 5) return;
		const bool bBack = Screen == ParentOf(Walked);
		int32 Traps = 0;
		TArray<FString> Unreached;
		for (int32 i = 0; i < WalkNodes.Num(); ++i)
		{
			if (!WalkReached.Contains(i)) Unreached.Add(WalkLabel(WalkNodes[i].Get()));
			else if (WalkNodes.Num() > 1 && WalkOut[i] == 0) ++Traps;
		}
		const FString OpensOnText = WalkNodes.IsValidIndex(WalkStart) ? WalkLabel(WalkNodes[WalkStart].Get()) : FString(TEXT("nothing"));
		bWalkBlind = WalkNodes.Num() > 1 && WalkEdges == 0 && !bWalkGridHit;
		const TCHAR* BackText = bMain ? TEXT("n/a") : (bBack ? TEXT("yes") : TEXT("NO"));
		if (bWalkBlind)
		{
			UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU walk screen=%s focusables=%d grid=blind moves=0 grid_hit=0 opens_on=%s back=%s"), *ScreenName(Walked), WalkNodes.Num(), *OpensOnText, BackText);
			++WalkBlindScreens;
			Traps = 0;
			WalkReached.Reset();
		}
		else
		{
			UE_LOG(LogJamMenuUi, Log, TEXT("JAMMENU walk screen=%s focusables=%d reached=%d traps=%d moves=%d grid_hit=%d opens_on=%s back=%s unreachable=[%s]"), *ScreenName(Walked),
				WalkNodes.Num(), WalkReached.Num(), Traps, WalkEdges, bWalkGridHit ? 1 : 0, *OpensOnText, BackText, *FString::Join(Unreached, TEXT(" | ")));
		}
		WalkTotals[0] += WalkNodes.Num();
		WalkTotals[1] += WalkReached.Num();
		WalkTotals[2] += Traps;
		if (!bBack && !bMain) ++WalkTotals[3];
		++WalkScreen;
		WalkNode = -2;
		WalkPhase = 0;
		return;
	}

	// Probing one place in one direction: stand on it (tick 1), press (tick 3), see where the focus went (tick 4).
	UWidget* Here = WalkNodes.IsValidIndex(WalkNode) ? WalkNodes[WalkNode].Get() : nullptr;
	// Left / right on the slider move the volume, and on PLAY NOW they step the level: neither is a way to another place.
	const bool bSkip = !Here || !Here->GetCachedWidget().IsValid() || ((Here->IsA<USlider>() || Here == PlayNowButton.Get()) && WalkDir >= 2);
	if (!bSkip)
	{
		if (WalkPhase == 1) FSlateApplication::Get().SetUserFocus(WalkUser, Here->GetCachedWidget(), EFocusCause::SetDirectly);
		if (WalkPhase == 3) WalkPress(Dirs[WalkDir]);
		if (WalkPhase < 4) return;
		const int32 To = WalkFocusedIndex();
		if (To >= 0 && To != WalkNode)
		{
			++WalkOut[WalkNode];
			++WalkEdges;
			if (!WalkReached.Contains(To)) { WalkReached.Add(To); WalkQueue.Add(To); }
		}
	}
	WalkPhase = 0;
	if (++WalkDir >= 4) WalkNode = -1;
}
