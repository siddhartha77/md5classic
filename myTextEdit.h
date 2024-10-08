#include <Controls.h>
#include <Memory.h>
#include <MixedMode.h>
#include <OSUtils.h>
#include <TextEdit.h>
#include <Windows.h>

enum {
    /* Delay in ticks so scrolling isn't crazy fast. */
    kClickLoopScrollDelay   = 2
};

void SetupTE(WindowPtr winPtr);
void GetTERect(WindowPtr winPtr, Rect *teRect);
void AdjustViewRect(TEHandle teHandl);
void AdjustTE(WindowPtr winPtr);
void UpdateTEWordWrap(WindowPtr winPtr);
void ChangeMouse(TEHandle teHndl);
pascal Boolean ClickLoop(void);