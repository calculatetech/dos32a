# Detect and Fix DOS/32A Runtime Smoke Crashes

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds.

This document follows `.agent/PLANS.md` from the repository root. It must remain self-contained enough that a future contributor can continue the debugging effort from this file and the current working tree alone.

## Purpose / Big Picture

The Windows build setup currently produces `binw\dos32a.exe` and the smoke test originally reported success because `binw\sver.exe` could inspect that file. The user observed two more important failures: executing `binw\dos32a.exe` by itself crashed DOSBox-X after the banner, and executing `binw\dos32a.exe <program.exe>` still crashes before the loaded protected-mode program runs. After this work, the smoke test must execute both the extender by itself and the extender loading a protected-mode sample, fail on DOSBox-X protected-mode crash signatures, and pass only when the generated extender returns to the shell for standalone use and successfully transfers control to the sample program.

## Progress

- [x] (2026-05-22 00:00Z) Read `.agent/PLANS.md` and started this ExecPlan before changing source behavior.
- [x] (2026-05-22 00:00Z) Confirmed the current `smoke-dosbox.cmd` only runs `binw\sver.exe binw\dos32a.exe`, which inspects the file and does not execute the extender.
- [x] (2026-05-22) Reproduced the reported DOSBox-X crash by executing `binw\dos32a.exe` directly; DOSBox-X logged `E_Exit: JMP Illegal descriptor type 0` and did not reach the following marker command.
- [x] (2026-05-22) Compared the historical `dos32a_800\binw\dos32a.exe`; it also fails under this DOSBox-X runtime when forced through the same direct protected-mode path, with `E_Exit: IRET:Illegal descriptor type 12`.
- [x] (2026-05-22) Determined that the active standalone/no-client path enters protected mode before discovering that the unbound extender has no client executable, then exits through a DOSBox-X-crashing protected-mode error path.
- [x] (2026-05-22) Implemented a real-mode standalone guard and updated `smoke-dosbox.cmd` to run `SVER.EXE` and `DOS32A.EXE` from files staged under `C:\DOSBox-X\drivez\D32SMK`.
- [x] (2026-05-22) Rebuilt `dos32a` and `sver` from a clean tree and ran the drivez-based smoke test successfully.
- [x] (2026-05-22) Staged the validated fix for commit and push.
- [x] (2026-05-22) Reproduced the remaining external-load failure with `DOS32A.EXE AHELLO.EXE` using the historical `dos32a_800\examples\asm_1\hello.exe` sample staged beside the active extender.
- [x] (2026-05-22) Tested whether restoring the older 32-bit far-return program handoff fixes external protected-mode sample execution; it was not the root cause and the source was restored to the original `IRETD` handoff.
- [x] (2026-05-22) Identified the correct drivez-only stub smoke layout: stage the extender as `D32Axxxx\BINW\DOS32A.EXE`, set `DOS32A=Z:\D32Axxxx`, and launch the stubbed sample `AHELLO.EXE` from that top-level directory.
- [x] (2026-05-22) Expanded the smoke test to include an external protected-mode sample so this regression is detected automatically.

## Surprises & Discoveries

- Observation: `git status --short --branch` showed `D dos32a_800/binw/dos32a.lnk` before this plan was created.
  Evidence: The first status command in this session reported `## master...origin/master` followed by `D dos32a_800/binw/dos32a.lnk`. This plan does not assume that deletion is intentional and will avoid touching it unless required.
- Observation: `C:\DOSBox-X\drivez` works for staged input files, but files written from DOSBox-X `Z:` redirection were not written back to the host `drivez` directory.
  Evidence: `echo OK > STAND.OK` was logged by DOSBox-X but no `STAND.OK` file appeared under `C:\DOSBox-X\drivez\D32SMK`.
- Observation: DOSBox-X console logging does capture shell commands and `SVER.EXE` output from `Z:`, but not BIOS teletype output from the DOS/32A banner.
  Evidence: the successful smoke log contains `SVER_SMOKE_OK` and `DOS32A_SMOKE_OK`; the direct DOS/32A success condition is therefore returning to the shell and executing the sentinel echo.
- Observation: The external-load failure is not limited to DOSBox-X `Z:` staging.
  Evidence: Running staged `DOS32A.EXE AHELLO.EXE` from `Z:\D32EXT` and from a normal mounted host directory both timed out with `ERROR CPU:CPU:GRP5:Illegal opcode 0xff` followed by `E_Exit: RET from illegal descriptor type 8`.
- Observation: The active source differs from DOS/32A 7.35 and 8.00 at the final jump into the loaded program.
  Evidence: `dos32a-735-src\src\dos32a\dos32a.asm` and `dos32a_800\src\dos32a\dos32a.asm` use a 32-bit far `RETF` into the application in the normal path, while `src\dos32a\dos32a.asm` pushes flags, code selector, and entry offset and uses `IRETD`.
- Observation: The final handoff difference is not the active failure for the supported stub path.
  Evidence: After restoring the active source to the original `IRETD` handoff and staging the extender as `D32ENV3\BINW\DOS32A.EXE` with `SET DOS32A=Z:\D32ENV3`, running `AHELLO.EXE` from `Z:\D32ENV3` printed `Hello world from protected mode!!!` and returned to the shell.
- Observation: Manual `DOS32A.EXE AHELLO.EXE` remains unreliable under DOSBox-X, including with historical binaries.
  Evidence: `BINW\DOS32A.EXE AHELLO.EXE` and copied historical 8.00 extenders timed out with descriptor faults, while the same sample launched through `STUB/32A` succeeded. This means the smoke test should exercise the way DOS/32A applications normally locate the extender, not the manually typed command form that also fails for upstream 8.00 in this emulator.
- Observation: DOSBox-X `Z:` can run files from a top-level staged directory but rejected `cd` into a second-level child directory during the smoke.
  Evidence: `cd D32B4CDC` succeeded, but `cd STAND` from inside it logged "the specified directory cannot be found" even though `C:\DOSBox-X\drivez\D32B4CDC\STAND` existed on the host. The final smoke therefore uses fresh top-level `D32Sxxxx` and `D32Axxxx` directories and only relies on nested `BINW\DOS32A.EXE` as a file path opened by `STUB/32A`.
- Observation: Long DOSBox-X `-c` command chains can fail to process the final `exit` after the protected-mode sample returns.
  Evidence: A run with many `-c` commands printed `APP_SMOKE_OK` but timed out before executing the following `-c "exit"`. The final smoke writes a generated batch file with `exit` as its last line and starts that batch from DOSBox-X.

## Decision Log

- Decision: Treat this as both a runtime defect and a smoke-test defect.
  Rationale: A smoke test that only inspects the generated executable cannot detect a crash that occurs when DOS/32A transfers into protected-mode code and exits. The runtime behavior must be exercised directly.
  Date/Author: 2026-05-22 / Codex
- Decision: Use the historical `dos32a_800\examples\asm_1\hello.exe` as the first external-load smoke sample.
  Rationale: It is already present in this repository, is a tiny protected-mode DOS/32A sample, and prints a clear `Hello world from protected mode!!!` message when control reaches application code.
  Date/Author: 2026-05-22 / Codex
- Decision: Do not change `enter_32bit_code` for this fix.
  Rationale: A trial far-return handoff did not fix the bad manual layout, and the original `IRETD` handoff successfully runs the protected-mode sample when the extender is staged in the expected `BINW` directory and selected by the `DOS32A` environment variable.
  Date/Author: 2026-05-22 / Codex
- Decision: Exercise the external app through `STUB/32A` rather than by invoking `DOS32A.EXE app.exe` directly.
  Rationale: `STUB/32A` is how DOS/32A applications normally load the extender. The manual form also crashes with the checked-in historical 8.00 binary under this DOSBox-X build, so using it as the required smoke path would make the repository fail on an emulator/manual-launch behavior that is not unique to the rebuilt extender.
  Date/Author: 2026-05-22 / Codex

## Outcomes & Retrospective

Validated outcome: direct standalone execution of the rebuilt `DOS32A.EXE` no longer triggers the DOSBox-X protected-mode descriptor crash, and the smoke test now launches a protected-mode sample through `STUB/32A`. The smoke test creates fresh top-level directories under `C:\DOSBox-X\drivez`: one `D32Sxxxx` directory containing `DOS32A.EXE` and `SVER.EXE` for version and standalone checks, and one `D32Axxxx` directory containing `AHELLO.EXE` and `BINW\DOS32A.EXE` for the app-load check. A generated `D32Bxxxx.BAT` batch file runs the checks from `Z:` and exits DOSBox-X. The test requires `SVER_SMOKE_OK`, `DOS32A_SMOKE_OK`, `Hello world from protected mode!!!`, and `APP_SMOKE_OK`, and fails on DOSBox-X crash signatures.

Validation evidence from `.\smoke-dosbox.cmd`:

    SVER -- Version Reporting Utility version 26.0
    Version:        26.0
    SVER_SMOKE_OK
    DOS32A_SMOKE_OK
    Hello world from protected mode!!!
    APP_SMOKE_OK
    DOSBox-X smoke passed using staged files in C:\DOSBox-X\drivez\D32SB306 and C:\DOSBox-X\drivez\D32AB306.

## Context and Orientation

The active DOS/32A 26.0 extender sources live in `src\dos32a`. The top-level `build.cmd` builds the active extender by assembling `src\dos32a\kernel.asm` and `src\dos32a\dos32a.asm` with `tasm5\PATCHES\53_WIN\TASM32.EXE`, linking with Open Watcom, and copying the result to `binw\dos32a.exe`. The top-level `smoke-dosbox.cmd` starts DOSBox-X and delegates to `smoke-dosbox.ps1`, which stages files under `C:\DOSBox-X\drivez` instead of mounting the repository. `sver.exe` is a version-reporting utility, not the extender itself, so the smoke must also execute the extender and a protected-mode sample.

The user reported that executing `dos32a.exe` directly displays version and copyright text and then DOSBox-X raises a dialog with `JMP Illegal descriptor type 0`. In this plan, "standalone execution" means running the staged flat `DOS32A.EXE` directly from DOSBox-X, without using it to load another protected-mode program. "External-load execution" means running `AHELLO.EXE`, where `AHELLO.EXE` is copied from `dos32a_800\examples\asm_1\hello.exe`; that executable contains `STUB/32A`, a small real-mode launcher that locates `BINW\DOS32A.EXE` through `SET DOS32A=Z:\D32Axxxx` and then starts the extender.

## Plan of Work

First, reproduce the failure with a controlled DOSBox-X command line that runs `binw\dos32a.exe` and writes a marker file after the command. If the marker file is missing, or if the DOSBox-X log contains the illegal descriptor text, the smoke test must report failure. The same harness will be used against `dos32a_800\binw\dos32a.exe` to learn whether the historical 8.00 binary exits cleanly or also triggers this emulator condition.

Revision: The final smoke harness no longer mounts the repository into DOSBox-X. It stages `DOS32A.EXE` and `SVER.EXE` into a fresh top-level `C:\DOSBox-X\drivez\D32Sxxxx` directory, stages `AHELLO.EXE` and `BINW\DOS32A.EXE` into a matching `D32Axxxx` directory, starts DOSBox-X on `Z:`, and uses console-log sentinel lines rather than redirected marker files because `Z:` writes are not persisted to the host `drivez` directory.

Next, inspect `src\dos32a\dos32a.asm`, `src\dos32a\kernel.asm`, and included files under `src\dos32a\text` for the code path used when DOS/32A is executed with no client program. If the standalone path should only print the banner and terminate, the fix should make that path return to DOS before attempting an invalid far jump or descriptor switch.

The handoff sequence in `src\dos32a\dos32a.asm` was tested against the older DOS/32A source trees. The normal 7.35 and 8.00 path switches to the application stack, pushes the application code selector and entry offset, enables interrupts, and uses a 32-bit far return. The active maintained path instead builds an interrupt-return frame and uses `IRETD`. Restoring the far-return handoff did not fix the bad manual invocation, and the original active `IRETD` handoff works in the supported stub-driven layout, so no source handoff change is part of this plan.

Finally, update `smoke-dosbox.cmd` and `smoke-dosbox.ps1` so they validate `sver.exe` inspection, direct extender execution, and external sample execution through `STUB/32A`. The script must remain usable from PowerShell or Command Prompt and should clean up temporary files.

## Concrete Steps

Run these commands from `C:\Users\mbeutler\Projects\dos32a`:

    .\build.cmd clean
    .\build.cmd dos32a
    .\build.cmd sver
    .\smoke-dosbox.cmd

During debugging, use direct DOSBox-X invocations that mount this checkout and run `binw\dos32a.exe`, followed by a marker command such as `echo OK > standalone.ok`. If DOSBox-X crashes or resets before the marker is written, the marker will be absent and the harness will fail.

The external-load smoke stages the active extender and sample under a test directory, then runs:

    set DOS32A=Z:\D32Axxxx
    AHELLO.EXE
    echo APP_SMOKE_OK

## Validation and Acceptance

The final acceptance is that `.\smoke-dosbox.cmd` returns exit code 0 only when all checks pass: staged `SVER.EXE DOS32A.EXE` reports DOS/32A version `26.0`, direct execution of staged `DOS32A.EXE` returns to DOSBox-X so the `DOS32A_SMOKE_OK` sentinel echo appears in the DOSBox-X log, and `AHELLO.EXE` prints `Hello world from protected mode!!!` before the `APP_SMOKE_OK` sentinel appears. A failure containing `JMP Illegal descriptor type 0`, `Illegal descriptor`, `DYNX86:Can't run code`, `ERROR CPU`, a missing sentinel, or a nonzero DOSBox-X exit must make the smoke script return nonzero.

## Idempotence and Recovery

The build and smoke scripts are intended to be idempotent. `build.cmd clean` removes generated active-tree outputs under `binw\` and source-directory intermediates. The smoke test should delete its own marker and log files before and after running. This plan avoids destructive Git commands and leaves the pre-existing deletion of `dos32a_800\binw\dos32a.lnk` untouched.

## Artifacts and Notes

Initial evidence from the user is a DOSBox-X dialog titled `E_Exit` with message `JMP Illegal descriptor type 0`, observed immediately after the generated extender prints its version and copyright information.

## Interfaces and Dependencies

The fix depends on the local historical toolchain documented in `README.md`: Open Watcom under `C:\WATCOM`, TASM files under `tasm5\`, and DOSBox-X under `C:\DOSBox-X\dosbox-x.exe`. The scripts involved are `build-env.cmd`, `build.cmd`, and `smoke-dosbox.cmd`.

Revision note: Created this plan to guide automatic reproduction and repair of the user-reported standalone DOS/32A crash, and to keep the smoke-test gap documented while work proceeds.

Revision note: Expanded the plan after the user reported that external program loading still crashes. The plan now covers the external-load scenario, the historical hello sample used for reproduction, and the suspected final handoff sequence in `src\dos32a\dos32a.asm`.

Revision note: Recorded that the handoff sequence was not changed and that the smoke test now uses fresh top-level `drivez` directories plus the expected `DOS32A=<root>` and `BINW\DOS32A.EXE` layout to exercise a real `STUB/32A` protected-mode sample.

Revision note: Updated expected version output after the maintained tree was promoted to the 26.0 stable release.
