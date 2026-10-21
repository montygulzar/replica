#pragma once
#include <cstdint>

// Live offsets from imtheo.lol dump, roblox version-2366ba214ec740ca (2026-09-22).
// Re-dump every roblox patch. Source: https://offsets.imtheo.lol
namespace off {

    // Entry: RobloxPlayerBeta.exe base + Pointer -> FakeDataModel*
    namespace FakeDataModel {
        constexpr uintptr_t Pointer       = 0x8EE1728;
        constexpr uintptr_t RealDataModel = 0x1F8;
    }

    namespace DataModel {
        constexpr uintptr_t Workspace      = 0x150;
        constexpr uintptr_t ScriptContext  = 0x440;
        constexpr uintptr_t GameLoaded     = 0x5D0;
        constexpr uintptr_t PlaceId        = 0x188;
        constexpr uintptr_t JobId          = 0x110;
    }

    namespace Instance {
        constexpr uintptr_t NameContainer   = 0x70;
        constexpr uintptr_t NameInContainer = 0x8;
        constexpr uintptr_t Parent          = 0x68;
        constexpr uintptr_t ChildrenStart   = 0x78;
        constexpr uintptr_t ChildrenEnd     = 0x80;
        constexpr uintptr_t ClassDescriptor = 0x18;
        constexpr uintptr_t ClassNameInDesc = 0x8;
    }

    namespace Player {
        constexpr uintptr_t LocalPlayer   = 0x120;
        constexpr uintptr_t ModelInstance = 0x288;
        constexpr uintptr_t UserId        = 0xC0;
        constexpr uintptr_t DisplayName   = 0x128;
        constexpr uintptr_t Team          = 0x2C8;
    }

    namespace Clothing {
        constexpr uintptr_t Template = 0xF0;
        constexpr uintptr_t Color3   = 0x110;
    }

    namespace CharacterMesh {
        constexpr uintptr_t BaseTextureId    = 0xB8;
        constexpr uintptr_t MeshId           = 0xE8;
        constexpr uintptr_t OverlayTextureId = 0x118;
        constexpr uintptr_t BodyPart         = 0x138;
    }

    namespace SpecialMesh {
        constexpr uintptr_t Scale  = 0xB4;
        constexpr uintptr_t MeshId = 0xE8;
    }

    namespace Textures {
        constexpr uintptr_t Texture = 0x1D0;
    }

    namespace Lighting {
        constexpr uintptr_t ClockTime                = 0xB8;
        constexpr uintptr_t Ambient                  = 0xC0;
        constexpr uintptr_t ColorShift_Top           = 0xCC;
        constexpr uintptr_t ColorShift_Bottom        = 0xD8;
        constexpr uintptr_t FogColor                 = 0xE4;
        constexpr uintptr_t OutdoorAmbient           = 0xF0;
        constexpr uintptr_t Brightness               = 0x108;
        constexpr uintptr_t EnvironmentDiffuseScale  = 0x10C;
        constexpr uintptr_t EnvironmentSpecularScale = 0x110;
        constexpr uintptr_t ExposureCompensation     = 0x114;
        constexpr uintptr_t FogEnd                   = 0x11C;
        constexpr uintptr_t FogStart                 = 0x120;
        constexpr uintptr_t GeographicLatitude       = 0x124;
        constexpr uintptr_t GlobalShadows            = 0x134;
        constexpr uintptr_t Sky                      = 0x1B8;
    }

    namespace Humanoid {
        constexpr uintptr_t WalkSpeed   = 0x1C0;
        constexpr uintptr_t JumpPower   = 0x194;
        constexpr uintptr_t JumpHeight  = 0x190;
        constexpr uintptr_t HipHeight   = 0x184;
        constexpr uintptr_t DisplayName = 0xA8;
    }

    // Accessory instance (child of Character). Contains a Handle BasePart,
    // which contains a SpecialMesh whose MeshId is the hat/hair mesh.
    namespace Accessory {
        // walk Accessory children for "Handle" (BasePart), then its SpecialMesh
    }

    // BodyColors: six BrickColor fields (Head, Torso, LeftArm, RightArm,
    // LeftLeg, RightLeg). Field offsets are not in the imtheo dump — leave
    // skin color swapping disabled until these are added to the dump or
    // sig-scanned at startup. Not shipping guessed values.
    namespace BodyColors {
        // TODO: populate from a fresh dump that includes BodyColors
    }
}
