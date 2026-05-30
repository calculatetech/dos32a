# Repository Guidelines

## Project Structure

This repository contains the DOS/32 Advanced source tree. Top-level reference files are `README`, `ChangeLog`, and `LICENSE`; implementation sources live under `src/`.

- `src/dos32a/` contains the DOS/32A extender assembly.
- `src/dos32a/text/` contains included kernel/client assembly text.
- `src/stub32a/` contains stub assembly.
- `src/sb/`, `src/sc/`, `src/ss/`, and `src/sver/` contain the bind, compress, setup, and version utilities.

## Build, Test, and Development Commands

This checkout includes Windows batch wrappers for the historical Open Watcom and TASM toolchain. Use the checked-in scripts rather than introducing a modern build system.

Useful inspection commands:

```sh
find src -maxdepth 2 -type f | sort
git status --short
```

Useful Windows build and smoke commands:

```bat
build.cmd
build.cmd clean
smoke-dosbox.cmd -TimeoutSeconds 10
```

## Coding Style and Naming

Preserve the existing source style. C files keep their large license headers, tab indentation, and compact K&R-era formatting. Assembly files use uppercase directives and labels, semicolon comments, and existing filename/module naming. Keep edits local to the relevant utility or extender component, and avoid broad formatting churn.

DOS-visible filenames and paths in scripts, documentation, generated artifacts, patch bundles, and validation output must be uppercase, for example `C:\DOS32A\BINW\DOS32A.EXE`. Do not rename historical host-side source directories only for casing; preserve existing repository paths unless a functional DOS-facing path is being changed.

Follow `.gitattributes` for line endings. Keep broad line-ending normalization in a dedicated mechanical commit, separate from logic changes, so diffs remain reviewable.

## Testing Guidance

Because automated coverage is limited to the DOSBox-X smoke wrapper, validation should be explicit and reproducible. For source changes, document the exact SDK/toolchain, host environment, and DOS/DPMI runtime used for any build or manual exercise. For utilities, verify the specific command path touched. For extender or stub changes, record the affected binary path and any observed runtime behavior.

Every change set must receive a clean-context adversarial review against the active codebase before commit or release. Run the reviewer without relying on the current conversational context, instruct it to make no edits, and ask it to focus on dormant code, obvious bugs, inconsistencies, deviations from original style, DOS-visible path/name casing, stale docs, line-ending churn, and regression risk. Verify each finding locally before acting on it or reporting it as accepted risk.

## Commit and PR Guidance

Git history currently has no commits on `master`, so there is no observed local commit convention. Use concise imperative commit messages, for example `Preserve stub relocation flags`. PRs should summarize the affected component, explain validation performed or why it was not possible, and call out any binary compatibility, SDK, or runtime assumptions.

## Security and License Notes

Retain existing license headers in C and assembly files and defer to the top-level `LICENSE`. Treat generated binaries, compression/bind behavior, and low-level extender changes as security-sensitive. Do not add third-party code, binary blobs, or generated artifacts without documenting provenance and license compatibility.
