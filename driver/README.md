# Larp Driver

Minimal kernel driver exposing read/write primitives for the usermode controller.

## Build

Open in Visual Studio with the WDK installed:

- New Project → Kernel Mode Driver, Empty (KMDF or WDM — this uses WDM-style)
- Add `driver.c`, `comm.c`, `comm.h`
- Target x64, Release
- Output: `Larp.sys`

## Load

Not meant to be signed. Load via a vulnerable-driver manual mapper (kdmapper
or your own). For dev-box testing you can use `bcdedit /set testsigning on`
and a test cert.

## IOCTLs

- `IOCTL_LARP_PID`    — resolve process name to pid
- `IOCTL_LARP_BASE`   — module base in a given process
- `IOCTL_LARP_READ`   — read process memory
- `IOCTL_LARP_WRITE`  — write process memory

Device: `\\.\Larp`
