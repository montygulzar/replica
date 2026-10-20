#include "roblox.h"
#include "offsets.h"
#include <winioctl.h>
#include <cstring>

#define IOCTL_LARP_READ  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LARP_WRITE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LARP_BASE  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LARP_PID   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)

struct LARP_RW   { uint32_t pid; void* addr; void* buf; size_t size; };
struct LARP_BASE { uint32_t pid; wchar_t name[64]; void* base; size_t size; };
struct LARP_PID  { wchar_t name[64]; uint32_t pid; };

// --- Driver ---

bool Driver::open() {
    m_h = CreateFileW(L"\\\\.\\Larp", GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                      OPEN_EXISTING, 0, nullptr);
    return m_h != INVALID_HANDLE_VALUE;
}

Driver::~Driver() { if (m_h != INVALID_HANDLE_VALUE) CloseHandle(m_h); }

uint32_t Driver::find_pid(const std::wstring& name) {
    LARP_PID p{}; wcsncpy_s(p.name, name.c_str(), 63);
    DWORD bytes = 0;
    if (!DeviceIoControl(m_h, IOCTL_LARP_PID, &p, sizeof(p), &p, sizeof(p), &bytes, nullptr))
        return 0;
    return p.pid;
}

bool Driver::get_module_base(uint32_t pid, const std::wstring& mod, uintptr_t& base, size_t& size) {
    LARP_BASE b{}; b.pid = pid; wcsncpy_s(b.name, mod.c_str(), 63);
    DWORD bytes = 0;
    if (!DeviceIoControl(m_h, IOCTL_LARP_BASE, &b, sizeof(b), &b, sizeof(b), &bytes, nullptr))
        return false;
    base = (uintptr_t)b.base; size = b.size;
    return b.base != nullptr;
}

bool Driver::raw_read(uintptr_t addr, void* out, size_t sz) {
    LARP_RW r{}; r.pid = m_pid; r.addr = (void*)addr; r.buf = out; r.size = sz;
    DWORD bytes = 0;
    return !!DeviceIoControl(m_h, IOCTL_LARP_READ, &r, sizeof(r), &r, sizeof(r), &bytes, nullptr);
}

bool Driver::raw_write(uintptr_t addr, const void* in, size_t sz) {
    LARP_RW r{}; r.pid = m_pid; r.addr = (void*)addr; r.buf = (void*)in; r.size = sz;
    DWORD bytes = 0;
    return !!DeviceIoControl(m_h, IOCTL_LARP_WRITE, &r, sizeof(r), &r, sizeof(r), &bytes, nullptr);
}

bool Driver::attach(const std::wstring& processName, const std::wstring& moduleName) {
    m_pid = find_pid(processName);
    if (!m_pid) return false;
    return get_module_base(m_pid, moduleName, m_base, m_size);
}

// --- Roblox ---

uintptr_t Roblox::data_model() {
    auto fdm = drv.read<uintptr_t>(drv.base() + off::FakeDataModel::Pointer);
    if (!fdm || !*fdm) return 0;
    return drv.read<uintptr_t>(*fdm + off::FakeDataModel::RealDataModel).value_or(0);
}

std::vector<uintptr_t> Roblox::children_of(uintptr_t parent) {
    std::vector<uintptr_t> out;
    if (!parent) return out;
    auto start = drv.read<uintptr_t>(parent + off::Instance::ChildrenStart);
    auto end   = drv.read<uintptr_t>(parent + off::Instance::ChildrenEnd);
    if (!start || !end || !*start || *end <= *start) return out;
    size_t count = (*end - *start) / sizeof(uintptr_t);
    if (count > 4096) return out;
    for (size_t i = 0; i < count; ++i) {
        auto c = drv.read<uintptr_t>(*start + i * sizeof(uintptr_t));
        if (c && *c) out.push_back(*c);
    }
    return out;
}

std::string Roblox::class_name_of(uintptr_t inst) {
    auto desc = drv.read<uintptr_t>(inst + off::Instance::ClassDescriptor);
    if (!desc || !*desc) return {};
    auto strPtr = drv.read<uintptr_t>(*desc + off::Instance::ClassNameInDesc);
    if (!strPtr || !*strPtr) return {};
    char buf[128]{};
    drv.raw_read(*strPtr, buf, sizeof(buf) - 1);
    return std::string(buf);
}

uintptr_t Roblox::find_child_by_class(uintptr_t parent, const char* cls) {
    for (auto c : children_of(parent)) {
        if (class_name_of(c) == cls) return c;
    }
    return 0;
}

uintptr_t Roblox::service(const char* className) {
    auto dm = data_model();
    if (!dm) return 0;
    return find_child_by_class(dm, className);
}

uintptr_t Roblox::local_player() {
    auto players = service("Players");
    if (!players) return 0;
    return drv.read<uintptr_t>(players + off::Player::LocalPlayer).value_or(0);
}

uintptr_t Roblox::character() {
    auto lp = local_player();
    if (!lp) return 0;
    return drv.read<uintptr_t>(lp + off::Player::ModelInstance).value_or(0);
}

uintptr_t Roblox::lighting() {
    return service("Lighting");
}

// --- Theme ---

bool Roblox::set_ambient(Color3 c) {
    auto l = lighting(); if (!l) return false;
    return drv.write<Color3>(l + off::Lighting::Ambient, c);
}

bool Roblox::set_fog(Color3 c, float start, float finish) {
    auto l = lighting(); if (!l) return false;
    bool ok = drv.write<Color3>(l + off::Lighting::FogColor, c);
    ok &= drv.write<float>(l + off::Lighting::FogStart, start);
    ok &= drv.write<float>(l + off::Lighting::FogEnd,   finish);
    return ok;
}

bool Roblox::set_clock_time(float t) {
    auto l = lighting(); if (!l) return false;
    return drv.write<float>(l + off::Lighting::ClockTime, t);
}

bool Roblox::set_brightness(float b) {
    auto l = lighting(); if (!l) return false;
    return drv.write<float>(l + off::Lighting::Brightness, b);
}

// --- Items ---

// Roblox std::string layout (SSO):
//   [0x00..0x0F] inline buffer when size <= 15
//   [0x10]       size
//   [0x18]       capacity (high bit set => heap-backed, buffer ptr lives at [0x00])
bool Roblox::write_rbx_string(uintptr_t strField, const std::string& s) {
    if (s.size() <= 15) {
        char buf[16]{};
        memcpy(buf, s.data(), s.size());
        if (!drv.raw_write(strField, buf, 16)) return false;
        return drv.write<uint64_t>(strField + 0x10, (uint64_t)s.size());
    }
    auto heapPtr = drv.read<uintptr_t>(strField).value_or(0);
    auto cap     = drv.read<uint64_t>(strField + 0x18).value_or(0) & ~(uint64_t(1) << 63);
    if (!heapPtr || s.size() > cap) return false;
    if (!drv.raw_write(heapPtr, s.data(), s.size() + 1)) return false;
    return drv.write<uint64_t>(strField + 0x10, (uint64_t)s.size());
}

bool Roblox::set_shirt(const std::string& assetUrl) {
    auto ch = character(); if (!ch) return false;
    auto shirt = find_child_by_class(ch, "Shirt");
    if (!shirt) return false;
    return write_rbx_string(shirt + off::Clothing::Template, assetUrl);
}

bool Roblox::set_pants(const std::string& assetUrl) {
    auto ch = character(); if (!ch) return false;
    auto pants = find_child_by_class(ch, "Pants");
    if (!pants) return false;
    return write_rbx_string(pants + off::Clothing::Template, assetUrl);
}
