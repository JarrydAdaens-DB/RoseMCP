#pragma once

// The overlay's keyboard shortcuts: which chord does what, and how a tooltip spells it.
//
// Framework-free, like tap_measure.h. A key arrives as a virtual-key code and three modifiers on both
// frameworks, so what a chord *means* is written once, above the aliases, and each provider only has
// to say how its framework delivers a key. The providers include it themselves rather than through
// tap_overlay.h, because their key seam is defined before the overlay is, and it speaks in
// RoseKeyPress.
//
// One table, read by both the dispatch and the tooltips, so the key a tooltip names is the key that
// works. Visual Studio's in-app toolbar binds no keys at all, so there is nothing to match; the
// chords are three-key ones, because a two-key shortcut the app itself uses cannot collide with them,
// and the letters are ones a developer already knows from elsewhere where there is such a letter --
// Ctrl+Shift+C is "inspect element" in every browser's dev tools.

#include <cstdint>
#include <string>

// One key going down, as the provider's seam saw it.
struct RoseKeyPress
{
	uint32_t Key = 0;
	bool Control = false;
	bool Shift = false;
	bool Alt = false;

	// Auto-repeat from a key being held. A toggle that acted on every repeat would flicker for as long
	// as the key was down, so most actions ignore it.
	bool Repeat = false;
};

enum class RoseHotkey
{
	None,
	Idle,
	Select,
	Rulers,
	JustMyXaml,
	Deselect,
	Zoom,
	ZoomIn,
	ZoomOut,
	Hide,
};

struct RoseChord
{
	RoseHotkey Action;
	uint32_t Key;
	bool Control;
	bool Shift;

	// How a tooltip spells it, or null for a second binding of an action that already has one -- the
	// numeric keypad's plus and minus, which a tooltip has no reason to list twice.
	const wchar_t* Text;
};

// Virtual-key codes, which are the same numbers on both frameworks: VirtualKey is the Win32 VK_ set.
// The letters are their own ASCII capitals.
static constexpr uint32_t RoseKeyEscape = 0x1B;
static constexpr uint32_t RoseKeyPlus = 0xBB;     // VK_OEM_PLUS, the =/+ key on the main block
static constexpr uint32_t RoseKeyMinus = 0xBD;    // VK_OEM_MINUS
static constexpr uint32_t RoseKeyPadPlus = 0x6B;  // VK_ADD
static constexpr uint32_t RoseKeyPadMinus = 0x6D; // VK_SUBTRACT

static constexpr RoseChord RoseChords[] = {
	{ RoseHotkey::Select, 'C', true, true, L"Ctrl+Shift+C" },
	{ RoseHotkey::Rulers, 'M', true, true, L"Ctrl+Shift+M" },
	{ RoseHotkey::Idle, RoseKeyEscape, false, false, L"Esc" },
	{ RoseHotkey::JustMyXaml, 'J', true, true, L"Ctrl+Shift+J" },
	{ RoseHotkey::Deselect, 'D', true, true, L"Ctrl+Shift+D" },

	// G for glass. Z is the obvious letter and the wrong one: Ctrl+Shift+Z is redo in a great many
	// apps, text boxes included, and this would take it from them.
	{ RoseHotkey::Zoom, 'G', true, true, L"Ctrl+Shift+G" },
	{ RoseHotkey::ZoomIn, RoseKeyPlus, true, true, L"Ctrl+Shift+Plus" },
	{ RoseHotkey::ZoomIn, RoseKeyPadPlus, true, true, nullptr },
	{ RoseHotkey::ZoomOut, RoseKeyMinus, true, true, L"Ctrl+Shift+Minus" },
	{ RoseHotkey::ZoomOut, RoseKeyPadMinus, true, true, nullptr },
	{ RoseHotkey::Hide, 'H', true, true, L"Ctrl+Shift+H" },
};

// The action a key press asks for, or None. The modifiers match exactly, so Ctrl+Alt+Shift+C is not
// Ctrl+Shift+C: an app that binds the four-key chord keeps it.
static RoseHotkey RoseHotkeyFor(RoseKeyPress const& press)
{
	for (const auto& chord : RoseChords)
	{
		const bool matches = press.Key == chord.Key
			&& press.Control == chord.Control
			&& press.Shift == chord.Shift
			&& !press.Alt;
		if (matches) return chord.Action;
	}

	return RoseHotkey::None;
}

static const wchar_t* RoseHotkeyName(RoseHotkey action)
{
	switch (action)
	{
		case RoseHotkey::Idle: return L"idle";
		case RoseHotkey::Select: return L"select";
		case RoseHotkey::Rulers: return L"rulers";
		case RoseHotkey::JustMyXaml: return L"just-my-xaml";
		case RoseHotkey::Deselect: return L"deselect";
		case RoseHotkey::Zoom: return L"zoom";
		case RoseHotkey::ZoomIn: return L"zoom-in";
		case RoseHotkey::ZoomOut: return L"zoom-out";
		case RoseHotkey::Hide: return L"hide";
		default: return L"none";
	}
}

// A chip's tooltip with its shortcut in it, where Windows puts one: straight after the name. The
// overlay's longer tips are "Name -- what it does", and a shortcut appended after a sentence of
// explanation is a shortcut nobody reads, so it goes before the dash when there is one.
static std::wstring RoseWithShortcut(const wchar_t* tip, RoseHotkey action)
{
	std::wstring text = tip ? tip : L"";

	const wchar_t* shortcut = nullptr;
	for (const auto& chord : RoseChords)
	{
		if (chord.Action == action && chord.Text)
		{
			shortcut = chord.Text;
			break;
		}
	}

	if (!shortcut) return text;

	const std::wstring label = std::wstring(L" (") + shortcut + L")";
	const auto dash = text.find(L" -- ");
	if (dash == std::wstring::npos) return text + label;

	return text.substr(0, dash) + label + text.substr(dash);
}
