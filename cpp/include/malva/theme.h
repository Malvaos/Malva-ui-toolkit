// The theme for C++ code, with or without Slint.
//
//   malva::Theme theme;                 // follows nothing, starts light
//   theme.set_mode(malva::Mode::Dark);
//   malva::Rgba accent = theme.colors().accent;
//
// With a Slint window, switch the kit's `Theme` global (the generated class is named `Theme` in your header):
//   malva::set_dark<Theme>(window, true);   // template: needs nothing from this header but the name
#pragma once
#include "color.h"
#include "tokens.h"

#include <functional>
#include <string_view>

namespace malva {

class Theme {
public:
    explicit Theme(Mode m = Mode::Light) : mode_(m) {}
    Mode mode() const { return mode_; }
    bool dark() const { return mode_ == Mode::Dark; }
    const Colors& colors() const { return malva::colors(mode_); }
    void set_mode(Mode m)
    {
        if (m == mode_)
            return;
        mode_ = m;
        if (on_change_)
            on_change_(mode_);
    }
    void toggle() { set_mode(dark() ? Mode::Light : Mode::Dark); }
    // Called after the mode changes, so an application can push the new colours to its toolkit.
    void on_change(std::function<void(Mode)> f) { on_change_ = std::move(f); }

    // A role by its token name ("accent-hover"); nullopt for an unknown name.
    std::optional<Rgba> get(std::string_view name) const
    {
        for (const Role& r : roles) {
            if (r.name == name)
                return colors().*(r.member);
        }
        return std::nullopt;
    }

private:
    Mode mode_;
    std::function<void(Mode)> on_change_;
};

// Switches the kit's Slint `Theme` global on a window. GlobalT is the generated class, e.g. `Theme`.
template<class GlobalT, class WindowPtr>
void set_dark(const WindowPtr& window, bool dark)
{
    window->template global<GlobalT>().set_dark(dark);
}

}  // namespace malva
