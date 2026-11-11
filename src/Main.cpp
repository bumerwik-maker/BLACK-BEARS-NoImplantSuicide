
#include <API/ARK/Ark.h>
#include <IHooks.h>

#include <fstream>
#include <string>

namespace {

constexpr auto hookName =
    "AShooterPlayerController.ServerSuicide_Implementation()";

constexpr auto folder =
    "ArkApi/Plugins/BlackBearsNoImplantSuicide/";

using SuicideFn =
    void(__fastcall*)(AShooterPlayerController*);

SuicideFn original = nullptr;

bool hooked = false;
bool blocking = false;

void log(const char* line) {
    std::ofstream out(
        std::string(folder) + "implant_test.log",
        std::ios::app
    );

    if (out) {
        out << line << '\n';
    }
}

void __fastcall detour(AShooterPlayerController* player) {
    if (blocking) {
        return;
    }

    if (original) {
        original(player);
    }
}

extern "C" __declspec(dllexport)
void Plugin_Init() {

    std::ifstream mode(
        std::string(folder) + "mode.txt"
    );

    std::string value;

    if (mode) {
        std::getline(mode, value);
    }

    blocking = (value == "BLOCK");

    log(blocking
        ? "Starting BLOCK"
        : "Starting DIAGNOSE"
    );

    hooked = AsaApi::GetHooks().SetHook(
        hookName,
        reinterpret_cast<void*>(&detour),
        &original
    );

    log(hooked
        ? "Hook installed"
        : "Hook FAILED"
    );
}

extern "C" __declspec(dllexport)
void Plugin_Unload() {

    if (hooked) {
        AsaApi::GetHooks().DisableHook(
            hookName,
            reinterpret_cast<void*>(&detour)
        );
    }

    log("Unloaded");
}
