#include "roblox.h"
#include "presets.h"
#include "ui.h"
#include "watchdog.h"
#include <cstdio>

int wmain() {
    Driver drv;
    if (!drv.open()) {
        wprintf(L"driver not loaded (\\\\.\\Larp missing)\n");
        return 1;
    }
    if (!drv.attach(L"RobloxPlayerBeta.exe", L"RobloxPlayerBeta.exe")) {
        wprintf(L"roblox not running\n");
        return 1;
    }

    Roblox rbx(drv);
    Presets ps;
    ps.load_themes("presets/themes.json");
    ps.load_items("presets/items.json");

    Watchdog wd(rbx, ps);
    wd.start();

    run_ui(drv, rbx, ps, wd);

    wd.stop();
    return 0;
}
