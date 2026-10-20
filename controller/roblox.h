#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <windows.h>

struct Color3 { float r, g, b; };

class Driver {
public:
    bool open();
    ~Driver();
    uint32_t find_pid(const std::wstring& name);
    bool get_module_base(uint32_t pid, const std::wstring& mod, uintptr_t& base, size_t& size);

    template <typename T> std::optional<T> read(uintptr_t a) {
        T v{}; if (!raw_read(a, &v, sizeof(T))) return std::nullopt; return v;
    }
    template <typename T> bool write(uintptr_t a, const T& v) {
        return raw_write(a, &v, sizeof(T));
    }
    bool raw_read(uintptr_t, void*, size_t);
    bool raw_write(uintptr_t, const void*, size_t);

    uint32_t  pid()  const { return m_pid; }
    uintptr_t base() const { return m_base; }

    bool attach(const std::wstring& processName, const std::wstring& moduleName);

private:
    HANDLE    m_h    = INVALID_HANDLE_VALUE;
    uint32_t  m_pid  = 0;
    uintptr_t m_base = 0;
    size_t    m_size = 0;
};

class Roblox {
public:
    explicit Roblox(Driver& d) : drv(d) {}

    uintptr_t data_model();
    uintptr_t service(const char* className);
    uintptr_t local_player();
    uintptr_t character();
    uintptr_t lighting();

    // Theme
    bool set_ambient(Color3 c);
    bool set_fog(Color3 c, float start, float finish);
    bool set_clock_time(float t);
    bool set_brightness(float b);

    // Items
    bool set_shirt(const std::string& assetUrl);
    bool set_pants(const std::string& assetUrl);

private:
    Driver& drv;
    std::vector<uintptr_t> children_of(uintptr_t parent);
    std::string class_name_of(uintptr_t inst);
    uintptr_t   find_child_by_class(uintptr_t parent, const char* cls);
    bool        write_rbx_string(uintptr_t strField, const std::string& s);
};
