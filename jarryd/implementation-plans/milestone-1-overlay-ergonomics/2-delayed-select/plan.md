# Plan: Delayed select, like the Snipping Tool's timer

- **Story:** [Milestone 1, story 2](../../../milestones/milestone-1-overlay-ergonomics.md#2-delayed-select-like-the-snipping-tools-timer)
- **Related:** [Story 1's plan](../1-overlay-toolbar-hotkeys/plan.md) (hotkeys). This story does
  not depend on it. Anything that needs both, such as Escape cancelling the countdown, the timer
  chip's shortcut, or a "select after delay" hotkey, is
  [Story 3](../../../milestones/milestone-1-overlay-ergonomics.md#3-hotkeys-for-the-timer).
- **Branch:** `ja/delayed-select`, from `45a35f4`, the same commit `ja/overlay-hotkeys` started
  from. The two stories go upstream as separate PRs, in either order.
- **Mockup:** `C:\Users\Drawboard\Pictures\rose\timer-concept.png`, which is local and not in the
  repo. It shows:
  - the timer chip lit rose between Select element and Rulers;
  - a dark drop-down under it, the same panel colour as the bar, with four rows (No delay, 3-, 5- and
    10-second delay), each with a stopwatch icon;
  - the current row marked by a short rose bar on its left edge.
- **Status:** In progress (built on ja/delayed-select, waiting on a try-out). Phase A decides whether the rest of the
  story is possible as written.
- **Revised 12 October 2026:** this plan first assumed it would build on Story 1's hotkeys. It no
  longer does. The Escape and hotkey steps moved to Story 3.
- **Last updated:** 12 October 2026
- **Size:** large. Phase A is a real unknown about popups. The menu is the overlay's first drop-down,
  and the sweep is new drawing.

## Objective

A delay picked from a new timer chip makes Select element count down on the chip, without touching
the app. When the countdown ends, it picks whatever is under the pointer, the same way a click would,
including when what is under the pointer is an open flyout, menu or tooltip.

## Scope

**In:**

- Popup hit-testing for a pick (if Phase A shows it is missing).
- The timer chip and its icons.
- The drop-down menu.
- The countdown: the digit and the sweep.
- The pick when the countdown ends.
- Cancel paths.
- A flyout, a menu and a tooltip added to the probe apps, so all of this can be checked.
- Records.

**Out:**

- The "Next popup" trigger (a follow-up, per the milestone).
- Delay for Rulers.
- A delay the host can arm over the pipe. `rose_xaml_select_element` keeps arming immediately.
- Remembering the delay across app restarts.

## Current understanding

- **No hotkeys on this branch.** It starts from `45a35f4`, before Story 1's code, so there is no
  key seam, no `tap_keys.h` and no Escape handling. Every way to open, close or cancel is the
  pointer.
  - Both stories edit `BuildBar()` and the chip tooltips in `tap_overlay.h`, and both append to
    `docs/invariants/overlay.md`. Whichever PR merges second resolves a small conflict there.
  - Neither story should change code only to make that merge easier.

- **Chip row:** `BuildBar()` in `tap_overlay.h` appends `m_idleButton`, `m_selectButton`,
  `m_rulersButton`, then Just my XAML, Deselect, Zoom and Hide. The timer chip goes between Select
  and Rulers. `Chrome()` paints the accent per mode and has to paint the timer chip too: rose while
  a delay is set.
- **The pick path is already separate from the click.**
  - `OnPick(e)` reads the point and calls `Beneath(point, rect)`, then
    `m_pick.Take(element, Record(element, point), rect)`.
  - `Beneath` and `Record` both take a plain `Point`, and both apply the hit-test and Just my XAML
    rules from the [pick decision](../../../../docs/decisions/a-pick-honours-hit-testing-and-prefers-the-apps-markup.md).
  - So a timed pick is: lift the body of `OnPick` into `PickAt(Point)` and call it from the expiry,
    with no capture layer involved.
- **Where the pointer is.** `m_pointer` is kept up to date by the passive watcher (`Proximity`), in
  the layer's coordinates.
  - On leaving the window it becomes `(-1, -1)`.
  - On WinUI 3 the watcher is the main island's `InputPointerSource`. A *windowed* popup (a flyout
    with `ShouldConstrainToRootBounds = false`) is a separate island, so pointer moves over it may
    never arrive, and the exit event may fire instead. That would make `m_pointer` wrong at exactly
    the moment this story needs it. This is Phase A's second question.
- **Popups are missed today.** `Beneath` and `Record` hit-test under `Content()`, which is
  `XamlRoot.Content()`. Popups (flyouts, menus, tooltips) live in the popup root, not under
  `Content()`, and nothing in the tap mentions popups. So today a pick over an open flyout almost
  certainly lands on the page behind it. `VisualTreeHelper::GetOpenPopupsForXamlRoot` exists in both
  frameworks and is the likely fix.
- **The menu cannot be a XAML `Popup` or `Flyout`.** `overlay.md` records that "a popup opened from
  the tap fail-fasts", with no exception for any catch to see. The menu has to be an ordinary
  `Border` on the overlay's own `m_canvas`, positioned under the chip, the same way `m_panel` is.
- **Timers.** `RoseZoomTool::StartLens` and `StopLens` in `tap_tool_zoom.h` are the pattern: a
  `xaml::DispatcherTimer` member, created lazily, with a `try`/`catch` in `Tick`, stopped in one
  place. A `DispatcherTimer` has no one-shot mode, so the `Tick` handler stops it first.
- **Lifetime.** The overlay is a leaked singleton that is never torn down (`g_overlay`). A tap's
  `EndSession` (detach, or the pipe closing) releases the tap's interfaces and does not touch the
  overlay. So "when the tap goes away" means: cancel the countdown on the overlay's own exits.
  - `GoIdle` and `Collapse(true)` cancel it.
  - A pipe-driven arm, select-by-handle or deselect also cancels it.
  - There is no destructor to hook.
- **Panel fade.** `Operation` / `BeginOperation` pins the bar at full opacity while something is in
  progress. The countdown has to be an operation too, or the sweep runs at half opacity while the
  pointer is away, which is exactly when it is happening.
- **Mode reporting.** `ModeName()` returns `idle`, `select` or `rulers`, and the host's
  hand-back check treats "idle" as "the app is clickable". During a countdown nothing is captured, so
  reporting `idle` is the truthful answer and needs no wire change. See Q3.
- **Glyphs.** Codepoints are named at the top of `tap_widgets.h`. `SelectIcon()` is the precedent
  for a composed icon (shapes plus a smaller glyph in a 16x16 `Grid`).
- **Probe apps.** `tests/apps/uwp-classic`, `uwp-modern` and `winui` contain no flyout, menu or
  tooltip today, so there is nothing to test a popup pick against.

## Questions / unknowns

> **Answered 12 October 2026:** "use your assumption. This is for me more than for some agent." So
> Q3 is `idle` with no wire change, Q4 cancels, and Q7 is a log line. None of these is worth more
> effort on the agent-facing side.

- **Q1. Do picks see open popups, in each framework?** Status: OPEN. Phase A answers it.
  - Assumption: they don't, so Phase B is needed.
- **Q2. On WinUI 3, is `m_pointer` right while the pointer is over a windowed popup?** Status: OPEN.
  Phase A answers it.
  - If it isn't, read the cursor directly at expiry through a provider seam, e.g.
    `RoseTapPointerNow(anchor, Point&)`.
    - UWP: `CoreWindow::PointerPosition`.
    - WinUI: `GetCursorPos`, then the window's client origin, then divide by `RasterizationScale`.
  - `m_pointer` stays the fallback.
- **Q3. What does the host see during a countdown?** Status: OPEN, needs your call.
  - Recommendation: `idle`, since nothing is captured and the app is clickable. A new `countdown`
    word would need `XamlWire.ProtocolVersion` raised for little gain.
  - The resulting pick reaches the host the normal way: `SelectionRows`, read on request.
- **Q4. What does a pipe-driven arm, select-by-handle or deselect do during a countdown?** Status:
  OPEN.
  - Assumption: it cancels the countdown, then does its own thing. An agent's explicit request beats
    a timer the person may have forgotten about.
- **Q5. Pointer over the toolbar (or the menu) at zero.** Status: ANSWERED by design. It is a no-op,
  logged. `Beneath` would otherwise pick the app element underneath the bar, which nobody pointed at.
- **Q6. Does Segoe MDL2 / Fluent have a stopwatch and a slashed stopwatch?** Status: OPEN.
  - Check in the character map. If either is missing, compose it the way `SelectIcon()` does: the
    stopwatch glyph with a `Line` for the slash, or a small `TextBlock` for the digit.
  - Do not ship a codepoint that hasn't been checked. A wrong one renders as a hollow box.
- **Q7. Is "nothing pickable" at zero just a log line, or also visible on the chip?** Status: OPEN.
  - Assumption: a log line, with the chip going back to rest. Revisit after you've used it.

## Execution steps

### Phase A: prove the pick can see popups (spike, before any UI)

1. **Add a popup target to each probe app.**
   - Add a `Button` with a `Flyout`, a `Button` with a `MenuFlyout`, and an element with a
     `ToolTip`, with names the tests can find.
   - Check whether any existing live-app test asserts a node count or order that this disturbs, and
     fix the assertion rather than avoiding the addition.
2. **Measure, in UWP and WinUI 3.**
   - Open the flyout, then call `rose_xaml_select_element` and click inside it. Record what was
     picked.
   - Log `m_pointer` while the pointer is over the open flyout, on WinUI with both
     `ShouldConstrainToRootBounds` values.
   - Write the answers into Q1 and Q2 and the Evidence section.
3. **Decide.**
   - If popups are seen and `m_pointer` is right, skip Phase B.
   - If a popup can't be hit-tested at all in some case, write down why here, before falling back to
     "select mode comes on" for that case (the milestone requires this).

### Phase B: popup-aware hit testing (if Phase A says so)

4. **Make the pick see popups.**
   - In `Beneath` and `Record`, first walk `GetOpenPopupsForXamlRoot(Root())`, topmost first. For
     each popup, hit-test `FindElementsInHostCoordinates(point, popup.Child(), m_includeAllElements)`.
     Fall through to `Content()` only if nothing usable is found.
   - Keep `IsOurs`, `Bounds` and Just my XAML exactly as they are.
   - This also fixes ordinary click-picks over a flyout, so it is worth having on its own.
   - If the coordinates differ for a windowed popup, convert them there, and say so in a comment.
5. **Add `RoseTapPointerNow`** in both providers, only if Q2 came back "wrong". Its signature is
   free of projected types (`tap-tiers.md`).

### Phase C: chip, icons and menu

6. **Icons** (Q6).
   - Add the stopwatch codepoints to the named list in `tap_widgets.h`.
   - Add a `TimerIcon(int seconds)` factory next to `SelectIcon()`:
     - `0` gives the slashed stopwatch;
     - `n` gives the stopwatch with `n` laid over it.
   - The countdown reuses this, so make the digit `TextBlock` reachable (through `Tag`, like `Chip`
     does) and set its text in place rather than rebuilding the icon.
7. **Chip.**
   - Add `m_timerButton = Chip(TimerIcon(m_delay), L"Delay before select", ...)` after
     `m_selectButton`.
   - `Chrome()` paints it `Accent()` when `m_delay > 0`, and swaps its icon when the delay changes.
   - Its click opens or closes the menu. During a countdown it cancels instead (step 12).
8. **Menu.** Put this in a new tier-3 header, `tap_tool_timer.h`, holding the delay, the menu, the
   countdown and the sweep, following `RoseZoomTool` and `RoseRulersTool`.
   - Register it in `TapTierTests.Tiers` at tier 3, and include it in both providers next to the
     other tools, in tier order.
   - Build the menu as a `Border` on `m_canvas`:
     - the panel's background and border colours, and the same corner radius;
     - a vertical `StackPanel` of four rows, each with the icon, the label, and a 3px rose bar on
       the left that is visible only on the current row;
     - hover wash on rows, using `HoverWash` / `PressWash`.
   - Position it under the timer chip with `TransformToVisual(m_canvas)`, clamped inside the window
     the way `Place()` clamps the bar.
   - Closing the menu:
     - Picking a row closes it.
     - The chip toggles it.
     - Clicking anywhere else closes it. While it is open, a transparent full-bleed layer under the
       menu swallows one press and closes it, the same mechanism as the select capture layer. The
       target's own UI is never open at that point, so this cannot dismiss anything of the app's.
   - The overlay asks the tool for the menu through `IRoseOverlaySurface`, the same way the other
     tools reach back, so the tool doesn't hold a `RoseOverlay&`.

### Phase D: countdown and timed pick

9. **Start.**
   - The Select chip's click becomes `m_delay > 0 ? StartCountdown() : BeginSelect(false)`.
   - Only the chip changes. The `BeginSelect` override that the host calls is untouched, so the pipe
     arms immediately as before (scope).
   - `StartCountdown`:
     1. Calls `GoIdle()` first, so nothing is captured and any mode is left.
     2. Records the start time.
     3. Starts one `DispatcherTimer` for the pick, with its interval set to the delay; `Tick`
        stops the timer and then picks.
     4. Begins one `Storyboard` of the same duration for the visuals.
     5. Calls `BeginOperation(Operation::Countdown)`.
   - This is the milestone's "one one-shot `DispatcherTimer`, sweep from a storyboard of the same
     duration", with both started in the same call, so they cannot drift apart by more than a frame.
10. **Visuals, in the one storyboard.**
    - **Digit:** an `ObjectAnimationUsingKeyFrames` on the digit's `Text`, with one
      `DiscreteObjectKeyFrame` per second (`n` ... `1`). The digit can only be at the same point in
      time as the sweep, and there is no once-a-second callback.
    - **Sweep:** a wedge over the chip that starts at 12 o'clock and shrinks counter-clockwise to
      nothing, in the accent at low opacity under the glyph.
      - Prefer independent animations, so the sweep costs the app no UI-thread frames. Use the
        two-half-disc technique: two semicircles, each clipped to half the chip, rotated by
        `RotateTransform.Angle` keyframes. Rotation is independent; geometry is not.
      - If that cannot be made to read well at 24px, fall back to an `ArcSegment` updated from the
        same start time on `CompositionTarget::Rendering`. It lasts at most 10 seconds. Unhook
        `Rendering` on every exit, and say in the code why it is acceptable.
    - The constraints from the story: on or immediately around the chip, the accent colour, smooth,
      and readable at chip size.
11. **Expiry.**
    1. Stop the storyboard.
    2. `EndOperation(Countdown)`.
    3. Put the chip back to its resting icon.
    4. Call `PickAt(pointer)`, where the pointer comes from `RoseTapPointerNow` if it exists and
       `m_pointer` otherwise.
    - `PickAt` is the extracted body of `OnPick`, without `e.Handled` or the mode transition.
    - It returns early, with a log line, in these cases:
      - the pointer is unknown (`-1, -1`);
      - the pointer is inside `PanelRect()` or the menu (Q5);
      - `Beneath` finds nothing.
    - Nothing is placed under the pointer first. That is the whole point of the story.
12. **Cancel**, through one `CancelCountdown()` that does what expiry does minus the pick, and is
    safe to call when nothing is running. It is called from:
    - the Idle chip, the Select chip and the timer chip during a countdown;
    - `Collapse(true)`;
    - `GoIdle()` from any source;
    - `BeginSelect` / `BeginRulers` from the pipe;
    - `SelectByHandle` and `Deselect` (Q4).

    It stops and releases the timer, and stops the storyboard. Leave no `Rendering` handler hooked.
13. **Records.**
    - `docs/invariants/overlay.md`:
      - a pick sees open popups, topmost first, and why;
      - the timed pick puts nothing under the pointer, and why (light-dismiss and pointer-exit);
      - one clock for the pick and the sweep, and every exit cancels;
      - the menu is not a XAML `Popup`, because popups opened from the tap fail-fast (link to the
        existing paragraph rather than restating it).
    - Update the pick decision if popup ordering changes what "what a click would hit" means.
    - Add an `inbox.md` line for the wiki page and for the "Next popup" follow-up.

## Validation

**Automated:**

- Both provider `build.ps1` scripts build clean at x64 Debug.
- `RoseMcp.UnitTests.exe` passes, including `TapTierTests` with `tap_tool_timer.h` registered.
- Live-app overlay tests (UWP and WinUI) stay green, so the pipe-driven select is unchanged.
- **New integration test:** after Phase B, open the probe app's flyout (by invoking its button over
  the pipe or through UIA), then `SelectByHandle` / arm select and confirm that the popup's elements
  are in the tree and that a pick reports them. If a pointer-free assertion of the popup hit test is
  possible, add it, because it covers the click path too.
- `RoseMcp.IntegrationTests.Windows.exe`, `dotnet format --verify-no-changes`, and
  `tools/Check-Comments.ps1`.

**Manual** (deploy, then use all three probe apps):

1. The timer chip sits between Select and Rulers. The menu matches the mockup. The current row has
   the rose bar. Picking a row closes the menu and changes the chip's icon. The chip is rose while a
   delay is set. The choice persists after a pick.
2. With 3s set: click Select, move to the flyout's button, open the flyout, and hover an item inside
   it. The digit counts 3, 2, 1, the sweep runs smoothly counter-clockwise from 12, and at zero the
   flyout item is picked and outlined. The flyout is still open. `rose_xaml_selection` reports it.
3. The same with a tooltip and with a `MenuFlyout`, and, on WinUI, with an unconstrained (windowed)
   flyout.
4. Cancel through every path in step 12. Each one stops the sweep and returns the chip to rest, with
   no pick later.
5. At zero with the pointer on the toolbar, or outside the window, nothing is picked, a log line
   says so, and nothing crashes.
6. No delay: Select behaves exactly as before.
7. Collapse during a countdown cancels it. Re-expanding shows a resting chip.

**Done when:** the milestone's nine "What done looks like" points hold in UWP and WinUI 3, and a
light-dismiss flyout survives being picked.

## Risks

- **Popups can't be hit-tested in some case** (a windowed popup on WinUI).
  - Mitigation: Phase A runs first. Write down why, then fall back for that case only.
- **`m_pointer` is stale over a windowed popup.**
  - Mitigation: the `RoseTapPointerNow` seam, measured in Phase A.
- **Timers inside someone else's app** (Steve's concern).
  - Mitigation: one `DispatcherTimer` on the UI thread, stopped as the first thing in `Tick`. One
    storyboard. A single `CancelCountdown` called from every exit. `try`/`catch` in `Tick`. No pool
    threads and no sleeping.
- **The sweep costs the app frames.**
  - Mitigation: independent rotation animations first. The `Rendering` fallback is bounded to 10
    seconds and unhooked on every exit.
- **A drop-down drawn in the overlay looks or behaves unlike the bar.**
  - Mitigation: build it only from the `RoseWidgets` palette and constants. Check it against the
    mockup.
- **Probe-app additions break existing assertions.**
  - Mitigation: step 1 checks for that, and fixes the assertions rather than avoiding the change.

## Evidence / references

- `docs/invariants/overlay.md`: a tap-opened `Popup` fail-fasts; the pointer seam's behaviour on
  WinUI; the capture layer is shared by both modes.
- `docs/invariants/xaml-tap-lifecycle.md`: the overlay outlives every tap; everything runs on the UI
  thread.
- `docs/decisions/a-pick-honours-hit-testing-and-prefers-the-apps-markup.md`
- `tap_tool_zoom.h`: `StartLens` / `StopLens` is the timer pattern.
- Phase A results: not measured. Skipped on 12 October 2026 at my call, to save time; Phase B was
  built straight away (see the log).

## Execution Log

**12 October 2026, on `ja/delayed-select`:**

- `a5edb74`: a pick sees open popups (Phase B). `Under(point)` in `tap_overlay.h` hit-tests
  `GetOpenPopupsForXamlRoot` from the end of the list, then the content. `Beneath` and `Record` both
  use it, so click-picks over a flyout change too: popup rows now come ahead of page rows in
  `rose_xaml_selection`. This is intended. The invariant is in `overlay.md`.
- `83d81ce`: the timer chip, menu, countdown and timed pick (Phases C and D), in a new
  `tap_tool_timer.h`, registered as tier 3.
  - The icon is MDL2 E916 (Stopwatch), checked by rendering it. A badge holds the number, and a
    slash is two `Line`s.
  - The sweep is the two-half-disc rotation. The digit is an `ObjectAnimationUsingKeyFrames` on
    `Text`, in the same storyboard as the sweep.
  - Menu rows use `Tapped`. The shield is a transparent `Grid` adorned after the panel, so it also
    covers the toolbar.
  - `PickAt` is shared by the click and the timed pick.
  - At zero, it picks nothing (with a log line) when the pointer is unknown or on the toolbar.
  - Cancel paths: Idle, Select or the timer chip again; `Point()` (either mode, from the toolbar or
    the pipe); `SelectByHandle`; `Deselect`; `Collapse(true)`.
- **Checks:**
  - Both providers build at x64 Debug.
  - Unit suite: 1810/1810.
  - `Check-Comments`: none new.
  - `LiveAppUwpOverlayTests`: 5/5.
  - `LiveAppWinUiTests`: 8/8.
- **Not checked yet: nothing has been tried by hand.** In particular:
  - whether the sweep and the digit animate at all;
  - whether `Tapped` fires on the menu rows;
  - where the menu lands;
  - a timed pick over a real flyout, in both UWP and WinUI;
  - a windowed WinUI flyout (Q2).
- **Tooltips are unsettled.** Neither framework's default `ToolTip` style sets
  `IsHitTestVisible`, so the style doesn't rule them out. But the framework may make the popup
  itself non-hit-testable in code, and the pick uses `includeAllElements = false`. Try it. If
  tooltips are missed, hit-testing popups with `true` is a design call, not a quiet fix.
- Not done: probe-app flyouts, and the integration test for a popup pick (both Phase A material).
