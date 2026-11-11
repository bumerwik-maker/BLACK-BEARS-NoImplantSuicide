# BLACK BEARS – No Implant Suicide (experimental)

**Unverified test plugin.** The hook name/signature and actual implant button call path have not been confirmed. A build may fail; even a successful build does not prove runtime safety.

## GitHub Actions
Upload the **contents of this extracted folder**, preserving `.github/workflows/build.yml`. Open **Actions → Build Windows ASA plugin → Run workflow**. Download the artifact after a successful run.

## Installation (test server only)
Copy the artifact folder `BlackBearsNoImplantSuicide` into `ShooterGame/Binaries/Win64/ArkApi/Plugins/`. Start with `mode.txt` set to `DIAGNOSE`, restart server, and inspect `implant_test.log` after pressing the implant respawn button. Change to `BLOCK` only if the hook is confirmed; restart. This may block other suicide methods too. Back up the server and remove the plugin if it crashes.

## Important
Do not upload `AsaApi.pdb`, private server configs, tokens, or credentials to a public repository. The packaged `.lib` is included for linking. No installation on your PC is needed. The source header commit is pinned to the uploaded source archive commit.
