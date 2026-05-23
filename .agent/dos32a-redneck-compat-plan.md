# Debug DOS/32A DOS4GW Replacement With Redneck Rampage

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds.

This document follows `.agent/PLANS.md` from the repository root. It is self-contained so a future contributor can continue from the current working tree and the staged Redneck Rampage files under `C:\DOSBox-X\drivez\REDNECK`.

## Purpose / Big Picture

The current rebuilt DOS/32A extender can pass the small smoke sample, but the user reports that a real DOS/4GW game, Redneck Rampage, hard-locks real hardware immediately after the DOS/32A version banner when the game's `DOS4GW.EXE` is replaced by this repository's `binw\dos32a.exe`. After this work, the repository should either contain a runtime fix that lets this real-world DOS/4GW replacement case progress past extender startup, or a narrowed, reproducible failure with enough evidence to continue safely.

## Progress

- [x] (2026-05-22) Confirmed the working tree starts clean on `master` and `origin/master`.
- [x] (2026-05-22) Listed `C:\DOSBox-X\drivez\REDNECK`; it contains `RR.EXE`, `RAMPAGE.BAT`, original `DOS4GW.4gw`, older `DOS4GW.32A`, and current `DOS4GW.EXE` sized like the rebuilt extender.
- [x] (2026-05-22) Read `RAMPAGE.BAT`; it runs `bmouse launch rr.exe`.
- [x] (2026-05-22) Reproduced the failure in DOSBox-X from `Z:\REDNECK` without mounting: current rebuilt DOS/32A as `DOS4GW.EXE` hits an FPU warning, illegal opcode `0xff`, then `E_Exit: RET from illegal descriptor type 8`.
- [x] (2026-05-22) Compared variants. Original `DOS4GW.4gw` did not hit the DOS/32A descriptor failure; older `DOS4GW.32A` also failed under DOSBox-X, confirming this is a DOS/32A replacement-configuration path, not just the latest rebuild.
- [x] (2026-05-22) Parsed `RR.EXE`; it is a three-object LE executable with entry `obj1:00079e94`, stack `obj3:0022df20`, and page-aligned object bases.
- [x] (2026-05-22) Tested the bundled `dos4gw.d32` header against only the staged `REDNECK\DOS4GW.EXE`; the immediate illegal opcode/descriptor failure disappeared.
- [x] (2026-05-22) Implemented a targeted runtime fix: when the extender's own executable name is `DOS4GW` or `DOS4GW.EXE`, apply the known DOS/4GW-compatible defaults before reading `DOS32A=` environment overrides.
- [x] (2026-05-22) Rebuilt `binw\dos32a.exe`, copied it to `C:\DOSBox-X\drivez\REDNECK\DOS4GW.EXE`, and reran `RAMPAGE.BAT`; the test stayed running past the previous failure point with no DOSBox-X illegal opcode, descriptor abort, double fault, or packed-file message in the 20 second log window.
- [x] (2026-05-22) Verified the normal smoke still passes with `cmd /c "build.cmd dos32a && smoke-dosbox.cmd"`.

## Surprises & Discoveries

- Observation: `RAMPAGE.BAT` does not directly invoke `RR.EXE`; it runs the mouse helper first.
  Evidence: The file contains `bmouse launch rr.exe`.
- Observation: The generic DOS/32A default header and the older unconfigured DOS/32A binary both fail Redneck under DOSBox-X, while the DOS/4GW-compatible header avoids the immediate transfer failure.
  Evidence: Current default ended with `E_Exit: RET from illegal descriptor type 8`; older DOS/32A ended with ROM-write/triple-fault style failures; applying `dos4gw.d32` caused both direct `RR.EXE` and `RAMPAGE.BAT` to remain running for the timeout without those errors.
- Observation: `dos4gw.d32` differs from the generic default in ways that matter for DOS/4GW replacement use: page-aligned high object loading, more selectors, larger virtual stacks, smaller DOS transfer buffer, and DOS/4GW-oriented reporting defaults.
  Evidence: The config bytes are `49 44 33 32 0A 10 04 10 00 10 08 08 40 00 40 00 FF FF FF FF 34 0D 00 01`.

## Decision Log

- Decision: Use Redneck Rampage as the compatibility target in addition to the small protected-mode smoke sample.
  Rationale: The small sample validates the basic `STUB/32A` path, but Redneck Rampage is the user's real hardware failure and exercises a DOS/4GW replacement path with a large Watcom-era game executable.
  Date/Author: 2026-05-22 / Codex
- Decision: Apply the DOS/4GW-compatible defaults dynamically when the executable basename is `DOS4GW` or `DOS4GW.EXE` instead of changing the generic `DOS32A.EXE` default header.
  Rationale: The documented replacement workflow is to copy/rename `DOS32A.EXE` to `DOS4GW.EXE`. Name-triggered defaults fix that path while preserving existing DOS/32A defaults and still allowing explicit `DOS32A=` environment settings to override the automatic configuration.
  Date/Author: 2026-05-22 / Codex

## Outcomes & Retrospective

The source now applies DOS/4GW-compatible runtime defaults automatically when launched as `DOS4GW.EXE`. This removes the immediate Redneck Rampage DOSBox-X descriptor failure and keeps the existing DOS/32A smoke passing. Real-hardware validation is still required because the user's original symptom was a hard lock on physical hardware.

## Context and Orientation

The active extender source lives under `src\dos32a`. The build output `binw\dos32a.exe` can be copied and renamed to `DOS4GW.EXE` so DOS/4GW-stubbed programs attempt to use DOS/32A as a replacement extender. Redneck Rampage is staged outside the repository at `C:\DOSBox-X\drivez\REDNECK`; its main executable is `RR.EXE`, its launcher batch is `RAMPAGE.BAT`, and the current replacement extender is `DOS4GW.EXE`.

## Plan of Work

First, run Redneck Rampage under DOSBox-X from `Z:\REDNECK` with logging enabled and a short timeout. Run both `RAMPAGE.BAT` and `RR.EXE` directly because the launcher includes `BMOUSE.EXE`, and the first failure to isolate is whether the extender hangs before the game code or after the mouse helper starts the game.

Next, compare the current `DOS4GW.EXE` replacement against the original `DOS4GW.4gw` and the older `DOS4GW.32A` already present in the directory. Preserve those files by copying them over `DOS4GW.EXE` only inside a guarded script that restores the current file afterward.

Then inspect the DOS/4GW stub and DOS/32A external-loader path. If Redneck reaches the DOS/32A banner and then hangs before verbose loader output, instrument the real-mode entry and protected-mode initialization points in `src\dos32a\dos32a.asm` temporarily, rebuild, and re-run. Remove temporary instrumentation before committing.

## Concrete Steps

Run commands from `C:\Users\mbeutler\Projects\dos32a`. Use DOSBox-X at `C:\DOSBox-X\dosbox-x.exe` and the staged game directory `C:\DOSBox-X\drivez\REDNECK`.

The first reproduction command starts DOSBox-X hidden, changes to `Z:\REDNECK`, runs `RR.EXE`, and captures the console log. A timeout is treated as a reproduced hang.

## Validation and Acceptance

The final acceptance is that the repository build still passes `.\smoke-dosbox.cmd`, and the Redneck Rampage launcher progresses past the DOS/32A banner without descriptor crashes or immediate hard-lock behavior under DOSBox-X. If the game requires interactive video input after startup, reaching stable game initialization or a logged game error after protected-mode transfer is acceptable evidence.

## Idempotence and Recovery

Do not permanently modify `C:\DOSBox-X\drivez\REDNECK` without preserving the user's files. When swapping `DOS4GW.EXE`, copy the original current file to a temporary backup and restore it in a `finally` block. Avoid destructive Git commands.

## Artifacts and Notes

The user's real-hardware symptom is: after replacing Redneck Rampage's `DOS4GW.EXE` with the repository's DOS/32A build, the extender displays its version and then the machine hard-locks; keyboard reset does not work.

## Interfaces and Dependencies

The local build depends on Open Watcom under `C:\WATCOM`, TASM under `tasm5`, and DOSBox-X under `C:\DOSBox-X`. The relevant scripts are `build.cmd`, `build-env.cmd`, and `smoke-dosbox.cmd`. The relevant source files are expected to be under `src\dos32a`, especially `src\dos32a\dos32a.asm` and included client loader files under `src\dos32a\text\client`.

Revision note: Created this plan to guide the Redneck Rampage DOS4GW replacement investigation.
