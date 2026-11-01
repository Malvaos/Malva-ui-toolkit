// A stand-in backend for the Files UI: opens a window on a folder and browses the file system.
//
//   files [folder] [--dark] [--list] [--frameless] [--accent=#3385ff] [--icon-size=64]
//         [--size=980x640] [--snapshot=out.ppm]
//
// --snapshot renders the window once, writes it as a PPM file and exits: for testing the UI without
// anyone looking at it.
#include "app.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>

using namespace files;

static bool takeValue(const char* arg, const char* name, std::string& value)
{
    const std::size_t length = std::strlen(name);
    if (std::strncmp(arg, name, length) != 0 || arg[length] != '=')
        return false;
    value = arg + length + 1;
    return true;
}

static bool writePpm(const slint::SharedPixelBuffer<slint::Rgba8Pixel>& pixels, const char* path)
{
    std::FILE* out = std::fopen(path, "wb");
    if (!out)
        return false;
    std::fprintf(out, "P6\n%u %u\n255\n", pixels.width(), pixels.height());
    const slint::Rgba8Pixel* p = pixels.begin();
    for (std::size_t i = 0; i < std::size_t(pixels.width()) * pixels.height(); ++i) {
        const unsigned char rgb[3] = {p[i].r, p[i].g, p[i].b};
        std::fwrite(rgb, 1, 3, out);
    }
    std::fclose(out);
    return true;
}

int main(int argc, char** argv)
{
    Options options;
    std::string snapshot, size, value;
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (!std::strcmp(arg, "--dark"))
            options.dark = true;
        else if (!std::strcmp(arg, "--light"))
            options.dark = false;
        else if (!std::strcmp(arg, "--list"))
            options.list = true;
        else if (!std::strcmp(arg, "--frameless"))
            options.frameless = true;
        else if (takeValue(arg, "--accent", value))
            options.accent = value;
        else if (takeValue(arg, "--icon-size", value))
            options.iconSize = std::stof(value);
        else if (takeValue(arg, "--size", size) || takeValue(arg, "--snapshot", snapshot))
            ;
        else if (!std::strcmp(arg, "--help") || !std::strcmp(arg, "-h")) {
            std::puts("usage: files [folder] [--dark] [--list] [--frameless] [--accent=#rrggbb] [--icon-size=N]\n"
                      "             [--size=WxH] [--snapshot=out.ppm]");
            return 0;
        } else if (arg[0] != '-')
            options.start = arg;
        else {
            std::fprintf(stderr, "files: unknown option %s\n", arg);
            return 2;
        }
    }

    App app(options);
    auto ui = app.window();

    unsigned width = 0, height = 0;
    if (std::sscanf(size.c_str(), "%ux%u", &width, &height) == 2)
        ui->window().set_size(slint::LogicalSize({float(width), float(height)}));

    if (snapshot.empty()) {
        ui->run();
        return 0;
    }

    int status = 0;
    ui->show();
    // Long enough for the items to have arrived.
    slint::Timer::single_shot(std::chrono::milliseconds(1500), [&] {
        if (auto pixels = ui->window().take_snapshot(); pixels && writePpm(*pixels, snapshot.c_str())) {
            std::printf("wrote %s (%ux%u)\n", snapshot.c_str(), pixels->width(), pixels->height());
        } else {
            std::fprintf(stderr, "files: could not take a snapshot with this renderer\n");
            status = 1;
        }
        slint::quit_event_loop();
    });
    slint::run_event_loop();
    return status;
}
