// Lane A213 (track U3, the night of 14 to 15 Nov; docs/experiments/SCREEN-TELLS.md section 2): one line of words at a time. The inventory's stills of
// the screen of before showed four lines of words at the first faceoff (the MATCH card, the faceoff hint, the commentator's caption, the controls strip)
// and two in play (the kept-puck prompt with a caption under it; the prompt still up five seconds after the human lost the puck). Presentation only:
// UpdateOneLine runs last in RefreshFromFrame, after every element's own update decided whether it is due this frame; it only hides (Collapsed) the
// lines that lose, for this frame. Each line's own rule shows it again on the next frame it is still due. jam.hud.OneLine 0 = the screen of before.
#include "HUDWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HokejGameMode.h"
#include "JamSim.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/PlayerController.h"

static TAutoConsoleVariable<int32> CVarJamHudOneLine(TEXT("jam.hud.OneLine"), 1,
	TEXT("Lane A213: 1 (default) at most one line of the HUD's words over the play at a time (contact line, prompt, controls strip, faceoff hint, caption, in that order) and the kept-puck prompt only while the human carries; 0 = the screen of before."), ECVF_Default);

bool UJamHudWidget::OneLineOn() { return CVarJamHudOneLine.GetValueOnGameThread() != 0; }
static constexpr int64 AfterDropQuietFrames = 60;   // 4d: the draw's own second (the dial's WON / LOST stands then)
static TAutoConsoleVariable<int32> CVarJamHudKeptPrompt(TEXT("jam.hud.KeptPrompt"), 0,
	TEXT("Lane A213 (wave 4): 0 (default) the kept-puck prompt (THE PUCK IS YOURS UNTIL SOMEONE PRESSES FOR IT) is not drawn: the double ring says it; 1 = drawn (while he carries)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudOwnThird(TEXT("jam.hud.OwnThird"), 1,
	TEXT("Lane A213 (wave 3): 1 (default) the own-net label (own NN m, its diamond and arrow) shows only while the human is in his own third and never on the radar's frame; 0 = always (as before)."), ECVF_Default);

// Wave 4c (A211's fourth list, "a small grey box with marks at the bottom centre", pro f280): it was the off-screen puck's marker ("3 m" and its
// corner chevron) fading out while the puck itself was in the frame a few metres away. The marker is for a puck the player cannot see: not drawn on a
// frame the puck's own point projects inside the frame (with a margin). jam.hud.PuckLabelOnScreen 1 = as before.
static TAutoConsoleVariable<int32> CVarJamHudPuckLabelOnScreen(TEXT("jam.hud.PuckLabelOnScreen"), 0,
	TEXT("Lane A213 (wave 4c): 0 (default) the off-screen puck's marker (distance and chevron) is not drawn while the puck is inside the frame; 1 = as before (it faded out over the visible puck: the grey box)."), ECVF_Default);

void UJamHudWidget::ApplyOwnLabelRule(const FJamPresentationFrame& F)
{
	if (CVarJamHudPuckLabelOnScreen.GetValueOnGameThread() == 0)
	{
		FVector2D Px;
		const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this) / FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, UJamSim::ToWorld(F.PuckM, 0.05), Px, false)
				&& Px.X > 24.f && Px.Y > 24.f && Px.X < VS.X - 24.f && Px.Y < VS.Y - 24.f)
			{
				UWidget* const Puck[] = { PuckArrow.Get(), PuckText.Get(), PuckPlate.Get() };
				for (UWidget* W : Puck) if (W) W->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
	if (CVarJamHudOwnThird.GetValueOnGameThread() == 0 || !F.Skaters.IsValidIndex(F.HumanIndex)) return;
	bool bHide = !InOwnThird(F.Skaters[F.HumanIndex].PosM.X, F.AttackSign, F.RinkLengthM);
	if (!bHide && OwnText && Radar)
	{
		// The radar's frame: bottom right, CornerFurnitureW x CornerFurnitureH (the ability plate's band included): the label slid under it (the neon stills).
		FBox2D B(ForceInit);
		const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this) / FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
		if (WidgetBoxOnScreen(OwnText.Get(), B) && BoxesMeet(B.Min, B.Max, FVector2D(VS.X - CornerFurnitureW, VS.Y - CornerFurnitureH), VS, 4.f)) bHide = true;
	}
	if (!bHide) return;
	UWidget* const Own[] = { OwnText.Get(), OwnDiamond.Get(), OwnArrow.Get(), OwnPlate.Get() };
	for (UWidget* W : Own) if (W) W->SetVisibility(ESlateVisibility::Collapsed);
}
uint64 UJamHudWidget::OneLineTick = 0;
bool UJamHudWidget::bOneLineUp = false;
bool UJamHudWidget::OneLineUpNow() { return OneLineOn() && bOneLineUp && GFrameCounter <= OneLineTick + 1; }

void UJamHudWidget::UpdateOneLine(const FJamPresentationFrame& F)
{
	ApplyOwnLabelRule(F);   // wave 3: its own switch (jam.hud.OwnThird)
	if (!OneLineOn()) { OneLineShown = -1; return; }
	auto Up = [](const UWidget* W) { return W && W->GetVisibility() != ESlateVisibility::Collapsed && W->GetRenderOpacity() > 0.f; };
	// The kept-puck prompt says "the puck is yours": not on a frame it is not (the sim's carrier, never the figure's flag).
	if (Up(PromptPlate.Get()) && PromptTextBlock && PromptTextBlock->GetText().ToString() == LivePromptText(EPrompt::Kept))
	{
		const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
		const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
		if (Sim && PossessFromSim(*Sim) != EPossess::You) PromptPlate->SetVisibility(ESlateVisibility::Collapsed);
	}
	// Wave 4 (A211's fourth list): "THE PUCK IS YOURS UNTIL SOMEONE PRESSES FOR IT" said what the double ring already shows: cut (jam.hud.KeptPrompt 0).
	if (CVarJamHudKeptPrompt.GetValueOnGameThread() == 0 && Up(PromptPlate.Get()) && PromptTextBlock && PromptTextBlock->GetText().ToString() == LivePromptText(EPrompt::Kept))
		PromptPlate->SetVisibility(ESlateVisibility::Collapsed);
	// Wave 4 (pro f200): HE JUST GOT IT names the other man's take; once the human carries the puck himself it stood under HIS ring: not drawn then.
	if (Up(ContactPlate.Get()) && ContactText && ContactText->GetText().ToString() == CarrierCueLine(false, true))
	{
		const AHokejGameMode* GM2 = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
		// 4d (the critic, pro seed 5 f100-120: the dial said WON while this line said HE JUST GOT IT): not in the draw's first second either.
		const bool bJustDropped = LastDropFrame >= 0 && F.SimFrame >= LastDropFrame && F.SimFrame - LastDropFrame < AfterDropQuietFrames;
		if (bJustDropped || (GM2 && GM2->GetSim() && PossessFromSim(*GM2->GetSim()) == EPossess::You)) ContactPlate->SetVisibility(ESlateVisibility::Collapsed);
	}
	UWidget* const Lines[5] = { ContactPlate.Get(), PromptPlate.Get(), ControlsStrip.Get(), FaceoffHintPlate.Get(), CommentaryStrip.Get() };
	int32 Win = OneLineWinner(Up(Lines[0]), Up(Lines[1]), Up(Lines[2]), Up(Lines[3]), Up(Lines[4]));
	for (int32 i = 0; i < 5; ++i)
	{
		if (i == Win || !Lines[i] || !Up(Lines[i])) continue;
		Lines[i]->SetVisibility(ESlateVisibility::Collapsed);
		if (i == 2 && ControlsText) ControlsText->SetVisibility(ESlateVisibility::Collapsed);
	}
	// The caption never sits on another label (A211's list 2: in its slot under the scoreboard it lay on the far net's distance label, f180 to 300).
	if (Win == 4 && CommentaryStrip)
	{
		FBox2D Cap(ForceInit);
		if (WidgetBoxOnScreen(CommentaryStrip.Get(), Cap))
		{
			const UWidget* const Labels[] = { NetDiamond.Get(), NetText.Get(), NetPlate.Get(), NetArrow.Get(), OwnDiamond.Get(), OwnText.Get(), OwnArrow.Get(), PuckArrow.Get(), PuckText.Get(), PromptPlate.Get() };
			for (const UWidget* L : Labels)
			{
				FBox2D B(ForceInit);
				if (L && Up(L) && WidgetBoxOnScreen(L, B) && BoxesMeet(Cap.Min, Cap.Max, B.Min, B.Max, KeepClearPadPx))
				{
					CommentaryStrip->SetVisibility(ESlateVisibility::Collapsed);
					Win = -1;
					break;
				}
			}
		}
	}
	bOneLineUp = Win >= 0;
	OneLineTick = GFrameCounter;
	if (Win != OneLineShown)
	{
		static const TCHAR* Names[5] = { TEXT("contact"), TEXT("prompt"), TEXT("strip"), TEXT("faceoff_hint"), TEXT("caption") };
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld one_line=%s"), F.SimFrame, Win >= 0 ? Names[Win] : TEXT("none"));
		OneLineShown = Win;
	}
}
