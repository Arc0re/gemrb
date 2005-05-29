#character generation, race (GUICG2)
import GemRB

RaceWindow = 0
TextAreaControl = 0
DoneButton = 0
RaceTable = 0
SubRacesTable = 0

def OnLoad():
	global RaceWindow, TextAreaControl, DoneButton
	global RaceTable, SubRacesTable
	
	GemRB.LoadWindowPack("GUICG", 800 ,600)
	RaceWindow = GemRB.LoadWindow (8)

	RaceTable = GemRB.LoadTable ("races")
	RaceCount = GemRB.GetTableRowCount (RaceTable)
	
	SubRacesTable = GemRB.LoadTable ("SUBRACES")

	for i in range(2,9):
		Button = GemRB.GetControl (RaceWindow,i)
		GemRB.SetButtonFlags (RaceWindow,Button,IE_GUI_BUTTON_RADIOBUTTON,OP_OR)
		
	for i in range(7):
		Button = GemRB.GetControl (RaceWindow,i+2)
		GemRB.SetText (RaceWindow,Button, GemRB.GetTableValue (RaceTable,i,0) )
		GemRB.SetButtonState (RaceWindow,Button,IE_GUI_BUTTON_ENABLED)
		GemRB.SetEvent (RaceWindow,Button,IE_GUI_BUTTON_ON_PRESS,"RacePress")
		GemRB.SetVarAssoc (RaceWindow,Button,"BaseRace",GemRB.GetTableValue (RaceTable, i, 3) )

	BackButton = GemRB.GetControl (RaceWindow,11) 
	GemRB.SetText (RaceWindow,BackButton,15416)
	DoneButton = GemRB.GetControl (RaceWindow,0)
	GemRB.SetText (RaceWindow,DoneButton,36789)
	GemRB.SetButtonState (RaceWindow,DoneButton,IE_GUI_BUTTON_DISABLED)
	GemRB.SetButtonFlags (RaceWindow, DoneButton, IE_GUI_BUTTON_DEFAULT,OP_OR)

	TextAreaControl = GemRB.GetControl (RaceWindow, 9)
	GemRB.SetText (RaceWindow,TextAreaControl,17237)

	GemRB.SetEvent (RaceWindow,DoneButton,IE_GUI_BUTTON_ON_PRESS,"NextPress")
	GemRB.SetEvent (RaceWindow,BackButton,IE_GUI_BUTTON_ON_PRESS,"BackPress")
	GemRB.SetVisible(RaceWindow,1)
	return

def RacePress():
	global RaceWindow, RaceTable, SubRacesTable
	Race = GemRB.GetVar ("BaseRace")
	GemRB.SetVar ("Race", Race)
	RaceID = GemRB.GetTableRowName (RaceTable, GemRB.FindTableValue (RaceTable, 3, Race) )
	HasSubRaces = GemRB.GetTableValue (SubRacesTable, RaceID, "PURE_RACE")
	if HasSubRaces == 0:
		GemRB.SetText (RaceWindow,TextAreaControl, GemRB.GetTableValue (RaceTable,RaceID,"DESC_REF") )
		GemRB.SetButtonState (RaceWindow,DoneButton,IE_GUI_BUTTON_ENABLED)
		return
	GemRB.UnloadWindow (RaceWindow)
	GemRB.SetNextScript ("SubRaces")
	return

def BackPress():
	GemRB.UnloadWindow (RaceWindow)
	GemRB.SetNextScript ("CharGen2")
	GemRB.SetVar ("BaseRace",0)  #scrapping the race value
	return

def NextPress():
	GemRB.UnloadWindow (RaceWindow)
	GemRB.SetNextScript ("CharGen3") #class
	return
