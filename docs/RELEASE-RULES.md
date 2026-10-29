# BEETLE BLUE — release hygiene

This file describes the project publishing rules; it is not part of the end-user README.

1. `README.md` is the **current English user guide**: feature explanations, controls, compatibility, installation, safety and credits. Never append release stories, bug histories, test tables for old versions or changelog entries.
2. `CHANGELOG.md` owns all version-by-version history. Place detailed verification for the current build in `validation/`; older reports in `validation/archive/`.
3. Firmware HELP (`firmware/fm1-native/beetle_help.h`) must reflect the actual current controls, project format and Live Mode. Test the 240x240 card layout (three cards per page; title <=28 glyphs, detail <=33 glyphs), topic/page navigation, readable native screenshot captures and native compilation after every HELP edit.
4. Ship a clean project root. No obsolete firmware binaries, WebInstallers, Managers, version-specific JSON manifests or temporary build outputs. Keep only the latest usable tools and the current manifest, with historical technical reports under `validation/archive/`.
5. Run `python3 tools/check_blue_release_hygiene.py` before each release. Fix failures rather than renaming them out of detection.
6. Never claim that source edits changed an earlier `.fwsc`. Build, validate and package a new firmware using the release's actual recovery/packaging gates.
7. Keep third-party licences, source attributions, and original required notices intact.
