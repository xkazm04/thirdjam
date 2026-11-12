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
	// Lane A42 (A48's storyboard finding 5): a speaker farther than this from the camera, or outside the frame, gets no bubble.
	static constexpr float MaxBubbleDistanceM = 24.f;

	int32 GetVisibleBubbleCount() const;
	FString GetStripText() const;

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
