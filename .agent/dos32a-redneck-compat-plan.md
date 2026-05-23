# Fix DOS/32A DOS4GW Replacement Startup With Redneck Rampage

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds.

This document follows `.agent/PLANS.md` from the repository root. It is self-contained so a future contributor can continue from the current working tree and the staged Redneck Rampage files under `C:\DOSBox-X\drivez\REDNECK`.

## Purpose / Big Picture

The user is using this repository's rebuilt DOS/32A extender as a drop-in `DOS4GW.EXE` replacement for Redneck Rampage. A small smoke test originally passed, but it missed the real failure: running another protected-mode executable through `DOS32A.EXE`, or launching Redneck Rampage through the renamed extender, crashed or hard-locked immediately after the DOS/32A banner. After this change, the rebuilt extender can load a child protected-mode program through the external-loader path, and the Redneck Rampage startup path reaches the expected sound-configuration failure instead of silently crashing.

The user-visible proof is not merely returning to DOS. For Redneck Rampage, the accepted DOSBox-X log signature is that `RR.EXE` reaches `KB_Startup`, compiles `GAME.CON`, reads `REDNECK.CFG`, prints `Checking sound inits`, then prints `Playback failed, possibly due to an invalid or conflicting IRQ.`, followed by `Returned from rr.exe`. A bare `Returned from rr.exe` without the preceding game startup and IRQ message is a failure because the game can silently crash early.

## Progress

- [x] (2026-05-22) Confirmed the working tree and remote baseline before starting the Redneck-focused debugging work.
- [x] (2026-05-22) Verified `C:\DOSBox-X\drivez\REDNECK` contains the user's real test case. `RAMPAGE.BAT` runs `bmouse launch rr.exe`; `DOS4GW.4gw` is the original extender; `DOS4GW.EXE` is the replacement slot used for this repository's DOS/32A build.
- [x] (2026-05-22) Reproduced that the earlier smoke test was insufficient. It tested the standalone banner and the `STUB/32A` sample path but did not run `DOS32A.EXE HELLO.EXE`, the external-loader path that was crashing.
- [x] (2026-05-22) Fixed the Open Watcom link layout in `build.cmd` by adding `option packcode=1,packdata=1` and suppressing WLINK warning 2083. This keeps the historical DOS/32A segments separated enough for the raw protected-mode transfer code.
- [x] (2026-05-22) Expanded `smoke-dosbox.ps1` so it stages test files under `C:\DOSBox-X\drivez`, runs `DOS32A.EXE HELLO.EXE`, validates `DIRECT_LOADER_SMOKE_OK`, enables execution logging, and deletes its temporary `D32*` staging files on normal pass/fail exits.
- [x] (2026-05-22) Reproduced the remaining Redneck Rampage failure after the link-layout fix. Direct external loading worked, but Redneck still failed before the real game startup signature.
- [x] (2026-05-22) Narrowed the Redneck failure with temporary, reverted binary probes in `RR.EXE`. The crash happens in Redneck startup helper code around object offset `0x55768` after it asks DPMI function `INT 31h AX=0300h` to simulate real-mode `INT 2Fh AX=1510h`, an MSCDEX CD-ROM device-driver request.
- [x] (2026-05-22) Implemented a narrow kernel fix in `src/dos32a/text/kernel/int31h.asm`: when protected-mode code requests simulated real-mode interrupt `2Fh` with AX `1510h`, mark the caller's MSCDEX request packet as failed and return DPMI success instead of reflecting into the real-mode handler path that corrupts the caller state.
- [x] (2026-05-22) Rebuilt the full tree, copied `binw\dos32a.exe` to `C:\DOSBox-X\drivez\REDNECK\DOS4GW.EXE`, and validated that Redneck reaches the accepted startup and IRQ-failure signature within a 10 second DOSBox-X run.
- [x] (2026-05-22) Confirmed no DOSBox-X processes needed for testing should be left running, and future iteration tests should use a timeout of 10 seconds or less.

## Surprises & Discoveries

- Observation: Returning to the DOS prompt is not enough to prove Redneck works.
  Evidence: The user observed that `Returned from rr.exe` without the IRQ error can be a silent early game crash. Acceptance now requires the preceding `KB_Startup`, setup/CON output, `Checking sound inits`, and IRQ playback failure text.
- Observation: Open Watcom 2 links the DOS/32A extender differently from the historical environment unless code and data packing are constrained.
  Evidence: With the default link line, running `DOS32A.EXE HELLO.EXE` could crash immediately after the banner. With `-"option packcode=1,packdata=1"` and separated segments, the same loader path prints `Hello world from protected mode!!!` and returns.
- Observation: WLINK warning 2083 becomes fatal enough to stop the build when the segments are separated.
  Evidence: The build succeeds only after adding `-"disable 2083"` beside the packing option. The warning corresponds to historical cross-segment offset fixups used by this extender code.
- Observation: Redneck's remaining crash was not a generic DOS/4GW handoff problem after the link fix.
  Evidence: Temporary `EB FE` loop probes showed execution reached Redneck helper code around `0x55768` and failed after the simulated `INT 2Fh AX=1510h` request. Temporarily patching that helper to `xor eax,eax; ret` let the game reach the accepted IRQ-failure signature; the temporary game patch was restored and is not part of the repository change.
- Observation: DOSBox-X may print `ERROR FPU:8087 only fpu code used esc 3: group 4: subfunction :1` during the passing Redneck run.
  Evidence: The final accepted run still printed that line before `KB_Startup`, so it is not the failure being fixed here.

## Decision Log

- Decision: Treat Redneck Rampage as the primary compatibility target in addition to the small protected-mode sample.
  Rationale: The small sample validates one protected-mode path, but the user's real use case is a DOS/4GW-stubbed game using a renamed DOS/32A executable.
  Date/Author: 2026-05-22 / Codex
- Decision: Put all automated DOSBox-X smoke staging under `C:\DOSBox-X\drivez` instead of mounting repository directories.
  Rationale: The user explicitly requested `drivez` staging, and it avoids DOSBox mount behavior from becoming a variable in extender tests.
  Date/Author: 2026-05-22 / Codex
- Decision: Add direct external-loader coverage to `smoke-dosbox.ps1`.
  Rationale: The original smoke missed `DOS32A.EXE HELLO.EXE`, which was the critical crash point for the standalone extender loading another executable.
  Date/Author: 2026-05-22 / Codex
- Decision: Fix the Open Watcom linker layout in `build.cmd` rather than patching generated binaries.
  Rationale: The raw DOS/32A protected-mode transition expects historical segment placement. Keeping code and data segments separated produces reproducible binaries from source and preserves the documented build workflow.
  Date/Author: 2026-05-22 / Codex
- Decision: Handle only `INT 31h AX=0300h` simulated real-mode interrupt `2Fh` with register AX `1510h` specially.
  Rationale: This is the request Redneck makes before the crash. Marking the MSCDEX request packet as failed tells the caller the CD-ROM device-driver path is unavailable, which is compatible with a no-CD or failed-CD startup path and avoids changing general interrupt reflection behavior.
  Date/Author: 2026-05-22 / Codex

## Outcomes & Retrospective

The repository now builds a DOS/32A executable that passes the direct external-loader smoke and reaches Redneck Rampage's expected startup flow when installed as `C:\DOSBox-X\drivez\REDNECK\DOS4GW.EXE`. The work found two separate issues: a modern Open Watcom link layout mismatch that broke `DOS32A.EXE program.exe`, and a Redneck-specific MSCDEX simulated-interrupt path that could corrupt caller state and triple-fault or hard-lock. The remaining DOSBox-X FPU warning is logged but does not block the accepted Redneck behavior. The user later confirmed the fixed extender works on real hardware, write combining is fixed there, and Redneck Rampage gains about 20 FPS; that result promoted the maintained tree to the 26.0 stable release.

## Context and Orientation

The active extender source lives under `src\dos32a`. The file `src\dos32a\text\kernel\int31h.asm` implements DPMI interrupt `31h` services. DPMI is the DOS Protected Mode Interface, the API that lets protected-mode DOS programs ask the extender to perform services such as simulating a real-mode interrupt. Real mode is the original 16-bit DOS execution mode; protected mode is the 32-bit CPU mode used by Watcom-era games. MSCDEX is the DOS CD-ROM extension. Its multiplex interrupt is `INT 2Fh`, and function AX `1510h` asks a CD-ROM device driver to process a request packet.

The build script is `build.cmd`. It uses TASM32 and Open Watcom to assemble and link `src\dos32a\dos32a.asm` and `src\dos32a\kernel.asm` into `binw\dos32a.exe`. That executable can be copied and renamed to `DOS4GW.EXE` so DOS/4GW-stubbed programs attempt to use DOS/32A as a replacement extender.

The automated smoke entry point is `smoke-dosbox.cmd`, which calls `smoke-dosbox.ps1`. It uses DOSBox-X from `C:\DOSBox-X`, stages files under `C:\DOSBox-X\drivez`, and runs in the DOSBox `Z:` drive. The real game test is outside the repository at `C:\DOSBox-X\drivez\REDNECK`; its launcher is `RAMPAGE.BAT`, and the main program is `RR.EXE`.

## Plan of Work

Keep the linker command in `build.cmd` configured with `-"disable 2083"` and `-"option packcode=1,packdata=1"` when linking `dos32a.obj kernel.obj`. This preserves the segment layout needed by the DOS/32A transfer code when built with the installed Open Watcom toolchain.

Keep the smoke expansion in `smoke-dosbox.ps1`. It must copy `DOS32A.EXE`, `SVER.EXE`, and `HELLO.EXE` into temporary `D32*` directories below `C:\DOSBox-X\drivez`, run `SVER.EXE DOS32A.EXE`, run `DOS32A.EXE HELLO.EXE`, run bare `DOS32A.EXE`, and run the stubbed sample `AHELLO.EXE`. It must validate the `DIRECT_LOADER_SMOKE_OK` marker so a future external-loader regression is caught.

Keep the `int31h_0300` change in `src\dos32a\text\kernel\int31h.asm`. Before reflecting a simulated real-mode interrupt, check whether BL is `2Fh` and the protected-mode register structure's AX value is `1510h`. If so, compute the linear address of the MSCDEX request packet from the register structure's ES:BX values, set the request packet status word at offset `+3` to `8000h`, restore registers, and return through `int31ok`. Otherwise, continue to the existing interrupt reflection path.

## Concrete Steps

Run commands from `C:\Users\mbeutler\Projects\dos32a`.

Build just the extender during quick iteration:

    cmd /c build.cmd dos32a

Build the full tree before committing:

    cmd /c build.cmd

Run the automated DOSBox-X smoke with a 10 second cap:

    cmd /c "smoke-dosbox.cmd -TimeoutSeconds 10"

Expected smoke evidence:

    SVER -- Version Reporting Utility version 26.0
    Version:        26.0
    SVER_SMOKE_OK
    Hello world from protected mode!!!
    DIRECT_LOADER_SMOKE_OK
    DOS32A_SMOKE_OK
    Hello world from protected mode!!!
    APP_SMOKE_OK
    DOSBox-X smoke passed using temporary staged files in C:\DOSBox-X\drivez\D32S... and C:\DOSBox-X\drivez\D32A...

Before the Redneck test, copy the freshly built extender into the replacement slot:

    Copy-Item -LiteralPath binw\dos32a.exe -Destination C:\DOSBox-X\drivez\REDNECK\DOS4GW.EXE -Force

Run Redneck through DOSBox-X from `Z:\REDNECK` with logging enabled and a timeout of 10 seconds or less. The test should be scripted so DOSBox-X is killed in a `finally` block if it is still running.

Expected Redneck evidence:

    ERROR FPU:8087 only fpu code used esc 3: group 4: subfunction :1
    KB_Startup: Keyboard Started
    Compiling: 'GAME.CON'.
    Code Size:49164 bytes(1325 labels).
    Using Setup file: 'REDNECK.CFG'
    CONTROL_Startup: External controller found on vector 60
    * Hold Esc to Abort. *
    Loading art header.
    Checking music inits.
    Checking sound inits.
    Playback failed, possibly due to an invalid or conflicting IRQ.
    Returned from rr.exe

## Validation and Acceptance

Acceptance requires all of these results:

- `cmd /c build.cmd` exits with code 0 and leaves outputs in `binw`.
- `cmd /c "smoke-dosbox.cmd -TimeoutSeconds 10"` exits with code 0 and includes `DIRECT_LOADER_SMOKE_OK`.
- Running Redneck with the rebuilt `binw\dos32a.exe` copied to `C:\DOSBox-X\drivez\REDNECK\DOS4GW.EXE` reaches the full accepted startup and IRQ-failure signature within 10 seconds.
- After tests, `Get-Process -Name dosbox-x -ErrorAction SilentlyContinue` returns no processes started by the test harness, and no temporary `D32*` smoke directories remain under `C:\DOSBox-X\drivez`.

## Idempotence and Recovery

The build commands can be rerun safely. The smoke script deletes its temporary staging files before it starts and on normal pass/fail exits. If an unexpected script exception leaves `C:\DOSBox-X\drivez\D32*` files behind, delete those scratch directories and logs; do not delete `C:\DOSBox-X\drivez\REDNECK`.

Do not permanently modify the user's original `DOS4GW.4gw`. The replacement slot `C:\DOSBox-X\drivez\REDNECK\DOS4GW.EXE` may be overwritten with the current repository build for validation because that is the user's intended test location.

Temporary `RR.EXE` binary probes are not part of the solution. If future debugging uses game binary patches, make a backup, restore it in a `finally` block, and verify the file hash or timestamp afterward.

## Artifacts and Notes

The final automated smoke passed with this concise evidence:

    SVER -- Version Reporting Utility version 26.0
    Version:        26.0
    SVER_SMOKE_OK
    Hello world from protected mode!!!
    DIRECT_LOADER_SMOKE_OK
    DOS32A_SMOKE_OK
    Hello world from protected mode!!!
    APP_SMOKE_OK

The final Redneck run passed the accepted signature. The sound setup failure is expected because the sound configuration is not valid in DOSBox-X:

    KB_Startup: Keyboard Started
    Checking sound inits.
    Playback failed, possibly due to an invalid or conflicting IRQ.
    Returned from rr.exe

## Interfaces and Dependencies

The local build depends on Open Watcom under `C:\WATCOM`, TASM 5.3 setup files under `tasm5`, and DOSBox-X under `C:\DOSBox-X`. The repository scripts used by this plan are `build.cmd`, `build-env.cmd`, `smoke-dosbox.cmd`, and `smoke-dosbox.ps1`. The source files modified by this plan are `build.cmd`, `smoke-dosbox.ps1`, and `src\dos32a\text\kernel\int31h.asm`.

Revision note: Replaced the stale plan text after the user clarified that a bare return from `rr.exe` is a failure. This revision records the real acceptance signature, the Open Watcom link-layout fix, the direct external-loader smoke coverage, and the MSCDEX simulated-interrupt compatibility fix.

Revision note: Updated expected smoke version output after the maintained tree was promoted to the 26.0 stable release.

Revision note: Recorded the user's real-hardware confirmation that Redneck Rampage works and gains about 20 FPS after the write-combining fix.
