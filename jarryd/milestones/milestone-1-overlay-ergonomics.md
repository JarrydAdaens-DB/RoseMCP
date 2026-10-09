# Milestone 1: Overlay ergonomics

**Theme:** make the in-app overlay (the toolbar the XAML tap draws over a debugged app) quicker to
drive, so picking and measuring elements doesn't mean reaching for the mouse every time.

**Status:** Not started

| # | Story | Status |
| --- | --- | --- |
| 1 | [Hotkeys for the overlay toolbar](#1-hotkeys-for-the-overlay-toolbar) | Not started |
| 2 | [Delayed select, like the Snipping Tool's timer](#2-delayed-select-like-the-snipping-tools-timer) | Not started |

---

### 1. Hotkeys for the overlay toolbar

**Status:** Not started

**Why:** every overlay action is a click on a small chip floating over someone else's app. Switching
between Select and Rulers while inspecting a layout is constant, and doing it by mouse breaks the
flow.

**What done looks like:** each toolbar action has a keyboard shortcut. Where Visual Studio's in-app
XAML toolbar has an equivalent action, the shortcut matches VS's. Each chip's tooltip shows its
shortcut.

**Notes:**

- The toolbar is built in `BuildBar()` in `src/RoseMcp.Xaml.Tap/tap_overlay.h`. Its chips, left to
  right:

  | Chip | What it does | Closest VS in-app toolbar action |
  | --- | --- | --- |
  | Idle | pointer mode: do nothing | none |
  | Select element | pointer mode: pick an element | Select Element |
  | Rulers | pointer mode: margins and padding, measure to the pointer | Display Layout Adorners |
  | Just my XAML | toggle: pick from the app's own markup, not template parts | none in the in-app bar (VS has "Just My XAML" on the Live Visual Tree window) |
  | Deselect | clear the pick | none |
  | Zoom | the zoom tool, with its own Zoom in / Zoom out chips | none |
  | Hide | collapse the bar to its grip | the bar's collapse arrow |

  VS's bar also has Go to Live Visual Tree, Track Focused Element, an accessibility button and the
  Hot Reload status. Rose has no equivalent of those, so they're out of scope.

- **Open question: what VS actually binds.** I couldn't find documented hotkeys for VS's in-app
  toolbar; Microsoft's pages only describe the buttons. Check in VS under Tools > Options >
  Environment > Keyboard, filtering on "XAML" and "Live Visual Tree", and record what's there. If
  VS has no bindings, "match VS" collapses to "pick sensible ones". Browser dev tools'
  Ctrl+Shift+C for "inspect element" is the obvious convention for Select.

- **Risk: the keys arrive in the debugged app, not in Rose.** The tap runs inside the target
  process, so a shortcut has to be caught without stealing keys the app uses itself. Decide when the
  hotkeys are live: only while the bar is visible? only in a non-Idle mode? Escape back to Idle is
  probably the one that should always work.

- Both the UWP and WinUI bindings render the overlay, so the change has to work in both.

- Read `docs/invariants/overlay.md` and `docs/invariants/xaml-tap-lifecycle.md` before touching
  the tap.

- Possible follow-up: the Inspector's XAML pane drives the same modes (`select`, `rulers`, `idle`).
  Matching hotkeys there would be a separate story.

**Plan:** none yet.

---

### 2. Delayed select, like the Snipping Tool's timer

**Status:** Not started

**Why:** the UI I most need to inspect is the UI that goes away when I reach for the overlay:
flyouts, tooltips, hover states, menus, anything that dismisses on a pointer exit or a click
elsewhere. With Select element as it is, moving to the toolbar and clicking closes the thing I
wanted to pick.

**What done looks like:** a new chip beside Select element opens a menu modelled on the Windows
Snipping Tool's delay picker:

- No delay
- 3-second delay
- 5-second delay
- 10-second delay

The chosen delay sticks, and the chip shows which one is active, the way the Snipping Tool marks
the current choice. With a delay set, clicking Select element starts a countdown instead of
selecting straight away. I then go and open the menu or hover the control, and when the countdown
ends, select mode comes on over whatever is showing. No delay behaves exactly as Select element
does now.

**Notes:**

- I asked for No / 3 / 10 seconds. The Snipping Tool also offers 5, so the list above matches it.
  Drop 5 if it turns out to be noise.
- **The big open question: does the click that picks dismiss the UI it's picking?** Select works
  by clicking an element. A light-dismiss flyout closes on a click outside it, and a hover state
  ends when the pointer moves. If the pick still needs a click, the delay alone may not be enough.
  Options to weigh:
  - When the countdown ends, pick whatever is under the pointer straight away, with no click.
  - Make select mode swallow the press before the app sees it.
  - Freeze the frame, which is what the Snipping Tool actually does.

  Try it against a real flyout before choosing.
- **Popups are a separate layer.** Flyouts, menus and tooltips render in the window's popup root,
  not under the page's visual tree. Check that the pick hit-tests open popups, or a delayed select
  will land on the page behind them.
- **Show the countdown.** Without visible feedback, a 10-second wait reads as broken. Something
  small on the toolbar or near the pointer, and a way to cancel it (Idle, or Escape once Story 1
  exists).
- Where it lives: `BuildBar()` and `BeginSelect` in `src/RoseMcp.Xaml.Tap/tap_overlay.h`; the
  chips come from `tap_widgets.h`. The zoom tool (`tap_tool_zoom.h`) already runs a
  `DispatcherTimer`, which is a pattern to copy. The toolbar has no drop-down menu yet, so the
  delay picker is new UI for the overlay, and it has to look like the rest of the bar
  (`tap_widgets.h` explains why every chip is drawn the same way).
- Opening the overlay's own menu is itself a popup in the target app. Choose the delay first,
  then open the target's UI, so the two don't fight over light-dismiss.
- **Works with Story 1.** A hotkey for "select after the current delay" means the countdown can
  start without the pointer ever leaving the target UI. That may be the version that works best.
- Read `docs/invariants/overlay.md` before starting, as for Story 1.

**Plan:** none yet.
