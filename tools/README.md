# tools

Small utilities that live alongside the main product.

## LarpCompatCheck.exe

Pre-flight compatibility checker. Ship this on the storefront page as a
free download so prospective buyers can confirm their system can run the
tool before purchasing.

Checks:

- running as administrator
- Windows 10 1803 or later / Windows 11
- Memory Integrity (HVCI) is off
- Vulnerable driver blocklist is off
- (advisory) Windows Defender exclusion reminder

Exit code `0` means all required checks passed; `1` means at least one
required item needs attention.

Built as part of the main CMake project:

```
cmake -B build -A x64
cmake --build build --config Release --target LarpCompatCheck
```

Output lands at `build/Release/LarpCompatCheck.exe`.
