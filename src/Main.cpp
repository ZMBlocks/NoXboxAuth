#include <glacie/memory/Hook.h>

#define XStoreQueryGameLicenseAsync                                                                                    \
    "48 8B C4 57 48 83 EC 30 48 C7 40 E8 FE FF FF FF 48 89 58 08 48 89 70 10 48 8B FA 48 8B F1 4C 8D 40 18 48 8D 15 "  \
    "?? ?? ?? 00 48 8D 0D ?? ?? ?? 00 E8 ?? ?? FF FF 8B D8 85 C0 78 2F 48 8B 4C 24 50 48 8B 01 4C 8B C7 48 8B D6 48 "  \
    "8B 80 E0 00 00 00 FF 15 ?? ?? ?? 00 8B D8 48 8B 4C 24 50 48 8B 01 48 8B 40 10 FF 15 ?? ?? ?? 00 8B C3 48 8B 5C "  \
    "24 40 48 8B 74 24 48 48 83 C4 30 5F C3"

#define XStoreQueryGameLicenseResult                                                                                   \
    "48 8B C4 57 48 83 EC 30 48 C7 40 E8 FE FF FF FF 48 89 58 08 48 89 70 10 48 8B FA 48 8B F1 4C 8D 40 18 48 8D 15 "  \
    "?? ?? ?? 00 48 8D 0D ?? ?? ?? 00 E8 ?? ?? FF FF 8B D8 85 C0 78 2F 48 8B 4C 24 50 48 8B 01 4C 8B C7 48 8B D6 48 "  \
    "8B 80 E8 00 00 00 FF 15 ?? ?? ?? 00 8B D8 48 8B 4C 24 50 48 8B 01 48 8B 40 10 FF 15 ?? ?? ?? 00 8B C3 48 8B 5C "  \
    "24 40 48 8B 74 24 48 48 83 C4 30 5F C3"

#define XStoreRegisterGameLicenseChanged                                                                               \
    "48 8B C4 41 56 48 83 EC 50 48 C7 40 F0 FE FF FF FF 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 49 8B F9 49 "  \
    "8B F0 48 8B EA 4C 8B F1 4C 8D 40 E8 48 8D 15 ?? ?? ?? 00 48 8D 0D ?? ?? ?? 00 E8 ?? ?? FF FF 8B D8 85 C0 78 44 "  \
    "48 8B 4C 24 40 48 8B 01 48 8B 94 24 80 00 00 00 48 89 54 24 28 48 89 7C 24 20 4C 8B CE 4C 8B C5 49 8B D6 48 8B "  \
    "80 10 02 00 00 FF 15 ?? ?? ?? 00 8B D8 48 8B 4C 24 40 48 8B 01 48 8B 40 10 FF 15 ?? ?? ?? 00 8B C3 48 8B 5C 24 "  \
    "60 48 8B 6C 24 68 48 8B 74 24 70 48 8B 7C 24 78 48 83 C4 50 41 5E C3"

// from https://learn.microsoft.com/zh-cn/gaming/gdk/docs/reference/system/xstore/structs/xstoregamelicense
struct XStoreGameLicense {
    char     skuStoreId[18];
    bool     isActive;
    bool     isTrialOwnedByThisUser;
    bool     isDiscLicense;
    bool     isTrial;
    uint32_t trialTimeRemainingInSeconds;
    char     trialUniqueId[64];
    time_t   expirationDate;
};

GLACIE_STATIC_HOOK(XStoreQueryGameLicenseAsyncHook, XStoreQueryGameLicenseAsync, uint32_t, void*, void* a2) {
    glacie::memory::dAccess<uint64_t>(a2, 0x18) = 0;
    if (auto* func = glacie::memory::dAccess<void (*)(void*)>(a2, 0x10); func) {
        func(a2);
    }
    return 0;
}

GLACIE_STATIC_HOOK(
    XStoreQueryGameLicenseResultHook,
    XStoreQueryGameLicenseResult,
    int64_t,
    void*,
    XStoreGameLicense& license
) {
    std::copy_n("9NBLGGH2JHXJ/0010", sizeof(license.skuStoreId), license.skuStoreId);
    license.isActive                    = true;
    license.isTrialOwnedByThisUser      = false;
    license.isDiscLicense               = false;
    license.isTrial                     = false;
    license.trialTimeRemainingInSeconds = UINT_MAX;
    std::fill_n(license.trialUniqueId, sizeof(license.trialUniqueId), 0);
    license.expirationDate = LLONG_MAX;
    return 0;
}

GLACIE_STATIC_HOOK(
    XStoreRegisterGameLicenseChangedHook,
    XStoreRegisterGameLicenseChanged,
    int64_t,
    void*,
    void*,
    void*,
    void*,
    void*
) {
    return 0;
}

enum class DLL_REASON : unsigned long {
    DLL_PROCESS_ATTACH = 1,
    DLL_THREAD_ATTACH  = 2,
    DLL_THREAD_DETACH  = 3,
    DLL_PROCESS_DETACH = 0,
};

int __stdcall DllMain(void*, DLL_REASON ul_reason_for_call, void*) {
    using Hooks = glacie::memory::HookRegistrar<
        XStoreQueryGameLicenseAsyncHook,
        XStoreQueryGameLicenseResultHook,
        XStoreRegisterGameLicenseChangedHook>;

    switch (ul_reason_for_call) {
    case DLL_REASON::DLL_PROCESS_ATTACH:
        Hooks::hook();
        break;
    case DLL_REASON::DLL_THREAD_ATTACH:
        break;
    case DLL_REASON::DLL_THREAD_DETACH:
        break;
    case DLL_REASON::DLL_PROCESS_DETACH:
        Hooks::unhook();
        break;
    }
    return 1;
}