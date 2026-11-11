
#include <windows.h>
#include <API/ARK/Ark.h>
#include <IHooks.h>

#include <fstream>
#include <string>
#include <atomic>

namespace {

constexpr auto hookServer =
    "AShooterPlayerController.ServerSuicide_Implementation()";

constexpr auto hookSuicide =
    "AShooterPlayerController.Suicide()";

constexpr auto hookImplant =
    "AShooterCharacter.BPCanImplantSuicide()";

constexpr auto folder =
    "ArkApi/Plugins/BlackBearsNoImplantSuicide/";

using SuicideFn =
    void(__fastcall*)(AShooterPlayerController*);

using ImplantFn =
    bool(__fastcall*)(AShooterCharacter*);

SuicideFn originalServer = nullptr;
SuicideFn originalSuicide = nullptr;
ImplantFn originalImplant = nullptr;

bool hookedServer = false;
bool hookedSuicide = false;
bool hookedImplant = false;
bool blocking = false;

std::atomic<int> serverLogCount{0};
std::atomic<int> suicideLogCount{0};
std::atomic<int> implantLogCount{0};

constexpr int maxLogs = 10;

void log(const char* message) {
    std::ofstream out(
        std::string(folder) + "implant_test.log",
        std::ios::app
    );

    if (out) {
        out << message << '\n';
    }
}

void __fastcall detourServer(
    AShooterPlayerController* player
) {
    if (blocking) {
        if (serverLogCount.fetch_add(1) < maxLogs) {
            log("ServerSuicide BLOCKED");
        }
        return;
    }

    if (originalServer) {
        originalServer(player);
    }
}

void __fastcall detourSuicide(
    AShooterPlayerController* player
) {
    if (blocking) {
        if (suicideLogCount.fetch_add(1) < maxLogs) {
            log("Suicide BLOCKED");
        }
        return;
    }

    if (originalSuicide) {
        originalSuicide(player);
    }
}

bool __fastcall detourImplant(
    AShooterCharacter* character
) {
    if (implantLogCount.fetch_add(1) < maxLogs) {
        log("BPCanImplantSuicide hook called");
    }

    if (blocking) {
        return false;
    }

    if (originalImplant) {
        return originalImplant(character);
    }

    return false;
}

} // namespace

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

    hookedServer = AsaApi::GetHooks().SetHook(
        hookServer,
        reinterpret_cast<LPVOID>(&detourServer),
        &originalServer
    );

    log(hookedServer
        ? "ServerSuicide hook installed"
        : "ServerSuicide hook FAILED"
    );

    hookedSuicide = AsaApi::GetHooks().SetHook(
        hookSuicide,
        reinterpret_cast<LPVOID>(&detourSuicide),
        &originalSuicide
    );

    log(hookedSuicide
        ? "Suicide hook installed"
        : "Suicide hook FAILED"
    );

    hookedImplant = AsaApi::GetHooks().SetHook(
        hookImplant,
        reinterpret_cast<LPVOID>(&detourImplant),
        &originalImplant
    );

    log(hookedImplant
        ? "Implant hook installed"
        : "Implant hook FAILED"
    );
}

extern "C" __declspec(dllexport)
void Plugin_Unload() {

    if (hookedImplant) {
        AsaApi::GetHooks().DisableHook(
            hookImplant,
            reinterpret_cast<LPVOID>(&detourImplant)
        );
        hookedImplant = false;
    }

    if (hookedSuicide) {
        AsaApi::GetHooks().DisableHook(
            hookSuicide,
            reinterpret_cast<LPVOID>(&detourSuicide)
        );
        hookedSuicide = false;
    }

    if (hookedServer) {
        AsaApi::GetHooks().DisableHook(
            hookServer,
            reinterpret_cast<LPVOID>(&detourServer)
        );
        hookedServer = false;
    }

    log("Plugin unloaded");
}

BOOL APIENTRY DllMain(
    HMODULE,
    DWORD,
    LPVOID
) {
    return TRUE;
}
