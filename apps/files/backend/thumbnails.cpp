#include "thumbnails.h"

#include <algorithm>
#include <cctype>

namespace files {

std::optional<slint::Image> Thumbnails::get(const Item& item)
{
    if (item.kind != Kind::Image || item.bytes == 0 || item.bytes > kMaxBytes)
        return std::nullopt;

    // The formats the Slint runtime reads.
    std::string ext = item.path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".svg" && ext != ".gif" && ext != ".bmp")
        return std::nullopt;

    const std::string key = item.path.string();
    if (const auto it = m_cache.find(key); it != m_cache.end() && it->second.modified == item.modified)
        return it->second.image;

    if (m_budget <= 0)
        return std::nullopt;
    --m_budget;

    Entry entry;
    entry.modified = item.modified;
    slint::Image image = slint::Image::load_from_path(slint::SharedString(key));
    if (image.size().width > 0 && image.size().height > 0)
        entry.image = image;

    if (m_cache.size() >= kMaxCached)
        m_cache.clear();
    return m_cache.insert_or_assign(key, std::move(entry)).first->second.image;
}

} // namespace files
