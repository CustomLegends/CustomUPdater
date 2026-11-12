#include "utils.hpp"

#include <switch.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <thread>

#include "current_cfw.hpp"
#include "download.hpp"
#include "extract.hpp"
#include "fs.hpp"
#include "main_frame.hpp"
#include "progress_event.hpp"
#include "reboot_payload.h"
#include "unistd.h"
#include "verify.hpp"
#include "wipe.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

namespace util {

    bool isArchive(const std::string& path)
    {
        if (std::filesystem::exists(path)) {
            std::ifstream is(path, std::ifstream::binary);
            const std::array<char, 4> zip_signature = {0x50, 0x4B, 0x03, 0x04};
            std::array<char, 4> signature = {};
            is.read(signature.data(), 4);
            if (is.good() && signature == zip_signature) {
                return true;
            }
        }
        return false;
    }

    void downloadArchive(const std::string& url, contentType type)
    {
        long status_code;
        downloadArchive(url, type, status_code);
    }

    void downloadArchive(const std::string& url, contentType type, long& status_code)
    {
        fs::createTree(DOWNLOAD_PATH);
        switch (type) {
            case contentType::custom:
                status_code = download::downloadFile(url, CUSTOM_FILENAME, OFF);
                break;
            case contentType::cheats:
                status_code = download::downloadFile(url, CHEATS_FILENAME, OFF);
                break;
            case contentType::fw:
                status_code = download::downloadFile(url, FIRMWARE_FILENAME, OFF);
                break;
            case contentType::app:
                status_code = download::downloadFile(url, APP_FILENAME, OFF);
                break;
            case contentType::bootloaders:
                status_code = download::downloadFile(url, BOOTLOADER_FILENAME, OFF);
                break;
            case contentType::ams_cfw:
                status_code = download::downloadFile(url, AMS_FILENAME, OFF);
                break;
            case contentType::apps:
                status_code = download::downloadFile(url, CUSTOM_FILENAME, OFF);
                break;
            case contentType::full_pack:
                status_code = download::downloadFile(url, FULLPACK_FILENAME, OFF);
                break;
            default:
                break;
        }
        ProgressEvent::instance().setStatusCode(status_code);
    }

    void showDialogBoxInfo(const std::string& text)
    {
        brls::Dialog* dialog;
        dialog = new brls::Dialog(text);
        brls::GenericEvent::Callback callback = [dialog](brls::View* view) {
            dialog->close();
        };
        dialog->addButton("menus/common/ok"_i18n, callback);
        dialog->setCancelable(true);
        dialog->open();
    }

    int showDialogBoxBlocking(const std::string& text, const std::string& opt)
    {
        int result = -1;
        brls::Dialog* dialog = new brls::Dialog(text);
        brls::GenericEvent::Callback callback = [dialog, &result](brls::View* view) {
            result = 0;
            dialog->close();
        };
        dialog->addButton(opt, callback);
        dialog->setCancelable(false);
        dialog->open();
        while (result == -1) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        std::this_thread::sleep_for(std::chrono::microseconds(800000));
        return result;
    }

    int showDialogBoxBlocking(const std::string& text, const std::string& opt1, const std::string& opt2)
    {
        int result = -1;
        brls::Dialog* dialog = new brls::Dialog(text);
        brls::GenericEvent::Callback callback1 = [dialog, &result](brls::View* view) {
            result = 0;
            dialog->close();
        };
        brls::GenericEvent::Callback callback2 = [dialog, &result](brls::View* view) {
            result = 1;
            dialog->close();
        };
        dialog->addButton(opt1, callback1);
        dialog->addButton(opt2, callback2);
        dialog->setCancelable(false);
        dialog->open();
        while (result == -1) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        std::this_thread::sleep_for(std::chrono::microseconds(800000));
        return result;
    }

    int showDialogBoxBlocking(const std::string& text, const std::string& opt1, const std::string& opt2, const std::string& opt3)
    {
        int result = -1;
        brls::Dialog* dialog = new brls::Dialog(text);
        brls::GenericEvent::Callback callback1 = [dialog, &result](brls::View* view) {
            result = 0;
            dialog->close();
        };
        brls::GenericEvent::Callback callback2 = [dialog, &result](brls::View* view) {
            result = 1;
            dialog->close();
        };
        brls::GenericEvent::Callback callback3 = [dialog, &result](brls::View* view) {
            result = 2;
            dialog->close();
        };
        dialog->addButton(opt1, callback1);
        dialog->addButton(opt2, callback2);
        dialog->addButton(opt3, callback3);
        dialog->setCancelable(false);
        dialog->open();
        while (result == -1) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        std::this_thread::sleep_for(std::chrono::microseconds(800000));
        return result;
    }

    void crashIfNotArchive(contentType type)
    {
        std::string filename;
        switch (type) {
            case contentType::custom:
                filename = CUSTOM_FILENAME;
                break;
            case contentType::cheats:
                filename = CHEATS_FILENAME;
                break;
            case contentType::fw:
                filename = FIRMWARE_FILENAME;
                break;
            case contentType::app:
                filename = APP_FILENAME;
                break;
            case contentType::bootloaders:
                filename = BOOTLOADER_FILENAME;
                break;
            case contentType::ams_cfw:
                filename = AMS_FILENAME;
                break;
            case contentType::apps:
                filename = CUSTOM_FILENAME;
                break;
            case contentType::full_pack:
                filename = FULLPACK_FILENAME;
                break;
            default:
                return;
        }
        if (!isArchive(filename)) {
            ProgressEvent::instance().setStatusCode(406);
            ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
        }
    }

    namespace {
        void failStage(long code)
        {
            ProgressEvent::instance().setStatusCode(code);
            ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
        }

        void cancelStage()
        {
            ProgressEvent::instance().setInterupt(true);
            ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
        }

        void installFullPack()
        {
            if (ProgressEvent::instance().getStatusCode() > 399) return;

            verify::ArchiveInfo info;
            std::string error;
            if (!verify::verifyArchive(FULLPACK_FILENAME, info, error)) {
                brls::Logger::error("Full pack verification failed: {}", error);
                if (ProgressEvent::instance().getInterupt()) {
                    cancelStage();
                }
                else {
                    failStage(406);
                }
                return;
            }

            s64 total = 0;
            if (R_SUCCEEDED(fs::getTotalStorageSD(total)) && static_cast<double>(info.uncompressedSize) * 1.1 > static_cast<double>(total)) {
                failStage(507);
                return;
            }

            const std::string text = fmt::format(
                "menus/full_pack/verified"_i18n,
                info.files, static_cast<double>(info.uncompressedSize) / 1048576.0);
            const int choice = showDialogBoxBlocking(text, "menus/full_pack/cancel"_i18n, "menus/full_pack/no_wipe"_i18n, "menus/full_pack/wipe"_i18n);
            if (choice == 0 || ProgressEvent::instance().getInterupt()) {
                cancelStage();
                return;
            }

            ProgressEvent::instance().setInterupt(false);

            if (choice == 2) {
                std::set<std::string> keep;
                for (const char* k : WIPE_ALWAYS_KEEP) keep.insert(k);
                for (const auto& folder : AMS_CONTENTS_KEEP) keep.insert(std::string(AMS_CONTENTS) + folder);
                keep.insert(getAppPath());
                keep.insert("/atmosphere/package3");
                keep.insert("/atmosphere/stratosphere.romfs");
                keep.insert("/atmosphere/logs");
                keep.insert("/atmosphere/fatal_reports");
                keep.insert("/config/aio-switch-updater");
                for (const auto& line : fs::readLineByLine(FILES_IGNORE)) {
                    const std::string clean = wipe::cleanKeepEntry(line);
                    if (!clean.empty()) keep.insert(clean);
                }

                ProgressEvent::instance().setStep(0);
                const auto stats = wipe::wipeExcept(
                    ROOT_PATH, keep,
                    [](const std::string& dir) { return fs::removeDir(dir); },
                    [](size_t done, size_t count) {
                        ProgressEvent::instance().setTotalSteps(static_cast<int>(count) + 1);
                        ProgressEvent::instance().setStep(static_cast<int>(done));
                    });
                brls::Logger::info("SD wipe: removed {}, kept {}, failed {}", stats.removed, stats.kept, stats.failed);
            }

            ProgressEvent::instance().setStep(0);
