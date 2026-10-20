# Loader

The driver (`Larp.sys`) is unsigned and will not load through the normal
Service Control Manager under Windows x64 without `testsigning`. For a
clean deployment, map it through a vulnerable-driver mapper.

## Options

- **kdmapper** — original by TheCruZ, several maintained forks on GitHub.
  Usage: `kdmapper.exe path\to\Larp.sys`
- **Any forked kdmapper** using `iqvw64e.sys` (Intel), `RTCore64.sys` (MSI),
  `dbutil_2_3.sys` (Dell) or similar BYOVD primitive.

## Dev-box testing (unsigned load, no mapper)

Open an elevated prompt:

```
bcdedit /set testsigning on
bcdedit /debug on
shutdown /r /t 0
```

Then after reboot create a service pointing at your `.sys`:

```
sc create Larp type= kernel binPath= C:\path\to\Larp.sys
sc start Larp
```

Note: with Hyperion in play on the target process, dev-cert loading is
visible. Prefer the mapper for anything real.
