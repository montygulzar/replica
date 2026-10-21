#include "watchdog.h"
#include <chrono>

void Watchdog::set_item(const ItemPreset* p) {
    std::lock_guard<std::mutex> lk(m_mu);
    m_item = p;
    // Reset the baseline so the next tick re-applies even if the
    // character pointer hasn't changed yet.
    m_lastCharacter = 0;
}

void Watchdog::start() {
    if (m_run.exchange(true)) return;
    m_thread = std::thread([this] { loop(); });
}

void Watchdog::stop() {
    if (!m_run.exchange(false)) return;
    if (m_thread.joinable()) m_thread.join();
}

void Watchdog::loop() {
    using namespace std::chrono_literals;
    while (m_run.load()) {
        uintptr_t ch = rbx.character();
        const ItemPreset* item = nullptr;
        {
            std::lock_guard<std::mutex> lk(m_mu);
            item = m_item;
            if (ch && ch != m_lastCharacter && item) {
                m_lastCharacter = ch;
            } else {
                item = nullptr;  // nothing to do this tick
            }
        }
        if (item) {
            ps.apply_item(rbx, *item);
        }
        std::this_thread::sleep_for(1500ms);
    }
}
