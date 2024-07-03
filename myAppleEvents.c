#include "myAppleEvents.h"
#include "main.h"
#include "globals.h"

AEEventHandlerUPP   ODOCHandlerUPP;

void InitAppleEvents()  {
	ODOCHandlerUPP = NewAEEventHandlerProc(HandleODOC);
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, ODOCHandlerUPP, 0, false);
}

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

pascal OSErr HandleODOC(AppleEvent *appleEvent, AppleEvent *aeReply, long handlerRefcon) {
#pragma unused (handlerRefcon)	
#pragma unused (aeReply)
	FSSpec		    myFSS;
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
        
        /* ProcessFile() uses the ancient FSOpen routine. It's expecting an SFReply.vRefNum,
           which is actually a working directory reference number, not a volume reference number */
	    OpenWD(myFSS.vRefNum, myFSS.parID, NULL, &wdRefNum);	    
	    ProcessFile(myFSS.name, wdRefNum);
	}
		
	AEDisposeDesc(&docList);

	return err;
}