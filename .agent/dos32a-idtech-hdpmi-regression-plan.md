# Investigate id Tech HDPMI32 regression

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds.

This plan follows `.agent/PLANS.md` from the repository root.

## Purpose / Big Picture

The user reports that id Tech games such as Doom II freeze or reset on real hardware when HDPMI32 is loaded, while the same games work without HDPMI32. A Doom II executable bound with DOS/32A 7.35 works, while one bound with DOS/32A 8.00 resets, which suggests the regression came from 8.00-era code that was later ported into the maintained 26.x line.

The immediate goal is to diagnose efficiently despite the failure not reproducing in DOSBox. The useful output is a small, clearly labeled set of Doom II probe binaries that the user can test on real hardware to identify which 8.00 change group introduces the reset. Once a change group is isolated, a follow-up source fix can be made and validated with a much smaller test matrix.

## Progress

- [x] (2026-05-23 16:52-04:00) Confirmed the working tree is clean on branch `26.1`.
- [x] (2026-05-23 16:52-04:00) Confirmed HDPMI32 is available at `C:\DOSBox-X\drivec\HXRT\HDPMI32.EXE` and the real-hardware startup batch is `C:\DOSBox-X\drivec\TWEAKS.BAT`.
- [x] (2026-05-23 16:55-04:00) Compared 7.35 and 8.00 source changes. The highest-value buckets are external-DPMI startup/kernel removal, speed-loop substitutions, INT 21h read/write changes, DPMI detection, loader changes, and data/misc buffer changes.
- [x] (2026-05-23 16:56-04:00) Built 13 DOS/32A probe extenders: 7.35 good control, 8.00 bad control, seven 8.00 file-level reversions, one 8.00 CLI/STI restoration probe, and three 26.1 probes.
- [x] (2026-05-23 16:57-04:00) Bound all 13 probes against `C:\DOSBox-X\drivec\DOOM2\DOOM2.BAK` in four DOSBox-X batches, each under 10 seconds. SVER verified expected embedded DOS/32A versions.
- [x] (2026-05-23 16:58-04:00) Packaged probes at `dist\idtech-hdpmi-doom2-probes.zip` with `TESTPLAN.TXT`, `MANIFEST.TXT`, `RESULTS.CSV`, and `APPLYONE.BAT`.
- [x] (2026-05-23 16:59-04:00) Removed temporary `C:\DOSBox-X\drivec\D32PROBE` and `dist\idtech-hdpmi-probes` scratch directories; no DOSBox-X processes or `D32*`/`.le` scratch files remained.
- [x] (2026-05-23 17:15-04:00) Read the updated real-hardware results from `dist\idtech-hdpmi-doom2-probes\RESULTS.CSV`.
- [x] (2026-05-23 17:23-04:00) Built a focused 26.1 detect-path package at `dist\idtech-hdpmi-detect-probes.zip`.
- [x] (2026-05-23 17:24-04:00) Verified the package contains five bound Doom II probes and removed temporary `C:\DOSBox-X\drivec\D32DP`; no DOSBox-X processes remained.

## Surprises & Discoveries

- Observation: The 8.00 source has several `GUTI` speed changes, including `loop` substitutions and commented `cli`/`sti` around `remove_kernel`.
  Evidence: `dos32a_800\src\dos32a\dos32a.asm` has `;cli ;GUTI` and `;sti ;GUTI` in `remove_kernel`; 7.35 and current 26.1 have active `cli`/`sti`.

- Observation: Current 26.1 does not contain the 8.00 INT 21h read/write rewrite wholesale.
  Evidence: `src\dos32a\text\client\int21h.asm` retains the 7.35-style `@__4402h`, `@__4404h`, and `@__cp2` paths, with a later stdin guard.

- Observation: The current tree has a custom HDPMI detection path not present in 7.35 or 8.00.
  Evidence: `src\dos32a\text\kernel\detect.asm` contains `@@detect_HDPMI` and calls it before normal VCPI/DPMI detection.

- Observation: Current 26.1 passes Doom II on the user's real hardware when only `detect.asm` is reverted to the 7.35 version, while disabling the external-DPMI `remove_kernel` call still freezes.
  Evidence: User-updated results show `260D735` as `pass`, `260NOKRM` as `freeze` at `calling DMX_Init`, and `260BASE` as `freeze` at `S_Init: Setting up sound.`

- Observation: The historical 8.00 reset is isolated to a different nearby subsystem than the current 26.1 freeze.
  Evidence: User-updated results show every 8.00 reversion probe still resetting except `800I31`, which passes when 8.00 uses the 7.35 `text\kernel\int31h.asm`.

## Decision Log

- Decision: Use real-hardware probe binaries instead of trying to rely on DOSBox reproduction.
  Rationale: The user reports Doom II works in DOSBox but resets on real hardware only when HDPMI32 is loaded. A DOSBox-only test cannot distinguish the failing path.
  Date/Author: 2026-05-23 / Codex

- Decision: Start from the known 7.35 good and 8.00 bad boundary.
  Rationale: The user already validated that 7.35 works and 8.00 resets, giving a strong historical regression window and avoiding a broad search across unrelated 26.x changes.
  Date/Author: 2026-05-23 / Codex

- Decision: Bind all probe executables with the current fixed SB/32A while changing only the `DOS32A.EXE` used as the embedded extender.
  Rationale: Stock 8.00 SB/32A previously stalled while binding Doom II, and the purpose of these probes is to test extender runtime behavior, not binder behavior. SVER verifies the embedded extender version after binding.
  Date/Author: 2026-05-23 / Codex

- Decision: Include current-tree probes `260NOKRM` and `260D735` in addition to historical 8.00 reversions.
  Rationale: The 8.00 bad boundary identifies the historical source of the regression, but the actionable fix must land in 26.1. `260NOKRM` tests external-DPMI kernel compaction directly, and `260D735` tests whether the newer HDPMI detection path is involved.
  Date/Author: 2026-05-23 / Codex

- Decision: Build a smaller 26.1-only detect-path probe package before editing the maintained source.
  Rationale: Reverting all of `detect.asm` proves the current failure is in the detection path, but the 26.0 write-combining fix depends on detecting HDPMI32 even when the IOPL shortcut would otherwise skip DPMI probing. The next package must distinguish the HDPMI signature shortcut from normal DPMI detection ordered before the IOPL shortcut.
  Date/Author: 2026-05-23 / Codex

## Outcomes & Retrospective

Produced a real-hardware probe package at `dist\idtech-hdpmi-doom2-probes.zip`. It contains 13 Doom II executables, each in a directory named for the probe, plus a test plan and result sheet. The package is designed so the first seven tests should usually identify the relevant subsystem; the remaining tests provide file-level fallback if the high-value probes do not flip the result.

No source fix has been made yet. The first probe package showed that current 26.1's Doom II failure is in `src\dos32a\text\kernel\detect.asm`, and that 8.00's reset is fixed by reverting `text\kernel\int31h.asm`. The focused detect-path package is now available at `dist\idtech-hdpmi-detect-probes.zip`; it determines whether the fix can be a narrow replacement of the HDPMI-specific detection shortcut with normal DPMI detection before the IOPL shortcut.

## Context and Orientation

The repository root is `C:\Users\mbeutler\Projects\dos32a`. The maintained source is under `src\dos32a`. Historical 7.35 source is under `dos32a-735-src\src\dos32a`, and historical 8.00 source and binaries are under `dos32a_800`.

HDPMI32 is a DPMI host, meaning it provides protected-mode services to DOS programs before DOS/32A starts. The user loads it with `C:\DOSBox-X\drivec\TWEAKS.BAT`, which sets `HDPMI=513`, sets `DOS32A=C:\DOS32A /DPMITST:ON /DOSBUF:16 /NOWARN:9003`, and runs `C:\HXRT\HDPMI32 -r`. The failure happens on real hardware with HDPMI32 loaded, not in DOSBox.

The test application is Doom II. The original executable is `C:\DOSBox-X\drivec\DOOM2\DOOM2.BAK`; generated probe binaries must not overwrite it.

## Plan of Work

First, inspect the differences between `dos32a-735-src\src\dos32a` and `dos32a_800\src\dos32a`, starting with the 8.00 change notes. The likely high-value files are `dos32a.asm` for startup and configuration data, `kernel.asm` and `text\kernel\*.asm` for DPMI host interaction, and `text\client\*.asm` for DOS/4GW emulation and interrupt forwarding.

Second, identify a small number of 8.00 change groups that can plausibly affect id Tech under HDPMI32. For each change group, create a temporary source variant by copying either the 7.35 or 8.00 source tree to a scratch directory under `dist`, applying the minimal file-level or block-level change, building `DOS32A.EXE`, and binding `DOOM2.BAK` into a labeled executable.

Third, package the probes in a zip with a manifest that states exactly which source variant each executable represents and the order to test them. The expected result is not that all probes pass locally; the expected result is that the user's real hardware can identify the first bad variant or the first good reverted variant.

## Concrete Steps

Run source comparison from the repository root:

    git diff --no-index dos32a-735-src\src\dos32a dos32a_800\src\dos32a

Build probe extenders with the existing historical toolchain wrappers:

    cmd /v:on /c "call build-env.cmd && ..."

Bind Doom II probes under DOSBox-X, always mounting the game tree explicitly:

    C:\DOSBox-X\dosbox-x.exe -c "mount c C:\DOSBox-X\drivec" -c "c:" -c "C:\D32PROBE\BUILD.BAT"

The probe package path is:

    C:\Users\mbeutler\Projects\dos32a\dist\idtech-hdpmi-doom2-probes.zip

The probe names are:

    735GOOD   7.35 source control
    800BAD    8.00 source control
    260BASE   current 26.1 control
    260NOKRM  current 26.1 with remove_kernel call disabled
    260D735   current 26.1 with 7.35 detect.asm
    800CLI    8.00 with CLI/STI restored around remove_kernel
    800DASM   8.00 with 7.35 dos32a.asm
    800INIT   8.00 with 7.35 init.asm
    800I21    8.00 with 7.35 int21h.asm
    800MISC   8.00 with 7.35 misc.asm and data.asm
    800DET    8.00 with 7.35 detect.asm
    800LOAD   8.00 with 7.35 loader.asm
    800I31    8.00 with 7.35 int31h.asm

Focused detect-path probe names are:

    260BASE   current 26.1 detect path control
    260D735   current 26.1 with full 7.35 detect.asm
    260NOHDP  current 26.1 with only the HDPMI-specific shortcut calls disabled
    260DPMI1  current 26.1 with the HDPMI shortcut replaced by normal early DPMI detection
    260NOIOP  current 26.1 with the HDPMI shortcut disabled and the IOPL shortcut changed to run normal VCPI/DPMI tests

## Validation and Acceptance

This phase is acceptable when:

The source comparison has a concise list of candidate 8.00 change groups.
The package contains labeled Doom II probe executables and a manifest explaining what each one tests.
SVER reports the expected DOS/32A version or probe label for each generated executable when possible.
The live `C:\DOSBox-X\drivec\DOOM2\DOOM2.EXE` and `DOOM2.BAK` are not overwritten.
No DOSBox-X process remains running, and scratch files are removed.

The real acceptance result will come from the user running the probes on hardware with `TWEAKS.BAT` loaded and reporting which binaries pass, freeze, or reset.

Observed local validation:

    All 13 probe extenders built.
    All 13 Doom II probes bound from DOOM2.BAK.
    735GOOD SVER: Version 7.35.
    800* probes SVER: Release 9, Version 8.0.
    260* probes SVER: Version 26.1.
    Zip entries: 17.
    Focused package extenders built: 260BASE, 260D735, 260NOHDP, 260DPMI1, 260NOIOP.
    Focused package Doom II probes bound from DOOM2.BAK in two DOSBox-X batches, each under 10 seconds.
    Focused package zip size: 1413883 bytes.

## Idempotence and Recovery

All generated source variants and package files should live under `dist` or a temporary `C:\DOSBox-X\drivec\D32PROBE` folder. Do not modify historical source trees in place except through temporary copied trees. Do not overwrite `C:\DOSBox-X\drivec\DOOM2\DOOM2.EXE` or `.BAK` during probe generation.

If a DOSBox-X bind step stalls, stop the DOSBox-X process, inspect the partial log, and continue with smaller batches under the user's 10-second iteration limit.

## Artifacts and Notes

Generated artifacts:

    dist\idtech-hdpmi-doom2-probes.zip
    dist\idtech-hdpmi-doom2-probes\TESTPLAN.TXT
    dist\idtech-hdpmi-doom2-probes\MANIFEST.TXT
    dist\idtech-hdpmi-doom2-probes\RESULTS.CSV
    dist\idtech-hdpmi-doom2-probes\APPLYONE.BAT
    dist\idtech-hdpmi-detect-probes.zip
    dist\idtech-hdpmi-detect-probes\TESTPLAN.TXT
    dist\idtech-hdpmi-detect-probes\MANIFEST.TXT
    dist\idtech-hdpmi-detect-probes\RESULTS.CSV
    dist\idtech-hdpmi-detect-probes\APPLYONE.BAT

The package directories `735GOOD`, `800BAD`, `260BASE`, `260NOKRM`, `260D735`, `800CLI`, `800DASM`, `800INIT`, `800I21`, `800MISC`, `800DET`, `800LOAD`, and `800I31` each contain a `DOOM2.EXE` probe.

The focused detect-path package directories `260BASE`, `260D735`, `260NOHDP`, `260DPMI1`, and `260NOIOP` each contain a `DOOM2.EXE` probe.

## Interfaces and Dependencies

The build depends on Open Watcom at `C:\WATCOM`, TASM 5.3 under `tasm5`, and DOSBox-X at `C:\DOSBox-X\dosbox-x.exe`. The real-hardware runtime dependency is HDPMI32 at `C:\HXRT\HDPMI32.EXE` on the target DOS drive.

Revision note, 2026-05-23 / Codex: Initial investigation plan created after the user reported 7.35 passes and 8.00 resets with HDPMI32 on real hardware.

Revision note, 2026-05-23 / Codex: Updated after producing and validating the Doom II probe package, recording candidate change buckets, probe names, package path, and cleanup status.

Revision note, 2026-05-23 / Codex: Updated after reading the user's real-hardware results. `260D735` isolates the current 26.1 failure to `detect.asm`; `800I31` isolates the historical 8.00 reset to the INT 31h handler.

Revision note, 2026-05-23 / Codex: Updated after producing the focused 26.1 detect-path probe package and cleaning the DOSBox-X scratch drive.
