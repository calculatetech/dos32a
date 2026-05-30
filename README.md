# DOS/32 Advanced DOS Extender

This repository preserves and maintains the DOS/32 Advanced DOS Extender
source. The active tree is the DOS/32A 26.1.2 maintenance branch under `src/`, with
older 7.35 and 8.00 trees kept as historical references.

DOS/32A is a 32-bit DOS extender and a drop-in replacement for DOS/4GW and
compatible extenders. The maintenance goal is to combine useful fixes and
behavior from the 7.35, 8.00, and 9.1.2 releases: HDPMI32 compatibility for
linear frame buffer write-combining, the 8.00 speed-loop changes, and the 9.x
SSE support. Version 26.0 was the first stable release from this maintained
line; version 26.1.2 carries low-risk compatibility and code-correctness fixes
on top of it.

## Repository Layout

- `src/dos32a/` - active DOS/32A extender assembly sources.
- `src/dos32a/text/` - included kernel/client assembly text.
- `src/stub32a/` - active STUB/32A and STUB/32C assembly sources.
- `src/sb/` - SUNSYS Bind Utility sources.
- `src/sc/` - SUNSYS Compress Utility sources.
- `src/ss/` - SUNSYS Setup Utility sources.
- `src/sver/` - SUNSYS Version Utility sources.
- `dos32a-735-src/` - historical 7.35 source tree.
- `dos32a_800/` - historical 8.00 reference distribution, including binaries,
  documentation, examples, and sources.

Useful references include `ChangeLog`, archived readmes in the source trees,
and the HTML documentation under `dos32a_800\docs\`.

## Windows Build Setup

The active 26.1.2 sources are built with the historical DOS toolchain family.
This checkout provides Windows batch wrappers so the tools do not need to be
added to the machine-wide `PATH`.

Required local tools:

- Open Watcom installed at `C:\WATCOM`. If it is installed somewhere else, set
  `WATCOM` before running the build.
- TASM 5.3 setup/patch files under `tasm5\` in this checkout. The build uses
  `tasm5\PATCHES\53_WIN\TASM32.EXE`.
- DOSBox-X installed at `C:\DOSBox-X\dosbox-x.exe` for runtime smoke tests. If
  it is installed somewhere else, set `DOSBOX_X` before running the smoke test.
  Use a DOSBox-X build with integrated DOS; the OS-Free build requires a
  separate bootable DOS image and will not run the provided smoke test directly.

From a Windows Command Prompt or PowerShell in the repository root, verify the
expected tool paths:

```bat
dir C:\WATCOM\BINNT\WCL.EXE
dir tasm5\PATCHES\53_WIN\TASM32.EXE
dir C:\DOSBox-X\dosbox-x.exe
```

To configure an interactive Command Prompt for ad-hoc tool use, run:

```bat
build-env.cmd
```

When calling it from another batch file, use `call build-env.cmd` so control
returns to the caller. PowerShell users can run `build.cmd` and
`smoke-dosbox.cmd` directly; environment changes made by `build-env.cmd` do not
persist back into a parent PowerShell session.

## Building

Build all active 26.1.2 components:

```bat
build.cmd
```

The build writes generated binaries to `BINW\` using uppercase DOS-style file
names and removes source-directory intermediates. Individual components can be
built with:

```bat
build.cmd dos32a
build.cmd stub32a
build.cmd sb
build.cmd sc
build.cmd ss
build.cmd sver
```

Remove generated outputs and intermediates with:

```bat
build.cmd clean
```

The build wrapper uses the archived 8.00 `h32\` and `src\sutils\misc\` include
files already in this checkout. The original 9.1.2 readme states that building
requires the v7.1 SDK, and the historical 8.00 source documentation records
Borland Turbo Assembler 5.x and Watcom C/C++ as the toolchain family used for
that line.

The checked-in `tasm5\` directory contains third-party historical assembler
tool media and patch files used only to reproduce this build environment. Those
files are not part of DOS/32A and are not covered by the DOS/32A license.

Do not assume a modern host compiler can reproduce the historical binaries.
Any build notes should identify the exact SDK, TASM and Watcom versions, host
environment, DOS or DPMI runtime, and source tree used.

## Runtime Smoke Test

After `build.cmd`, run:

```bat
smoke-dosbox.cmd
```

The smoke test stages temporary files under `C:\DOSBox-X\drivez` and runs:

```bat
SVER.EXE DOS32A.EXE
DOS32A.EXE HELLO.EXE
AHELLO.EXE
```

A successful run reports the generated DOS/32A version, for example:

```text
DOS/32 Advanced DOS Extender:
Version:        26.1.2
```

## Validation

This checkout has one scripted DOSBox-X smoke test. It is useful for checking
the build products and simple loader paths, but extender compatibility still
requires explicit manual validation on the relevant DOS, DPMI, or real-hardware
runtime.

For utility changes, document the exact command path exercised and the files
used as input and output. For extender or stub changes, record the affected
binary path, the DOS/DPMI runtime, host environment, and observed runtime
behavior. Low-level extender, binding, compression, and generated binary
changes should be treated as compatibility-sensitive.

## License

The active source is distributed under the DOS/32 Advanced DOS Extender
Software License. See `LICENSE` for the complete terms.
