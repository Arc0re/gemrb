/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header: /data/gemrb/cvs2svn/gemrb/gemrb/gemrb/plugins/Core/Game.h,v 1.12 2004/02/16 21:10:56 avenger_teambg Exp $
 *
 */

class Game;

#ifndef GAME_H
#define GAME_H

#ifdef WIN32

#ifdef GEM_BUILD_DLL
#define GEM_EXPORT __declspec(dllexport)
#else
#define GEM_EXPORT __declspec(dllimport)
#endif

#else
#define GEM_EXPORT
#endif

#include <vector>
#include "Actor.h"
#include "Map.h"

class GEM_EXPORT Game : public Scriptable
{
public:
	Game(void);
	~Game(void);
private:
	std::vector<Actor*> PCs;
	std::vector<Actor*> NPCs;
	std::vector<Map*> Maps;
public:
	int PartySize;
public:
	Actor* GetPC(unsigned int slot);
	/* finds an actor in party, returns slot, if not there, returns -1*/
	int InParty(Actor *pc);
	/* finds an actor in store, returns slot, if not there, returns -1*/
	int InStore(Actor *pc);
	/* joins party (if already an npc) */
	int JoinParty(Actor *pc);
	/* return current party size */
	int GetPartySize();
	/* leaves party (if in there) */
	int LeaveParty(Actor *pc);
	/*returns slot*/
	int SetPC(Actor *pc);
	int DelPC(unsigned int slot, bool autoFree = false);
	int DelNPC(unsigned int slot, bool autoFree = false);
	Map * GetMap(unsigned int index);
	int AddMap(Map* map);
	int LoadMap(char *ResRef);
	int DelMap(unsigned int index, bool autoFree = false);
	int AddNPC(Actor *npc);
	Actor* GetNPC(unsigned int Index);
};

#endif
