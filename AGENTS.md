# Repository Guidelines

## Project Structure

This repository contains the DOS/32 Advanced source tree. Top-level reference files are `README`, `ChangeLog`, and `LICENSE`; implementation sources live under `src/`.

- `src/dos32a/` contains the DOS/32A extender assembly.
- `src/dos32a/text/` contains included kernel/client assembly text.
- `src/stub32a/` contains stub assembly.
- `src/sb/`, `src/sc/`, `src/ss/`, and `src/sver/` contain the bind, compress, setup, and version utilities.

## Build, Test, and Development Commands

This checkout does not include a Makefile, CMake project, build script, or automated test suite. The `README` states that building requires the historical v7.1 SDK. Do not add or document modern build commands unless the corresponding project files are introduced.

Useful inspection commands:

```sh
find src -maxdepth 2 -type f | sort
git status --short
```

## Coding Style and Naming

Preserve the existing source style. C files keep their large license headers, tab indentation, and compact K&R-era formatting. Assembly files use uppercase directives and labels, semicolon comments, and existing filename/module naming. Keep edits local to the relevant utility or extender component, and avoid broad formatting churn.

## Testing Guidance

Because no automated tests are present, validation should be explicit and reproducible. For source changes, document the exact SDK/toolchain, host environment, and DOS/DPMI runtime used for any build or manual exercise. For utilities, verify the specific command path touched. For extender or stub changes, record the affected binary path and any observed runtime behavior.

## Commit and PR Guidance

Git history currently has no commits on `master`, so there is no observed local commit convention. Use concise imperative commit messages, for example `Preserve stub relocation flags`. PRs should summarize the affected component, explain validation performed or why it was not possible, and call out any binary compatibility, SDK, or runtime assumptions.

## Security and License Notes

Retain existing license headers in C and assembly files and defer to the top-level `LICENSE`. Treat generated binaries, compression/bind behavior, and low-level extender changes as security-sensitive. Do not add third-party code, binary blobs, or generated artifacts without documenting provenance and license compatibility.
