# Plan: Hotkeys for the overlay toolbar

- **Story:** [Milestone 1, story 1](../../../milestones/milestone-1-overlay-ergonomics.md#1-hotkeys-for-the-overlay-toolbar)
- **Related:** [Story 2's plan](../2-delayed-select/plan.md). Its timer chip and Escape-to-cancel need
  this story's key path.
- **Status:** In progress
- **Last updated:** 12 October 2026
- **Size:** medium. The bindings are trivial. The risk is in the key seam: it is per-framework,
  untested by any suite, and runs inside someone else's app.

## Objective

Every toolbar action can be done from the keyboard while the pointer stays over the target app. The
shortcuts match Visual Studio's where VS has them. Every chip's tooltip names its shortcut. The
hotkeys behave the same in a UWP target and a WinUI 3 target.

## Scope

**In:** Idle, Select element, Rulers, Just my XAML, Deselect, Zoom (plus Zoom in and Zoom out while
zoom is on), and Hide/Show. Escape back to Idle. Tooltip text. A key-observer seam in both providers.
The invariant and decision records that go with it.

**Out:** the Inspector's XAML pane hotkeys (a separate story, per the milestone). User-configurable
bindings. VS actions Rose has no equivalent for (Live Visual Tree, Track Focused Element,
accessibility, Hot Reload status). Story 2's timer chip, unless Story 2 lands first (see step 6).

## Current understanding

- **Toolbar:** `BuildBar()` in `src/RoseMcp.Xaml.Tap/tap_overlay.h` creates `m_idleButton`,
  `m_selectButton`, `m_rulersButton`, `m_myXamlButton` and `m_deselectButton`, plus the Hide chip,
  which is a local and not a member. The Zoom chip comes from `m_zoomTool.Button()` in
  `tap_tool_zoom.h`, and its in/out chips come from `BuildZoomRow()`.
- **Actions already exist as methods:** `GoIdle()`, `BeginSelect(false)`, `BeginRulers()`,
  `ToggleMyXaml()`, `Deselect()`, `Collapse(bool)`, and the zoom tool's own handlers. A hotkey only
  dispatches to these. No new behaviour is needed behind any key.
- **Tooltips:** `Chip(content, tip, action)` in `tap_widgets.h` sets the tip with
  `ToolTipService::SetToolTip` from a plain string. Appending the shortcut to the string is enough.
- **No key handling exists in the tap.** The overlay's header says click-through is structural, with
  "no input hooks, no window subclassing, and nothing that could collide with a modifier chord the
  app itself wants to use". This story adds a key path, so it has to get that comment right again
  and keep the "no subclassing" part.
- **The precedent for a framework-specific input seam is `RoseTapWatchPointer`.** Each provider
  defines it before including `tap_overlay.h`:
  - UWP: `CoreWindow` pointer events.
  - WinUI 3: `InputPointerSource::GetForIsland`, with the island found through
    `ContentIsland.FindAllForCurrentThread`.

  `overlay.md` records how that seam was measured: whether it consumes input, and whether it sees
  input the app has already marked handled. A key seam has to be measured the same way.
- **Candidate key sources:**
  - UWP: `CoreDispatcher.AcceleratorKeyActivated`. It sees every key before XAML routes it, and it
    can set `Handled` to swallow one.
  - WinUI 3: the candidates are `InputKeyboardSource.GetForIsland(island)`, the sibling of the
    pointer source, and `UIElement.AddHandler(KeyDownEvent, ..., handledEventsToo: true)` on the
    root content. Neither has been measured. `KeyboardAccelerator`s on the overlay's own elements
    are a third option. Whether they fire from the diagnostics UI layer is unknown.
- **Threading:** the key callbacks arrive on the UI thread, the same as the pointer watcher. The
  overlay is a leaked singleton that is only ever touched on that thread, so dispatching to it inline
  is safe.
- **Tests:**
  - No suite can press a key in a probe app. Nothing in `tests/` injects input.
  - `LiveAppUwpOverlayTests` drives the modes over the pipe, so it is the regression guard for the
    actions themselves, not for the keys.
  - `TapTierTests` fails a new header that has no tier.
  - The tap is built only by the two `build.ps1` scripts, never by `dotnet build`.

## Questions / unknowns

> **Answered 12 October 2026:** "Just mimic Visual Studio, and implement something. If it sucks,
> we'll follow up and change it." So Q2 and Q3 take the recommendations below, and the draft table
> ships as it is. Criticism comes after use.

- **Q1. What does VS bind?** Status: ANSWERED. Nothing in-app.
  - The markup for VS's in-app toolbar is embedded in its taps
    (`XamlDiagnostics/x64/Microsoft.VisualStudio.DesignTools.UwpTap.dll` and
    `x64/WinUI3/Microsoft.VisualStudio.DesignTools.WinUITap.dll`, VS 18 Professional).
  - Its buttons (`InAppSelection`, `LayoutAdorners`, `TrackFocusedElement`, `ShowLiveVisualTree` and
    the others) declare no `KeyboardAccelerator` and no shortcut in their tooltip.
  - The only shortcut text in the package is Live Preview's "Ctrl + Plus/Minus to zoom", which the
    zoom row's draft keys already resemble.
  - So the draft table in step 2 stands.
- **Q2. When are the hotkeys live?** Status: ANSWERED, as recommended.
  - Recommendation:
    - The modifier chords are live whenever the overlay is installed, collapsed or not. Hide and
      Show must work from the keyboard, or a collapsed bar can only be opened with the mouse.
    - Escape is live only while something is armed: select mode, rulers mode, an open menu, or Story
      2's countdown. In idle, Escape belongs to the app.
- **Q3. Does a hotkey reach the app as well?** Status: ANSWERED, as recommended.
  - Recommendation: swallow a key only when it did something, and pass it through otherwise. An
    unused chord costs the app nothing.
  - Escape is the case that matters. Swallowing it while select mode is armed stops it closing an
    app dialog behind the capture layer.
  - Whether swallowing is possible at all depends on the WinUI seam (Q4).
- **Q4. Which WinUI 3 key source works?** Status: OPEN. Step 3 answers it by measurement.
  - It has to see a key when focus is in the app's own content, including when the app marked the
    key handled.
  - Ideally it also sees a key when focus is in one of the app's popups, which matters for Story 2.
  - If no source can swallow a key, Q3's answer is "never swallow" on WinUI. Record that in the
    invariant.
- **Q5. Does a key reach us at all when focus is outside the main island?** Status: OPEN.
  - Example: a WinUI windowed popup that has focus. Record the answer. It is not a blocker for this
    story.

## Execution steps

1. **Settle the bindings (Q1).**
   - Check VS by hand and write down what it binds.
   - Where VS has an equivalent, use its key. Otherwise keep the draft row from step 2.
2. **Fix the binding table.** The draft is below. Every row is a three-key chord, so a two-key
   shortcut in the app cannot collide with it, and each one reuses a letter developers already know
   from other tools.

   | Action | Draft key | Reason |
   | --- | --- | --- |
   | Select element | Ctrl+Shift+C | Browser dev tools' "inspect element" |
   | Rulers | Ctrl+Shift+M | "measure" |
   | Idle | Escape (when armed) | Universal "stop" |
   | Just my XAML | Ctrl+Shift+J | "just" |
   | Deselect | Ctrl+Shift+D | |
   | Zoom on/off | Ctrl+Shift+Z | |
   | Zoom in / out (zoom on) | Ctrl+Shift+= / Ctrl+Shift+- | |
   | Hide / Show | Ctrl+Shift+H | Toggles, so it also brings the bar back |

   - Write the table into one `static constexpr` array in tier 3 (`tap_overlay.h`, or `tap_widgets.h`
     beside the icon codepoints). One array lets the dispatch and the tooltips read the same row, so
     they cannot disagree.
3. **Add the key seam (Q4, Q5), measured the way the pointer seam was.**
   - Declare `RoseTapWatchKeys(anchor, onKey)` beside `RoseTapWatchPointer`.
     - The callback gets a virtual-key code and the modifier state.
     - It returns whether it acted, which gives the swallow decision for Q3.
   - Keep the signature free of projected types (`uint32_t` key plus flags) so it obeys
     `tap-tiers.md`, even though the bodies live in the providers.
   - UWP body (`RoseMcp.Xaml.Uwp.Tap.cpp`): `AcceleratorKeyActivated` on the current
     `CoreDispatcher`. Read modifiers with `CoreWindow::GetKeyState`.
   - WinUI body (`RoseMcp.Xaml.WinUi.Tap.cpp`):
     - Try `InputKeyboardSource::GetForIsland` on the same island the pointer seam finds.
     - Measure it against the app's own handled keys, using a probe-app element that marks
       `KeyDown` handled.
     - If it fails either property, fall back to `AddHandler(KeyDownEvent, ..., true)` on the root
       content, and write down what that costs.
   - A provider that cannot install the seam logs which step gave up and carries on. Losing the
     hotkeys must not lose the toolbar.
4. **Dispatch in the overlay.**
   - In `Build()`, install the seam next to `WatchPointer()`.
   - `OnKey` looks the chord up in the table, applies Q2's liveness rule, and calls the existing
     method.
   - Escape cancels the most specific armed thing first. Today that is just the mode. Story 2 adds
     its menu and countdown in front of it.
   - Wrap the body in a `try`/`catch` like the pointer callbacks. It runs on every keystroke in
     someone else's app and must never throw out.
   - Zoom: the overlay is not the zoom tool, so expose a small `HandleKey` on `RoseZoomTool`. Its
     chips are its own business, the same way `RefreshChrome` is.
5. **Tooltips.**
   - Append the shortcut to each chip's tip, e.g. `Select element (Ctrl+Shift+C)`, taking the key
     text from the same table.
   - The zoom tool's chips do the same in `tap_tool_zoom.h`.
6. **Story 2 timer chip.** Only if Story 2 landed first: give it a row in the table. Story 2's plan
   assumes the reverse order and adds the row itself.
7. **Records.**
   - `docs/invariants/overlay.md`: what the key seam must and must not do, and what was measured for
     each framework (numbers only where a decision rests on them).
   - Rewrite the header comment in `tap_overlay.h` so it no longer says the overlay takes no input
     hooks.
   - A short decision in `docs/decisions/` if Q3 lands on swallowing: why we swallow, and why the
     chords are three-key.
   - Wiki: a note to add the key table to the user page. The wiki lives outside this repo, so it is a
     follow-up line in `inbox.md`.

## Validation

**Automated:**

- `./src/RoseMcp.Xaml.Uwp.Tap/build.ps1 -Platform x64 -Configuration Debug` and the WinUI equivalent
  both build clean.
- `RoseMcp.UnitTests.exe` passes. `TapTierTests` covers a header added or an include moved, and
  `StdoutRuleTests` is unaffected.
- Integration: `--treenode-filter '/*/*/LiveAppUwpOverlayTests/*'`, plus the WinUI overlay tests, are
  still green. They prove the dispatch targets still behave.
- `RoseMcp.IntegrationTests.Windows.exe`, because the tap changed.
- `dotnet format --verify-no-changes`, and `tools/Check-Comments.ps1`.

**Manual** (deploy with `./tools/deploy.ps1`, inject into `uwp-classic`, `uwp-modern` and `winui`):

1. Each chord does what its chip does. The accent moves exactly as it does for a click.
2. Escape in select mode returns to Idle and does not reach the app. Escape in Idle does reach the
   app: open a dialog and close it with Escape.
3. With focus in a `TextBox` in the app, typing `C` (without the chord) types normally, and
   Ctrl+Shift+C still arms select.
4. Collapse the bar with the chord, then bring it back with the chord.
5. Hovering each chip shows its shortcut.
6. Run the probe element that marks `KeyDown` handled, and confirm the chords still fire (step 3's
   measurement).

**Done when:**

- Every chip in the milestone's table has a working shortcut in both frameworks.
- Each shortcut matches VS where VS has one, and its tooltip shows it.
- A target that types normally is not disturbed by the overlay.

## Risks

- **A seam that silently doesn't fire on one framework.** This is the pointer seam's history
  repeating: it was broken on WinUI and nobody noticed.
  - Mitigation: measure as in step 3, have every give-up path log, and run the manual check on all
    three probe apps.
- **Stealing a key the app needs.**
  - Mitigation: three-key chords. Swallow only on action. Escape only while armed.
- **A key callback throwing inside the target.**
  - Mitigation: `try`/`catch` around the dispatch, the same as `WatchPointer`.
- **A future regression that no suite catches.**
  - Accepted: there is no input injection in the tests. The invariant entry is the guard.

## Evidence / references

- `docs/invariants/overlay.md` covers the pointer seam's two required properties and how they were
  measured on WinUI.
- `docs/invariants/tap-tiers.md` says seam signatures carry no projected types.
- `docs/invariants/xaml-tap-lifecycle.md` covers threading. Everything here runs on the UI thread.
- VS bindings (Q1): none in the in-app toolbar. Read from the markup embedded in VS 18's UWP and
  WinUI taps on 12 October 2026.
