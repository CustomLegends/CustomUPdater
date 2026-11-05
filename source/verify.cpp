#include "verify.hpp"

#include <minizip/unzip.h>

#include <vector>

#include "progress_event.hpp"

namespace verify {

    bool verifyArchive(const std::string& archivePath, ArchiveInfo& info, std::string& error)
    {
        info = ArchiveInfo();
        unzFile zfile = unzOpen(archivePath.c_str());
        if (!zfile) {
            error = "Downloaded file is not a valid ZIP.";
            return false;
        }

        unz_global_info gi;
        if (unzGetGlobalInfo(zfile, &gi) != UNZ_OK || gi.number_entry == 0) {
            unzClose(zfile);
            error = "ZIP is empty or unreadable.";
            return false;
        }

        // Keep the step counter strictly below max until we are really done:
        // the UI treats step == max as "stage finished".
        ProgressEvent::instance().setStep(0);
        ProgressEvent::instance().setTotalSteps(static_cast<int>(gi.number_entry) + 1);

        std::vector<char> buf(0x10000);
        bool ok = true;

        if (unzGoToFirstFile(zfile) != UNZ_OK) ok = false;

        for (uLong i = 0; ok && i < gi.number_entry; ++i) {
            if (ProgressEvent::instance().getInterupt()) {
                error = "Cancelled.";
                ok = false;
                break;
            }

            char name[0x301] = "";
            unz_file_info fi;
            if (unzGetCurrentFileInfo(zfile, &fi, name, sizeof(name), NULL, 0, NULL, 0) != UNZ_OK) {
                error = "ZIP directory is corrupt.";
                ok = false;
                break;
            }

            const std::string entry = name;
            if (entry.find("../") != std::string::npos || entry.find("..\\") != std::string::npos || entry == "..") {
                error = "ZIP contains unsafe path: " + entry;
                ok = false;
                break;
            }

            info.entries++;
            if (!entry.empty() && entry.back() != '/') {
                if (unzOpenCurrentFile(zfile) != UNZ_OK) {
                    error = "Cannot open entry: " + entry;
                    ok = false;
                    break;
                }
                int n;
                while ((n = unzReadCurrentFile(zfile, buf.data(), static_cast<unsigned>(buf.size()))) > 0) {
                }
                const bool readFailed = n < 0;
                // unzCloseCurrentFile reports CRC mismatches.
                const int closeRes = unzCloseCurrentFile(zfile);
                if (readFailed || closeRes != UNZ_OK) {
                    error = "ZIP is corrupt (CRC error): " + entry;
                    ok = false;
                    break;
                }
                info.files++;
                info.uncompressedSize += fi.uncompressed_size;
            }

            ProgressEvent::instance().setStep(static_cast<int>(i));

            if (i + 1 < gi.number_entry && unzGoToNextFile(zfile) != UNZ_OK) {
                error = "ZIP directory is corrupt.";
                ok = false;
            }
        }

        unzClose(zfile);

        if (ok && info.files == 0) {
            error = "ZIP contains no files.";
            ok = false;
        }
        return ok;
    }
}  // namespace verify
