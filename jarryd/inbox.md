# Inbox

Unsorted. One line each is fine. Move an item into a milestone when it's clear enough to act on.

- Tray menu follow-up: Steve suggested making `MainWindow` a `UserControl` with a ViewModel and
  `x:Bind` on `Command=`. Any refactor has to keep the native-menu workarounds: refresh on
  right-click, no `Visibility`, toggles inverting their own state. See PR #368.
