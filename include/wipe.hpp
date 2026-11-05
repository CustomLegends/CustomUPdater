#pragma once

#include <cstddef>
#include <functional>
#include <set>
#include <string>

// Selective "wipe everything except..." for the SD card.
// Deliberately free of libnx/borealis dependencies so the logic can be
// unit-tested on a PC.
namespace wipe {

    struct Stats
    {
        size_t removed = 0;  // top-level entries removed
        size_t failed = 0;   // entries that could not be removed
        size_t kept = 0;     // top-level entries kept because they are protected
    };

    // Lower-level helpers (exposed for tests).
    std::string normalize(const std::string& path);
    // True if `prefix` equals `path` or is one of its parent directories.
    // Comparison is case-insensitive (FAT/exFAT) and respects path separators.
    bool isPathPrefix(const std::string& prefix, const std::string& path);

    // Make a user-supplied protect-list line a clean absolute path
    // ("Nintendo", "/Nintendo/", "\\Nintendo" -> "/Nintendo"). Returns "" for
    // blank/comment lines.
    std::string cleanKeepEntry(const std::string& line);

    // Deletes everything below `root` EXCEPT:
    //   - any path listed in `keep` (and everything inside it),
    //   - the parent directories needed to reach a kept path (those are
    //     entered and cleaned recursively, never deleted).
    // `removeDirFn` is used for fast recursive directory removal (e.g. the
    // libnx fsFsDeleteDirectoryRecursively wrapper); if it fails or is empty,
    // std::filesystem::remove_all is used as fallback.
    // `progress(done, total)` is called for each top-level entry of `root`.
    Stats wipeExcept(const std::string& root,
                     const std::set<std::string>& keep,
                     const std::function<bool(const std::string&)>& removeDirFn = {},
                     const std::function<void(size_t, size_t)>& progress = {});
}  // namespace wipe
