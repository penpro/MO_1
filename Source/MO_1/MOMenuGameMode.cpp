#include "MOMenuGameMode.h"
#include "MOMenuPlayerController.h"

AMOMenuGameMode::AMOMenuGameMode()
{
	PlayerControllerClass = AMOMenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;  // no pawn for menu map
}
