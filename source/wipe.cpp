#include "wipe.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <vector>

namespace wipe {

    std::string normalize(const std::string& path)
    {
        std::string out;
        out.reserve(path.size());
        for (char c : path) {
            if (c == '\\') c = '/';
            if (c == '/' && !out.empty() && out.back() == '/') continue;
            out.push_back(c);
        }
        while (out.size() > 1 && out.back() == '/') out.pop_back();
        return out;
    }

    bool isPathPrefix(const std::string& prefix, const std::string& path)
    {
        if (prefix.empty() || prefix.size() > path.size()) return false;
        const bool same = std::equal(prefix.begin(), prefix.end(), path.begin(), [](char a, char b) {
            return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
        });
        if (!same) return false;
        if (prefix.size() == path.size()) return true;
        if (prefix == "/") return true;
        return path[prefix.size()] == '/';
    }

    std::string cleanKeepEntry(const std::string& line)
    {
        std::string s = line;
        // trim whitespace / CR
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
        s = s.substr(start);
        if (s.empty() || s[0] == '#') return "";
        s = normalize(s);
        if (s.empty()) return "";
        if (s[0] != '/') s.insert(s.begin(), '/');
        // Never let a protect entry be "/" (that would protect everything) –
        // ignore it so the wipe still does what the user asked for.
        if (s == "/") return "";
        return s;
    }

    namespace {
        bool removeEntry(const std::string& path, bool isDir, const std::function<bool(const std::string&)>& removeDirFn)
        {
            std::error_code ec;
            if (isDir) {
                if (removeDirFn && removeDirFn(path)) return true;
                std::filesystem::remove_all(path, ec);
                return !ec && !std::filesystem::exists(path, ec);
            }
            std::filesystem::remove(path, ec);
            return !ec;
        }

        void wipeDir(const std::string& dir, const std::set<std::string>& keepNorm, const std::function<bool(const std::string&)>& removeDirFn, Stats* stats,
                     const std::function<void(size_t, size_t)>& progress)
        {
            std::error_code ec;
            std::vector<std::pair<std::string, bool>> entries;  // path, isDir
            for (std::filesystem::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec)) {
                std::error_code dec;
                const bool isDir = it->is_directory(dec) && !it->is_symlink(dec);
                entries.emplace_back(it->path().string(), isDir);
            }

            size_t done = 0;
            const size_t total = entries.size();
            for (const auto& [path, isDir] : entries) {
                if (progress) progress(done, total);
                done++;

                const std::string e = normalize(path);

                bool protectedEntry = false;  // e is a kept path or inside one
                bool ancestorOfKept = false;  // a kept path lives below e
                for (const auto& k : keepNorm) {
                    if (isPathPrefix(k, e)) {
                        protectedEntry = true;
                        break;
                    }
                    if (isPathPrefix(e, k)) ancestorOfKept = true;
                }

                if (protectedEntry) {
                    if (stats) stats->kept++;
                    continue;
                }
                if (ancestorOfKept && isDir) {
                    // Enter it and clean around the protected path(s).
                    wipeDir(path, keepNorm, removeDirFn, nullptr, nullptr);
                    if (stats) stats->kept++;
                    continue;
                }
                if (removeEntry(path, isDir, removeDirFn)) {
                    if (stats) stats->removed++;
                }
                else if (stats) {
                    stats->failed++;
                }
            }
            if (progress) progress(total, total);
        }
    }  // namespace

    Stats wipeExcept(const std::string& root, const std::set<std::string>& keep, const std::function<bool(const std::string&)>& removeDirFn,
                     const std::function<void(size_t, size_t)>& progress)
    {
        Stats stats;
        std::set<std::string> keepNorm;
        for (const auto& k : keep) {
            const std::string clean = cleanKeepEntry(k);
            if (clean.empty()) continue;
            // Re-root the entry below `root` ("/" on the Switch).
            if (root == "/" || root.empty()) {
                keepNorm.insert(clean);
            }
            else {
                keepNorm.insert(normalize(root) + clean);
            }
        }
        wipeDir(root, keepNorm, removeDirFn, &stats, progress);
        return stats;
    }
}  // namespace wipe
