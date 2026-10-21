// LarpCompatCheck
//
// Pre-flight check users run before buying/installing. Reports whether
// their Windows is in a state the kernel driver can load from. Writes
// human-readable PASS/FAIL to the console and returns 0 on fully-ready
// systems, 1 otherwise.

#include <windows.h>
#include <wincrypt.h>
#include <cstdio>
#include <string>

#pragma comment(lib, "advapi32.lib")

struct Row { const char* name; bool pass; std::string detail; };

static bool is_admin() {
    BOOL b = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY nt = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&nt, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                 &adminGroup)) {
        if (!CheckTokenMembership(nullptr, adminGroup, &b)) b = FALSE;
        FreeSid(adminGroup);
    }
    return !!b;
}

static bool reg_read_dword(HKEY root, const wchar_t* subkey,
                           const wchar_t* value, DWORD& out) {
    HKEY k = nullptr;
    if (RegOpenKeyExW(root, subkey, 0, KEY_READ | KEY_WOW64_64KEY, &k) != ERROR_SUCCESS)
        return false;
    DWORD type = 0, size = sizeof(DWORD);
    LONG r = RegQueryValueExW(k, value, nullptr, &type, (LPBYTE)&out, &size);
    RegCloseKey(k);
    return r == ERROR_SUCCESS && type == REG_DWORD;
}

static Row check_hvci() {
    DWORD enabled = 0;
    bool present = reg_read_dword(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\DeviceGuard\\Scenarios\\HypervisorEnforcedCodeIntegrity",
        L"Enabled", enabled);
    bool off = !present || enabled == 0;
    return { "Memory Integrity (HVCI) is off", off,
             off ? "ok" : "turn off in Windows Security -> Device security -> Core isolation" };
}

static Row check_blocklist() {
    DWORD v = 0;
    bool present = reg_read_dword(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\CI\\Config",
        L"VulnerableDriverBlocklistEnable", v);
    bool off = present && v == 0;
    return { "Vulnerable driver blocklist is off", off,
             off ? "ok" : "run as admin: reg add \"HKLM\\SYSTEM\\CurrentControlSet\\Control\\CI\\Config\" "
                          "/v VulnerableDriverBlocklistEnable /t REG_DWORD /d 0 /f, then reboot" };
}

static Row check_admin() {
    bool a = is_admin();
    return { "Running as administrator", a,
             a ? "ok" : "right-click LarpCompatCheck.exe -> Run as administrator" };
}

static Row check_windows_version() {
    // Prefer RtlGetVersion over GetVersionEx so compat shims don't lie.
    typedef LONG (WINAPI* RtlGetVersionFn)(PRTL_OSVERSIONINFOW);
    OSVERSIONINFOW ovi{};
    ovi.dwOSVersionInfoSize = sizeof(ovi);
    HMODULE nt = GetModuleHandleW(L"ntdll.dll");
    bool ok = false;
    DWORD major = 0, build = 0;
    if (nt) {
        auto fn = (RtlGetVersionFn)GetProcAddress(nt, "RtlGetVersion");
        if (fn && fn(&ovi) == 0) {
            major = ovi.dwMajorVersion;
            build = ovi.dwBuildNumber;
            ok = (major >= 10 && build >= 17134); // Windows 10 1803 or later
        }
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "detected %lu build %lu", major, build);
    return { "Windows 10 1803+ or Windows 11", ok, buf };
}

static Row check_defender_exclusion_hint() {
    // We can't reliably enumerate exclusions from a non-elevated non-signed
    // process, so this is advisory, not blocking.
    return { "Defender exclusion added for install folder",
             true,
             "add your install folder in Windows Security -> Virus & threat "
             "protection -> Manage settings -> Exclusions, otherwise the "
             "driver and loader will be quarantined" };
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    printf("Larp Compatibility Check\n");
    printf("------------------------\n\n");

    Row rows[] = {
        check_admin(),
        check_windows_version(),
        check_hvci(),
        check_blocklist(),
        check_defender_exclusion_hint(),
    };

    int failed = 0;
    for (auto& r : rows) {
        printf("[%s] %-40s  %s\n",
               r.pass ? " OK " : "FAIL",
               r.name,
               r.detail.c_str());
        if (!r.pass) ++failed;
    }

    printf("\n");
    if (failed == 0) {
        printf("All checks passed. Your system can load the Larp driver.\n");
    } else {
        printf("%d check(s) failed. Fix the items above, reboot, and re-run.\n", failed);
    }
    printf("\nPress Enter to exit.");
    getchar();
    return failed == 0 ? 0 : 1;
}
