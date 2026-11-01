// Thumbnails for the icon view. A stand-in: it decodes the picture itself, whole, and keeps what
// it decoded. A real backend generates small thumbnails off the UI's thread and caches them on
// disk; the UI only asks for a slint::Image and does not care where it came from.
#pragma once

#include "fs.h"

#include <slint.h>

#include <optional>
#include <unordered_map>

namespace files {

class Thumbnails
{
public:
    // The picture for an item, if it has one the UI can show.
    std::optional<slint::Image> get(const Item& item);

    // Stop decoding new pictures; the ones already decoded still show. A folder with thousands of
    // pictures should not be decoded whole on opening.
    void startFolder() { m_budget = kPerFolder; }

private:
    static constexpr int kPerFolder = 300;
    static constexpr std::uintmax_t kMaxBytes = 24u * 1000 * 1000;
    static constexpr std::size_t kMaxCached = 1500;

    struct Entry {
        std::int64_t modified = 0;
        std::optional<slint::Image> image;
    };
    std::unordered_map<std::string, Entry> m_cache;
    int m_budget = kPerFolder;
};

} // namespace files
