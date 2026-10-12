# Milestone 1: Overlay ergonomics

**Theme:** make the in-app overlay (the toolbar the XAML tap draws over a debugged app) quicker to
drive, so picking and measuring elements doesn't mean reaching for the mouse every time.

**Status:** In progress

| # | Story | Status |
| --- | --- | --- |
| 1 | [Hotkeys for the overlay toolbar](#1-hotkeys-for-the-overlay-toolbar) | In progress |
| 2 | [Delayed select, like the Snipping Tool's timer](#2-delayed-select-like-the-snipping-tools-timer) | In progress (works; popup checks left) |
| 3 | [Hotkeys for the timer](#3-hotkeys-for-the-timer) | Waiting on 1 and 2 |

Stories 1 and 2 are independent: each has its own branch from the same commit, and its own PR.
Anything that needs both lives in Story 3, so neither PR waits on the other.

---

### 1. Hotkeys for the overlay toolbar

**Status:** In progress (built on `ja/overlay-hotkeys`, waiting on a try-out)

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

  Story 2's timer chip gets its shortcut in Story 3, not here.

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

**Plan:** [1-overlay-toolbar-hotkeys/plan.md](../implementation-plans/milestone-1-overlay-ergonomics/1-overlay-toolbar-hotkeys/plan.md)

---

### 2. Delayed select, like the Snipping Tool's timer

**Status:** In progress (zero enters select mode, tried and working on 13 October 2026; popup
checks left before a PR)

**Why:** the UI I most need to inspect is the UI that goes away when I reach for the overlay:
flyouts, tooltips, hover states, menus, anything that dismisses on a pointer exit or a click
elsewhere. With Select element as it is, moving to the toolbar and clicking closes the thing I
wanted to pick.

**Design mockup:** `C:\Users\Drawboard\Pictures\rose\timer-concept.png` (on my machine, not in the
repo). It shows the toolbar with the new timer chip and its menu open.

**What done looks like:**

1. **A timer chip** sits on the toolbar straight after Select element, before Rulers, as in the
   mockup. It's drawn like every other chip (`Chip(...)` in `tap_widgets.h`).
2. **Clicking it opens a drop-down menu** under the chip, modelled on the Windows Snipping Tool's
   delay picker. Each item has a stopwatch icon and a label:

   | Item | Icon |
   | --- | --- |
   | No delay | stopwatch with a slash through it |
   | 3-second delay | stopwatch with a 3 |
   | 5-second delay | stopwatch with a 5 |
   | 10-second delay | stopwatch with a 10 |

   The current choice is marked with the rose accent bar on its left edge, as in the mockup.
   Picking an item closes the menu.
3. **The chip's icon shows the chosen delay:** the slashed stopwatch for No delay, the numbered
   stopwatch otherwise. As with the Snipping Tool, you can read the delay without opening the menu.
   The chip is drawn active (rose) while a delay is set.
4. **The choice sticks** for the life of the overlay. It doesn't reset after each pick.
5. **With a delay set, clicking Select element starts the countdown** instead of entering select
   mode. During the countdown the app behaves normally: no select layer, no capture of the pointer
   or clicks. I use that time to open the flyout, hover the control, and so on.
6. **During the countdown, the timer chip animates.**
   - The number in the stopwatch counts down each second: 10, 9, 8 ... 1.
   - A radial sweep runs over the chip for exactly the length of the delay, like a little clock
     hand. It starts at 12 o'clock and moves **counter-clockwise**, and the remaining wedge shrinks
     to nothing as time runs out. This goes further than the Snipping Tool, which only shows a
     number.
   - The implementer has freedom with the exact look: wedge or ring, with or without a visible
     hand, the colours. The constraints: use the overlay's accent, keep it on or immediately around
     the chip, keep it smooth (not a once-a-second jump), and make it read well at chip size.
7. **When the countdown ends, the overlay goes into Select element mode**, exactly as if I'd
   clicked Select with No delay: the mode switches from Idle to Select in the tap, the Select chip
   lights up, and I hover and click to pick as usual. This is how the Snipping Tool works: the timer
   delays entering snip mode, it doesn't take the snip for you. Nothing is picked automatically.
8. **Cancellable.** Clicking Idle, Select element or the timer chip during the countdown cancels
   it, and the chip goes back to its resting icon. Cancelling leaves nothing running. (Escape is
   Story 3.)
9. **No delay behaves exactly as Select element does now.**

**Notes:**

- **Zero enters select mode; it doesn't pick.** The first build picked whatever was under the
  pointer at zero, without entering select mode, because the story said so. That was my mistake in
  writing the story, not what I wanted (revised 13 October 2026). What I want is the Snipping Tool's
  behaviour: the delay goes before the mode, and the mode then works as it always does.
  - Watch for: arming select mode puts the capture layer over the app. That may end a tooltip or
    a hover state (pointer-exit), and a click may dismiss a light-dismiss flyout. Try it with real
    flyouts and tooltips and write down what happens. Don't design around it before seeing it.
- **Keep the countdown visual on the toolbar, away from the pointer.** Anything drawn under or
  next to the pointer can steal hover from the target UI and dismiss it. That's why the sweep lives
  on the chip and not as a cursor decoration.
- **Popups are a separate layer.** Flyouts, menus and tooltips render in the window's popup root,
  not under the page's visual tree, and I found no popup handling in the tap or LiveApp. Before
  anything else, verify against a real flyout in a test app that a pick hit-tests open popups, in
  both UWP and WinUI. If it doesn't, that's the first piece of work, or a delayed select will land
  on the page behind the flyout.
- **Steve is wary of timers; they've bitten him before.** Build it so there's nothing to go wrong:
  - One one-shot `DispatcherTimer` on the UI thread, like the zoom tool's `m_lensTimer` in
    `tap_tool_zoom.h`. No thread-pool timers, no sleeping, no work posted from another thread.
  - Stop and release it on cancel, on expiry, when the overlay is hidden or torn down, and when the
    tap goes away. `docs/invariants/xaml-tap-lifecycle.md` says when that happens.
  - Drive the sweep from the same clock, or from a storyboard or composition animation of the same
    duration, so the sweep and the switch into select mode can't drift apart.
- **Steve's alternative, considered and not taken for this story:** a conditional breakpoint that
  stops the app while the UI is open, then inspecting it. It doesn't work for picking: while the
  target is stopped its UI thread can't answer any XAML request, and `TargetXaml.WhyUnservable` in
  `src/RoseMcp.LiveApp/Xaml/TargetXaml.cs` refuses them. Breakpoints and tracepoints stay the right
  tool for reading the C# side.
- **Possible follow-up, not this story:** a "Next popup" item in the same menu that fires the pick
  when a popup opens, rather than after a fixed time. The tap already sees every element added
  (`OnVisualTreeChange` in `tap_object.h`), so that could be the trigger. Steve may prefer it.
- **The menu is new UI for the overlay.** The toolbar has no drop-down yet. Build it from the same
  widget language (`tap_widgets.h` explains why everything on the bar is drawn the same way), so it
  matches the mockup and the rest of the bar.
- **The menu is itself a popup in the target app.** The order is: choose the delay (the menu
  closes), click Select element, then open the target's UI. So the overlay's menu and the target's
  flyout are never open together, and never fight over light-dismiss.
- **Icons.** The overlay's glyphs are Segoe MDL2 / Fluent codepoints, named at the top of
  `tap_widgets.h`; its comment explains why a wrong one renders as a hollow box rather than
  failing. Check the font's character map for a stopwatch and a slashed stopwatch. The numbered
  stopwatches probably don't exist as glyphs, so expect to compose them: the stopwatch glyph with a
  small number laid over it. The countdown reuses the same composition.
- Where it lives: `BuildBar()`, `BeginSelect` and `GoIdle` in `src/RoseMcp.Xaml.Tap/tap_overlay.h`;
  chips come from `tap_widgets.h`. Read `docs/invariants/tap-tiers.md` before adding a file, and
  `docs/invariants/overlay.md` before starting.
- **Doesn't depend on Story 1.** Everything here works by pointer. Its hotkeys are Story 3.

**Plan:** [2-delayed-select/plan.md](../implementation-plans/milestone-1-overlay-ergonomics/2-delayed-select/plan.md)

---

### 3. Hotkeys for the timer

**Status:** Waiting on Stories 1 and 2 (both merged upstream, or at least both on one branch)

**Why:** Stories 1 and 2 ship separately, so neither knows about the other. Once both are in, the
timer should be reachable from the keyboard like the rest of the bar. The best version of the timer
may be the one where the countdown starts without the pointer ever leaving the target UI.

**What done looks like:**

1. The timer chip has a shortcut, and its tooltip names it, like every other chip.
2. A "select after the current delay" chord starts the countdown from the keyboard.
3. Escape closes the timer's menu if it's open, and otherwise cancels a running countdown, before it
   falls back to leaving a mode.
4. Story 1's shortcuts and Story 2's pointer behaviour are unchanged.

**Notes:**

- Branch from `develop` as usual, once `main` has moved to a Steve release that has both stories in
  it (see `git-flow.md`).
- The chord table is `src/RoseMcp.Xaml.Tap/tap_keys.h`; dispatch is `OnKey` in `tap_overlay.h`.
- Story 1's plan Q4 and Q5 ask whether a key reaches the tap while focus is in one of the app's
  popups or outside the main island. It
  matters here: the timer exists for flyouts, and a hotkey that dies inside one is half the feature.
- Plan not written yet.
