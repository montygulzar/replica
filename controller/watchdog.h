#pragma once
#include "roblox.h"
#include "presets.h"
#include <atomic>
#include <mutex>
#include <thread>

// Some Roblox places strip the local HumanoidDescription on spawn and
// rebuild it from server state, which wipes any item swap the moment the
// character respawns. The Watchdog polls the Character pointer; when it
// changes (respawn, teleport, team switch), it re-applies the last-chosen
// item preset so the swap sticks across lifecycles.
class Watchdog {
public:
    Watchdog(Roblox& r, Presets& p) : rbx(r), ps(p) {}
    ~Watchdog() { stop(); }

    void set_item(const ItemPreset* p);  // nullptr clears
    void start();
    void stop();

private:
    void loop();

    Roblox&  rbx;
    Presets& ps;

    std::mutex              m_mu;
    const ItemPreset*       m_item = nullptr;
    uintptr_t               m_lastCharacter = 0;

    std::thread             m_thread;
    std::atomic<bool>       m_run{false};
};
