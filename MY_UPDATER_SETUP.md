# CustomUpdater – setup

Based on OQB-updater / AIO-switch-updater (GPL-3). Everything the original app
does is still there (Atmosphere/CFW, bootloaders, firmware, cheats, sigpatch/app
store tabs, payloads, tools ...). What's new:

## New: "Full pack" install (Custom packs tab -> first list)
Every entry in the `"ams"` section of YOUR `custom_packs.json` is installed like this:

1. **Download** the zip from your GitHub link.
2. **Verify** the entire zip (every file decompressed + CRC checked, size vs. SD size).
   Broken/partial download => error screen, **nothing is deleted**.
3. **Ask**: `Cancel` (default) / `Install without wiping` / `Wipe SD & install`.
4. **Wipe** the SD card (only if you chose it). Always kept:
   - `/Nintendo` (installed games, updates, DLC, album)
   - `/emuMMC` (emuNAND)
   - `/backup` (hekate NAND backups)
   - `/payload.bin`
   - this app + `/config/aio-switch-updater/` (settings, the downloaded zip)
   - anything you list in `/config/aio-switch-updater/preserve.txt` (one path per line, e.g. `/roms` or `/switch/MyApp`)
   - `/atmosphere/contents/0100B00B51230000`
5. **Extract** the zip to the SD root, then run `copy_files.txt` (if present).

Everything else (other homebrew, /atmosphere, /bootloader, /switch/*, /config/*, your own folders like /roms, ...) is deleted before the pack is extracted.
Put anything you want to survive into `preserve.txt`.

Name an entry `[PACK]` (e.g. `"My CFW Pack [PACK]"`) if your zip already contains hekate;
otherwise the app also offers the stock hekate download after the pack.

## Your link list = your GitHub releases (automatic)
The app reads `https://github.com/CustomLegends/Download-all/releases` through the GitHub API.
Every release whose **name starts with "Switch"** (see `RELEASE_NAME_FILTER` in `include/constants.hpp`;
e.g. "Switch CFW 23.0.1 AMS 1.12.1 Nexus") is listed, newest first; each `.zip` asset is one full pack
(if a release has several zips, the list shows "Release name - file.zip"). The Xbox360 release is skipped by the filter.
Nothing to maintain: publish a new release and it shows up in the app.

Extras (optional): a `custom_packs.json` in the repo root (`main` branch) is merged in:
```json
{ "ams": { "Name [PACK]": "https://.../pack.zip" }, "misc": { "Some tool": "https://.../tool.nro" } }
```
- `"ams"` -> full-pack flow above. `"misc"` -> plain extract/copy, no wipe.
- Change the link source without rebuilding: URL on line 1 of `/config/aio-switch-updater/links_url.txt`.
- Release entries are marked `[PACK]` = "zip contains its own hekate", so the stock hekate download is skipped.

## Build (no PC toolchain needed)
1. Easiest: fork https://github.com/ohquebacan/OQB-updater (keeps the submodules) – or use your own repo and add the submodules from .gitmodules.
2. Copy these files over your repo (same paths). Your GitHub name/repo (CustomLegends/Download-all) is already filled in.
3. Commit/push. **Actions -> Build my AIO updater -> Artifacts -> CustomUpdater.nro**.
   Pushing to `main`/`master` also publishes the app as the **pre-release tagged `app`** (never becomes your "latest" CFW release).
4. Copy `CustomUpdater.nro` to `sdmc:/switch/`.

> The file name stays `CustomUpdater.nro` on purpose (paths inside the app depend on it).
> "Update app" downloads from your `app` pre-release; the automatic new-version notice is switched off (your repo's latest release is a CFW pack).

## Safety notes
- A wipe that is interrupted (battery, SD removed) can leave a console that cannot boot CFW.
  Keep it plugged in; you can always re-inject hekate/fusee via RCM and run it again.
- Keep a backup of anything not in the protected list **before** the first wipe.

## Languages
Only **English (en-US)** and **German (de)** are included; every text is translated.
When you copy the files over your fork, also DELETE these folders from `resources/i18n/`
(the zip can only add files, not delete): `es fr it ja ko pl pt-BR ro ru zh-CN zh-TW`.
(The `.patch` file already contains the deletions: `git apply my-aio-updater.patch`.)

## Atmosphère tab, logo, file name
- **Atmosphère aktualisieren / Update Atmosphère** lists ONLY the packs from your GitHub releases
  (the old "OQB HATS PACK" / "Overclock HATSPACK" lists are gone). Same full-pack flow as above.
- The header shows the animated Custom Legends wordmark (`resources/branding/logo_anim.png`, a sprite
  sheet: 27 build-up frames + the finished logo as the last frame).
- The built file is now `CustomUpdater.nro` (copy it to `sdmc:/switch/`). Delete the old `OQB-updater.nro` there.

## Updating the app from inside the app
- Every push to `master` builds `CustomUpdater.nro` as version `2.0.<run number>` and publishes it as a
  normal release of `CustomLegends/CustomUPdater` (tag = version).
- The app checks `releases/latest` of that repo at start: if the tag differs from its own version it shows a
  notice ("Update now / Later"). The same update is available under **Über / About -> update app**.
- Updating downloads `CustomUpdater.zip`, replaces `sdmc:/switch/CustomUpdater.nro` and restarts the app.
- The very first build must be copied to the SD card by hand; after that updates work from inside the app.
- You can delete the old pre-release called "app" in your Releases list.
