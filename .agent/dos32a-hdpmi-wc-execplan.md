# DOS/32A HDPMI/WC Merge ExecPlan

- [x] Add HDPMI32 detection before the IOPL/XMS early skip.
- [x] Restore configurable DPMI/VCPI versus VCPI/DPMI detection order.
- [x] Re-enable `/DPMITST:ON|OFF` with 9.1.2 `bptr` configuration writes.
- [x] Restore setup display/help for the first kernel option and leave the second option reserved.
- [x] Port only the approved `dec ecx`/`jnz` loop speed changes.
- [x] Preserve existing VCPI PIC validation, `fastimul`, and VCPI page attribute behavior.
