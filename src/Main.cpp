
#include <windows.h>
#include <API/ARK/Ark.h>
#include <IHooks.h>

#include <fstream>
#include <string>

namespace {

constexpr auto hookServer =
    "AShooterPlayerController.ServerSuicide_Implementation()";

constexpr auto hookSuicide =
    "AShooterPlayerController.Suicide()";

constexpr auto folder =
    "ArkApi/Plugins/BlackBearsNoImplantSuicide/";

using SuicideFn =
    void(__fastcall*)(AShooterPlayerController*);

SuicideFn originalServer = nullptr;
SuicideFn originalSuicide = nullptr;

bool hookedServer = false;
bool hookedSuicide = false;
bool blocking = false;

// Limit diagnostic logging to avoid huge log files.
int serverLogCount = 0;
int suicideLogCount = 0;
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
    if (serverLogCount < maxLogs) {
        log("ServerSuicide hook called");
        ++serverLogCount;
    }

    if (blocking) {
        return;
    }

    if (originalServer) {
        originalServer(player);
    }
}

void __fastcall detourSuicide(
    AShooterPlayerController* player
) {
    if (suicideLogCount < maxLogs) {
        log("Suicide hook called");
        ++suicideLogCount;
    }

    if (blocking) {
        return;
    }

    if (originalSuicide) {
        originalSuicide(player);
    }
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
}

extern "C" __declspec(dllexport)
void Plugin_Unload() {

    if (hookedServer) {
        AsaApi::GetHooks().DisableHook(
            hookServer,
            reinterpret_cast<LPVOID>(&detourServer)
        );
        hookedServer = false;
    }

    if (hookedSuicide) {
        AsaApi::GetHooks().DisableHook(
            hookSuicide,
            reinterpret_cast<LPVOID>(&detourSuicide)
        );
        hookedSuicide = false;
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

