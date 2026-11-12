#include "JamSkaterVisual.h"

namespace JamSkaterSockets
{
	const FName Stick(TEXT("stick_socket"));
	const FName Puck(TEXT("puck_socket"));
	const FName Head(TEXT("head_socket"));
}

namespace JamMaterialParams
{
	const FName TeamColour(TEXT("TeamColour"));
	const FName NumberTens(TEXT("NumberTens"));
	const FName NumberOnes(TEXT("NumberOnes"));
	const FName Emissive(TEXT("Emissive"));
	const FName Wet(TEXT("Wet"));
	const FName Scuff(TEXT("Scuff"));
}

namespace JamBoostKinds
{
	const FName Speed(TEXT("speed"));
	const FName Precision(TEXT("precision"));
	const FName HeavyPuck(TEXT("heavy_puck"));
	const FName BigPuck(TEXT("big_puck"));
	const FName ReverseOther(TEXT("reverse_other"));
}
