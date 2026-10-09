# Milestone 1: Overlay ergonomics

**Theme:** make the in-app overlay (the toolbar the XAML tap draws over a debugged app) quicker to
drive, so picking and measuring elements doesn't mean reaching for the mouse every time.

**Status:** Not started

| # | Story | Status |
| --- | --- | --- |
| 1 | [Hotkeys for the overlay toolbar](#1-hotkeys-for-the-overlay-toolbar) | Not started |

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
