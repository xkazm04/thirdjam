// The match HUD: score as a film counter, "TAKE n", timer, far-net screen marker with distance, radar, Director's Call prompt,
// win/lose cards and the restart hint. A C++ UMG widget whose tree is built in code (no widget blueprint needed). Lane A5.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDFeed.h"
#include "RadarWidget.h"
#include "HUDWidget.generated.h"

class UCanvasPanel;
class UTextBlock;
class UBorder;

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
	// Frames of a match during which the controls hint shows (10 s).
	static constexpr int64 ControlsHintFrames = 600;
	static bool ShowControlsHint(const FJamPresentationFrame& F) { return F.SimFrame < ControlsHintFrames && F.Phase == EJamPhase::Play; }
	// The Directors Call prompt shows only after the first goal.
	static bool ShowDirectorsCall(const FJamPresentationFrame& F) { return F.bDirectorsCallReady && (F.ScoreHome + F.ScoreAway) > 0 && F.Phase != EJamPhase::Win && F.Phase != EJamPhase::Lose; }
	static FString CardFor(EJamPhase Phase);

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

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
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
	UPROPERTY(Transient) TObjectPtr<UJamRadarWidget> Radar;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CallText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CardTextBlock;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> HintText;

	FJamNetMarker Marker;
	FJamNetMarker OwnMarker;
};
