// A stand-in backend for the dock UI: opens a window with the dock in it, on a plain "desk".
//
//   dock [--dark] [--edge=bottom|left|right] [--accent=#3385ff] [--icon-size=56] [--magnification=1.65]
//        [--no-launcher] [--autohide] [--no-entrance] [--desk=#rrggbb] [--size=WxH]
//        [--snapshot=out.ppm] [--record=dir [--fps=20] [--seconds=6]]
//
// --snapshot renders the window once and exits. --record writes the window as numbered PPM frames
// (named with the time they were taken) while it runs, for filming what a person or a script does.
#include "app.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>

using namespace dockui;

static bool takeValue(const char* arg, const char* name, std::string& value)
{
    const std::size_t length = std::strlen(name);
    if (std::strncmp(arg, name, length) != 0 || arg[length] != '=')
        return false;
    value = arg + length + 1;
    return true;
}

static bool writePpm(const slint::SharedPixelBuffer<slint::Rgba8Pixel>& pixels, const std::string& path)
{
    std::FILE* out = std::fopen(path.c_str(), "wb");
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
    options.desk = "#6f6fb8";
    options.deferFill = true;
    std::string snapshot, size, record, value;
    int fps = 20;
    double seconds = 6;
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (!std::strcmp(arg, "--dark"))
            options.dark = true;
        else if (!std::strcmp(arg, "--light"))
            options.dark = false;
        else if (!std::strcmp(arg, "--no-launcher"))
            options.launcher = false;
        else if (!std::strcmp(arg, "--autohide"))
            options.autoHide = true;
        else if (!std::strcmp(arg, "--no-entrance"))
            options.animateEntrance = false;
        else if (takeValue(arg, "--edge", options.edge) || takeValue(arg, "--accent", options.accent)
                 || takeValue(arg, "--desk", options.desk) || takeValue(arg, "--size", size)
                 || takeValue(arg, "--snapshot", snapshot) || takeValue(arg, "--record", record))
            ;
        else if (takeValue(arg, "--icon-size", value))
            options.iconSize = std::stof(value);
        else if (takeValue(arg, "--magnification", value))
            options.magnification = std::stof(value);
        else if (takeValue(arg, "--fps", value))
            fps = std::stoi(value);
        else if (takeValue(arg, "--seconds", value))
            seconds = std::stod(value);
        else if (takeValue(arg, "--launch-ms", value))
            options.launchMs = std::stoi(value);
        else if (!std::strcmp(arg, "--help") || !std::strcmp(arg, "-h")) {
            std::puts("usage: dock [--dark] [--edge=bottom|left|right] [--accent=#rrggbb] [--icon-size=N]\n"
                      "            [--magnification=X] [--no-launcher] [--autohide] [--no-entrance] [--desk=#rrggbb]\n"
                      "            [--size=WxH] [--snapshot=out.ppm] [--record=dir [--fps=N] [--seconds=S]]");
            return 0;
        } else {
            std::fprintf(stderr, "dock: unknown option %s\n", arg);
            return 2;
        }
    }

    App app(options);
    auto ui = app.window();

    unsigned width = 1100, height = 440;
    std::sscanf(size.c_str(), "%ux%u", &width, &height);
    ui->window().set_size(slint::LogicalSize({float(width), float(height)}));

    if (snapshot.empty() && record.empty()) {
        ui->show();
        slint::Timer::single_shot(std::chrono::milliseconds(1), [&] { app.sync(); });
        slint::run_event_loop();
        return 0;
    }

    int status = 0;
    ui->show();
    if (!snapshot.empty()) {
        slint::Timer::single_shot(std::chrono::milliseconds(1), [&] { app.sync(); });
        // Long enough for the icons to have arrived.
        slint::Timer::single_shot(std::chrono::milliseconds(2200), [&] {
            if (auto pixels = ui->window().take_snapshot(); pixels && writePpm(*pixels, snapshot)) {
                std::printf("wrote %s (%ux%u)\n", snapshot.c_str(), pixels->width(), pixels->height());
            } else {
                std::fprintf(stderr, "dock: could not take a snapshot with this renderer\n");
                status = 1;
            }
            slint::quit_event_loop();
        });
        slint::run_event_loop();
        return status;
    }

    // Recording: a frame every 1/fps second, named with its number and the milliseconds since the start.
    std::filesystem::create_directories(record);
    const auto start = std::chrono::steady_clock::now();
    int frame = 0;
    slint::Timer recorder;
    recorder.start(slint::TimerMode::Repeated, std::chrono::milliseconds(1000 / fps), [&] {
        // The window is up by the first tick: fill the dock now, so its entrance is on film.
        if (frame == 0)
            app.sync();
        const auto now = std::chrono::steady_clock::now();
        const long ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        if (ms > long(seconds * 1000)) {
            slint::quit_event_loop();
            return;
        }
        if (auto pixels = ui->window().take_snapshot()) {
            char name[64];
            std::snprintf(name, sizeof name, "frame-%05d-%06ldms.ppm", frame++, ms);
            writePpm(*pixels, record + "/" + name);
        }
    });
    slint::run_event_loop();
    std::printf("recorded %d frames in %s\n", frame, record.c_str());
    return 0;
}
