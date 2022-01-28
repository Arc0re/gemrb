/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#include "TLKImporter.h"

#include "Audio.h"
#include "Calendar.h"
#include "DialogHandler.h"
#include "Game.h"
#include "Interface.h"
#include "TableMgr.h"
#include "GUI/GameControl.h"
#include "Scriptable/Actor.h"

using namespace GemRB;

struct gt_type
{
	int type;
	ieStrRef male;
	ieStrRef female;
};

TLKImporter::TLKImporter(void)
{
	gtmap.RemoveAll(NULL);
	gtmap.SetType(GEM_VARIABLES_POINTER);

	if (core->HasFeature(GF_CHARNAMEISGABBER)) {
		charname=-1;
	}

	AutoTable tm = gamedata->LoadTable("gender");
	int gtcount = 0;
	if (tm) {
		gtcount = tm->GetRowCount();
	}
	for(int i=0;i<gtcount;i++) {
		ieVariable key;

		strnuprcpy(key, tm->GetRowName(i), sizeof(ieVariable)-1 );
		gt_type *entry = new gt_type;
		entry->type = atoi(tm->QueryField(i,0));
		entry->male = tm->QueryFieldAsStrRef(i,1);
		entry->female = tm->QueryFieldAsStrRef(i,2);
		gtmap.SetAt(key, (void *) entry);
	}
}

static void ReleaseGtEntry(void *poi)
{
	delete (gt_type *) poi;
}

TLKImporter::~TLKImporter(void)
{
	delete str;
	
	gtmap.RemoveAll(ReleaseGtEntry);

	CloseAux();
}

void TLKImporter::CloseAux()
{
	if (OverrideTLK) {
		delete OverrideTLK;
	}
	OverrideTLK = NULL;
}

void TLKImporter::OpenAux()
{
	CloseAux();
	OverrideTLK = new CTlkOverride();
	if (OverrideTLK && !OverrideTLK->Init()) {
		CloseAux();
		Log(ERROR, "TlkImporter", "Cannot open tlk override!");
	}
}

bool TLKImporter::Open(DataStream* stream)
{
	if (stream == NULL) {
		return false;
	}
	delete str;
	str = stream;
	char Signature[8];
	str->Read( Signature, 8 );
	if (strncmp( Signature, "TLK\x20V1\x20\x20", 8 ) != 0) {
		Log(ERROR, "TLKImporter", "Not a valid TLK File.");
		return false;
	}
	str->ReadWord(Language); // English is 0
	str->ReadDword(StrRefCount);
	str->ReadDword(Offset);
	if (StrRefCount >= ieDword(ieStrRef::OVERRIDE_START)) {
		Log(ERROR, "TLKImporter", "Too many strings (%d), increase OVERRIDE_START.", StrRefCount);
		return false;
	}
	return true;
}

/* -1	 - GABBER
		0	 - PROTAGONIST
		1-9 - PLAYERx
*/
static inline Actor *GetActorFromSlot(int slot)
{
	if (slot==-1) {
		const GameControl *gc = core->GetGameControl();
		if (gc) {
			return gc->dialoghandler->GetSpeaker();
		}
		return NULL;
	}
	const Game *game = core->GetGame();
	if (!game) {
		return NULL;
	}
	if (slot==0) {
		return game->GetPC(0, false); //protagonist
	}
	return game->FindPC(slot);
}

String TLKImporter::Gabber() const
{
	const Actor *act = core->GetGameControl()->dialoghandler->GetSpeaker();
	if (act) {
		return act->GetName(1);
	}
	return L"?";
}

String TLKImporter::CharName(int slot) const
{
	const Actor *act = GetActorFromSlot(slot);
	if (act) {
		return act->GetName(1);
	}
	return L"?";
}

ieStrRef TLKImporter::ClassStrRef(int slot) const
{
	int clss = 0;
	const Actor *act = GetActorFromSlot(slot);
	if (act) {
		clss = act->GetActiveClass();
	}

	AutoTable tab = gamedata->LoadTable("classes");
	if (!tab) {
		return ieStrRef::INVALID;
	}
	int row = tab->FindTableValue("ID", clss, 0);
	return tab->QueryFieldAsStrRef(row, 0);
}

ieStrRef TLKImporter::RaceStrRef(int slot) const
{
	int race = 0;
	const Actor *act = GetActorFromSlot(slot);
	if (act) {
		race=act->GetStat(IE_RACE);
	}

	AutoTable tab = gamedata->LoadTable("races");
	if (!tab) {
		return ieStrRef::INVALID;
	}
	int row = tab->FindTableValue(3, race, 0);
	return tab->QueryFieldAsStrRef(row,0);
}

ieStrRef TLKImporter::GenderStrRef(int slot, ieStrRef malestrref, ieStrRef femalestrref) const
{
	const Actor *act = GetActorFromSlot(slot);
	if (act && (act->GetStat(IE_SEX)==SEX_FEMALE) ) {
		return femalestrref;
	}
	return malestrref;
}

//if this function returns nullptr then it is not a built in token
String TLKImporter::BuiltinToken(const char* Token)
{
	gt_type *entry = NULL;

	//these are gender specific tokens, they are customisable by gender.2da
	if (gtmap.Lookup(Token, (void *&) entry) ) {
		return GetString(GenderStrRef(entry->type, entry->male, entry->female));
	}

	//these are hardcoded, all engines are the same or don't use them
	if (!strcmp( Token, "DAYANDMONTH")) {
		ieDword dayandmonth=0;
		core->GetDictionary()->Lookup("DAYANDMONTH",dayandmonth);
		//preparing sub-tokens
		core->GetCalendar()->GetMonthName((int) dayandmonth);
		return GetString(ieStrRef::DAYANDMONTH, STRING_FLAGS::RESOLVE_TAGS);
	}

	if (!strcmp( Token, "FIGHTERTYPE" )) {
		return GetString(ieStrRef::FIGHTERTYPE, STRING_FLAGS::NONE);
	}
	if (!strcmp( Token, "CLASS" )) {
		//allow this to be used by direct setting of the token
		ieStrRef strref = ClassStrRef(-1);
		return GetString(strref, STRING_FLAGS::NONE);
	}

	if (!strcmp( Token, "RACE" )) {
		return GetString(RaceStrRef(-1), STRING_FLAGS::NONE);
	}

	// handle Player10 (max for MaxPartySize), then the rest
	if (!strncmp(Token, "PLAYER10", 8)) {
		return CharName(10);
	}
	if (!strncmp( Token, "PLAYER", 6 )) {
		return CharName(Token[strlen(Token)-1]-'1');
	}

	if (!strcmp( Token, "GABBER" )) {
		return Gabber();
	}
	if (!strcmp( Token, "CHARNAME" )) {
		return CharName(charname);
	}
	if (!strcmp( Token, "PRO_CLASS" )) {
		return GetString(ClassStrRef(0), STRING_FLAGS::NONE);
	}
	if (!strcmp( Token, "PRO_RACE" )) {
		return GetString(RaceStrRef(0), STRING_FLAGS::NONE);
	}
	if (!strcmp( Token, "MAGESCHOOL" )) {
		ieDword row = 0; //default value is 0 (generalist)
		//this is subject to change, the row number in magesch.2da
		core->GetDictionary()->Lookup( "MAGESCHOOL", row ); 
		AutoTable tm = gamedata->LoadTable("magesch");
		if (tm) {
			ieStrRef value = tm->QueryFieldAsStrRef(row, 2);
			return GetString(value, STRING_FLAGS::NONE);
		}
	}
	if (!strcmp( Token, "TM" )) {
		return L"\x99";
	}

	return L"";
}

String TLKImporter::ResolveTags(const String& source)
{
	const size_t strLen = source.length();
	auto mystrncpy = [&source, &strLen](char* dest, size_t idx, size_t maxlength,
		char delim)
	{
		maxlength = std::min(maxlength, strLen);
		while (idx < source.length() && (source[idx] != delim) && maxlength--) {
			char chr = source[idx++];
			if (chr != ' ') *dest++ = chr;
		}
		*dest = '\0';
		return idx;
	};
	
	char Token[MAX_VARIABLE_LENGTH + 1];
	String dest;
	for (size_t i = 0; source[i]; i++) {
		auto srcch = source[i];
		if (srcch == L'<') {
			i = mystrncpy(Token, i + 1, MAX_VARIABLE_LENGTH, '>');
			String str = BuiltinToken(Token);
			if (str.empty()) {
				int TokenLength = core->GetTokenDictionary()->GetValueLength(Token);
				if (TokenLength) {
					char* tokVal = new char[TokenLength + 1];
					core->GetTokenDictionary()->Lookup(Token, tokVal, TokenLength);
					String* tmp = StringFromCString(tokVal);
					assert(tmp);
					dest.append(std::move(*tmp));
					delete tmp;
					delete[] tokVal;
				}
			} else {
				dest.append(str);
			}
		} else if (srcch == L'%' && i + 1 < strLen - 1) {
			// also resolve format strings
			// we assume they are % followed by a single character
			auto nextch = source[i + 1];
			// we are going to only work with %d
			// the originals dont seem to use anything else so we will play it safe
			//if (isspace(nextch) || ispunct(nextch)) {
			if (nextch != L'd') {
				dest.push_back(srcch);
			} else {
				dest.append(L"{}");
				++i;
			}
		} else if (srcch == L'[') {
			// voice actor directives
			i = source.find_first_of(L']', i + 1);
			if (i == String::npos) break;
		} else {
			dest.push_back(srcch);
		}
	}
	return dest;
}

ieStrRef TLKImporter::UpdateString(ieStrRef strref, const String& newvalue)
{
	if (!OverrideTLK) {
		Log(ERROR, "TLKImporter", "Custom string is not supported by this game format.");
		return ieStrRef::INVALID;
	}

	return OverrideTLK->UpdateString(strref, newvalue);
}

String TLKImporter::GetString(ieStrRef strref, STRING_FLAGS flags)
{
	String string;
	bool empty = !(flags & STRING_FLAGS::ALLOW_ZERO) && !strref;
	ieWord type;
	ResRef SoundResRef;

	if (empty || strref >= ieStrRef::OVERRIDE_START || (strref >= ieStrRef::BIO_START && strref <= ieStrRef::BIO_END)) {
		if (OverrideTLK) {
			size_t Length;
			char* cstr = OverrideTLK->ResolveAuxString(strref, Length);
			String* tmp = StringFromCString(cstr);
			std::swap(string, *tmp);
			free(cstr);
			delete tmp;
		}
		type = 0;
		SoundResRef.Reset();
	} else {
		ieDword Volume, Pitch, StrOffset;
		ieDword l;
		if (str->Seek( 18 + (ieDword(strref) * 0x1A), GEM_STREAM_START ) == GEM_ERROR) {
			return L"";
		}
		str->ReadWord(type);
		str->ReadResRef( SoundResRef );
		// volume and pitch variance fields are known to be unused at minimum in bg1
		str->ReadDword(Volume);
		str->ReadDword(Pitch);
		str->ReadDword(StrOffset);
		str->ReadDword(l);
				
		if (type & 1) {
			str->Seek( StrOffset + Offset, GEM_STREAM_START );
			char* cstr = (char *)malloc(l + 1);
			cstr[l] = '\0';
			str->Read(cstr, l);
			String* tmp = StringFromCString(cstr);
			std::swap(string, *tmp);
			delete tmp;
			free(cstr);
		}
	}

	if (bool(flags & STRING_FLAGS::RESOLVE_TAGS) || (type & 4)) {
		string = ResolveTags(string);
	}
	if (type & 2 && bool(flags & STRING_FLAGS::SOUND) && !SoundResRef.IsEmpty()) {
		// GEM_SND_SPEECH will stop the previous sound source
		unsigned int flag = GEM_SND_RELATIVE | (uint32_t(flags) & (GEM_SND_SPEECH | GEM_SND_QUEUE));
		core->GetAudioDrv()->Play(SoundResRef, SFX_CHAN_DIALOG, Point(), flag);
	}
	if (bool(flags & STRING_FLAGS::STRREFON)) {
		string = std::to_wstring(ieDword(strref)) + L": " + string;
	}
	return string;
}

bool TLKImporter::HasAltTLK() const
{
	// only English (language id 0) has no alt files
	return Language;
}

StringBlock TLKImporter::GetStringBlock(ieStrRef strref, STRING_FLAGS flags)
{
	bool empty = !(flags & STRING_FLAGS::ALLOW_ZERO) && !strref;
	if (empty) {
		return StringBlock();
	}
	ieWord type;
	str->Seek( 18 + (ieDword(strref) * 0x1A), GEM_STREAM_START );
	str->ReadWord(type);
	ResRef soundRef;
	str->ReadResRef( soundRef );
	return StringBlock(GetString( strref, flags ), soundRef);
}

#include "plugindef.h"

GEMRB_PLUGIN(0xBB6F380, "TLK File Importer")
PLUGIN_CLASS(IE_TLK_CLASS_ID, TLKImporter)
END_PLUGIN()
