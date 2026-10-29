# BEETLE BLUE RC12 — HELP readability validation

## Scope

RC12 modifies the native HELP data, page rendering, HELP navigation, firmware version text and documentation only. BDR1 V33, sample engine, project storage and Live Mode playback remain unchanged.

## Checks

| Check | Result |
| :-- | :-- |
| Ten topics / 66 cards / three per page | PASS, C layout test and Python contract test |
| Topic bounds, K1 pages, K2 topics, reset page when switching topic | PASS |
| Max 28-title/33-detail glyphs on a 240×240 LCD | PASS |
| Native C-rendered 240×240 reference captures, scaled nearest-neighbor to 1920×1920 | PASS, five sample pages visually reviewed |
| Writable JieLi FM1 app, with V2 storage enabled | PASS, 538,660 bytes; 42,904 bytes headroom |
| Read-only FM1 app, write-symbol audit | PASS, 465,308 bytes |
| Independent lane lengths and RC10 V32 migration regression | PASS |
| README guide only, CHANGELOG and clean source root | PASS |
| Flashable FWSC release | **NOT PRODUCED**, missing verified R0 recovery proof |
| Hardware FM1 screen, flash, OTA, battery and audio | NOT TESTED |

## Integrity

- Writable app SHA-256: `cf6df255d937436fabb6803104d7d97258e2819c64c58962c97dbacc743ea9d6`
- Read-only app SHA-256: `64ee2b258c0612a6f8d23a030ce333fa59aafb184056881e35f11df2c68c22ea`
- Non-flashable lab image SHA-256: `4679c508844850f7970786444fc5443027b63931337674923144f50e3a9dab27`
- Lab-image structure rebuilt from the prior RC11 reference; stock boot/OTA assets reused by the packaging code.

**Do not rename the .fwsc.lab file to .fwsc or attempt to flash it.** This file is only for controlled packaging analysis.
