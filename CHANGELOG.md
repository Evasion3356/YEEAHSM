# Changelog

All notable user-facing changes to YEEAHSM are recorded here. Format
loosely follows [Keep a Changelog](https://keepachangelog.com/).

## [1.1.0] - 2026-09-23

### Fixed
- The game could hang on exit if it was closed while YEEAHSM was still
  waiting for the game to finish unpacking (the first couple of minutes
  after launch). Shutdown now skips cleanup when the game is exiting.

### Changed
- The weapon-stow hook no longer does any logging work in Release builds.
- Debug builds only: `YEEAHSM.log` falls back to
  `%LOCALAPPDATA%\RDR2ASIMods\YEEAHSM.log` when the game folder can't be
  written.

## [1.0.0] - 2026-09-17

### Added
- Weapons are no longer stowed on your horse when you dismount or walk
  into camp.
