# Detect and Fix DOS/32A Standalone Runtime Crash

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds.

This document follows `.agent/PLANS.md` from the repository root. It must remain self-contained enough that a future contributor can continue the debugging effort from this file and the current working tree alone.

## Purpose / Big Picture

The Windows build setup currently produces `binw\dos32a.exe` and the smoke test reports success because `binw\sver.exe` can inspect that file. The user observed a more important failure: executing `binw\dos32a.exe` in DOSBox-X prints the DOS/32A banner and then crashes DOSBox-X with `JMP Illegal descriptor type 0`. After this work, the smoke test must execute the extender itself and fail if DOSBox-X reports that protected-mode crash, and the generated extender should exit cleanly when run standalone.

## Progress

- [x] (2026-05-22 00:00Z) Read `.agent/PLANS.md` and started this ExecPlan before changing source behavior.
- [x] (2026-05-22 00:00Z) Confirmed the current `smoke-dosbox.cmd` only runs `binw\sver.exe binw\dos32a.exe`, which inspects the file and does not execute the extender.
- [x] (2026-05-22) Reproduced the reported DOSBox-X crash by executing `binw\dos32a.exe` directly; DOSBox-X logged `E_Exit: JMP Illegal descriptor type 0` and did not reach the following marker command.
- [x] (2026-05-22) Compared the historical `dos32a_800\binw\dos32a.exe`; it also fails under this DOSBox-X runtime when forced through the same direct protected-mode path, with `E_Exit: IRET:Illegal descriptor type 12`.
- [x] (2026-05-22) Determined that the active standalone/no-client path enters protected mode before discovering that the unbound extender has no client executable, then exits through a DOSBox-X-crashing protected-mode error path.
- [x] (2026-05-22) Implemented a real-mode standalone guard and updated `smoke-dosbox.cmd` to run `SVER.EXE` and `DOS32A.EXE` from files staged under `C:\DOSBox-X\drivez\D32SMK`.
- [x] (2026-05-22) Rebuilt `dos32a` and `sver` from a clean tree and ran the drivez-based smoke test successfully.
- [x] (2026-05-22) Staged the validated fix for commit and push.

## Surprises & Discoveries

- Observation: `git status --short --branch` showed `D dos32a_800/binw/dos32a.lnk` before this plan was created.
  Evidence: The first status command in this session reported `## master...origin/master` followed by `D dos32a_800/binw/dos32a.lnk`. This plan does not assume that deletion is intentional and will avoid touching it unless required.
- Observation: `C:\DOSBox-X\drivez` works for staged input files, but files written from DOSBox-X `Z:` redirection were not written back to the host `drivez` directory.
  Evidence: `echo OK > STAND.OK` was logged by DOSBox-X but no `STAND.OK` file appeared under `C:\DOSBox-X\drivez\D32SMK`.
- Observation: DOSBox-X console logging does capture shell commands and `SVER.EXE` output from `Z:`, but not BIOS teletype output from the DOS/32A banner.
  Evidence: the successful smoke log contains `SVER_SMOKE_OK` and `DOS32A_SMOKE_OK`; the direct DOS/32A success condition is therefore returning to the shell and executing the sentinel echo.

## Decision Log

- Decision: Treat this as both a runtime defect and a smoke-test defect.
  Rationale: A smoke test that only inspects the generated executable cannot detect a crash that occurs when DOS/32A transfers into protected-mode code and exits. The runtime behavior must be exercised directly.
  Date/Author: 2026-05-22 / Codex

## Outcomes & Retrospective

Validated outcome: direct standalone execution of the rebuilt `DOS32A.EXE` no longer triggers the DOSBox-X protected-mode descriptor crash. The smoke test now stages `DOS32A.EXE` and `SVER.EXE` under `C:\DOSBox-X\drivez\D32SMK`, runs them from `Z:`, requires `SVER_SMOKE_OK` and `DOS32A_SMOKE_OK` sentinel lines, and fails on DOSBox-X crash signatures.

Validation evidence from `.\smoke-dosbox.cmd`:

    SVER -- Version Reporting Utility version 9.1.2
    Version:        9.1.2
    SVER_SMOKE_OK
    DOS32A_SMOKE_OK
    DOSBox-X smoke passed using staged files in C:\DOSBox-X\drivez\D32SMK.

## Context and Orientation

The active DOS/32A 9.1.2 extender sources live in `src\dos32a`. The top-level `build.cmd` builds the active extender by assembling `src\dos32a\kernel.asm` and `src\dos32a\dos32a.asm` with `tasm5\PATCHES\53_WIN\TASM32.EXE`, linking with Open Watcom, and copying the result to `binw\dos32a.exe`. The top-level `smoke-dosbox.cmd` currently starts DOSBox-X, mounts the repository as drive C, and runs `binw\sver.exe binw\dos32a.exe`; `sver.exe` is a version-reporting utility, not the extender itself.

The user reported that executing `dos32a.exe` directly displays version and copyright text and then DOSBox-X raises a dialog with `JMP Illegal descriptor type 0`. In this plan, "standalone execution" means running `binw\dos32a.exe` directly from DOSBox-X, without using it to load another protected-mode program.

## Plan of Work

First, reproduce the failure with a controlled DOSBox-X command line that runs `binw\dos32a.exe` and writes a marker file after the command. If the marker file is missing, or if the DOSBox-X log contains the illegal descriptor text, the smoke test must report failure. The same harness will be used against `dos32a_800\binw\dos32a.exe` to learn whether the historical 8.00 binary exits cleanly or also triggers this emulator condition.

Revision: The final smoke harness no longer mounts the repository into DOSBox-X. It stages `DOS32A.EXE` and `SVER.EXE` into `C:\DOSBox-X\drivez\D32SMK`, starts DOSBox-X on `Z:`, and uses console-log sentinel lines rather than redirected marker files because `Z:` writes are not persisted to the host `drivez` directory.

Next, inspect `src\dos32a\dos32a.asm`, `src\dos32a\kernel.asm`, and included files under `src\dos32a\text` for the code path used when DOS/32A is executed with no client program. If the standalone path should only print the banner and terminate, the fix should make that path return to DOS before attempting an invalid far jump or descriptor switch.

Finally, update `smoke-dosbox.cmd` so it validates both `sver.exe` inspection and direct extender execution. The script must remain usable from PowerShell or Command Prompt and should clean up temporary files.

## Concrete Steps

Run these commands from `C:\Users\mbeutler\Projects\dos32a`:

    .\build.cmd clean
    .\build.cmd dos32a
    .\build.cmd sver
    .\smoke-dosbox.cmd

During debugging, use direct DOSBox-X invocations that mount this checkout and run `binw\dos32a.exe`, followed by a marker command such as `echo OK > standalone.ok`. If DOSBox-X crashes or resets before the marker is written, the marker will be absent and the harness will fail.

## Validation and Acceptance

The final acceptance is that `.\smoke-dosbox.cmd` returns exit code 0 only when both checks pass: staged `SVER.EXE DOS32A.EXE` reports DOS/32A version `9.1.2`, and direct execution of staged `DOS32A.EXE` returns to DOSBox-X so the `DOS32A_SMOKE_OK` sentinel echo appears in the DOSBox-X log. A failure containing `JMP Illegal descriptor type 0`, `Illegal descriptor`, `DYNX86:Can't run code`, `ERROR CPU`, a missing sentinel, or a nonzero DOSBox-X exit must make the smoke script return nonzero.

## Idempotence and Recovery

The build and smoke scripts are intended to be idempotent. `build.cmd clean` removes generated active-tree outputs under `binw\` and source-directory intermediates. The smoke test should delete its own marker and log files before and after running. This plan avoids destructive Git commands and leaves the pre-existing deletion of `dos32a_800\binw\dos32a.lnk` untouched.

## Artifacts and Notes

Initial evidence from the user is a DOSBox-X dialog titled `E_Exit` with message `JMP Illegal descriptor type 0`, observed immediately after the generated extender prints its version and copyright information.

## Interfaces and Dependencies

The fix depends on the local historical toolchain documented in `README.md`: Open Watcom under `C:\WATCOM`, TASM files under `tasm5\`, and DOSBox-X under `C:\DOSBox-X\dosbox-x.exe`. The scripts involved are `build-env.cmd`, `build.cmd`, and `smoke-dosbox.cmd`.

Revision note: Created this plan to guide automatic reproduction and repair of the user-reported standalone DOS/32A crash, and to keep the smoke-test gap documented while work proceeds.
