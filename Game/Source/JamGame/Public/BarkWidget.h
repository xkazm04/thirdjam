// Lane A12: the bark presentation. A C++-built UMG widget (no assets) with one speech bubble per speaking skater, positioned by world-to-screen
// above the figure's head each render frame, and a one-line HUD strip at the bottom. Timing counts sim frames (UJamBarkSystem calls TickFrames
// once per simulation step): a bubble holds for BubbleHoldFrames, then fades over BubbleFadeFrames.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BarkSystem.h"
#include "BarkWidget.generated.h"

class UCanvasPanel;
class UBorder;
class UTextBlock;
class AActor;

USTRUCT()
struct FJamBarkBubble
{
	GENERATED_BODY()
	UPROPERTY(Transient) TObjectPtr<UBorder> Box;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Text;
	// Lane A18: the bubble body (the brush carries the team edge) and the tail under it, pointing at the figure.
	UPROPERTY(Transient) TObjectPtr<UBorder> Body;
	UPROPERTY(Transient) TObjectPtr<UBorder> Tail;
	UPROPERTY(Transient) TWeakObjectPtr<AActor> Anchor;
	int32 Speaker = -1;
	int32 Team = 0;   // lane A135: the lane's side
	int64 ShownFrame = -1000000;
};

UCLASS()
class JAMGAME_API UJamBarkWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Shows P over Anchor (the speaking skater's actor; null parks the bubble at the strip) in the team's colour tint.
	void ShowBark(const FJamBarkPick& P, AActor* Anchor, int32 Team);
	// Called once per simulation frame; drives hold and fade.
	void TickFrames(int64 SimFrame);

	// Opacity 1 while held, then linear to 0 over the fade; 0 when never shown. Pure (tests call it).
	static float OpacityFor(int64 ShownFrame, int64 Now);

	// Lane A42 (y-slots): a bubble whose box [Pos.Y - Height, Pos.Y] crosses the band [Top, Bottom] moves above it (when MinY allows) or
	// below it; Pos is the bubble's bottom-centre. Pure (tests call it). The bark's second copy on the strip is gone: the strip at the
	// bottom centre is the commentator's line, drawn by UJamHudWidget.
	static FVector2D PushOutOfBand(FVector2D Pos, float Height, float Top, float Bottom, float MinY);
	// Lane A123 (track U1, evolution 2): no bubble over the middle third of the frame (the centre rectangle [W/3, 2W/3] x [H/3, 2H/3], where the human and
	// the play are). A bubble whose box [Pos.X - HalfW, Pos.X + HalfW] x [Pos.Y - Height, Pos.Y] meets that rectangle (plus Pad) moves by the shortest of four
	// ways out: up, down, or sideways while the whole bubble stays in the frame. Pos is the bubble's bottom-centre. Pure (tests call it).
	// jam.hud.ClearMiddle 0 leaves the bubble over its speaker (235 cm over the feet), as before.
	static FVector2D PushOutOfMiddle(FVector2D Pos, float HalfW, float Height, float VW, float VH, float Pad = 8.f);
	// Lane A135 (track U1, evolution 3): bark text on a fixed lane. A bubble pushed out of the middle third landed by the human's skates or on a net label on
	// the frame's edge (evolution 2's stills), and over its speaker it lies on whatever figure stands behind him. The text now stands in the scoreboard's row,
	// beside its team's score plate: the human's team (team 0, the left plate) to the left of the board, the other to the right, its near end LaneGapX from the
	// frame's centre, its bottom at LaneBottomY (Slate units; the plates span 14 to 104). That row holds no net label and no puck arrow (their clamps keep them
	// at 105 and lower), no prompt (112 and lower), no ring, and it is a third of the frame above the human's skates. One bubble a side: the newest of a team
	// whose speaker is in the frame; the bubble has no tail (the plate's colour and the bubble's edge say whose line it is). jam.hud.BarkLane 0: the bubble
	// over its speaker, pushed out of the middle third, as before. Pos is the bubble's bottom-centre. Pure (tests call it).
	static constexpr float LaneBottomY = 84.f;
	static constexpr float LaneGapX = 262.f;
	static constexpr float LaneEdgePx = 8.f;
	static FVector2D LanePos(int32 Team, float HalfW, float VW);
	static bool LaneOn();
	// The lane yields: when the camera looks down from over the boards (lane A135's crane) the scoreboard's row is ice a few metres ahead, and a figure or the
	// puck's ring can stand there. A lane bubble whose box [Min, Max] (plus Pad) meets a figure's box is not drawn that frame (the voice plays on). The
	// figure's box: from its feet to its head on the screen, half that length to either side (a figure lying on the ice is as wide as it was tall). Pure.
	static bool LaneMeetsFigure(const FVector2D& Min, const FVector2D& Max, const FVector2D& FeetPx, const FVector2D& HeadPx, float Pad = 6.f);
	// Lane A42 (A48's storyboard finding 5): a speaker farther than this from the camera, or outside the frame, gets no bubble.
	static constexpr float MaxBubbleDistanceM = 24.f;

	int32 GetVisibleBubbleCount() const;
	FString GetStripText() const;

	// Lane A229 (HUDOneLook.cpp): a bubble in the HUD's one look (the menu's dark panel and type, the speaker's team colour on its edge), not the white
	// paper of before; jam.hud.OneLook 0 = before. Called from ShowBark after the bubble's own colours are set.
	static void OneLookBubble(FJamBarkBubble& B, const FLinearColor& Edge);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void BuildTree();
	void Layout();

	UPROPERTY(Transient) TObjectPtr<UCanvasPanel> Root;
	UPROPERTY(Transient) TObjectPtr<UBorder> StripBox;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> StripText;
	UPROPERTY(Transient) TArray<FJamBarkBubble> Bubbles;
	int64 Now = 0;
	int64 StripFrame = -1000000;
	// Lane A42: the overlay whose MATCH banner the bubbles keep clear of.
	TWeakObjectPtr<class UJamDirectorsCallHudWidget> Overlay;
	int32 OverlaySearchTick = 0;
};
