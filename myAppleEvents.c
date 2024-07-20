#include "myAppleEvents.h"
#include "main.h"
#include "globals.h"

AEEventHandlerUPP   ODOCHandlerUPP;

/* Install the event handler procs. */
void InitAppleEvents()  {
	ODOCHandlerUPP = NewAEEventHandlerProc(HandleODOC);
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, ODOCHandlerUPP, 0, false);
}

/* Check if the required Apple Event parameters are set. */
OSErr GotRequiredParams (AppleEvent *theAppleEvent) {
	DescType	typeCode;
	Size		actualSize;
	OSErr		retErr, err;

	err = AEGetAttributePtr ( theAppleEvent, keyMissedKeywordAttr,
                typeWildCard, &typeCode, NULL, 0, &actualSize );
	
	if ( err == errAEDescNotFound ) {
		retErr = noErr;
	} else if ( err == noErr ) {
		retErr = errAEEventNotHandled;
	} else {
		retErr = err;
    }
	
	return retErr;
}

/* OpenDocument (via drag and drop) Apple Event handler. Support is included for multiple files. */
pascal OSErr HandleODOC(AppleEvent *appleEvent, AppleEvent *aeReply, long handlerRefcon) {
#pragma unused (handlerRefcon)	
#pragma unused (aeReply)
	FSSpec		    myFSS;
	SFReply         sfReply;
	AEDescList	    docList;
	Size		    actualSize;
	AEKeyword	    keywd;
	DescType	    typeCode;
	OSErr		    err;
	long		    index, itemsInList;
	short           wdRefNum;

	PrintError(err = AEGetParamDesc(appleEvent, keyDirectObject, typeAEList, &docList));
	PrintError(err = GotRequiredParams(appleEvent));

	/*	How many items do we have?. */
	PrintError(err = AECountItems(&docList, &itemsInList));
	
	for (index = 1 ; index <= itemsInList ; index++) {
		PrintError(err = AEGetNthPtr(&docList, index, typeFSS, &keywd, &typeCode,
		                        (Ptr)&myFSS, sizeof(myFSS), &actualSize));
        
        if (err) return err;
        
        /* SFReply.vRefNum is actually a working directory reference number, not a volume reference number */
	    OpenWD(myFSS.vRefNum, myFSS.parID, NULL, &wdRefNum);
	    sfReply.vRefNum = wdRefNum;
	    sfReply.version = 0;
	    BlockMove(myFSS.name, sfReply.fName, myFSS.name[0] + 1);
	    ProcessFile(&sfReply);
	}
	
	AEDisposeDesc(&docList);

	return err;
}