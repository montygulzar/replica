#pragma once
#include "roblox.h"
#include <string>
#include <vector>

struct ItemPreset {
    std::string name;
    std::string shirtUrl;
    std::string pantsUrl;
    std::string faceUrl;
    std::string displayName;
    // Per-accessory-index hat/hair mesh overrides. Index 0 is the first
    // accessory parented to Character, index 1 the second, etc.
    std::vector<std::string> hatMeshes;
};

struct ThemePreset {
    std::string name;
    Color3 ambient{};
    Color3 fogColor{};
    float  fogStart = 0.f, fogEnd = 1000.f;
    float  brightness = 2.f;
    float  clockTime = 14.f;
};

class Presets {
public:
    bool load_items(const std::string& path);
    bool load_themes(const std::string& path);
    const std::vector<ItemPreset>&  items()  const { return m_items; }
    const std::vector<ThemePreset>& themes() const { return m_themes; }
    void apply_item(Roblox& r, const ItemPreset& p);
    void apply_theme(Roblox& r, const ThemePreset& t);
private:
    std::vector<ItemPreset>  m_items;
    std::vector<ThemePreset> m_themes;
};
