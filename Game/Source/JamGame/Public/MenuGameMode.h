// The game mode of Menu.umap (lane A8): no pawn, a spectator, the film-slate menu widget over a black screen, UI input mode
// with the cursor shown. The world settings of Menu.umap name this class.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MenuGameMode.generated.h"

class UJamMenuWidget;

UCLASS()
class JAMGAME_API AJamMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJamMenuGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintPure, Category = "Jam|Menu")
	UJamMenuWidget* GetMenu() const { return Menu; }

private:
	UPROPERTY(Transient) TObjectPtr<UJamMenuWidget> Menu;
};
