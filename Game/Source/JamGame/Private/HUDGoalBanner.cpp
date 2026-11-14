// Lane A188 (track F3, evolution 8, R-0064 point 9: "HUD middle screen large banner saying 'Goal'"; docs/experiments/BOOSTS.md). Presentation only.
//   The GOAL banner: on a goal, a plate across the middle band above the play in the scoring side's colour, the word GOAL large and the new score under
//   it. In over 12 sim frames (a punch from 125 % to 100 % while it fades in), held, out over 12 frames that end 6 frames before the goal's pause ends,
//   so it is gone before the faceoff's countdown. Its band: centre at 30 % of the screen's height (between the scoreboard and the human's figure, which
//   the camera holds in the middle third); 20.5 % (just under the scoreboard) when the scorer's figure would stand under it. One JAMHUD goal_banner line a goal.
//   Built on first use (not in BuildTree: other lanes edit it today); RefreshFromFrame calls UpdateGoalBanner once a frame.
//   jam.hud.GoalBanner 0 = no banner (the screen of before). jam.hud.GoalBannerStill N > 0: a still N sim frames into each banner (proof).
//   And the ability plate's words (lane A84's plate, R-0064 point 6): "F: SHOTGUN  READY", the key read from the live bindings; NEXT SHOT / NEXT HIT /
//   NEXT PASS while an armed kind waits for what fires it (the window lane read "F SHOTGUN ARMED" after `ability used` as a fault).
#include "HUDWidget.h"
#include "MenuStyle.h"
#include "InputMapping.h"
#include "HokejGameMode.h"
#include "JamSim.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Engine/Texture2D.h"

// Lane A196 (track F3, evolution 9, item 3): device B's GOAL banner art (A-to-B-0027 points 3b and 6), /JamAssets/UI/T_UI_Banner_Goal, by name: when the
// texture exists the plate draws it (B's art as drawn, a quarter of the scoring side's colour in the tint so the two sides still differ); else A's rounded
// plate stays. Looked up once a process (quiet). One JAMHUD goal_banner_art line the first time.
static bool GoalBannerArtBrush(UBorder* Plate, const FLinearColor& Team)
{
	static bool bLooked = false;
	static TWeakObjectPtr<UTexture2D> Art;
	if (!bLooked)
	{
		bLooked = true;
		Art = LoadObject<UTexture2D>(nullptr, TEXT("/JamAssets/UI/T_UI_Banner_Goal.T_UI_Banner_Goal"), nullptr, LOAD_NoWarn | LOAD_Quiet);
		UE_LOG(LogTemp, Log, TEXT("JAMHUD goal_banner_art=%s"), Art.IsValid() ? TEXT("T_UI_Banner_Goal") : TEXT("none (A's plate)"));
	}
	if (!Plate || !Art.IsValid()) return false;
	FSlateBrush B;
	B.SetResourceObject(Art.Get());
	B.DrawAs = ESlateBrushDrawType::Image;
	B.ImageSize = FVector2D(Art->GetSizeX(), Art->GetSizeY());
	B.TintColor = FSlateColor(FMath::Lerp(FLinearColor::White, FLinearColor(Team.R, Team.G, Team.B, 1.f), 0.25f));
	Plate->SetBrush(B);
	return true;
}

// Lane A196: a goal is a number that grew (the pause after a foul's whistle is the same phase).
static bool GoalBannerScoreGrew(int32 LastHome, int32 LastAway, int32 Home, int32 Away) { return Home > LastHome || Away > LastAway; }

static TAutoConsoleVariable<int32> CVarJamHudGoalBanner(TEXT("jam.hud.GoalBanner"), 1,
	TEXT("Lane A188: 1 (default) the GOAL banner across the middle band on a goal; 0 none (the screen of before)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudGoalBannerStill(TEXT("jam.hud.GoalBannerStill"), 0,
	TEXT("Proof (lane A188): N > 0 takes a still with the HUD N sim frames into each GOAL banner: u1-goal-<tag>-f<frame>.png (tag: jam.hud.MarkStillTag). 0 off."), ECVF_Default);

namespace
{
	constexpr float BannerW = 760.f;   // slate units at 1080p
	constexpr float BannerH = 176.f;
	bool GBannerStillTaken = false;
}

float UJamHudWidget::GoalBannerAlpha(int32 Since, int32 PauseFrames)
{
	if (Since < 0 || PauseFrames <= 0) return 0.f;
	int32 In = GoalBannerInFrames, Out = GoalBannerOutFrames, Lead = GoalBannerLeadFrames;
	if (PauseFrames < In + Out + Lead + 12) { In = Out = FMath::Max(1, PauseFrames / 3); Lead = 0; }   // a short pause: thirds
	const int32 End = PauseFrames - Lead;   // the frame it is gone by
	if (Since >= End) return 0.f;
	if (Since < In) return static_cast<float>(Since + 1) / static_cast<float>(In);
	if (Since >= End - Out) return FMath::Clamp(static_cast<float>(End - Since) / static_cast<float>(Out), 0.f, 1.f);
	return 1.f;
}

float UJamHudWidget::GoalBannerCentreY(float ScorerTopY, float ScorerFootY)
{
	const float Mid = 0.30f, High = 0.205f;   // 0.205: the plate's top just under the scoreboard (its foot at 0.09 of the height)
	if (ScorerTopY < 0.f) return Mid;
	// The band at 0.30 covers 0.30 +- half the banner's height (176 of 1080: +-0.0815); a figure that reaches into it moves the banner up.
	// Under both (a small figure far up the ice): the band he overlaps least.
	const float Half = BannerH * 0.5f / 1080.f;
	auto Overlap = [&](float Y) { return FMath::Max(0.f, FMath::Min(ScorerFootY, Y + Half) - FMath::Max(ScorerTopY, Y - Half)); };
	const float AtMid = Overlap(Mid);
	if (AtMid <= 0.f) return Mid;
	return Overlap(High) < AtMid ? High : Mid;
}

FString UJamHudWidget::GoalBannerScore(int32 Home, int32 Away)
{
	return FString::Printf(TEXT("%d : %d"), Home, Away);
}

FString UJamHudWidget::AbilityKeyName()
{
	// The ABILITY row of the live bindings ("ABILITY  F / R3"): its first key. The controls lane fills the table from the rebinding; the name stays.
	TArray<FString> Rows;
	UJamInputMap::BindingTableText(false).ParseIntoArrayLines(Rows);
	for (const FString& R : Rows)
	{
		FString L = R.TrimStartAndEnd();
		if (!L.StartsWith(TEXT("ABILITY"))) continue;
		L = L.RightChop(7).TrimStartAndEnd();
		FString First, Rest;
		if (!L.Split(TEXT("/"), &First, &Rest)) First = L;
		First = First.TrimStartAndEnd();
		if (!First.IsEmpty()) return First;
	}
	return TEXT("F");
}

FString UJamHudWidget::AbilityPlateText(const FString& Kind, const FString& State)
{
	return FString::Printf(TEXT("%s: %s  %s"), *AbilityKeyName(), *Kind, *State).TrimEnd();
}

FString UJamHudWidget::AbilityArmedWord(FName Kind)
{
	const FString K = Kind.ToString();
	if (K == TEXT("shotgun")) return TEXT("NEXT SHOT");
	if (K == TEXT("hip_check")) return TEXT("NEXT HIT");
	if (K == TEXT("slow_puck")) return TEXT("NEXT PASS");
	return TEXT("ARMED");
}

FString UJamHudWidget::GetGoalBannerText() const
{
	if (!GoalBannerPlate || GoalBannerPlate->GetVisibility() == ESlateVisibility::Collapsed || !GoalBannerWord) return FString();
	return GoalBannerWord->GetText().ToString() + TEXT(" ") + (GoalBannerScoreText ? GoalBannerScoreText->GetText().ToString() : FString());
}

void UJamHudWidget::UpdateGoalBanner(const FJamPresentationFrame& F)
{
	if (!WidgetTree || !Root) return;
	const bool bOn = CVarJamHudGoalBanner.GetValueOnGameThread() != 0;
	if (!GoalBannerPlate)
	{
		UWidgetTree* T = WidgetTree;
		GoalBannerPlate = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("GoalBannerPlate"));
		GoalBannerPlate->SetPadding(FMargin(24.f, 6.f, 24.f, 10.f));
		GoalBannerPlate->SetHorizontalAlignment(HAlign_Center);
		GoalBannerPlate->SetVerticalAlignment(VAlign_Center);
		UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("GoalBannerBox"));
		GoalBannerWord = T->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GoalBannerWord"));
		GoalBannerWord->SetFont(FJamUiStyle::Font(84, FName("Black"), 120, true));
		GoalBannerWord->SetColorAndOpacity(FSlateColor(FJamUiStyle::Paper()));
		GoalBannerWord->SetJustification(ETextJustify::Center);
		GoalBannerWord->SetText(FText::FromString(TEXT("GOAL")));
		GoalBannerScoreText = T->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GoalBannerScore"));
		GoalBannerScoreText->SetFont(FJamUiStyle::ScoreFont(32, true));
		GoalBannerScoreText->SetColorAndOpacity(FSlateColor(FJamUiStyle::Paper()));
		GoalBannerScoreText->SetJustification(ETextJustify::Center);
		if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(GoalBannerWord)) { S->SetHorizontalAlignment(HAlign_Center); S->SetPadding(FMargin(0, -12, 0, -8)); }
		if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(GoalBannerScoreText)) S->SetHorizontalAlignment(HAlign_Center);
		GoalBannerPlate->SetContent(Box);
		if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(GoalBannerPlate))
		{
			CS->SetAnchors(FAnchors(0.5f, 0.30f));
			CS->SetAlignment(FVector2D(0.5f, 0.5f));
			CS->SetPosition(FVector2D::ZeroVector);
			CS->SetSize(FVector2D(BannerW, BannerH));
			CS->SetZOrder(40);   // over the play's marks, under the pause and end cards' plates (they hide it: no banner outside the goal pause)
		}
		GoalBannerPlate->SetVisibility(ESlateVisibility::Collapsed);
		GoalBannerPlate->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	}
	const bool bPause = F.Phase == EJamPhase::GoalPause;
	if (GoalBannerLastHome < 0 || F.SimFrame < LastSimFrame) { GoalBannerLastHome = F.ScoreHome; GoalBannerLastAway = F.ScoreAway; }
	if (!bPause || !bOn)
	{
		GoalBannerStart = -1;
		GoalBannerOpacity = 0.f;
		GBannerStillTaken = false;
		GoalBannerLastHome = F.ScoreHome;
		GoalBannerLastAway = F.ScoreAway;
		if (GoalBannerPlate->GetVisibility() != ESlateVisibility::Collapsed) GoalBannerPlate->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const int32 Pause = GM && GM->GetSim() ? GM->GetSim()->GetConfig().Rules.GoalPauseFrames : 120;
	// Lane A196 (evolution 10, found in the strips): the sim's GoalPause also holds the play after a foul's whistle (the drills' checks on a man without
	// the puck): GOAL 0:0 showed on three hits a match. A banner only when a side's number grew.
	if (GoalBannerStart < 0 && !GoalBannerScoreGrew(GoalBannerLastHome, GoalBannerLastAway, F.ScoreHome, F.ScoreAway))
	{
		if (GoalBannerPlate->GetVisibility() != ESlateVisibility::Collapsed) GoalBannerPlate->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	if (GoalBannerStart < 0)
	{
		// The goal: who scored (the side whose number grew; home = the human's side), the band clear of the scorer, one line.
		GoalBannerStart = F.SimFrame;
		GoalBannerTeam = F.ScoreAway > GoalBannerLastAway && F.ScoreHome <= GoalBannerLastHome ? 1 : 0;
		const int32 HumanTeam = F.Skaters.IsValidIndex(F.HumanIndex) ? F.Skaters[F.HumanIndex].TeamId : 0;
		const int32 ScoringTeamId = GoalBannerTeam == 0 ? HumanTeam : 1 - HumanTeam;
		// The scorer, as the presentation frame knows him: the scoring side's skater nearest the puck in the net.
		int32 Scorer = -1;
		double Best = 1e18;
		for (int32 i = 0; i < F.Skaters.Num(); ++i)
		{
			if (F.Skaters[i].TeamId != ScoringTeamId) continue;
			const double D = FVector2D::DistSquared(F.Skaters[i].PosM, F.PuckM);
			if (D < Best) { Best = D; Scorer = i; }
		}
		float TopY = -1.f, FootY = -1.f;
		APlayerController* PC = GetOwningPlayer();
		const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this) / FMath::Max(0.01f, UWidgetLayoutLibrary::GetViewportScale(this));
		if (PC && Scorer >= 0 && VS.Y > 1.f)
		{
			FVector2D Head, Foot;
			if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, UJamSim::ToWorld(F.Skaters[Scorer].PosM, 2.0), Head, false)
				&& UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, UJamSim::ToWorld(F.Skaters[Scorer].PosM, 0.0), Foot, false)
				&& Head.X > -50.f && Head.X < VS.X + 50.f)
			{
				TopY = FMath::Min(Head.Y, Foot.Y) / VS.Y;
				FootY = FMath::Max(Head.Y, Foot.Y) / VS.Y;
				// Only the part of the banner's width he stands under counts: a scorer at the screen's side is never under a centred plate.
				const float HalfW = BannerW * 0.5f * VS.Y / 1080.f;
				if (Head.X < VS.X * 0.5f - HalfW - 40.f || Head.X > VS.X * 0.5f + HalfW + 40.f) TopY = FootY = -1.f;
			}
		}
		GoalBannerY = GoalBannerCentreY(TopY, FootY);
		if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(GoalBannerPlate->Slot)) CS->SetAnchors(FAnchors(0.5f, GoalBannerY));
		const FLinearColor Team = F.GetTeamColour(ScoringTeamId);
		GoalBannerPlate->SetBrush(FJamUiStyle::Rounded(FLinearColor(Team.R, Team.G, Team.B, 0.94f), 14.f, FJamUiStyle::Paper(), 4.f));
		// Lane A196 (R-0064 point 9): device B's banner art by name when it exists. B's art carries the word GOAL itself: A's word stays in the layout
		// (the score keeps its place under it) but is not drawn over B's (the first still showed GOAL twice).
		const bool bArt = GoalBannerArtBrush(GoalBannerPlate, Team);
		GoalBannerWord->SetRenderOpacity(bArt ? 0.f : 1.f);
		GoalBannerScoreText->SetText(FText::FromString(GoalBannerScore(F.ScoreHome, F.ScoreAway)));
		UE_LOG(LogTemp, Log, TEXT("JAMHUD goal_banner frame=%lld side=%s team=%d score=%d:%d band_y=%.2f scorer=%d scorer_y=%.2f..%.2f pause=%d in=%d out_by=%d"),
			F.SimFrame, GoalBannerTeam == 0 ? TEXT("home") : TEXT("away"), ScoringTeamId, F.ScoreHome, F.ScoreAway, GoalBannerY, Scorer, TopY, FootY, Pause,
			GoalBannerInFrames, Pause - GoalBannerLeadFrames);
	}
	const int32 Since = static_cast<int32>(F.SimFrame - GoalBannerStart);
	GoalBannerOpacity = GoalBannerAlpha(Since, Pause);
	GoalBannerPlate->SetVisibility(GoalBannerOpacity > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	GoalBannerPlate->SetRenderOpacity(GoalBannerOpacity);
	// The punch: 125 % to 100 % over the in; 100 % to 96 % over the out.
	const float S = Since < GoalBannerInFrames ? 1.25f - 0.25f * GoalBannerOpacity : 0.96f + 0.04f * GoalBannerOpacity;
	GoalBannerPlate->SetRenderScale(FVector2D(S, S));
	const int32 StillAfter = CVarJamHudGoalBannerStill.GetValueOnGameThread();
	if (StillAfter > 0 && !GBannerStillTaken && Since >= StillAfter && GoalBannerOpacity > 0.f)
	{
		GBannerStillTaken = true;
		IConsoleVariable* Tag = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.hud.MarkStillTag"));
		const FString Name = FString::Printf(TEXT("u1-goal-%s-f%06lld"), Tag ? *Tag->GetString() : TEXT("p3_clay"), F.SimFrame);
		FScreenshotRequest::RequestScreenshot(Name + TEXT(".png"), true, false);
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld goal_still=%s.png (%d sim frames into the banner)"), F.SimFrame, *Name, Since);
	}
}

#if WITH_DEV_AUTOMATION_TESTS
// Jam.Hud.GoalBanner: the banner's timing, band and words, and the ability plate's words (lane A188). Pure: no world.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudGoalBanner, "Jam.Hud.GoalBanner", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudGoalBanner::RunTest(const FString&)
{
	// The default pause (120 frames): in over 12, held, out over 12 ending at frame 114, gone 6 frames before the countdown.
	TestEqual(TEXT("before the goal: nothing"), UJamHudWidget::GoalBannerAlpha(-1, 120), 0.f);
	TestTrue(TEXT("the first frame: under a tenth"), UJamHudWidget::GoalBannerAlpha(0, 120) < 0.1f);
	TestEqual(TEXT("in after 12 frames"), UJamHudWidget::GoalBannerAlpha(11, 120), 1.f);
	TestEqual(TEXT("held"), UJamHudWidget::GoalBannerAlpha(60, 120), 1.f);
	TestTrue(TEXT("going out"), UJamHudWidget::GoalBannerAlpha(108, 120) < 1.f && UJamHudWidget::GoalBannerAlpha(108, 120) > 0.f);
	TestEqual(TEXT("gone by frame 114"), UJamHudWidget::GoalBannerAlpha(114, 120), 0.f);
	TestEqual(TEXT("gone at the countdown"), UJamHudWidget::GoalBannerAlpha(120, 120), 0.f);
	// A short pause (30, the schema's least): thirds, still gone by its end.
	TestTrue(TEXT("a short pause shows it"), UJamHudWidget::GoalBannerAlpha(12, 30) > 0.99f);
	TestEqual(TEXT("a short pause: gone at its end"), UJamHudWidget::GoalBannerAlpha(30, 30), 0.f);
	// The band: the middle band above the play; higher when the scorer would stand under it.
	TestEqual(TEXT("no scorer on screen: the middle band"), UJamHudWidget::GoalBannerCentreY(-1.f, -1.f), 0.30f);
	TestEqual(TEXT("the scorer low on the screen: the middle band"), UJamHudWidget::GoalBannerCentreY(0.45f, 0.70f), 0.30f);
	TestEqual(TEXT("the scorer's head in the band: up"), UJamHudWidget::GoalBannerCentreY(0.33f, 0.55f), 0.205f);
	TestEqual(TEXT("the scorer far up the ice, inside the band: up"), UJamHudWidget::GoalBannerCentreY(0.26f, 0.31f), 0.205f);
	TestEqual(TEXT("the score as the board reads"), UJamHudWidget::GoalBannerScore(2, 1), FString(TEXT("2 : 1")));
	// The plate: the key from the live bindings, the kind, the state; what fires an armed kind, not ARMED.
	TestEqual(TEXT("the key from the bindings"), UJamHudWidget::AbilityKeyName(), FString(TEXT("F")));
	TestEqual(TEXT("the plate"), UJamHudWidget::AbilityPlateText(TEXT("SHOTGUN"), TEXT("READY")), FString(TEXT("F: SHOTGUN  READY")));
	TestEqual(TEXT("an armed shotgun waits for the shot"), UJamHudWidget::AbilityArmedWord(TEXT("shotgun")), FString(TEXT("NEXT SHOT")));
	TestEqual(TEXT("an armed hip check waits for the hit"), UJamHudWidget::AbilityArmedWord(TEXT("hip_check")), FString(TEXT("NEXT HIT")));
	TestEqual(TEXT("an armed slow puck waits for the pass"), UJamHudWidget::AbilityArmedWord(TEXT("slow_puck")), FString(TEXT("NEXT PASS")));
	TestEqual(TEXT("the banner is on by default"), IConsoleManager::Get().FindConsoleVariable(TEXT("jam.hud.GoalBanner"))->GetInt(), 1);
	// Lane A196: no banner for a pause whose score did not grow (a foul's whistle).
	TestFalse(TEXT("a foul's pause: no GOAL"), GoalBannerScoreGrew(0, 0, 0, 0));
	TestTrue(TEXT("a goal: the banner"), GoalBannerScoreGrew(0, 1, 0, 2));
	return true;
}
#endif
