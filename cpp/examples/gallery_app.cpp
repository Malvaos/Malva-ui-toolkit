// A program that uses the kit from C++: the theme comes from malva::Theme (so the same colours are available
// to code that draws without Slint), the window is the one in gallery.slint, and a toast reports what was saved.
//
//   ./malva_gallery            opens the window
//   ./malva_gallery --dark     starts in the dark theme
#include "gallery.h"

#include "malva/slint_interop.h"
#include "malva/theme.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
    malva::Theme theme(argc > 1 && std::strcmp(argv[1], "--dark") == 0 ? malva::Mode::Dark : malva::Mode::Light);
    auto window = GalleryWindow::create();
    auto toasts = std::make_shared<slint::VectorModel<ToastItem>>();
    window->set_toasts(toasts);
    int next_id = 1;

    // The window owns "dark"; the C++ theme follows it, so code that draws by itself (a game view, a plot) can ask
    // theme.colors() for the same colours the widgets use.
    window->set_dark(theme.dark());
    theme.on_change([&](malva::Mode m) { std::printf("theme: %s, accent %s\n", m == malva::Mode::Dark ? "dark" : "light", malva::to_hex(theme.colors().accent).c_str()); });

    window->on_saved([&](slint::SharedString name) {
        ToastItem t;
        t.id = next_id++;
        t.tone = Tone::Success;
        t.title = "Saved";
        t.text = slint::SharedString("Hello, " + std::string(std::string_view(name)));
        toasts->push_back(t);
    });
    window->on_toast_gone([&](int id) {
        for (std::size_t i = 0; i < toasts->row_count(); ++i) {
            if (toasts->row_data(i)->id == id) {
                toasts->erase(i);
                break;
            }
        }
    });
    window->on_confirmed_reset([&] {
        window->set_user_name("");
        window->set_volume(40);
    });
    // Follow the window's theme switch (the segmented control) in the C++ theme.
    slint::Timer follow;
    follow.start(slint::TimerMode::Repeated, std::chrono::milliseconds(200), [&] { theme.set_mode(window->get_dark() ? malva::Mode::Dark : malva::Mode::Light); });
    window->run();
    return 0;
}
