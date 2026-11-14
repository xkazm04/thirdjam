// The match HUD: score as a film counter, "TAKE n", timer, far-net screen marker with distance, radar, Director's Call prompt,
// win/lose cards and the restart hint. A C++ UMG widget whose tree is built in code (no widget blueprint needed). Lane A5.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDFeed.h"
#include "RadarWidget.h"
#include "JamEffectTypes.h"
#include "JamEventBus.h"
#include "InputCoreTypes.h"
#include "HUDWidget.generated.h"

class UCanvasPanel;
class UTextBlock;
class UBorder;
class UImage;
class USceneComponent;

// Where the far-net marker is this frame (Slate units, i.e. DPI-scaled viewport coordinates).
USTRUCT(BlueprintType)
struct JAMGAME_API FJamNetMarker
{
	GENERATED_BODY()

	// Clamped to the viewport with a margin; this is where the diamond is drawn.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Hud")
	FVector2D ScreenPos = FVector2D::ZeroVector;

	// True when the net's own projected point lies inside the viewport and in front of the camera (no clamping needed).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Hud")
	bool bOnScreen = false;

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Hud")
	int32 DistanceM = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Hud")
	FString Text;
};

UCLASS()
class JAMGAME_API UJamHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Creates the HUD for a player and adds it to the viewport.
	UFUNCTION(BlueprintCallable, Category = "Jam|Hud")
	static UJamHudWidget* CreateAndAdd(APlayerController* PC);

	// Pure text helpers (tests call them).
	static FString FormatTimer(int32 FramesRemaining);
	static FString FormatTake(int32 Take, int32 ScoreToWin = 0);
	// Frames of a match during which the controls hint shows. Lane A111 (track U1, B-to-A-0038 fault 6): 8 s (was 10), and only in the first match of a
	// session (process): the hint is spent when its window has run out once or the human has taken a first shot (ControlsHintSpent); later matches show nothing
	// (the pause card has the full table). jam.hud.HintEveryMatch 1 gives back the strip in every match.
	static constexpr int64 ControlsHintFrames = 480;
	// Lane A171 (REVIEW-3 item 8: "the first screen never told me Space exists"): the poke and the ability's key are in the strip: nine verbs, three
	// spaces between them (it was four), 143 characters: under the 146 of the ten-verb line this strip's width and type were sized for.
	// Lane A183: from the live bindings (UJamInputMap::HintStripText; defined in InputMapping.cpp).
	static FString ControlsHintText();
	// Lane A191: what the two-role buttons do right now ("LMB  SHOOT     RMB  PASS" with the puck, "LMB  CROSS-CHECK     RMB  CHECK" without), from the
	// live bindings and the sim's possession (JamControls::HasPuckForButtons); a pill bottom left, at most ButtonsBadgeMaxW wide. Defined in InputMapping.cpp.
	static FString ButtonsBadgeLine(bool bWithPuck);
	static constexpr float ButtonsBadgeMaxW = 480.f;
	FString GetButtonsBadgeShown() const;
	static bool IsControlsHintSpent() { return bControlsHintSpent; }
	static void SetControlsHintSpent(bool bSpent) { bControlsHintSpent = bSpent; }   // tests and jam.ui.hintreset
	// The live rule: the frame window, and not spent this session (bEveryMatch: the neutral value, the strip in every match).
	static bool ShowControlsHintLive(const FJamPresentationFrame& F, bool bSpent, bool bEveryMatch) { return ShowControlsHint(F) && (bEveryMatch || !bSpent); }
	// ---- Lane A111 (track U1, B-to-A-0038 fault 7: "the human's ring is the only mark among five blue figures"): the marker over the human's helmet, always
	// on while the match plays: a take-yellow chevron with an ink edge (the HUD's accent, not the team's blue, so it reads on clay and on night ice), its tip
	// YouMarkerGapPx over the point YouMarkerHeadM above the ice (the helmet's top is at 1.65 m); the YOU tag rides above it. It stands over the head, so it never covers the stick or the puck.
	// bMarkerOn false (jam.hud.YouMarker 0) is lane A42's rule: the chevron only as the ring fades (20 to 24 m), the tag inside the ring's arc.
	static constexpr float YouMarkerHeadM = 1.8f;
	static constexpr float YouMarkerGapPx = 4.f;
	static constexpr float YouMarkerSizePx = 48.f;   // 34 read as two thin lines at 1280 x 720 (the first still of the proof run)
	static float YouMarkerAlpha(bool bMarkerOn, float RingAlpha) { return bMarkerOn ? 1.f : 1.f - FMath::Clamp(RingAlpha, 0.f, 1.f); }
	// ---- Lane A111 (B-to-A-0038 fault 6): the commentator's strip is docked on the bottom edge (CommentaryDockPx from it), under the play, not 110 px up over
	// the ice; it steps over the controls strip while that shows (first match only) and over the CALL READY line. bDock false is the earlier place.
	static constexpr float CommentaryDockPx = 24.f;
	static float CommentaryStripY(bool bDock, bool bControlsHint, bool bCallLine)
	{
		// The controls strip stands from 140 to 96 px over the edge (lane A8's ready pill is under it): the commentary keeps its place above it for those 8 s.
		if (!bDock || bControlsHint) return bControlsHint ? -150.f : -110.f;
		return bCallLine ? -76.f : -CommentaryDockPx;
	}
	// ---- Lane A111 (B-to-A-0038 fault 6): the distance labels (far net, own net, the off-screen puck) stand on a dark plate (LabelPlateOpacity, LabelPlatePad
	// around the type), and the net badges are not drawn during the goal pause (the sparks and the horn own the net then).
	static constexpr float LabelPlateOpacity = 0.62f;
	static constexpr float LabelPlatePadX = 8.f;
	static constexpr float LabelPlatePadY = 2.f;
	static bool ShowNetBadges(EJamPhase Phase, bool bHideInGoalPause = true) { return !(bHideInGoalPause && Phase == EJamPhase::GoalPause); }
	// Where a plate of a label stands: the label's canvas position moved so the plate overhangs the type by Pad on every side under the same alignment.
	static FVector2D LabelPlatePos(const FVector2D& LabelPos, const FVector2D& Align) { return LabelPos + FVector2D(LabelPlatePadX * (2.f * Align.X - 1.f), LabelPlatePadY * (2.f * Align.Y - 1.f)); }
	// ---- Lane A123 (track U1, evolution 2; docs/experiments/SCREEN.md part 2): nothing sits on the play, and the screen teaches the two new verbs.
	// "The middle third" is the centre rectangle [W/3, 2W/3] x [H/3, 2H/3] of the frame (lane A49's meaning: where the human and the play are).
	static bool InMiddleThird(const FVector2D& Min, const FVector2D& Max, float VW, float VH)
	{
		return Max.X > VW / 3.f && Min.X < VW * 2.f / 3.f && Max.Y > VH / 3.f && Min.Y < VH * 2.f / 3.f;
	}
	// jam.hud.ClearMiddle (1 default, 0 the screen of before): the bark bubbles, the YOU tag and the net labels keep out of the middle third during play.
	static bool ClearMiddleOn();
	// ---- Lane A158 (track U1, evolution 5; B-to-A-0041 point 1): nothing is drawn on top of the key feature (the fall, the lying, the get-up).
	// jam.hud.ClearDown 1 (default; 0 = the screen of before):
	//   - the ring under the human is not drawn while he is down (the chevron over the helmet stays); it comes back over RingBackFrames after the get-up;
	//   - a marker at a skater's feet or over the world (the carrier's ring, the net badges and their labels, the puck arrow) is not drawn on a frame its box
	//     meets a downed figure's box;
	//   - the commentator's caption leaves its dock on the bottom edge for the slot under the scoreboard while the human's box (up or down) or a downed
	//     figure's box is under the dock; it keeps the place it took until its line ends (no hopping); with both places taken it is not drawn that frame;
	//   - the contact line steps sideways along its own row, to the side with room, off the human and off a downed figure;
	//   - the bark lane yields to the same boxes and to a net (UJamBarkWidget reads GetKeepClear).
	// A figure's box: its bones projected onto the screen plus FigurePadCm (a skinned figure: device B's), else the bounds of its meshes.
	static bool ClearDownOn();
	static constexpr int32 RingBackFrames = 12;
	// The human's ring beside a downed figure (he stands over the man he felled, or beside a mate who is down): it lay across that body too. It fades out
	// over RingYieldFrames while its box meets a downed figure's and comes back the same way. Pure: the share after one sim frame.
	static constexpr int32 RingYieldFrames = 6;
	static float RingYieldStep(float Share, bool bOnDowned) { return FMath::Clamp(Share + (bOnDowned ? -1.f : 1.f) / static_cast<float>(RingYieldFrames), 0.f, 1.f); }
	static constexpr float FigurePadCm = 14.f;
	static constexpr float KeepClearPadPx = 6.f;
	// The ring's share while the rule is on: 0 down, a ramp over RingBackFrames from the frame he is up again (UpSinceFrame; < 0 = never down). Pure.
	static float RingDownAlpha(bool bRuleOn, bool bDown, int64 SimFrame, int64 UpSinceFrame)
	{
		if (!bRuleOn) return 1.f;
		if (bDown) return 0.f;
		if (UpSinceFrame < 0 || SimFrame < UpSinceFrame) return 1.f;
		return FMath::Clamp(static_cast<float>(SimFrame - UpSinceFrame + 1) / static_cast<float>(RingBackFrames), 0.f, 1.f);
	}
	static bool BoxesMeet(const FVector2D& AMin, const FVector2D& AMax, const FVector2D& BMin, const FVector2D& BMax, float Pad = 0.f)
	{
		return AMin.X - Pad < BMax.X && AMax.X + Pad > BMin.X && AMin.Y - Pad < BMax.Y && AMax.Y + Pad > BMin.Y;
	}
	static bool BoxMeetsAny(const FVector2D& Min, const FVector2D& Max, const TArray<FBox2D>& Boxes, float Pad = KeepClearPadPx)
	{
		for (const FBox2D& B : Boxes) if (BoxesMeet(Min, Max, B.Min, B.Max, Pad)) return true;
		return false;
	}
	// The caption's place: 0 its dock on the bottom edge, 1 the slot under the scoreboard, -1 not drawn this frame. Current: the place this line holds
	// (-1 or 0 at a new line). It stays where it is while that place is free; else it takes the other when that is free. Pure.
	static int32 CaptionPlace(int32 Current, bool bDockFree, bool bTopFree)
	{
		const int32 Cur = Current == 1 ? 1 : 0;
		if (Cur == 0 ? bDockFree : bTopFree) return Cur;
		if (Cur == 0 ? bTopFree : bDockFree) return 1 - Cur;
		return -1;
	}
	// The contact line's step along its row: the centre x (Slate units) nearest to WantX at which the box [x - HalfW, x + HalfW] x [Y0, Y1] meets none of
	// Boxes (plus Pad) inside [MinX, MaxX]; false when the row has no such place. Pure.
	static bool SideStep(float WantX, float HalfW, float Y0, float Y1, const TArray<FBox2D>& Boxes, float MinX, float MaxX, float Pad, float& OutX);
	// The boxes of this tick (Slate units of the viewport), for the bark lane: the human's figure, every downed figure, both nets. False when the HUD has
	// not ticked this frame or the rule is off.
	static bool GetKeepClear(TArray<FBox2D>& OutFigures, TArray<FBox2D>& OutNets);
	// A figure's box on the screen (see above). Source: 2 bones, 1 mesh bounds, 0 none. PadCm is added around it at the figure's depth.
	static int32 FigureBoxOnScreen(APlayerController* PC, const class AJamSkater* Skater, float PadCm, FBox2D& OutBox);
	// Lane A158, item 4: the own-net marker (the diamond and "own NN m") is not drawn while that net is in the frame: it stood on the goalie and the net
	// (the critic's fault 16); off screen it is the edge chevron with its distance, as before. jam.hud.OwnMarkOnNet 1 = drawn on the net as before.
	static bool ShowOwnMarker(bool bOnScreen, bool bDrawOnNet) { return bDrawOnNet || !bOnScreen; }
	// ---- Lane A171 (track U1, evolution 6): the last overlaps. jam.hud.ClearEdges 1 (default; 0 = as before):
	//   - the caption's place under the scoreboard is the first row clear of the overlay's line, the MATCH card, the one-time prompt and the net and puck
	//     markers standing in that band (the far net's distance pill stood under "Flattened!"), and there is none when that row would reach into the
	//     middle third; the caption keeps the row it took while the row is clear;
	//   - the one-time prompt stands under the MATCH card while the card is up (the card covered it at a faceoff);
	//   - the own-net marker and its label stand above the radar and the ability plate, never under them;
	//   - the Director's Call widget's plates (the CALL READY pill, the line, the MATCH card) are not drawn on a frame they would lie on a downed figure.
	static bool ClearEdgesOn();
	// The first y at or under StartY at which a row [X0, X1] x [y, y + H] meets none of Taken (each kept Gap away); -1 when the row would end below MaxY. Pure.
	static float FirstClearY(float StartY, float H, float X0, float X1, const TArray<FBox2D>& Taken, float Gap, float MaxY)
	{
		float Y = StartY;
		for (int32 Pass = 0; Pass < 8; ++Pass)
		{
			bool bMoved = false;
			for (const FBox2D& B : Taken)
			{
				if (X0 < B.Max.X + Gap && X1 > B.Min.X - Gap && Y < B.Max.Y + Gap && Y + H > B.Min.Y - Gap) { Y = static_cast<float>(B.Max.Y) + Gap; bMoved = true; }
			}
			if (!bMoved) break;
		}
		return Y + H <= MaxY ? Y : -1.f;
	}
	// The prompt under a band (the MATCH card's): its own y when its row [Y, Y + H] is clear of [BandTop, BandBottom], else Gap under the band. Pure.
	static float UnderBand(float Y, float H, float BandTop, float BandBottom, float Gap = 8.f) { return (Y < BandBottom && Y + H > BandTop) ? BandBottom + Gap : Y; }
	// A marker at P (its label HalfW to either side and Below under it) is lifted above the bottom right corner's furniture: the radar's frame (264 x 140
	// from the edge) and the ability plate over it (404 wide, up to 240 over the bottom edge). Pure.
	static constexpr float CornerFurnitureW = 410.f;
	static constexpr float CornerFurnitureH = 246.f;
	static FVector2D KeepOffRadar(FVector2D P, float VW, float VH, float HalfW, float Below)
	{
		if (P.X + HalfW > VW - CornerFurnitureW && P.Y + Below > VH - CornerFurnitureH) P.Y = VH - CornerFurnitureH - Below;
		return P;
	}
	// (1) The marker follows the figure: the chevron's tip stands YouMarkerGapPx over the projection of the figure's head anchor (the visual's head_socket, else the
	// top of its bounds, else the point YouMarkerHeadM over the skater's point on the ice: the rule of before, jam.hud.MarkerAnchor 0). HeadLiftCm is added to a
	// mesh-socket anchor along the head's own axis (from the "head" bone to the socket; the world's up where the mesh has no such bone), so it holds for a
	// figure that leans or lies. Device B's meshes carry head_socket about 19 cm over the helmet's top (47 cm over the head bone: the place for a has-puck
	// marker), measured in this lane's first stills (the tip stood 30 px over the helmet at the faceoff): -18 puts the tip on the helmet. The toy's socket
	// (a scene component at 1.65 m) is the helmet's top and takes no lift.
	static constexpr float YouMarkerHeadLiftCm = -18.f;
	static FVector2D YouMarkerTip(const FVector2D& HeadPx) { return HeadPx + FVector2D(0.f, -YouMarkerGapPx); }
	// The chevron's box and the tag's box from the tip (the chevron is YouMarkerSizePx square, its bottom centre on the tip; the tag is about 76 x 34 over it).
	static void YouChevronBox(const FVector2D& Tip, FVector2D& OutMin, FVector2D& OutMax)
	{
		OutMin = Tip + FVector2D(-YouMarkerSizePx * 0.5f, -YouMarkerSizePx); OutMax = Tip + FVector2D(YouMarkerSizePx * 0.5f, 0.f);
	}
	static void YouTagBox(const FVector2D& Tip, FVector2D& OutMin, FVector2D& OutMax)
	{
		OutMin = Tip + FVector2D(-38.f, -(YouMarkerSizePx + 4.f + 34.f)); OutMax = Tip + FVector2D(38.f, -(YouMarkerSizePx + 4.f));
	}
	// The marker never sits on the puck (or the faceoff dot under it): while the puck's projection is inside the chevron's box plus MarkerYieldPadPx the chevron
	// is a ghost (MarkerGhostAlpha); inside the tag's box the tag is not drawn.
	static constexpr float MarkerYieldPadPx = 10.f;
	static constexpr float MarkerGhostAlpha = 0.3f;
	static bool MarkerBoxHasPuck(const FVector2D& Min, const FVector2D& Max, const FVector2D& PuckPx, float Pad = MarkerYieldPadPx)
	{
		return PuckPx.X >= Min.X - Pad && PuckPx.X <= Max.X + Pad && PuckPx.Y >= Min.Y - Pad && PuckPx.Y <= Max.Y + Pad;
	}
	static float MarkerYieldAlpha(bool bPuckInChevronBox) { return bPuckInChevronBox ? MarkerGhostAlpha : 1.f; }
	// (2) The YOU tag is a box over the human: with the middle kept clear it shows while a faceoff counts down (the moment the player looks for himself) and
	// not after the drop; bClearMiddle false is lane A42's window (the first 5 s, 2 s after every drop).
	static bool ShowYouTagLive(bool bClearMiddle, bool bCountingDown, int64 SimFrame, int64 LastDropFrame)
	{
		return bClearMiddle ? bCountingDown : ShowYouTag(SimFrame, LastDropFrame);
	}
	// A net's distance label is not drawn while it would stand in the middle third (the net itself is in front of the player then); the diamond stays.
	static bool ShowNetLabel(bool bClearMiddle, const FVector2D& LabelMin, const FVector2D& LabelMax, float VW, float VH)
	{
		return !(bClearMiddle && InMiddleThird(LabelMin, LabelMax, VW, VH));
	}
	// The scoreboard's centre plate with no timer: "PERIOD n" over "FIRST TO n", centred on the plate (jam.hud.CleanPlate 0: one line on the top border, as before).
	static FString FormatPeriod(int32 Take) { return FString::Printf(TEXT("PERIOD %d"), FMath::Max(1, Take)); }
	static FString FormatGoalRule(int32 ScoreToWin) { return ScoreToWin > 0 ? FString::Printf(TEXT("FIRST TO %d"), ScoreToWin) : FString(); }
	// (3) The two skills of the vision, one short prompt each, once in a session (process), at the first moment it applies, never two at once:
	// Kept: the human's first carry (only where the cartridge keeps the puck: stick.passive_take false). Brake: the first time an opponent closes inside
	// BrakePromptRangeM of the carrying human. A prompt holds PromptFrames sim frames, the last PromptFadeFrames fading. jam.hud.Prompts 0: none.
	enum class EPrompt : uint8 { None, Kept, Brake };
	static constexpr float BrakePromptRangeM = 10.f;
	static constexpr int32 PromptFrames = 300;
	static constexpr int32 PromptFadeFrames = 30;
	static constexpr float PromptSlotY = 112.f;       // under the scoreboard (its plates end near 104): over the stands and the far boards, not the play
	static constexpr float PromptSlotLowY = 206.f;    // under the overlay's line and the effects row while the line shows
	static constexpr float PromptSlotH = 46.f;
	// An edge marker (a net badge, the puck's arrow) at P holds the top slot when it stands in the prompt's box: the prompt then takes the lower slot.
	static bool InPromptSlot(const FVector2D& P, float VW, float HalfW) { return P.Y < PromptSlotY + PromptSlotH + 24.f && FMath::Abs(P.X - VW * 0.5f) < HalfW + 80.f; }
	static float PromptSlotFor(bool bTopSlotTaken) { return bTopSlotTaken ? PromptSlotLowY : PromptSlotY; }
	// Lane A135, evolution 4 (its own still, 90 m sim frame 1440: in its lower slot the prompt lay on the net the human attacks). The lower slot was taken
	// when the far-net badge stood in the top slot, which is when the net is straight ahead and just under the badge. The prompt now takes the first of six
	// places whose box is inside the frame and does not meet the net's box (the net's own mouth on the screen joined with its badge and label, plus Pad):
	// top centre, the lower row centre, the top row beside the net (the nearer side first, 120 units clear of it), the lower row beside it, and last the
	// same four right beside the net (16 units). bTopTaken (the overlay's line, the
	// own-net badge or the puck's arrow in the top slot, or a prompt asked into the lower row) rules the top row out. The place is (the box's centre from the
	// frame's centre, the box's top), Slate units. Nothing fits: the lower row centre, as before. jam.hud.PromptOffNet 0: evolution 3's two slots. Pure.
	static FVector2D PromptPlace(float VW, float HalfW, bool bTopTaken, bool bHasNet, const FVector2D& NetMin, const FVector2D& NetMax, float Pad = 12.f);
	static bool PromptBoxMeets(const FVector2D& Place, float VW, float HalfW, const FVector2D& NetMin, const FVector2D& NetMax, float Pad = 12.f);
	static FString PromptText(EPrompt P)
	{
		switch (P)
		{
		case EPrompt::Kept: return TEXT("THE PUCK IS YOURS UNTIL SOMEONE PRESSES FOR IT");
		case EPrompt::Brake: return TEXT("S AGAINST YOUR MOTION: STOP, THEN GO");
		default: return FString();
		}
	}
	// Lane A135 (track U1, evolution 3): the brake prompt in the player's own input. "S AGAINST YOUR MOTION" named the keyboard's back key to a player holding
	// a pad. The prompt now names the controls bound to the two ways of stopping, read from the input map (UJamInputMap's context) for the device the player
	// last touched: the control that pulls back on IA_Move and the one on IA_Brake.
	//   keys:  S AGAINST YOUR MOTION, OR HOLD CTRL: STOP, THEN GO
	//   pad:   LEFT STICK AGAINST YOUR MOTION, OR HOLD L3: STOP, THEN GO
	// A rebinding changes the words with it. jam.hud.PromptKeys 0: the words of before. jam.hud.PromptPad -1 = the device last touched, 0 keys, 1 pad (stills).
	static FString KeyLabel(const FKey& Key);
	// The two controls of a device from the input map: false when the map has none for it.
	static bool BrakeKeysFor(bool bPad, FKey& OutBack, FKey& OutBrake);
	static FString BrakePromptFor(const FKey& Back, const FKey& Brake);
	// The brake prompt's words: bKeysOn false, or a map without the controls, gives PromptText(EPrompt::Brake).
	static FString BrakePromptText(bool bKeysOn, bool bPad);
	// The device the player last touched (a pad's buttons or sticks against the keyboard and the mouse buttons); the keyboard until a pad is touched.
	static bool IsPadInUse() { return bPadInUse; }
	static void SetPadInUse(bool bPad) { bPadInUse = bPad; }
	// The words the HUD shows for a prompt now.
	static FString LivePromptText(EPrompt P);
	// What becomes due this frame (the caller shows nothing new while a prompt is up). bPlayLive: phase Play, no faceoff countdown.
	static EPrompt PromptDue(bool bOn, bool bPlayLive, bool bHumanCarries, float NearestOpponentM, bool bClosing, bool bKeptSpent, bool bBrakeSpent, bool bPuckKeptRule)
	{
		if (!bOn || !bPlayLive || !bHumanCarries) return EPrompt::None;
		if (!bKeptSpent && bPuckKeptRule) return EPrompt::Kept;
		if (!bBrakeSpent && bClosing && NearestOpponentM <= BrakePromptRangeM) return EPrompt::Brake;
		return EPrompt::None;
	}
	static float PromptOpacity(int64 ShownFrame, int64 Now)
	{
		const int64 Age = Now - ShownFrame;
		if (ShownFrame < 0 || Age < 0 || Age >= PromptFrames) return 0.f;
		return Age < PromptFrames - PromptFadeFrames ? 1.f : static_cast<float>(PromptFrames - Age) / static_cast<float>(PromptFadeFrames);
	}
	static bool IsPromptSpent(EPrompt P) { return P == EPrompt::Kept ? bKeptPromptSpent : P == EPrompt::Brake ? bBrakePromptSpent : true; }
	static void SetPromptsSpent(bool bKept, bool bBrake) { bKeptPromptSpent = bKept; bBrakePromptSpent = bBrake; }   // tests and jam.ui.promptreset
	// The band of Slate y a prompt holds while one is up (the bark bubbles keep out of it); false when none shows.
	static bool GetPromptBandPx(float& OutTop, float& OutBottom);
	// jam.ui.prompt kept|brake: shows a prompt now (the proof's stills); it counts as spent like a live one.
	void PreviewPrompt(EPrompt P, bool bLowSlot = false) { PromptPreview = P; bPreviewLow = bLowSlot; }
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetPromptText() const;
	// Lane A64: the controls strip stops this far from the right edge: the radar frame (200 + 2 x 8 px) at EdgePx and a 16 px gap.
	static constexpr float ControlsRightInsetPx = 24.f + 216.f + 16.f;
	// Lane A64 (eyes grid premise 2): a notice in the top-left corner for ToastFrames sim frames (4 s), e.g. a rejected -cut cartridge.
	static constexpr int64 ToastFrames = 240;
	// Lane A64: the faceoff countdown / WON ring's x offset from the screen centre (Slate units).
	static constexpr float RingOffsetXPx = 260.f;
	// Lane A123: at 260 the ring's 220 px box reached into the middle third (its DROP / WON / LOST stand half a second into the play); with the middle kept
	// clear (jam.hud.ClearMiddle 1) it stands RingOffsetClearXPx right of centre: its left edge 10 px outside the rectangle at 16:9, the hint line under it.
	static constexpr float RingOffsetClearXPx = 440.f;
	static constexpr float RingBoxPx = 220.f;
	void ShowToast(const FString& Text);
	// Lane A101 (track F1): the contact line, bottom centre above the stamina bar, ContactLineFrames sim frames (the last ContactFadeFrames fade): what just
	// happened to the human's press. The HUD reads the human's counters from the sim (ChecksRefused, ContactRecoils), so a frame the hit-stop held is not lost.
	static constexpr int64 ContactLineFrames = 60;
	static constexpr int64 ContactFadeFrames = 15;
	// Lane A113 (track F1): the soft hit explains itself. The human's pressed check that resolved as a bump (ChecksSoft): TOO SOFT; one that met a skater inside
	// its release grace (ChecksGraced): JUST GOT UP; a window that ran out on nobody (ChecksMissed): MISSED. One line a frame: the grace first (a charge into a
	// skater that just got up can bounce the human too, and the grace is the reason), then the bounce, the refused press, the soft hit, the miss.
	// Lane A171 (track U1, evolution 6): a press the check cooldown dropped (game.contact.check_cooldown_frames, off as shipped; the read-out
	// FJamSkaterState::ChecksCooling) says TOO SOON: the sim drops it with no window and no cue, so a mashed button felt dead. It stands before MISSED (the
	// window of the press before ran out on the same frames: the press he just made is the one to explain) and after everything that landed or was refused.
	static FString ContactLineFor(bool bRefused, bool bBounced, bool bSoft = false, bool bMissed = false, bool bGraced = false, bool bCooling = false)
	{
		return bGraced ? TEXT("JUST GOT UP") : bBounced ? TEXT("BOUNCED OFF") : bRefused ? TEXT("NOT WITH THE PUCK") : bSoft ? TEXT("TOO SOFT") : bCooling ? TEXT("TOO SOON") : bMissed ? TEXT("MISSED") : TEXT("");
	}
	void ShowContactLine(const FString& Text);
	FString GetContactText() const;
	FString GetToastText() const;
	// ---- Lane A163 (track F2, evolution 6): the carrier's cues, one function (UpdateCarrierCues, called last in RefreshFromFrame). Presentation only: it
	// reads the sim's skaters and writes nothing back; no widget of its own.
	// The take grace (game.stick.take_grace_frames: the half second after a skater wins the puck in which no stick takes it): the ring under that carrier
	// (YouRing for the human, CarrierRing for anybody else) is drawn in the HUD's yellow at TakeGraceRingScale times its size: 1.5 as the grace begins, its
	// own size as it ends, so the ring closes like a clock. A ring UpdateYours hid this frame stays hidden.
	static constexpr float TakeGraceRingGrow = 0.5f;
	static float TakeGraceRingScale(int32 GraceLeft, int32 GraceFrames)
	{
		return GraceLeft <= 0 || GraceFrames <= 0 ? 1.f : 1.f + TakeGraceRingGrow * FMath::Clamp(static_cast<float>(GraceLeft) / static_cast<float>(GraceFrames), 0.f, 1.f);
	}
	// The stick lock (game.stick.miss_recovery_frames: after a person's missed poke Space and RMB do nothing while it runs) and the press a carrier's grace
	// refuses: the contact line says which. The lock wins (it is the one that costs him).
	static FString CarrierCueLine(bool bStickLocked, bool bPressIntoGrace) { return bStickLocked ? TEXT("STICK LOCKED") : bPressIntoGrace ? TEXT("JUST WON IT") : TEXT(""); }
	// STICK LOCKED stands for the lock's length: when the lock ends, the line is put at the start of its fade (ContactFadeFrames).
	static int64 CarrierCueFadeStart(int64 SimFrame) { return SimFrame - (ContactLineFrames - ContactFadeFrames); }
	void UpdateCarrierCues(const FJamPresentationFrame& Frame);
	// ---- Lane A192 (track F1, evolution 10): the held cross-check says so. One function (UpdateBraceLine, HUDBrace.cpp; called after UpdateCarrierCues):
	// the human's braced hit that landed (FJamSkaterState::CrossBraceHits grew) shows BraceLineFor(true) on the contact line; one he took out of an
	// opponent's brace (CrossBraceTaken grew) shows BraceLineFor(false). Presentation only; a restart (the counters back to 0) shows nothing.
	static FString BraceLineFor(bool bGiven) { return bGiven ? TEXT("BRACED HIT") : TEXT("RUN THROUGH"); }
	void UpdateBraceLine(const FJamPresentationFrame& Frame);
	int32 BraceSeenGiven = 0, BraceSeenTaken = 0;
	bool bCueStickLocked = false, bCuePressIntoGrace = false;
	int32 CueGraceCarrier = -1;
	// ---- Lane A166 (track F2, evolution 7): the two warnings on the ice, one function (UpdateMarks, HUDMarks.cpp; called after UpdateCarrierCues).
	// Presentation only; the rules are JamCarrierMarks.h's. jam.hud.Marks 0 = neither. Their two rings are made on first use and drawn under every element.
	// The contested puck: a ring of ContestRingRadiusM round the loose puck in the opponent's colour, breathing by MarkPulse.
	// The stop's distance (lane A171, REVIEW-3 item 1: "orange rings keep appearing and I do not know which one wants what": two rings in one colour asked
	// for opposite acts): never a ring. A LINE on the ice across the human's path, 8 m from him towards the closing opponent: StopBarHalfWidthM to either
	// side of that point (two stick lengths in all), perpendicular to his travel; chalk at StopMarkArmedAlpha and StopBarArmedThickM while armed; the
	// opponent's colour, StopBarLitThickM thick and breathing while lit. A ring says "this puck"; a line across the way says "stop here".
	// Lane A178 (track F2, evolution 8; what a real window read on package H30, docs/experiments/MOVEMENT-9.md): the contested ring was on screen 5 frames
	// (median) and seen on no still; the lit line was 8 px high at 1920 x 1080, the thinnest orange thing on the ice, and its chalk state lasted a tenth
	// of a second. So: the contested mark is two rings round the puck (ContestRingRadiusM and ContestInnerRadiusM: a target, larger than any ring under a
	// skater) and a ring under the opponent who is coming (ContestOppRadiusM), all three in his colour, breathing together, on screen for
	// jam.hud.ContestHold frames at least (JamCarrierMarks::StepShow); the lit line is a metre deep (StopBarLitMinPx at least); the chalk line 0.45 m, 9 px at least, at 90 %, and the chalk
	// line is there from the moment he carries at an opponent ahead (FStopRule::ArmClosingMps).
	static constexpr float ContestRingRadiusM = 1.8f;
	static constexpr float ContestInnerRadiusM = 1.1f;
	static constexpr float ContestOppRadiusM = 1.0f;
	static constexpr float StopBarHalfWidthM = 1.6f;   // two stick lengths in all, as lane A171 set it (Jam.Hud.ClearEdges holds it): the window asked for depth, not width
	static constexpr float StopBarArmedThickM = 0.45f;
	static constexpr float StopBarLitThickM = 1.0f;
	static constexpr float StopBarArmedMinPx = 9.f;    // at 6 px and 60 % the chalk line was faint on pale ice at 1920 x 1080 (capture a178-drill-1080, frame 511)
	static constexpr float StopBarLitMinPx = 20.f;
	static constexpr float StopMarkArmedAlpha = 0.9f;
	// The line on the screen from its four projected points (its two ends A and B; T0 and T1 the near and far edge of its thickness at its middle):
	// centre, size (length x thickness, the thickness measured across the line and never under MinThickPx), the angle to turn it by, and the box it
	// covers once turned (what the down rule tests). False for a line with no length. Pure (tests call it).
	static bool StopBarLayout(const FVector2D& A, const FVector2D& B, const FVector2D& T0, const FVector2D& T1, float MinThickPx, FVector2D& OutCentre, FVector2D& OutSize, float& OutAngleDeg,
		FVector2D& OutBoxMin, FVector2D& OutBoxMax)
	{
		const FVector2D Along = B - A;
		const double Len = Along.Size();
		if (Len < 1.0) return false;
		const FVector2D Dir = Along / Len;
		const FVector2D Tv = T1 - T0;
		const double Thick = FMath::Max(FMath::Abs(Tv.X * Dir.Y - Tv.Y * Dir.X), static_cast<double>(MinThickPx));
		OutCentre = (A + B) * 0.5;
		OutSize = FVector2D(Len, Thick);
		OutAngleDeg = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X)));
		const FVector2D Half(FMath::Abs(Dir.X) * Len * 0.5 + FMath::Abs(Dir.Y) * Thick * 0.5, FMath::Abs(Dir.Y) * Len * 0.5 + FMath::Abs(Dir.X) * Thick * 0.5);
		OutBoxMin = OutCentre - Half;
		OutBoxMax = OutCentre + Half;
		return true;
	}
	// Lane A171: the marks yield to lane A158's boxes of the downed figures (DownBoxesNow: one source for every mark and marker), with the markers' pad.
	static constexpr float MarkDownPadPx = 16.f;
	static constexpr int32 MarkPulseFrames = 20;
	// 1 to 1.2 and back every MarkPulseFrames sim frames (three times a second): by the sim's frame, so a capture shows the same ring every run.
	static float MarkPulse(int64 SimFrame)
	{
		const int32 P = static_cast<int32>(((SimFrame % MarkPulseFrames) + MarkPulseFrames) % MarkPulseFrames);
		const float Half = MarkPulseFrames * 0.5f;
		return 1.f + 0.2f * (P < Half ? P / Half : (MarkPulseFrames - P) / Half);
	}
	// A mark is not drawn where its box (grown by PadPx) meets a downed figure's box: nothing lies on the fall, the lying or the get-up (lane A158's rule).
	static bool MarkMeetsDown(const FVector2D& Min, const FVector2D& Max, const TArray<FBox2D>& Downs, float PadPx = 6.f)
	{
		for (const FBox2D& B : Downs) if (Min.X - PadPx < B.Max.X && Max.X + PadPx > B.Min.X && Min.Y - PadPx < B.Max.Y && Max.Y + PadPx > B.Min.Y) return true;
		return false;
	}
	void UpdateMarks(const FJamPresentationFrame& Frame);
	UPROPERTY(Transient) TObjectPtr<UImage> ContestRing;
	UPROPERTY(Transient) TObjectPtr<UBorder> StopMarkBar;   // lane A171: a line, not a ring
	UPROPERTY(Transient) TObjectPtr<UImage> ContestRingInner;   // lane A178: the second ring round the contested puck
	UPROPERTY(Transient) TObjectPtr<UImage> ContestOppRing;     // lane A178: the ring under the opponent who is coming
	int32 MarkContestOpp = -1;
	int32 MarkStopState = 0;
	static bool ShowControlsHint(const FJamPresentationFrame& F) { return F.SimFrame < ControlsHintFrames && F.Phase == EJamPhase::Play; }
	// The Directors Call prompt shows only after the first goal.
	static bool ShowDirectorsCall(const FJamPresentationFrame& F) { return F.bDirectorsCallReady && (F.ScoreHome + F.ScoreAway) > 0 && F.Phase != EJamPhase::Win && F.Phase != EJamPhase::Lose; }
	static FString CardFor(EJamPhase Phase);
	// Lane A144 (evolution 2, by the Director's order for this card): the end card says what happened. The word: YOU WIN, BEATEN, or NO WINNER
	// when the match stopped level at the horn (the sim names no winner; it used to read as a loss with no word of why). The score line is the
	// player's side first: "YOU  3 : 1  THEM". The key line outside a season match: EndCardKeys().
	static FString EndCardWord(EJamPhase Phase, int32 Mine, int32 Theirs);
	static FString EndCardScore(int32 Mine, int32 Theirs);
	// Lane A195: the restart key from the live bindings ("R  REMATCH      ESC  MENU" with the shipped keys).
	static FString EndCardKeys();
	// Lane A195: a key line ("R  REMATCH      ESC  MENU", the season's "SEASON COMPLETE      R  MENU") as { key, action } caps for the card, the restart
	// key as the bindings have it ("R / VIEW"); a part with no key ("SEASON COMPLETE") comes back in OutNote. Pure (tests call it).
	static TArray<TPair<FString, FString>> EndCardHints(const FString& KeyLine, FString& OutNote);
	// Lane A195: the end card's width and the word's size (Jam.Menu.FitsTheScreen measures the card).
	static constexpr float EndCardW = 704.f;   // 88 units: the between-takes card's keys with the longest next take ("THE FINAL CUT", 584 units) inside its 640
	static constexpr int32 EndCardWordPt = 72;
	UWidget* GetEndCardForTest() const;
	// Tests: the card filled as a match end fills it (the word, the score, the key line), then GetEndCardForTest() is measured.
	void ShowEndCardForTest(const FString& Word, const FString& Score, const FString& KeyLine);
	void PaintEndCardKeys(const FString& KeyLine, bool bWin);
	float GetEndCardHintRowWidth() const;   // lane A195: the keys' row as laid out (after a prepass)
	FString GetCardScoreText() const;
	FString GetCardKeysText() const;
	// Lane A14 (C-0005): the active-effects strip under the score, "BOLT speed 4.2s    WEIGHT heavy_puck 1.0s" (icon name, effect id, seconds left); empty = hidden.
	static FString FormatEffects(const TArray<FJamEffectHudEntry>& Rows);

	// Applies one frame to every element; called from NativeTick with the latest frame.
	void RefreshFromFrame(const FJamPresentationFrame& Frame);

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	const FJamNetMarker& GetNetMarker() const { return Marker; }

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	const FJamNetMarker& GetOwnNetMarker() const { return OwnMarker; }

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	UJamRadarWidget* GetRadar() const { return Radar; }

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetCardText() const;

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetScoreText() const;

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetTimerText() const;

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetTakeText() const;

	// Lane A18 (visual layer). The faceoff countdown reads the feed's FaceoffCountdownFrames when lane A16's field is there; "DROP" shows
	// for DropFrames once it reaches 0. Frames > 0 here previews it (the jam.ui.faceoff console command and the proof shots).
	static constexpr int32 DropFrames = 30;
	void PreviewFaceoff(int32 Frames) { FaceoffPreview = Frames; }
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetCountdownText() const;
	// The active-effect slot row: one icon (FJamUiStyle icon name) and a short label per slot, in order; empty hides the row.
	// Lane A14's effect feed calls it (or jam.ui.effects previews it).
	void SetEffectSlots(const TArray<TPair<FString, FString>>& Slots);

	// ---- Lane A42 (HUD pass 2, from the eyes pass of A27): which skater is yours, how a faceoff is won, the markers inside the edge.
	// The "YOU" tag over the human's head: the first 5 s of a match and 2 s after every drop (LastDropFrame -1 = no drop yet).
	static constexpr int64 YouTagStartFrames = 300;
	static constexpr int64 YouTagAfterDropFrames = 120;
	static bool ShowYouTag(int64 SimFrame, int64 LastDropFrame)
	{
		return SimFrame < YouTagStartFrames || (LastDropFrame >= 0 && SimFrame >= LastDropFrame && SimFrame - LastDropFrame < YouTagAfterDropFrames);
	}
	// The ring under the human: 1.1 m in team colour; from RingFadeStartM it fades out and the chevron over the head fades in, gone at RingFadeEndM.
	static constexpr float YouRingRadiusM = 1.1f;
	static constexpr float CarrierRingRadiusM = 0.8f;
	static constexpr float RingFadeStartM = 20.f;
	static constexpr float RingFadeEndM = 24.f;
	static float RingAlphaAt(float CameraDistanceM)
	{
		return 1.f - FMath::Clamp((CameraDistanceM - RingFadeStartM) / (RingFadeEndM - RingFadeStartM), 0.f, 1.f);
	}
	// The word in the ring during a faceoff (R-0013 point 2: the early mover loses, the first clean press wins): HOLD while it counts, DROP at zero.
	static FString FaceoffCue(int32 CountdownFrames, bool bDropped) { return CountdownFrames > 0 ? TEXT("HOLD") : (bDropped ? TEXT("DROP") : TEXT("")); }
	// The one-line hint under the ring for the first FaceoffHintCount faceoffs of a session.
	static constexpr int32 FaceoffHintCount = 3;
	static FString FaceoffHintText() { return TEXT("press shoot at the drop, not before"); }
	// After the draw: WON or LOST in the human's team colour for DrawResultFrames.
	static constexpr int32 DrawResultFrames = 40;
	static FString DrawResultText(bool bWon) { return bWon ? TEXT("WON") : TEXT("LOST"); }
	// Preview of the draw result for the proof shots (jam.ui.draw won|lost [frames]); counts sim frames.
	void PreviewDraw(bool bWon, int32 Frames) { bLastDrawWon = bWon; DrawPreviewFrames = FMath::Max(1, Frames); }
	// Markers stay MarkerEdgePx inside the screen edge (HalfW: half the marker's drawn width, Top: under the scoreboard, Bottom: above the strips).
	static constexpr float MarkerEdgePx = 48.f;
	static FVector2D ClampMarker(FVector2D Sp, float VW, float VH, float HalfW, float Top, float BottomPx)
	{
		return FVector2D(FMath::Clamp(Sp.X, MarkerEdgePx + HalfW, VW - MarkerEdgePx - HalfW), FMath::Clamp(Sp.Y, Top, VH - MarkerEdgePx - BottomPx));
	}
	// The commentator's line (lane A32's JAMVO text) on the bottom-centre strip: two lines at most, held for the line's duration plus
	// CommentaryHoldFrames, then faded over CommentaryFadeFrames. The barks keep their bubbles (one copy of each line on screen).
	static constexpr int32 CommentaryHoldFrames = 90;
	static constexpr int32 CommentaryFadeFrames = 30;
	// Lane A47 (B-to-A-0030 fault 7): the live strip passes HoldFrames 0 and the line's real end (a cut shortens DurationFrames), so the caption
	// fades the moment the voice stops; the default keeps the earlier reading for the tests.
	static float CommentaryOpacityFor(int64 ShownFrame, int32 DurationFrames, int64 Now, int32 HoldFrames = CommentaryHoldFrames)
	{
		const int64 Age = Now - ShownFrame;
		if (Age < 0 || ShownFrame <= -1000000) return 0.f;
		const int64 Hold = static_cast<int64>(FMath::Max(0, DurationFrames)) + FMath::Max(0, HoldFrames);
		if (Age <= Hold) return 1.f;
		const int64 Fade = Age - Hold;
		return Fade >= CommentaryFadeFrames ? 0.f : 1.f - static_cast<float>(Fade) / static_cast<float>(CommentaryFadeFrames);
	}
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetCommentaryText() const;
	// What the ring says this frame (HOLD / DROP / WON / LOST / empty); tests and the proof read it.
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetFaceoffCue() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	bool IsYouTagVisible() const;
	// ---- Lane A74 (R-0038): the off-screen puck marker. The camera stays bound to the human; when the puck is outside the inner 85 % of the frame an
	// arrow stands on the frame's edge at the puck's direction with the distance beside it, in the carrier's team colour (grey when loose). It fades in
	// and out over PuckMarkerFadeFrames sim frames; hidden for the faceoff card / countdown, a goal pause, the pause menu, the end cards.
	// ---- Lane A76: the far-net badge stands NetBadgeLiftCm over the crossbar (NetCrossbarCm: the sim default 0.61 m; a taller bar only adds gap) and at least
	// NetBadgeGapPx over the crossbar's projection. EdgeSlotPx is the least distance between two edge elements (the puck keeps its place, the nets slide).
	static constexpr float NetCrossbarCm = 61.f;
	static constexpr float NetBadgeLiftCm = 120.f;
	static constexpr float NetBadgeGapPx = 40.f;
	static constexpr float EdgeSlotPx = 48.f;
	static constexpr float PuckMarkerInnerFrac = 0.85f;
	static constexpr int32 PuckMarkerFadeFrames = 6;
	static constexpr int64 PuckMarkerCardFrames = 150;   // the MATCH card of a fresh match
	// True when Sp (Slate units) is outside the centred inner fraction of the viewport.
	static bool OutsideInnerFrame(const FVector2D& Sp, float VW, float VH, float Frac = PuckMarkerInnerFrac)
	{
		return Sp.X < VW * (1.f - Frac) * 0.5f || Sp.X > VW * (1.f + Frac) * 0.5f || Sp.Y < VH * (1.f - Frac) * 0.5f || Sp.Y > VH * (1.f + Frac) * 0.5f;
	}
	// Where the marker stands: the ray from the screen centre along Dir meets the rectangle given by the four edges.
	static FVector2D PuckMarkerEdgePoint(FVector2D Dir, float VW, float VH, float Left, float Right, float Top, float Bottom)
	{
		if (Dir.IsNearlyZero()) Dir = FVector2D(0.f, 1.f);
		const FVector2D C(VW * 0.5f, VH * 0.5f);
		const float TX = FMath::Abs(Dir.X) > KINDA_SMALL_NUMBER ? (Dir.X > 0 ? Right - C.X : C.X - Left) / FMath::Abs(Dir.X) : TNumericLimits<float>::Max();
		const float TY = FMath::Abs(Dir.Y) > KINDA_SMALL_NUMBER ? (Dir.Y > 0 ? Bottom - C.Y : C.Y - Top) / FMath::Abs(Dir.Y) : TNumericLimits<float>::Max();
		return C + Dir * FMath::Min(TX, TY);
	}
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	float GetPuckMarkerAlpha() const { return PuckMarkerAlpha; }
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetPuckMarkerText() const { return PuckMarkerText; }
	// Faceoffs seen this session (process), for the hint.
	static int32 GetFaceoffsThisSession() { return FaceoffsThisSession; }
	// The bus event handler: faceoff drop / won / lost (presentation only: nothing is written back).
	UFUNCTION() void HandleJamEvent(FName Event, const FJamEventPayload& Payload);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// Lane A191: the buttons' badge (built and updated in InputMapping.cpp: one call each from BuildTree and RefreshFromFrame).
	void BuildButtonsBadge();
	void UpdateButtonsBadge(const FJamPresentationFrame& Frame);
	UPROPERTY(Transient) TObjectPtr<UBorder> ButtonsBadge;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ButtonsBadgeText;
	// Lane A18 preview counter (declared by lane A16 to keep the tree compiling, R-0003 point 5).
	int32 FaceoffPreview = 0;
	void BuildTree();
	void UpdateMarker(const FJamPresentationFrame& Frame);
	void UpdateOwnMarker(const FJamPresentationFrame& Frame);

	UPROPERTY(Transient) TObjectPtr<UCanvasPanel> Root;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ScoreHome;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ScoreAway;
	UPROPERTY(Transient) TObjectPtr<UBorder> HomeBox;
	UPROPERTY(Transient) TObjectPtr<UBorder> AwayBox;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TakeText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TimerText;
	UPROPERTY(Transient) TObjectPtr<UBorder> NetDiamond;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> NetText;
	UPROPERTY(Transient) TObjectPtr<UBorder> OwnDiamond;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> OwnText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ControlsText;
	UPROPERTY(Transient) TObjectPtr<UBorder> ToastPlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ToastText;
	int64 ToastStart = -1;     // sim frame the toast first showed; -1 = waiting for the next frame
	bool bToastPending = false;
	UPROPERTY(Transient) TObjectPtr<UBorder> ContactPlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ContactText;
	int64 ContactStart = -1;   // sim frame the contact line first showed; -1 = none or waiting for the next frame
	bool bContactPending = false;
	int32 ContactSeenRefused = 0, ContactSeenRecoils = 0;
	int32 ContactSeenSoft = 0, ContactSeenMissed = 0, ContactSeenGraced = 0;   // lane A113
	int32 ContactSeenCooling = 0;                                              // lane A171
	void UpdateContactLine(const FJamPresentationFrame& Frame);
	UPROPERTY(Transient) TObjectPtr<UJamRadarWidget> Radar;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CallText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CardTextBlock;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> HintText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> EffectsText;
	// Lane A18: the plates and strips of the slate kit around the elements above.
	UPROPERTY(Transient) TObjectPtr<UBorder> ControlsStrip;
	UPROPERTY(Transient) TObjectPtr<UBorder> CardPlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CardScoreText;   // lane A144: the score on the end card
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CardNoteText;   // lane A195: SEASON COMPLETE
	UPROPERTY(Transient) TObjectPtr<class UHorizontalBox> CardHints;   // lane A195: the keys as caps
	FString CardHintKey;
	// Lane A49: the sprint stamina bar (bottom centre, above the strips): a dark back plate and a fill that shrinks with the stamina; hidden while full and idle.
	UPROPERTY(Transient) TObjectPtr<UBorder> StaminaBack;
	UPROPERTY(Transient) TObjectPtr<UBorder> StaminaFill;
	// Lane A84 (R-0041 point 3): the ability slot right of the stamina bar: a plate with "F  SHOTGUN  READY" and a cooldown fill under it; hidden with no ability.
	UPROPERTY(Transient) TObjectPtr<UBorder> AbilityPlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> AbilityText;
	UPROPERTY(Transient) TObjectPtr<UBorder> AbilityFill;
	UPROPERTY(Transient) TObjectPtr<UWidget> CountdownBox;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CountdownText;
	UPROPERTY(Transient) TObjectPtr<class UHorizontalBox> EffectsRow;
	UPROPERTY(Transient) TObjectPtr<UBorder> EffectsPlate;

	FJamNetMarker Marker;
	FJamNetMarker OwnMarker;
	int32 LastFaceoffFrames = 0;
	int64 DropUntil = -1;
	bool bEffectsFromFeed = false;

	// Lane A42: the yours / carrier rings, the YOU tag, the faceoff words, the marker arrows.
	void UpdateYours(const FJamPresentationFrame& Frame);
	// Projects a rink point (metres) with a radius around it to a screen ellipse; false when behind the camera.
	bool ProjectGroundRing(const FVector2D& PosM, float RadiusM, FVector2D& OutCentre, FVector2D& OutSize, float& OutCameraDistM) const;
	void PlaceMarkerArrow(UWidget* Arrow, const FVector2D& Raw, const FVector2D& Clamped, bool bBehind, float VW);
	static int32 FaceoffsThisSession;
	UPROPERTY(Transient) TObjectPtr<UImage> YouRing;
	UPROPERTY(Transient) TObjectPtr<UWidget> YouChevron;
	UPROPERTY(Transient) TObjectPtr<UBorder> YouTag;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> YouTagText;
	UPROPERTY(Transient) TObjectPtr<UImage> CarrierRing;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> FaceoffWord;
	UPROPERTY(Transient) TObjectPtr<UBorder> FaceoffHintPlate;
	UPROPERTY(Transient) TObjectPtr<UWidget> NetArrow;
	UPROPERTY(Transient) TObjectPtr<UWidget> OwnArrow;
	UPROPERTY(Transient) TObjectPtr<UBorder> CommentaryStrip;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CommentaryText;
	UPROPERTY(Transient) TObjectPtr<class UJamEventBus> Bus;
	void UpdateCommentary(const FJamPresentationFrame& Frame);
	// ---- Lane A158 (evolution 5): the boxes kept clear this tick, the yield pass, the probe.
	void UpdateKeepClear(const FJamPresentationFrame& Frame);
	void YieldToDowned();
	void PlaceContactLine();
	void ProbeDown(const FJamPresentationFrame& Frame);
	bool WidgetBoxOnScreen(const UWidget* W, FBox2D& OutBox) const;     // from the canvas slot, this tick's place
	bool PaintedBoxOnScreen(const UWidget* W, FBox2D& OutBox) const;    // from the last paint (any widget of any tree)
	FString ProbeNameOf(const UWidget* W) const;
	TArray<TWeakObjectPtr<class AJamSkater>> SkaterActors;
	int32 SkaterSearchTick = 0;
	bool bHumanDownNow = false;
	int64 HumanUpSince = -1;
	bool bHasHumanBox = false;
	FBox2D HumanBoxNow = FBox2D(ForceInit);
	TArray<FBox2D> DownBoxesNow;        // every downed figure's box (the human's too)
	TArray<FBox2D> KeepBoxesNow;        // the human's box (up or down) and every downed figure's
	TArray<int32> DownWhoNow;           // the skater of each of DownBoxesNow, and its box's source (2 bones, 1 bounds)
	TArray<int32> DownSourceNow;
	float RingYield = 1.f;              // the ring's share beside a downed figure (RingYieldStep)
	float CaptionTopYNow = -1.f;        // lane A171: the row under the scoreboard this caption line took (-1: none yet)
	int32 OverlayYieldTick = 0;
	void TopBandObstacles(TArray<FBox2D>& Out, float VW, float VH) const;
	void YieldOverlayToDowned();
	float StepRingYield(bool bOn, const FVector2D& Centre, const FVector2D& Size);
	int32 CaptionPlaceNow = 0;
	int64 CaptionPlaceLine = -1000000;
	float ContactStepX = 0.f;
	bool bContactStepped = false;
	static uint64 KeepClearTick;
	static TArray<FBox2D> KeepClearFigures;
	static TArray<FBox2D> KeepClearNets;
	// The probe (jam.hud.DownProbe N): what the last tick drew against the boxes of the last tick, per knockdown.
	struct FDownRun { int64 Start = -1; int32 Frames = 0, HitFrames = 0, ChevronFrames = 0, Source = 0, Stills = 0; TMap<FString, int32> By; };
	struct FDownProbe
	{
		TMap<int32, FDownRun> Open;      // by skater index
		struct FPrev { int32 Who = -1; bool bHuman = false; FBox2D Box = FBox2D(ForceInit); int32 Source = 0; };
		TArray<FPrev> Prev;              // the downed figures' boxes of the last tick
		bool bPrevHumanUp = false; FBox2D PrevHumanBox = FBox2D(ForceInit);
		int64 PrevFrame = -1, LastLogged = -1;
		int32 HumanDowns = 0, HumanClean = 0, OtherDowns = 0, OtherClean = 0;
		int64 HumanFrames = 0, HumanHit = 0, OtherFrames = 0, OtherHit = 0, UpFrames = 0, UpCaptionHit = 0, Elements = 0;
		TMap<FString, int32> HumanBy, OtherBy;
	};
	FDownProbe DownProbe;
	void UpdatePuckMarker(const FJamPresentationFrame& Frame, bool bCovered);
	void ResolveEdgeSlots();   // lane A76: one slot rule for the puck chevron, the net chevrons and their labels
	// Lane A111: the dark plates behind the three distance labels follow their labels after the slot rule has run.
	void UpdateLabelPlates();
	UPROPERTY(Transient) TObjectPtr<UBorder> NetPlate;
	UPROPERTY(Transient) TObjectPtr<UBorder> OwnPlate;
	UPROPERTY(Transient) TObjectPtr<UBorder> PuckPlate;
	static bool bControlsHintSpent;
	// ---- Lane A123: the head anchor (found once per visual), the yield, the prompts, the clean centre plate, the marker probe.
	// 2 = the visual's head socket, 1 = the top of its bounds, 0 = none (the caller falls back to the point over the feet).
	int32 HumanHeadWorld(FVector& OutWorld);
	void UpdatePrompts(const FJamPresentationFrame& Frame, bool bCountingDown, bool bSimAdvanced);
	void ProbeMarker(const FJamPresentationFrame& Frame, int32 Source, const FVector2D& Tip, const FVector2D& OldTip, const FVector& HeadWorld);
	TWeakObjectPtr<UObject> HeadVisual;
	TWeakObjectPtr<USceneComponent> HeadComp;
	int32 HeadCompKind = 0;   // 2 a mesh socket, 3 a scene component named head_socket, 1 the bounds of the largest primitive
	float MarkerYield = 1.f;
	float RingXApplied = -1.f;
	bool bCountingThisFrame = false;
	bool bSimAdvancedThisFrame = false;
	static bool bKeptPromptSpent;
	static bool bBrakePromptSpent;
	static uint64 PromptBandTick;           // GFrameCounter of the last tick a prompt was drawn (the band's getter reads it)
	static float PromptBandTop;
	EPrompt PromptNow = EPrompt::None;
	EPrompt PromptPreview = EPrompt::None;
	int64 PromptShown = -1;
	bool bPromptLow = false;   // this prompt took the lower slot (it does not jump back up)
	bool bPreviewLow = false;  // lane A135: a previewed prompt asked for the lower slot (the proof's still)
	// Lane A135, evolution 4: where this prompt stands (it keeps its place while the place is clear of the net), and the net's box on the screen.
	FVector2D PromptPlaceNow = FVector2D(0, PromptSlotY);
	bool bPromptPlaced = false;
	// A net sweeping across the picture as he turns does not push the prompt about (build 12 moved it after 12 frames: five moves in 5 s, and its end
	// stood on the net's corner in between). While the net is under the prompt the prompt is a ghost (a quarter of its ink, eased over 6 ticks: the net
	// shows through); it moves only off a net that has stood under it for PromptOnNetFrames (three quarters of a second: he is looking at it).
	static constexpr int32 PromptOnNetFrames = 45;
	static constexpr float PromptGhostAlpha = 0.25f;
	int64 PromptOnNetSince = -1;
	float PromptGhost = 1.f;
	bool NetBoxOnScreen(const FJamPresentationFrame& F, FVector2D& OutMin, FVector2D& OutMax) const;
	static bool bPadInUse;     // lane A135: the device the player last touched
	void WatchInputDevice();
	float LastNearestOpponentM = -1.f;
	TWeakObjectPtr<class UJamDirectorsCallHudWidget> OverlayForPrompt;
	int32 OverlaySearchTick = 0;
	UPROPERTY(Transient) TObjectPtr<UBorder> PromptPlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PromptTextBlock;
	UPROPERTY(Transient) TObjectPtr<UBorder> CentrePlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> GoalRuleText;
	int32 CentrePlateMode = -1;
	// The probe's running numbers (jam.hud.MarkerProbe N: a JAMSCREEN marker line every N sim frames).
	struct FMarkerProbe
	{
		int64 Frames = 0, Socket = 0, Bounds = 0, Fallback = 0, Down = 0, LastLogged = -1;
		float TipMaxPx = 0.f, OldMaxPx = 0.f, OldMaxDownPx = 0.f, BoneMaxPx = 0.f, HeadMinCm = 1e9f, HeadMaxCm = 0.f;
	};
	FMarkerProbe Probe;
	bool bHintThisFrame = false;                                  // the controls strip shows this frame (the commentary strip steps over it)
	FLinearColor YouChevronColour = FLinearColor::Transparent;   // the colour the chevron's bars were last built in (re-built on change only)
	UPROPERTY(Transient) TObjectPtr<UWidget> PuckArrow;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PuckText;
	float PuckMarkerAlpha = 0.f;
	int64 PuckMarkerLastFrame = -1;
	FString PuckMarkerText;
	int64 CommentaryFrame = -1000000;
	int32 CommentaryDuration = 0;
	int64 LastSimFrame = -1;
	int64 LastDropFrame = -1;
	int64 DrawResultUntil = -1;
	int32 DrawPreviewFrames = 0;
	bool bLastDrawWon = false;
	bool bCountdownSeen = false;

	// ---- Lane A188 (track F3, evolution 8, R-0064 points 6 and 9; HUDGoalBanner.cpp): the GOAL banner and the ability plate's words.
public:
	// The banner's opacity Since frames into a goal pause of PauseFrames: in over GoalBannerInFrames, held, out over GoalBannerOutFrames ending
	// GoalBannerLeadFrames before the pause ends (before the faceoff's countdown); 0 outside. A short pause shares its frames in thirds.
	static constexpr int32 GoalBannerInFrames = 12;
	static constexpr int32 GoalBannerOutFrames = 12;
	static constexpr int32 GoalBannerLeadFrames = 6;
	static float GoalBannerAlpha(int32 Since, int32 PauseFrames);
	// The banner's band: its centre's height as a share of the screen (0.30, the middle band above the play), or 0.205 when the scorer's figure
	// (ScorerTopY..ScorerFootY, shares of the screen) would be under the band at 0.30. ScorerTopY < 0 = no scorer on screen.
	static float GoalBannerCentreY(float ScorerTopY, float ScorerFootY);
	// "2 : 1", the scoring side's number first as the scoreboard reads (home : away).
	static FString GoalBannerScore(int32 Home, int32 Away);
	// The ability plate (lane A84's): "<key>: <KIND>  <state>", the key from the live bindings (UJamInputMap::BindingTableText's ABILITY row).
	static FString AbilityPlateText(const FString& Kind, const FString& State);
	static FString AbilityKeyName();
	// What fires an armed ability: NEXT SHOT (shotgun), NEXT HIT (hip_check), NEXT PASS (slow_puck), else ARMED.
	static FString AbilityArmedWord(FName Kind);
	UFUNCTION(BlueprintPure, Category = "Jam|Hud") FString GetGoalBannerText() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Hud") float GetGoalBannerOpacity() const { return GoalBannerOpacity; }
private:
	void UpdateGoalBanner(const FJamPresentationFrame& Frame);
	UPROPERTY(Transient) TObjectPtr<UBorder> GoalBannerPlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> GoalBannerWord;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> GoalBannerScoreText;
	int64 GoalBannerStart = -1;      // the sim frame the goal pause began (-1: no banner)
	int32 GoalBannerLastHome = -1;
	int32 GoalBannerLastAway = -1;
	int32 GoalBannerTeam = -1;       // 0 the human's side scored, 1 the other side
	float GoalBannerY = 0.30f;
	float GoalBannerOpacity = 0.f;
};
