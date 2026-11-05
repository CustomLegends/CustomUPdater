# AIO-Switch-Updater


A Nintendo Switch homebrew app to download and update CFWs, FWs and cheat codes. Supports Atmosphère, ReiNX and SXOS.

Works on **unpatched** (Erista) and **patched** (v2/Mariko) Switches.

## How to install
Copy the `aio-switch-updater/` directory to `/switch/` on your sdcard.

## Features
### ⬦ Update CFW
- Update the Atmosphère Switch Custom Firmware. AIO-Switch-Updater uses a custom RCM payload to finalise the install as it can't be performed while HOS is running.
  - If you would like to preserve additional files or directories, write their path (one line each) in `/config/aio-switch-updater/preserve.txt` and they won't be overwritten when updating.
  - Place [this file](https://github.com/HamletDuFromage/aio-switch-updater/blob/master/copy_files.txt) in `/config/aio-switch-updater/copy_files.txt` in order to have specific copy operations performed after each download. This is mainly meant for users with trinkets who want payloads automatically copied to a directory.

### ⬦ Update Hekate/Payload
- Download and update Hekate, as well as a selection of RCM payloads

### ⬦ Custom Downloads
- A custom Atmosphère url can be entered in [this file](https://github.com/HamletDuFromage/aio-switch-updater/blob/master/custom_packs.json). Once moved to `/config/aio-switch-updater/custom_packs.json`, it will show on the `Custom Download` menu. This can be used to support third-party packs through AIO-Switch-Updater. Packs whose name starts by `[PACK]` won't prompt for a Hekate download.
- Non-Atmosphère downloads can also be added in the `misc` category.

### ⬦ Download firmwares
- Download firmware files to `/firmware` that can then be installed using DayBreak.

### ⬦ Download cheats
- Download and extract daily-updated cheat code. The program will only extract cheat codes for the games you own. By default, this homebrew will overwrite the existing cheats. If you have your own cheat files that you'd like to keep as is, you can turn off cheat updates for specific titles in `Tools→Cheat Menu`.
- Download cheat sheets from [Cheat Slips](https://www.cheatslips.com/). 
- Download individual cheat codes from the `GBAtemp.net` database.

### ⬦ Keeping folders when installing a HATS pack
Installing a HATS pack wipes `/atmosphere/contents/` and `/SaltySD/` first, to avoid boot conflicts from incompatible leftovers. Anything listed in `/config/aio-switch-updater/preserve.txt` (one path per line, from the root of the SD card) is now kept instead of deleted — for example:

```
/atmosphere/contents/0100000000001000
```

The same file already prevented those paths from being overwritten during extraction; it now also protects them from the pre-install cleanup.

Some folders are kept by default without needing `preserve.txt` at all — see `AMS_CONTENTS_KEEP` in `include/constants.hpp`. Matching is case-insensitive, since the SD card's filesystem is.

### ⬦ Apps
- Download homebrew apps straight to `/switch/`.
- After downloading a `.nro`, the app offers to create a **forwarder**: a shortcut on the HOME menu that launches the app directly, with its own icon and name taken from the NRO itself.
  - No `prod.keys` needed — the only key involved (`header_key`) is derived from the console via `spl`.
  - Requires CFW with sigpatches, as with any forwarder.
  - Re-downloading the same app overwrites its existing shortcut instead of creating a duplicate.
  - The forwarder generator is a port of [sphaira](https://github.com/ITotalJustice/sphaira)'s `owo.cpp` (GPLv3), which builds on hacbrewpack and yati. The bundled loader comes from [nx-hbloader](https://github.com/switchbrew/nx-hbloader).

## Extras (in the `Tools` tab)
- Reboot to specific payload.
- Consult games with missing updates.
- Change software color scheme of Joy-Cons. Additional color profiles can be found in the releases and should be copied to `config/aio-switch-updater/jc_profiles.json`. Use [this webpage](https://hamletdufromage.github.io/JC-color-picker/JCpicker.html) to generate your own profiles.
- Change software color scheme of Pro Controllers (has to be paired as Player 1). Additional color profiles can be found in the releases and should be copied to `config/aio-switch-updater/pc_profiles.json`.
- Consult installed cheat codes.
- Launch the Switch's web browser.
- Edit internet settings (DNS, IP address, MTU, etc). Add you own configs to `config/aio-switch-updater/internet.json`. You can find a template in the root of the repo.
- Tabs can be hidden through the `Hide tabs` menu, and more entries can be hidden by manually editing [`config/aio-switch-updater/hide_tabs.json`](https://github.com/HamletDuFromage/aio-switch-updater/blob/master/hide_tabs.json).

## Screenshots
<details><summary>Expand to view the screenshots</summary>

![ams_tab](https://user-images.githubusercontent.com/61667930/193625554-ad9a8a5a-72ad-462e-95d9-94979c9750ac.jpg)
![cheats_tab](https://user-images.githubusercontent.com/61667930/193625551-9912210a-c99c-434f-ab5e-b468a698ddcf.jpg)
![individual_cheats](https://user-images.githubusercontent.com/61667930/193625547-18bff50c-1985-4ce5-aadf-2394fa5d29ca.jpg)
![tools_tab](https://user-images.githubusercontent.com/61667930/193625542-4722690a-a86f-48d1-8935-367b16f355f8.jpg)

</details>

## Build

<details><summary>Expand to view the build instructions</summary>

You need to have installed devkitPro and devkitARM in order to compile this project.

Install the required dependencies:
```bash
$ sudo (dkp-)pacman -Sy
```
```bash
$ sudo (dkp-)pacman -S  switch-glfw \
                        switch-curl \
                        switch-glad \
                        switch-glm \
                        switch-mbedtls \
                        switch-zlib \
                        devkitarm-rules
```
Use [`switch-ex-curl`](https://github.com/eXhumer/switch-ex-curl) instead of `switch-curl` to use this app with an invalid SSL certificate.

Clone the repository
```bash
$ git clone --recursive https://github.com/HamletDuFromage/aio-switch-updater
$ cd aio-switch-updater
```

Compile 
```bash
$ cd aiosu-forwarder
$ make
$ cd ..
$ make
```

</details>

## Contribute

PRs and suggestions are encouraged! If you wish to help with the localization of the app, you can translate the files in `resources/i18n/`. To easily find the non-translated strings and translate them, you may use `localizer.py` (e.g. `python localizer.py -r resources//i18n//en-US//menus.json -w resources//i18n//fr//menus.json`). 




