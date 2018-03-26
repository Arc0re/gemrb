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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#include "TextArea.h"

#include "win32def.h"

#include "Interface.h"
#include "Variables.h"
#include "GUI/EventMgr.h"
#include "GUI/Window.h"

namespace GemRB {
	
TextArea::SpanSelector::SpanSelector(TextArea& ta, const std::vector<const String*>& opts, bool numbered, Margin m)
: TextContainer(Region(), ta.ftext, ta.palettes[PALETTE_SELECTED]), ta(ta)
{
	SetFlags(RESIZE_WIDTH, OP_NAND);

	selectedSpan = NULL;
	hoverSpan = NULL;

	size = opts.size();

	Size s = ta.Dimensions();
	s.h = 0; // will dynamically size itself

	SetMargin(m);
	SetFrameSize(s);

	Size flexFrame(-1, 0); // flex frame for hanging indent after optnum
	Point origin(margin.left, margin.top);
	Region r(origin, Dimensions());
	r.w = std::max(r.w - margin.left - margin.right, 0);
	r.h = std::max(r.h - margin.top - margin.bottom, 0);

	for (size_t i = 0; i < opts.size(); i++) {
		TextContainer* selOption = new OptSpan(r, ta.ftext, ta.palettes[PALETTE_OPTIONS]);
		if (numbered) {
			wchar_t optNum[6];
			swprintf(optNum, sizeof(optNum)/sizeof(optNum[0]), L"%d. - ", i+1);
			// TODO: as per the original PALETTE_SELECTED should be updated to the PC color (same color their name is rendered in)
			// but that should probably actually be done by the dialog handler, not here.
			selOption->AppendContent(new TextSpan(optNum, NULL, ta.palettes[PALETTE_SELECTED]));
		}
		selOption->AppendContent(new TextSpan(*opts[i], NULL, NULL, &flexFrame));
		AddSubviewInFrontOfView(selOption);

		if (EventMgr::TouchInputEnabled) {
			// keeping the options spaced out (for touch screens)
			r.y += ta.LineHeight();
		}
		r.y += selOption->Dimensions().h; // FIXME: this can overflow the height
	}

	if (numbered) {
		// in a sane world we would simply focus the window and this View
		// unfortunately, focusing the window makes it overlap with the portwin/optwin...
		EventMgr::EventCallback* cb = new MethodCallback<SpanSelector, const Event&, bool>(this, &SpanSelector::KeyEvent);
		id = EventMgr::RegisterEventMonitor(cb, Event::KeyDownMask);
	} else {
		id = -1;
	}

	assert((Flags()&RESIZE_WIDTH) == 0);
}
	
TextArea::SpanSelector::~SpanSelector()
{
	EventMgr::UnRegisterEventMonitor(id);
}

bool TextArea::SpanSelector::KeyEvent(const Event& event)
{
	return OnKeyPress(event.keyboard, 0);
}

bool TextArea::SpanSelector::OnKeyPress(const KeyboardEvent& key, unsigned short /*mod*/)
{
	KeyboardKey chr = key.character;
	if (chr < '1' || chr > '9')
		return false;

	unsigned int idx = chr - '1';
	MakeSelection(idx);
	return true;
}

void TextArea::SpanSelector::ClearHover()
{
	if (hoverSpan) {
		if (hoverSpan == selectedSpan) {
			hoverSpan->SetPalette(ta.palettes[PALETTE_SELECTED]);
		} else {
			// reset the old hover span
			hoverSpan->SetPalette(ta.palettes[PALETTE_OPTIONS]);
		}
		hoverSpan = NULL;
	}
}

void TextArea::SpanSelector::MakeSelection(size_t idx)
{
	TextContainer* optspan = TextAtIndex(idx);

	if (optspan == selectedSpan) {
		return; // already selected
	}

	if (selectedSpan && selectedSpan != optspan) {
		// reset the previous selection
		selectedSpan->SetPalette(ta.palettes[PALETTE_OPTIONS]);
	}
	selectedSpan = optspan;
	
	if (selectedSpan) {
		selectedSpan->SetPalette(ta.palettes[PALETTE_SELECTED]);
	}

	// beware, this will recursively call this function.
	ta.UpdateState(static_cast<unsigned int>(idx));
}
	
TextContainer* TextArea::SpanSelector::TextAtPoint(const Point& p)
{
	// container only has text, so...
	return static_cast<TextContainer*>(SubviewAt(p, true, false));
}
	
TextContainer* TextArea::SpanSelector::TextAtIndex(size_t idx)
{
	if (subViews.empty()|| idx > subViews.size() - 1) {
		return NULL;
	}

	std::list<View*>::reverse_iterator it = subViews.rbegin();
	std::advance(it, idx);
	return static_cast<TextContainer*>(*it);
}

bool TextArea::SpanSelector::OnMouseOver(const MouseEvent& me)
{
	Point p = ConvertPointFromScreen(me.Pos());
	TextContainer* span = TextAtPoint(p);
	
	if (hoverSpan || span)
		MarkDirty();
	
	ClearHover();
	if (span) {
		hoverSpan = span;
		hoverSpan->SetPalette(ta.palettes[PALETTE_HOVER]);
	}
	return true;
}
	
bool TextArea::SpanSelector::OnMouseUp(const MouseEvent& me, unsigned short /*Mod*/)
{
	Point p = ConvertPointFromScreen(me.Pos());
	TextContainer* span = TextAtPoint(p);
	
	if (span) {
		std::list<View*>::reverse_iterator it = subViews.rbegin();
		unsigned int idx = 0;
		while (*it++ != span) { ++idx; };
		
		MakeSelection(idx);
	}
	return true;
}
	
void TextArea::SpanSelector::OnMouseLeave(const MouseEvent& me, const DragOp* op)
{
	ClearHover();
	TextContainer::OnMouseLeave(me, op);
}

TextArea::TextArea(const Region& frame, Font* text)
	: Control(frame), scrollview(Region(Point(), Dimensions())), ftext(text), palettes()
{
	palettes[PALETTE_NORMAL] = text->GetPalette();
	finit = ftext;
	Init();
}

TextArea::TextArea(const Region& frame, Font* text, Font* caps,
				   Color textcolor, Color initcolor, Color lowtextcolor)
	: Control(frame), scrollview(Region(Point(), Dimensions())), ftext(text), palettes()
{
	palettes[PALETTE_NORMAL] = new Palette( textcolor, lowtextcolor );

	// quick font optimization (prevents creating unnecessary cap spans)
	finit = (caps != ftext) ? caps : ftext;

	// in case a bad or missing font was specified, use an obvious fallback
	if (!finit) {
		Log(ERROR, "TextArea", "Tried to use missing font, resorting to a fallback!");
		finit = core->GetTextFont();
		ftext = finit;
	}

	if (finit->Baseline < ftext->LineHeight) {
		// FIXME: initcolor is only used for *some* initial fonts
		// this is a hack to workaround the INITIALS font getting its palette set
		// do we have another (more sane) way to tell if a font needs this palette? (something in the BAM?)
		SetPalette(&initcolor, PALETTE_INITIALS);
	} else {
		palettes[PALETTE_INITIALS] = finit->GetPalette();
	}

	parser.ResetAttributes(text, palettes[PALETTE_NORMAL], finit, palettes[PALETTE_INITIALS]);
	Init();
}

void TextArea::Init()
{
	ControlType = IE_GUI_TEXTAREA;
	strncpy(VarName, "Selected", sizeof(VarName));

	selectOptions = NULL;
	textContainer = NULL;
	historyTimer = NULL;
	
	AddSubviewInFrontOfView(&scrollview);

	// initialize the Text containers
	ClearSelectOptions();
	ClearText();
	SetAnimPicture(NULL);

	scrollview.SetScrollIncrement(LineHeight());
	scrollview.SetAutoResizeFlags(ResizeAll, OP_SET);
	scrollview.SetFlags(View::IgnoreEvents, (Flags()&View::IgnoreEvents) ? OP_OR : OP_NAND);
}

void TextArea::DrawSelf(Region drawFrame, const Region& /*clip*/)
{
	if (AnimPicture) {
		// speaker portrait
		core->GetVideoDriver()->BlitSprite(AnimPicture.get(), drawFrame.x, drawFrame.y);
	}
}

void TextArea::SetAnimPicture(Sprite2D* pic)
{
	Control::SetAnimPicture(pic);

	assert(textContainer);
	Region tf = textContainer->Frame();
	if (AnimPicture) {
		// shrink and shift the container to accommodate the image
		tf.x = AnimPicture->Width;
		tf.w = frame.w - frame.x;
	} else {
		tf.x = 0;
		tf.w = frame.w;
	}
	textContainer->SetFrame(tf);
}

ieDword TextArea::LineCount() const
{
	int rowHeight = LineHeight();
	if (rowHeight > 0)
		return (ContentHeight() + rowHeight - 1) / rowHeight; // round up
	else
		return 0;
}

void TextArea::UpdateScrollview()
{
	const Region textFrame = (textContainer) ? textContainer->Frame() : Region();

	if (selectOptions) {
		Point p(textFrame.x, textFrame.h);
		selectOptions->SetFrameOrigin(p);
	}

	if (Flags()&IE_GUI_TEXTAREA_AUTOSCROLL
		&& dialogBeginNode) {
		assert(textContainer && selectOptions);

		Region nodeBounds = textContainer->BoundingBoxForContent(dialogBeginNode);

		int optH = OptionsHeight();
		int blankH = frame.h - LineHeight() - nodeBounds.h - optH;
		if (blankH > 0) {
			optH += blankH;
			selectOptions->SetFrameSize(Size(textFrame.w, optH));
		}

		// now scroll dialogBeginNode to the top less a blank line
		int y = nodeBounds.y - LineHeight();
		// FIXME: must update before the scroll, but this should be automaticly done as a reaction to changing sizes/origins of subviews
		scrollview.Update();
		scrollview.ScrollTo(Point(0, -y), 0);
	} else {
		scrollview.Update();
	}
}

void TextArea::FlagsChanged(unsigned int oldflags)
{
	if (Flags()&View::IgnoreEvents) {
		scrollview.SetFlags(View::IgnoreEvents, OP_OR);
	} else if (oldflags&View::IgnoreEvents) {
		scrollview.SetFlags(View::IgnoreEvents, OP_NAND);
	}
}

/** Sets the Actual Text */
void TextArea::SetText(const String& text)
{
	ClearText();
	AppendText(text);
}

void TextArea::SetPalette(const Color* color, PALETTE_TYPE idx)
{
	assert(idx < PALETTE_TYPE_COUNT);
	if (color) {
		palettes[idx] = new Palette( *color, ColorBlack );
		palettes[idx]->release();
	} else if (idx > PALETTE_NORMAL) {
		// default to normal
		palettes[idx] = palettes[PALETTE_NORMAL];
	}
}

void TextArea::TrimHistory(size_t lines)
{
	if (dialogBeginNode) {
		// we don't trim history in dialog
		// this allows us to always reference the entire dialog no matter how long it is
		// we would also have to reapply the selection options origin since it will often be changed by trimming
		// e.g. selectOptions->SetFrameOrigin(Point(textFrame.x, textFrame.h));
		return;
	}

	int height = int(LineHeight() * lines);
	Region exclusion(Point(), Size(frame.w, height));
	scrollview.ScrollDelta(Point(0, exclusion.h));
	textContainer->DeleteContentsInRect(exclusion);
	scrollview.Update();

	if (historyTimer) {
		historyTimer->Invalidate();
		historyTimer = NULL;
	}
}

void TextArea::AppendText(const String& text)
{
	if ((flags&IE_GUI_TEXTAREA_HISTORY)) {
		if (historyTimer) {
			historyTimer->Invalidate();
			historyTimer = NULL;
		}

		int heightLimit = (ftext->LineHeight * 100); // 100 lines of content
		int currHeight = ContentHeight();
		if (currHeight > heightLimit) {
			size_t lines = (currHeight - heightLimit) / LineHeight();

			class AnimationEndEventHandler : public Callback<void, void> {
				TextArea* ta;
				const size_t lines;

			public:
				AnimationEndEventHandler(TextArea* ta, size_t lines)
				: ta(ta), lines(lines) {}

				void operator()() const {
					ta->TrimHistory(lines);
				}
			};

			EventHandler h = new AnimationEndEventHandler(this, lines);
			assert(historyTimer == NULL);
			historyTimer = &core->SetTimer(h, 500);
		}
	}

	size_t tagPos = text.find_first_of('[');
	if (tagPos != String::npos) {
		parser.ParseMarkupStringIntoContainer(text, *textContainer);
	} else if (text.length()) {
		if (finit != ftext) {
			// append cap spans
			size_t textpos = text.find_first_not_of(WHITESPACE_STRING);
			if (textpos != String::npos) {
				// first append the white space as its own span
				textContainer->AppendText(text.substr(0, textpos));

				// we must create and append this span here (instead of using AppendText),
				// because the original data files for the DC font specifies a line height of 13
				// that would cause overlap when the lines wrap beneath the DC if we didnt specify the correct size
				Size s = finit->GetGlyph(text[textpos]).size;
				if (s.h > ftext->LineHeight) {
					// pad this only if it is "real" (it is higher than the other text).
					// some text areas have a "cap" font assigned in the CHU that differs from ftext, but isnt meant to be a cap
					// see BG2 chargen
					s.w += 3;
				}
				TextSpan* dc = new TextSpan(text.substr(textpos, 1), finit, palettes[PALETTE_INITIALS], &s);
				textContainer->AppendContent(dc);
				textpos++;
				// FIXME: assuming we have more text!
				// FIXME: as this is currently implemented, the cap is *not* considered part of the word,
				// there is potential wrapping errors (BG2 char gen).
				// we could solve this by wrapping the cap and the letters remaining letters of the word into their own TextContainer
			} else {
				textpos = 0;
			}
			textContainer->AppendText(text.substr(textpos));
		} else {
			textContainer->AppendText(text);
		}
	}

	UpdateScrollview();

	if (flags&IE_GUI_TEXTAREA_AUTOSCROLL && !selectOptions)
	{
		// scroll to the bottom
		int bottom = ContentHeight() - frame.h;
		if (bottom > 0)
			ScrollToY(-bottom, 500);
	}
	MarkDirty();
}
/*
int TextArea::InsertText(const char* text, int pos)
{
	// TODO: actually implement this
	AppendText(text);
	return pos;
}
*/
/** Key Press Event */
bool TextArea::OnKeyPress(const KeyboardEvent& Key, unsigned short /*Mod*/)
{
	if (flags & IE_GUI_TEXTAREA_EDITABLE) {
		if (Key.character) {
			MarkDirty();
			// TODO: implement this! currently does nothing
			size_t CurPos = 0, len = 0;
			switch (Key.keycode) {
				case GEM_HOME:
					CurPos = 0;
					break;
				case GEM_UP:
					break;
				case GEM_DOWN:
					break;
				case GEM_END:
					break;
				case GEM_LEFT:
					if (CurPos > 0) {
						CurPos--;
					} else {

					}
					break;
				case GEM_RIGHT:
					if (CurPos < len) {
						CurPos++;
					} else {

					}
					break;
				case GEM_DELETE:
					if (CurPos>=len) {
						break;
					}
					break;
				case GEM_BACKSP:
					if (CurPos != 0) {
						if (len<1) {
							break;
						}
						CurPos--;
					} else {

					}
					break;
				case GEM_RETURN:
					//add an empty line after CurLine
					// TODO: implement this
					//copy the text after the cursor into the new line

					//truncate the current line
					
					//move cursor to next line beginning
					CurPos=0;
					break;
			}

			PerformAction(Action::Change);
		}
		return true;
	}

	return false;
}

ieWord TextArea::LineHeight() const
{
	return ftext->LineHeight;
}

/** Will scroll y pixels over duration */
void TextArea::ScrollToY(int y, ieDword duration)
{
	scrollview.ScrollTo(Point(0, y), duration);
}

/** Mousewheel scroll */
/** This method is key to touchscreen scrolling */
bool TextArea::OnMouseWheelScroll(const Point& delta)
{
	// the only time we should get this event is when AnimPicture is set
	// otherwise the scrollview would have recieved this
	assert(AnimPicture);
	scrollview.MouseWheelScroll(delta);
	return true;
}

void TextArea::UpdateState(unsigned int optIdx)
{
	if (!selectOptions) {
		// no selectable options present
		// set state to safe and return
		ClearSelectOptions();
		return;
	}

	if (!VarName[0] || optIdx >= selectOptions->NumOpts()) {
		return;
	}

	// always run the TextAreaOnSelect handler even if the value hasnt changed
	// the *context* of the value can change (dialog) and the handler will want to know 
	SetValue( values[optIdx] );

	// this can be called from elsewhere (GUIScript), so we need to make sure we update the selected span
	selectOptions->MakeSelection(optIdx);

	PerformAction(Action::Select);
}
	
int TextArea::TextHeight() const
{
	return (textContainer) ? textContainer->Dimensions().h : 0;
}
int TextArea::OptionsHeight() const
{
	return (selectOptions) ? selectOptions->Dimensions().h : 0;
}

int TextArea::ContentHeight() const
{
	return TextHeight() + OptionsHeight();
}

String TextArea::QueryText() const
{
	if (selectOptions) {
		return selectOptions->Selection()->Text();
	}
	if (textContainer) {
		return textContainer->Text();
	}
	return String();
}

void TextArea::ClearSelectOptions()
{
	values.clear();
	delete scrollview.RemoveSubview(selectOptions);
	dialogBeginNode = NULL;
	selectOptions = NULL;

	UpdateScrollview();
}

void TextArea::SetSelectOptions(const std::vector<SelectOption>& opts, bool numbered,
								const Color* color, const Color* hiColor, const Color* selColor)
{
	SetPalette(color, PALETTE_OPTIONS);
	SetPalette(hiColor, PALETTE_HOVER);
	SetPalette(selColor, PALETTE_SELECTED);

	ClearSelectOptions(); // deletes previous options

	ContentContainer::ContentList::const_reverse_iterator it = textContainer->Contents().rbegin();
	if (it != textContainer->Contents().rend()) {
		dialogBeginNode = *it; // need to get the last node *before* we append anything
	}

	values.reserve(opts.size());
	std::vector<const String*> strings(opts.size());
	for (size_t i = 0; i < opts.size(); i++) {
		values[i] = opts[i].first;
		strings[i] = &(opts[i].second);
	}

	ContentContainer::Margin m;
	size_t selectIdx = -1;
	if (dialogBeginNode) {
		m = ContentContainer::Margin(LineHeight(), 40, 0);
	} else {
		m = ContentContainer::Margin(0, 3);
		selectIdx = GetValue();
	}

	selectOptions = new SpanSelector(*this, strings, numbered, m);
	scrollview.AddSubviewInFrontOfView(selectOptions);
	selectOptions->MakeSelection(selectIdx);

	UpdateScrollview();
}

void TextArea::ClearText()
{
	delete scrollview.RemoveSubview(textContainer);
	Size textSize = scrollview.ContentRegion().Dimensions();
	textSize.h = 0; // auto grow

	parser.Reset(); // reset in case any tags were left open from before
	textContainer = new TextContainer(Region(Point(), textSize), ftext, palettes[PALETTE_NORMAL]);
	textContainer->SetMargin(0, 3);
	scrollview.AddSubviewInFrontOfView(textContainer);

	UpdateScrollview();
}

void TextArea::SetFocus()
{
	Control::SetFocus();
	if (IsFocused() && flags & IE_GUI_TEXTAREA_EDITABLE) {
		core->GetVideoDriver()->ShowSoftKeyboard();
	}
}

}
