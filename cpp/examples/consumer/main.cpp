// The theme comes from malva::Theme, the window from app.slint; both read the same tokens.
#include "app.h"

#include "malva/slint_interop.h"
#include "malva/theme.h"

#include <cstdio>
#include <cstring>

int main(int argc, char** argv)
{
    malva::Theme theme(malva::Mode::Light);
    auto app = App::create();
    app->set_dark(theme.dark());
    app->on_saved([&] { std::printf("saved; accent is %s\n", malva::to_hex(theme.colors().accent).c_str()); });
    std::printf("light accent %s, dark accent %s\n", malva::to_hex(malva::colors(malva::Mode::Light).accent).c_str(),
                malva::to_hex(malva::colors(malva::Mode::Dark).accent).c_str());
    // Creating the window needs a window system (a display, or xvfb-run on a server); --run also shows it.
    if (argc > 1 && std::strcmp(argv[1], "--run") == 0)
        app->run();
    return 0;
}
