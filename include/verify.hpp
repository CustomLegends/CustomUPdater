#pragma once

#include <cstdint>
#include <string>

namespace verify {
    struct ArchiveInfo
    {
        uint64_t entries = 0;
        uint64_t files = 0;
        uint64_t uncompressedSize = 0;
    };

    // Reads the WHOLE zip (decompressing every entry and checking its CRC)
    // BEFORE anything on the SD card is deleted, so a truncated/corrupt
    // download can never lead to a wiped card with nothing to restore.
    // Also rejects entries that try to escape the SD root ("../").
    // Returns false and fills `error` on any problem. Reports progress through
    // ProgressEvent and honours its interrupt flag.
    bool verifyArchive(const std::string& archivePath, ArchiveInfo& info, std::string& error);
}  // namespace verify
