#include "presets.h"
#include <fstream>
#include <nlohmann/json.hpp>

using nlohmann::json;

bool Presets::load_items(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    json j; f >> j;
    for (auto& e : j) {
        ItemPreset p;
        p.name     = e.value("name", "");
        p.shirtUrl = e.value("shirtUrl", "");
        p.pantsUrl = e.value("pantsUrl", "");
        m_items.push_back(p);
    }
    return true;
}

bool Presets::load_themes(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    json j; f >> j;
    for (auto& e : j) {
        ThemePreset t;
        t.name       = e.value("name", "");
        t.ambient    = { e["ambient"][0],  e["ambient"][1],  e["ambient"][2]  };
        t.fogColor   = { e["fogColor"][0], e["fogColor"][1], e["fogColor"][2] };
        t.fogStart   = e.value("fogStart",   0.f);
        t.fogEnd     = e.value("fogEnd",     1000.f);
        t.brightness = e.value("brightness", 2.f);
        t.clockTime  = e.value("clockTime",  14.f);
        m_themes.push_back(t);
    }
    return true;
}

void Presets::apply_item(Roblox& r, const ItemPreset& p) {
    if (!p.shirtUrl.empty()) r.set_shirt(p.shirtUrl);
    if (!p.pantsUrl.empty()) r.set_pants(p.pantsUrl);
}

void Presets::apply_theme(Roblox& r, const ThemePreset& t) {
    r.set_ambient(t.ambient);
    r.set_fog(t.fogColor, t.fogStart, t.fogEnd);
    r.set_brightness(t.brightness);
    r.set_clock_time(t.clockTime);
}
