# -*-python-*-
# GemRB - Infinity Engine Emulator
# Copyright (C) 2003-2004 The GemRB Project
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# $Header: /data/gemrb/cvs2svn/gemrb/gemrb/gemrb/GUIScripts/iwd2/GUICommonWindows.py,v 1.10 2005/05/29 22:16:18 avenger_teambg Exp $


# GUICommonWindows.py - functions to open common windows in lower part of the screen

import GemRB
from GUIDefines import *
from ie_stats import *

FRAME_PC_SELECTED = 0
FRAME_PC_TARGET   = 1

# Buttons:
# 0 CNTREACH
# 1 INVNT
# 2 MAP
# 3 MAGE
# 4 AI
# 5 STATS
# 6 JRNL
# 7 PRIEST
# 8 OPTION
# 9 REST
# 10 TXTE

def SetupMenuWindowControls (Window):

	# Spellbook
	Button = GemRB.GetControl (Window, 4)
	GemRB.SetTooltip (Window, Button, 16309)
	GemRB.SetButtonFlags(Window, Button, IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	GemRB.SetVarAssoc(Window, Button, "SelectedWindow", 0)
	GemRB.SetEvent (Window, Button, IE_GUI_BUTTON_ON_PRESS, "OpenSpellBookWindow")

	# Inventory
	Button = GemRB.GetControl (Window, 5)
	GemRB.SetTooltip (Window, Button, 16307)
	GemRB.SetButtonFlags(Window, Button, IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	GemRB.SetVarAssoc(Window, Button, "SelectedWindow", 1)
	GemRB.SetEvent (Window, Button, IE_GUI_BUTTON_ON_PRESS, "OpenInventoryWindow")

	# Journal
	Button = GemRB.GetControl (Window, 6)
	GemRB.SetTooltip (Window, Button, 16308)
	GemRB.SetButtonFlags(Window, Button, IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	GemRB.SetVarAssoc(Window, Button, "SelectedWindow", 2)
	GemRB.SetEvent (Window, Button, IE_GUI_BUTTON_ON_PRESS, "OpenJournalWindow")
	# Map
	Button = GemRB.GetControl (Window, 7)
	GemRB.SetTooltip (Window, Button, 16310)
	GemRB.SetButtonFlags(Window, Button, IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	GemRB.SetVarAssoc(Window, Button, "SelectedWindow", 3)
	GemRB.SetEvent (Window, Button, IE_GUI_BUTTON_ON_PRESS, "OpenMapWindow")

	# Records
	Button = GemRB.GetControl (Window, 8)
	GemRB.SetTooltip (Window, Button, 16306)
	GemRB.SetButtonFlags(Window, Button, IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	GemRB.SetVarAssoc(Window, Button, "SelectedWindow", 4)
	GemRB.SetEvent (Window, Button, IE_GUI_BUTTON_ON_PRESS, "OpenRecordsWindow")

	# Options
	Button = GemRB.GetControl (Window, 9)
	GemRB.SetTooltip (Window, Button, 16311)
	GemRB.SetButtonFlags(Window, Button, IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	GemRB.SetVarAssoc(Window, Button, "SelectedWindow", 7)
	GemRB.SetEvent (Window, Button, IE_GUI_BUTTON_ON_PRESS, "OpenOptionsWindow")

	# Select All
	Button = GemRB.GetControl (Window, 11)
	GemRB.SetTooltip (Window, Button, 10485)
	GemRB.SetEvent (Window, Button, IE_GUI_BUTTON_ON_PRESS, "SelectAllOnPress")

	# Rest
	Button = GemRB.GetControl (Window, 12)
	GemRB.SetTooltip (Window, Button, 11942)
	GemRB.SetEvent(Window, Button, IE_GUI_BUTTON_ON_PRESS, "OpenStoreWindow")

	# Character Arbitration
	Button = GemRB.GetControl (Window, 13)
	GemRB.SetTooltip (Window, Button, 16312)
	GemRB.SetEvent(Window, Button, IE_GUI_BUTTON_ON_PRESS, "CharacterWindow")

	return

def AIPress ():
	print "AIPress"
	return

def RestPress ():
	print "RestPress"
	return

def SetupActionsWindowControls (Window):
	# 41627 - Return to the Game World

	# Select all characters
	Button = GemRB.GetControl (Window, 1)
	GemRB.SetTooltip (Window, Button, 41659)

	# Abort current action
	Button = GemRB.GetControl (Window, 3)
	GemRB.SetTooltip (Window, Button, 41655)

	# Formations
	Button = GemRB.GetControl (Window, 4)
	GemRB.SetTooltip (Window, Button, 44945)
	return

def GetActorRaceTitle (actor):
        Table = GemRB.LoadTable ("races")
        RaceID = GemRB.GetPlayerStat (actor, IE_SUBRACE)
        if RaceID:
                RaceID += GemRB.GetPlayerStat (actor, IE_RACE)<<16
        else:
                RaceID = GemRB.GetPlayerStat (actor, IE_RACE)
        row = GemRB.FindTableValue (Table, 3, RaceID )
        RaceTitle = GemRB.GetTableValue (Table, row, 2)
        GemRB.UnloadTable (Table)
	return RaceTitle

def GetActorClassTitle (actor):
	ClassTitle = GemRB.GetPlayerStat (actor, IE_TITLE1)
	Kit = GemRB.GetPlayerStat (actor, IE_KIT)
	Class = GemRB.GetPlayerStat (actor, IE_CLASS)
	ClassTable = GemRB.LoadTable ("classes")
	if Kit==0x4000 or Kit==0: #pure class
		ClassIndex = Class
	else: #bad, because kit clashes with classid
		ClassIndex = GemRB.FindTableValue (ClassTable, Kit, 2)

	if ClassTitle==0:
		ClassTitle=GemRB.GetTableValue (ClassTable, ClassIndex, 0)

	GemRB.UnloadTable (ClassTable)
	return ClassTitle

def GetActorPaperDoll (actor):
	PortraitTable = GemRB.LoadTable ("avatars")
	anim_id = GemRB.GetPlayerStat (actor, IE_ANIMATION_ID)
	level = GemRB.GetPlayerStat (actor, IE_ARMOR_TYPE)
	row = "0x%04X" %anim_id
	which = "AT_%d" %(level+1)
	return GemRB.GetTableValue (PortraitTable, row, which)


SelectionChangeHandler = None

def SetSelectionChangeHandler (handler):
	global SelectionChangeHandler

	# Switching from walking to non-walking environment:
	#   set the first selected PC in walking env as a selected
	#   in nonwalking env
	if (not SelectionChangeHandler) and handler:
		sel = GemRB.GameGetFirstSelectedPC ()
		if not sel:
			sel = 1
		GemRB.GameSelectPCSingle (sel)

	SelectionChangeHandler = handler

	# redraw selection on change main selection | single selection
	SelectionChanged ()

def RunSelectionChangeHandler ():
	if SelectionChangeHandler:
		SelectionChangeHandler ()

def OpenPortraitWindow (needcontrols):
	global PortraitWindow

	PortraitWindow = Window = GemRB.LoadWindow(1)

	if needcontrols:
		# AI
		Button = GemRB.GetControl (Window, 6)
		GemRB.SetButtonState (Window, Button, IE_GUI_BUTTON_DISABLED)
		GemRB.SetEvent (Window, Button, IE_GUI_BUTTON_ON_PRESS, "AIPress")

		#Select All
		Button = GemRB.GetControl (Window, 7)
		GemRB.SetTooltip (Window, Button, 10485)
		GemRB.SetEvent (Window, Button, IE_GUI_BUTTON_ON_PRESS, "SelectAllOnPress")
	for i in range (PARTY_SIZE):
		Button = GemRB.GetControl (Window, i)
		GemRB.SetEvent (Window, Button, IE_GUI_BUTTON_ON_PRESS, "PortraitButtonOnPress")

		GemRB.SetButtonFlags(Window, Button, IE_GUI_BUTTON_ALIGN_BOTTOM|IE_GUI_BUTTON_ALIGN_LEFT|IE_GUI_BUTTON_PICTURE, OP_SET)

		GemRB.SetButtonBorder (Window, Button, FRAME_PC_SELECTED, 1, 1,
2, 2, 0, 255, 0, 255)
		GemRB.SetButtonBorder (Window, Button, FRAME_PC_TARGET, 3, 3, 4, 4, 255, 255, 0, 255)
		GemRB.SetVarAssoc (Window, Button, "PressedPortrait", i)
		GemRB.SetButtonFont (Window, Button, 'NUMFONT')

	UpdatePortraitWindow ()
	return Window
	
def UpdatePortraitWindow ():
	Window = PortraitWindow

	for i in range (PARTY_SIZE):
		Button = GemRB.GetControl (Window, i)
		pic = GemRB.GetPlayerPortrait (i+1,1)
		if not pic:
			GemRB.SetButtonFlags (Window, Button, IE_GUI_BUTTON_NO_IMAGE, OP_OR)
			continue

		GemRB.SetButtonFlags (Window, Button, IE_GUI_BUTTON_NO_IMAGE, OP_NAND)
		GemRB.SetButtonPicture(Window, Button, pic)

		hp = GemRB.GetPlayerStat (i+1, IE_HITPOINTS)
		hp_max = GemRB.GetPlayerStat (i+1, IE_MAXHITPOINTS)
		GemRB.SetText (Window, Button, "%d/%d" %(hp, hp_max))
		GemRB.SetTooltip (Window, Button, GemRB.GetPlayerName (i+1, 1) + "\n%d/%d" %(hp, hp_max))

	return

def PortraitButtonOnPress ():
	i = GemRB.GetVar ("PressedPortrait")

	if (not SelectionChangeHandler):
		GemRB.GameSelectPC (i + 1, True, SELECT_REPLACE)
	else:
		GemRB.GameSelectPCSingle (i + 1)
		SelectionChanged ()
		RunSelectionChangeHandler ()
	return

def PortraitButtonOnShiftPress ():
	i = GemRB.GetVar ('PressedPortrait')

	if (not SelectionChangeHandler):
		sel = GemRB.GameIsPCSelected (i + 1)
		sel = not sel
		GemRB.GameSelectPC (i + 1, sel)
	else:
		GemRB.GameSelectPCSingle (i + 1)
		SelectionChanged ()
		RunSelectionChangeHandler ()
	return

def SelectAllOnPress ():
	GemRB.GameSelectPC (0, 1)
	return

# Run by Game class when selection was changed
def SelectionChanged ():
	# FIXME: hack. If defined, display single selection
	if (not SelectionChangeHandler):
		for i in range (0, PARTY_SIZE):
			Button = GemRB.GetControl (PortraitWindow, i)
			GemRB.EnableButtonBorder (PortraitWindow, Button, FRAME_PC_SELECTED, GemRB.GameIsPCSelected (i + 1))
	else:
		sel = GemRB.GameGetSelectedPCSingle ()
		for i in range (0, PARTY_SIZE):
			Button = GemRB.GetControl (PortraitWindow, i)

		for i in range (0, PARTY_SIZE):
			Button = GemRB.GetControl (PortraitWindow, i)
			GemRB.EnableButtonBorder (PortraitWindow, Button, FRAME_PC_SELECTED, i + 1 == sel)

def GetSavingThrow (SaveName, row, level):
	SaveTable = GemRB.LoadTable (SaveName)
	tmp = GemRB.GetTableValue (SaveTable, level)
	GemRB.UnloadTable (SaveName)
	return tmp

def SetupSavingThrows (pc):
	level1 = GemRB.GetPlayerStat (pc, IE_LEVEL) - 1
	if level1 > 20:
		level1 = 20
	level2 = GemRB.GetPlayerStat (pc, IE_LEVEL2) - 1
	if level2 > 20:
		level2 = 20
	Class = GemRB.GetPlayerStat (pc, IE_CLASS)
	ClassTable = GemRB.LoadTable ("classes")
	Class = GemRB.FindTableValue (ClassTable, 5, Class)
	Multi = GemRB.GetTableValue (ClassTable, 4, Class)
	if Multi:
		if Class == 7:
			#fighter/mage
			Class = GemRB.FindTableValue (ClassTable, 5, 1)
		else:
			#fighter/thief
			Class = GemRB.FindTableValue (ClassTable, 5, 4)
		SaveName2 = GemRB.GetTableValue (ClassTable, Class, 3)
		Class = 0  #fighter
		print "SaveName2", SaveName2

	SaveName1 = GemRB.GetTableValue (ClassTable, Class, 3)
	print "SaveName1", SaveName1

	for row in range(5):
		tmp1 = GetSavingThrow (SaveName1, row, level1)
		if Multi:
			tmp2 = GetSavingThrow (SaveName2, row, level2)
			if tmp2<tmp1:
				tmp1=tmp2
		GemRB.SetPlayerStat (pc, IE_SAVEVSDEATH+row, tmp1)
		print "Savingthrow:", tmp1
	return

def SetEncumbranceLabels (Window, Label, Label2, pc):
	# encumbrance
	# Loading tables of modifications
	Table = GemRB.LoadTable("strmod")
	TableEx = GemRB.LoadTable("strmodex")
	# Getting the character's strength
	sstr = GemRB.GetPlayerStat (pc, IE_STR)
	ext_str = GemRB.GetPlayerStat (pc, IE_STREXTRA)

	max_encumb = GemRB.GetTableValue (Table, sstr, 3) + GemRB.GetTableValue(TableEx, ext_str, 3)
	encumbrance = GemRB.GetPlayerStat (pc, IE_ENCUMBRANCE)

	Label = GemRB.GetControl (Window, 0x10000043)
	GemRB.SetText (Window, Label, str(encumbrance) + ":")

	Label2 = GemRB.GetControl (Window, 0x10000044)
	GemRB.SetText (Window, Label2, str(max_encumb) + ":")
	ratio = (0.0 + encumbrance) / max_encumb
	if ratio > 1.0:
		GemRB.SetLabelTextColor (Window, Label, 255, 0, 0)
		GemRB.SetLabelTextColor (Window, Label2, 255, 0, 0)
	elif ratio > 0.8:
		GemRB.SetLabelTextColor (Window, Label, 255, 255, 0)
		GemRB.SetLabelTextColor (Window, Label2, 255, 0, 0)
	else:
		GemRB.SetLabelTextColor (Window, Label, 255, 255, 255)
		GemRB.SetLabelTextColor (Window, Label2, 255, 0, 0)
	return

