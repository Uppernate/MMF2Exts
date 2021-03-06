// ============================================================================
// This file contains routines that are handled only during the Edittime,
// under the Frame and Event editors.
//
// Including creating, display, and setting up your object.
// ============================================================================

#include "Common.h"
#include "DarkEdif.h"


// ============================================================================
// ROUTINES USED UNDER FRAME EDITOR
// ============================================================================

#if EditorBuild

// Called once object is created or modified, just after setup.
// Also called before showing the "Insert an object" dialog if your object
// has no icon resource
int FusionAPI MakeIconEx(mv *mV, cSurface *pIconSf, TCHAR *lpName, ObjInfo *oiPtr, EDITDATA *edPtr)
{
#pragma DllExportHint
	pIconSf->Delete();
	pIconSf->Clone(*SDK->Icon);

	pIconSf->SetTransparentColor(RGB(255, 0, 255));
	return 0;
}

// Called when you choose "Create new object". It should display the setup box
// and initialize everything in the datazone.
int FusionAPI CreateObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr)
{
#pragma DllExportHint
	// Check compatibility
	if (!IS_COMPATIBLE(mV))
		return -1;
	Edif::Init(mV, edPtr);

	if (edPtr->eHeader.extSize < sizeof(EDITDATA))
	{
		void* newEd = mvReAllocEditData(mV, edPtr, sizeof(EDITDATA));
		if (!newEd) {
			MessageBoxA(NULL, "Failed to allocate enough size for properites.", PROJECT_NAME " error", MB_ICONERROR);
			return -1;
		}
		edPtr = (EDITDATA *) newEd;
	}

	// Set default object settings from DefaultState.
	memset(((char *)edPtr) + sizeof(edPtr->eHeader), 0, sizeof(EDITDATA) - sizeof(EDITDATA::eHeader));

	const auto & propsJSON = CurLang["Properties"];
	edPtr->automaticClear = propsJSON[1]["DefaultState"];
	edPtr->isGlobal = propsJSON[2]["DefaultState"];
	if (strcpy_s(edPtr->edGlobalID, 255, propsJSON[3]["DefaultState"]))
		MessageBoxA(NULL, "Error initialising property 3; error copying string.", "DarkEdif - CreateObject() error", MB_OK);
	edPtr->multiThreading = propsJSON[4]["DefaultState"];
	edPtr->timeoutWarningEnabled = propsJSON[5]["DefaultState"];
	edPtr->fullDeleteEnabled = propsJSON[6]["DefaultState"];

	// ext version 1 = before Unicode port, 2 = after. Does not mean the properties are UTF16; they're UTF8.
	edPtr->eHeader.extVersion = 2;

	// InitialisePropertiesFromJSON(mV, edPtr);
	return 0;
}

cSurface * FusionAPI WinGetSurface(int idWin, int surfID = 0);
// Displays the object under the frame editor
void FusionAPI EditorDisplay(mv *mV, ObjectInfo * oiPtr, LevelObject * loPtr, EDITDATA * edPtr, RECT * rc)
{
#pragma DllExportHint
	cSurface * Surface = WinGetSurface((int) mV->IdEditWin);
	if (!Surface)
		return;

	DarkEdif::SDKUpdater::RunUpdateNotifs(mV, edPtr);

	SDK->Icon->Blit(*Surface, rc->left, rc->top, BMODE_TRANSP, BOP_COPY, 0);
}


// ============================================================================
// PROPERTIES
// ============================================================================

// Inserts properties into the properties of the object.
BOOL FusionAPI GetProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
#pragma DllExportHint
	mvInsertProps(mV, edPtr, ::SDK->EdittimeProperties, PROPID_TAB_GENERAL, TRUE);

	// OK
	return TRUE;
}

// Called when the properties are removed from the property window.
void FusionAPI ReleaseProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
#pragma DllExportHint
}

// Returns the value of properties that have a value.
// Note: see GetPropCheck for checkbox properties
Prop * FusionAPI GetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	if (PropID < PROPID_EXTITEM_CUSTOM_FIRST)
		return NULL; // built-in properties, not managed by ext

	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;

	if ((unsigned int) CurLang["Properties"].u.object.length > ID)
	{
		if (ID == 0)
		{
			char extVerBuffer[256];
			sprintf_s(extVerBuffer, CurLang["Properties"][ID]["DefaultState"], lacewing::relayclient::buildnum, STRIFY(CONFIG));
			return new Prop_Str(UTF8ToTString(extVerBuffer).c_str());
		}
		if (ID == 3)
			return new Prop_Str(UTF8ToTString(edPtr->edGlobalID).c_str());

		// Note that checkbox-only properties call GetPropValue() too for no reason, so
		return NULL;
	}

	MessageBoxA(NULL, "Invalid property ID given to GetPropValue() call.", PROJECT_NAME " - Invalid property", MB_OK);
	return NULL;
}

// Returns the checked state of properties that have a check box.
BOOL FusionAPI GetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
#pragma DllExportHint
	if (PropID < PROPID_EXTITEM_CUSTOM_FIRST)
		return FALSE; // built-in properties, not managed by ext

	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;

	if ((unsigned int) CurLang["Properties"].u.object.length > ID)
	{
		if (ID == 1)
			return edPtr->automaticClear;
		if (ID == 2)
			return edPtr->isGlobal;
		if (ID == 4)
			return edPtr->multiThreading;
		if (ID == 5)
			return edPtr->timeoutWarningEnabled;
		if (ID == 6)
			return edPtr->fullDeleteEnabled;
	}

	MessageBoxA(NULL, "Invalid property ID given to GetPropCheck() call.", "DarkEdif - Invalid property", MB_OK);
	return FALSE; // Unchecked
}

// This routine is called by MMF after a property has been modified.
void FusionAPI SetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID, Prop * NewParam)
{
#pragma DllExportHint
	if (PropID < PROPID_EXTITEM_CUSTOM_FIRST)
		return; // built-in properties, not managed by ext

	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;
	if (ID == 3)
	{
		const std::string newValAsU8 = TStringToUTF8(((Prop_Str *)NewParam)->String);
		if (strcpy_s(edPtr->edGlobalID, newValAsU8.c_str()))
			MessageBoxA(NULL, "Error setting global ID; too long?", PROJECT_NAME " - SetPropValue() error", MB_OK);
	}

	// You may want to have your object redrawn in the frame editor after the modifications,
	// in this case, just call this function
	// mvInvalidateObject(mV, edPtr);
}

// This routine is called by MMF when the user modifies a checkbox in the properties.
void FusionAPI SetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID, BOOL Check)
{
#pragma DllExportHint
	if (PropID < PROPID_EXTITEM_CUSTOM_FIRST)
		return; // built-in properties, not managed by ext

	unsigned int ID = PropID - PROPID_EXTITEM_CUSTOM_FIRST;

	if (CurLang["Properties"].u.object.length > ID)
	{
		// The ending ", (void)0" means that the expression evaluates to void overall
		if (ID == 1)
			return edPtr->automaticClear = (Check != 0), (void)0;
		if (ID == 2)
			return edPtr->isGlobal = (Check != 0), (void)0;
		if (ID == 4)
			return edPtr->multiThreading = (Check != 0), (void)0;
		if (ID == 5)
			return edPtr->timeoutWarningEnabled = (Check != 0), (void)0;
		if (ID == 6)
			return edPtr->fullDeleteEnabled = (Check != 0), (void)0;
	}

	MessageBoxA(NULL, "Invalid property ID given to SetPropCheck() call.", PROJECT_NAME " - Invalid property", MB_OK);
}

#endif // EditorBuild
