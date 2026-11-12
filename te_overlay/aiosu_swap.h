#pragma once

/*
 * CustomUpdater boot-time SD swap.
 *
 * The app extracts a full pack into sd:/aiosu_staging and writes
 *   sd:/config/aio-switch-updater/swap_keep.txt     (paths to keep, one per line)
 *   sd:/config/aio-switch-updater/swap_pending.txt  (marker)
 * and reboots into this payload. Here, with Horizon NOT running, everything on
 * the SD card except the keep list is deleted and the staged files are moved
 * into place. Nothing happens if the marker is missing.
 */
void aiosu_run_pending_swap(void);
