#include <Controls.h>
#include <Memory.h>
#include <TextEdit.h>
#include <Windows.h>

extern TEHandle        gTEHndl;

TEHandle SetupTE(WindowPtr winPtr);
void ScrollContents(TEHandle textHndl, short dh, short dv);
void UpdateText(TEHandle textHndl);
void ChangeMouse(TEHandle textHndl);
pascal Boolean ClickLoop(void);