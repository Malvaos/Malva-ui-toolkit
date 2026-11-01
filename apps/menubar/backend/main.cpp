// A stand-in backend for the menu bar UI: opens a screen-sized window with the bar at its top, and answers it
// with a sample machine.
//
//   menubar [--dark] [--accent=#3385ff] [--desk=#rrggbb] [--backdrop=blurred.png] [--under=desktop.ppm]
//           [--size=1440x900] [--battery-percent] [--live-clock] [--connect-ms=1100] [--power-seconds=30]
//           [--popup=wifi] [--at=SECONDS:EVENT ...] [--log-popups]
//           [--snapshot=out.ppm [--snapshot-ms=700]] [--record=dir [--fps=25] [--seconds=10]]
//
// --snapshot renders the window once and exits. --record writes the window as numbered PPM frames (named
// with the time they were taken) while it runs, for filming what a person or a script does.
// --under is a stand-in for the rest of the desktop: a P6 .ppm drawn behind the window in snapshots and
// recordings, as a compositor would (the window itself is clear wherever the bar and its menus are not).
// --log-popups prints each change of what is open ("popup: menu-1"), and every request, as it happens.
// --at schedules something the system does, to be seen (seconds from the start, or from the first frame when recording): `notify:mail`, `notify:calendar`, `notify:store`,
// `notify:chat`, `volume:+0.1`, `volume:-0.1`, `mute`, `brightness:+0.1`, `popup:wifi`, `close`, `power`.
#include "app.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace barui;

namespace {

bool takeValue(const char* arg, const char* name, std::string& value)
{
    const std::size_t length = std::strlen(name);
    if (std::strncmp(arg, name, length) != 0 || arg[length] != '=')
        return false;
    value = arg + length + 1;
    return true;
}

struct Under {
    unsigned width = 0, height = 0;
    std::vector<unsigned char> rgb;
};

bool loadPpm(const std::string& path, Under& out)
{
    std::FILE* in = std::fopen(path.c_str(), "rb");
    if (!in)
        return false;
    unsigned w = 0, h = 0, max = 0;
    const bool ok = std::fscanf(in, "P6 %u %u %u", &w, &h, &max) == 3 && max == 255 && std::fgetc(in) != EOF;
    if (ok) {
        out.width = w;
        out.height = h;
        out.rgb.resize(std::size_t(w) * h * 3);
        if (std::fread(out.rgb.data(), 1, out.rgb.size(), in) != out.rgb.size())
            out.rgb.clear();
    }
    std::fclose(in);
    return !out.rgb.empty();
}

// The window over what is under it, as a compositor blends it. A snapshot's pixels are premultiplied (a
// half-clear white is 128,128,128,128), so what is behind shows through by (1 - alpha) and nothing else is
// multiplied again.
bool writePpm(const slint::SharedPixelBuffer<slint::Rgba8Pixel>& pixels, const std::string& path, const Under& under)
{
    std::FILE* out = std::fopen(path.c_str(), "wb");
    if (!out)
        return false;
    const unsigned w = pixels.width(), h = pixels.height();
    std::fprintf(out, "P6\n%u %u\n255\n", w, h);
    std::vector<unsigned char> row(std::size_t(w) * 3);
    const slint::Rgba8Pixel* p = pixels.begin();
    for (unsigned y = 0; y < h; ++y) {
        for (unsigned x = 0; x < w; ++x, ++p) {
            const unsigned char a = p->a;
            unsigned char bg[3] = {0, 0, 0};
            if (x < under.width && y < under.height) {
                const unsigned char* u = &under.rgb[(std::size_t(y) * under.width + x) * 3];
                bg[0] = u[0];
                bg[1] = u[1];
                bg[2] = u[2];
            }
            const unsigned char src[3] = {p->r, p->g, p->b};
            for (int c = 0; c < 3; ++c)
                row[x * 3 + c] = (unsigned char)std::min(255, src[c] + (bg[c] * (255 - a) + 127) / 255);
        }
        std::fwrite(row.data(), 1, row.size(), out);
    }
    std::fclose(out);
    return true;
}

struct Scheduled {
    double at;
    std::string what;
};

void fire(App& app, const std::string& what)
{
    auto ui = app.window();
    const std::size_t colon = what.find(':');
    const std::string verb = what.substr(0, colon);
    const std::string arg = colon == std::string::npos ? "" : what.substr(colon + 1);
    if (verb == "notify") {
        if (arg == "mail")
            app.notify("Mail", "mail", "Aria Patel", "Are we still on for lunch? I booked the corner table by the window.", "mail", {"Reply", "Archive"});
        else if (arg == "calendar")
            app.notify("Calendar", "calendar", "Stand-up in 5 minutes", "Room 2 · 10:00", "calendar", {"Join", "Snooze"}, true);
        else if (arg == "store")
            app.notify("Store", "store", "Files 2.4 is ready", "A faster list view and a quieter sidebar.", "store", {"Update"});
        else
            app.notify("Music", "music", "New release", "Lumen has a new single out today.", "music");
    } else if (verb == "volume") {
        app.volumeKey(std::stof(arg));
    } else if (verb == "brightness") {
        app.brightnessKey(std::stof(arg));
    } else if (verb == "mute") {
        app.muteKey();
    } else if (verb == "popup") {
        ui->invoke_show_popup(slint::SharedString(arg));
    } else if (verb == "close") {
        ui->invoke_close_popup();
    } else if (verb == "power") {
        ui->invoke_open_power();
    } else {
        std::fprintf(stderr, "menubar: unknown event %s\n", what.c_str());
    }
}

} // namespace

int main(int argc, char** argv)
{
    Options options;
    std::string snapshot, size = "1440x900", record, under, popup, value;
    std::vector<Scheduled> schedule;
    bool logPopups = false;
    int fps = 25, snapshotMs = 700;
    double seconds = 10;
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (!std::strcmp(arg, "--dark"))
            options.dark = true;
        else if (!std::strcmp(arg, "--light"))
            options.dark = false;
        else if (!std::strcmp(arg, "--battery-percent"))
            options.showBatteryPercent = true;
        else if (!std::strcmp(arg, "--live-clock"))
            options.liveClock = true;
        else if (!std::strcmp(arg, "--log-popups"))
            logPopups = true;
        else if (takeValue(arg, "--accent", options.accent) || takeValue(arg, "--desk", options.desk)
                 || takeValue(arg, "--backdrop", options.backdrop) || takeValue(arg, "--under", under)
                 || takeValue(arg, "--size", size) || takeValue(arg, "--snapshot", snapshot)
                 || takeValue(arg, "--record", record) || takeValue(arg, "--popup", popup))
            ;
        else if (takeValue(arg, "--connect-ms", value))
            options.connectMs = std::stoi(value);
        else if (takeValue(arg, "--power-seconds", value))
            options.powerSeconds = std::stoi(value);
        else if (takeValue(arg, "--fps", value))
            fps = std::stoi(value);
        else if (takeValue(arg, "--seconds", value))
            seconds = std::stod(value);
        else if (takeValue(arg, "--snapshot-ms", value))
            snapshotMs = std::stoi(value);
        else if (takeValue(arg, "--at", value)) {
            const std::size_t colon = value.find(':');
            if (colon == std::string::npos) {
                std::fprintf(stderr, "menubar: --at wants SECONDS:EVENT\n");
                return 2;
            }
            schedule.push_back({std::stod(value.substr(0, colon)), value.substr(colon + 1)});
        } else if (!std::strcmp(arg, "--help") || !std::strcmp(arg, "-h")) {
            std::puts("usage: menubar [--dark] [--accent=#rrggbb] [--desk=#rrggbb] [--backdrop=blurred.png]\n"
                      "               [--under=desktop.ppm] [--size=WxH] [--battery-percent] [--live-clock]\n"
                      "               [--connect-ms=N] [--power-seconds=N] [--popup=NAME] [--at=SECONDS:EVENT ...]\n"
                      "               [--log-popups]\n"
                      "               [--snapshot=out.ppm [--snapshot-ms=N]] [--record=dir [--fps=N] [--seconds=S]]");
            return 0;
        } else {
            std::fprintf(stderr, "menubar: unknown option %s\n", arg);
            return 2;
        }
    }

    Under underPixels;
    if (!under.empty() && !loadPpm(under, underPixels)) {
        std::fprintf(stderr, "menubar: cannot read %s (a P6 .ppm)\n", under.c_str());
        return 2;
    }

    App app(options);
    app.onRequest = [](const std::string& what) { std::printf("request: %s\n", what.c_str()); std::fflush(stdout); };
    if (logPopups) {
        app.onPopup = [](const std::string& name) { std::printf("popup: %s\n", name.c_str()); std::fflush(stdout); };
    }
    auto ui = app.window();

    unsigned width = 1440, height = 900;
    std::sscanf(size.c_str(), "%ux%u", &width, &height);
    ui->window().set_size(slint::LogicalSize({float(width), float(height)}));

    const auto start = std::chrono::steady_clock::now();
    auto since = [&] { return (long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count(); };

    // Things the system does, at their times.
    std::ofstream eventLog;
    if (!record.empty()) {
        std::filesystem::create_directories(record);
        eventLog.open(record + "/events.txt");
    }
    ui->show();
    const auto scheduleAll = [&] {
        for (const Scheduled& s : schedule) {
            slint::Timer::single_shot(std::chrono::milliseconds(long(s.at * 1000)), [&app, &eventLog, &since, what = s.what] {
                fire(app, what);
                if (eventLog.is_open())
                    eventLog << since() << " " << what << std::endl;
            });
        }
    };
    // Recording: the times are from the first frame, so a script that follows the frames agrees with them.
    if (record.empty())
        scheduleAll();
    if (!popup.empty())
        slint::Timer::single_shot(std::chrono::milliseconds(150), [ui, popup] { ui->invoke_show_popup(slint::SharedString(popup)); });

    if (snapshot.empty() && record.empty()) {
        slint::run_event_loop();
        return 0;
    }

    int status = 0;
    if (!snapshot.empty()) {
        slint::Timer::single_shot(std::chrono::milliseconds(snapshotMs), [&] {
            if (auto pixels = ui->window().take_snapshot(); pixels && writePpm(*pixels, snapshot, underPixels)) {
                std::printf("wrote %s (%ux%u)\n", snapshot.c_str(), pixels->width(), pixels->height());
            } else {
                std::fprintf(stderr, "menubar: could not take a snapshot with this renderer\n");
                status = 1;
            }
            slint::quit_event_loop();
        });
        slint::run_event_loop();
        return status;
    }

    // Recording: a frame every 1/fps second, named with its number and the milliseconds since the start.
    const std::string frames = record + "/frames";
    std::filesystem::create_directories(frames);
    int frame = 0;
    slint::Timer recorder;
    recorder.start(slint::TimerMode::Repeated, std::chrono::milliseconds(1000 / fps), [&] {
        const long ms = since();
        if (frame == 0)
            scheduleAll();
        if (ms > long(seconds * 1000)) {
            slint::quit_event_loop();
            return;
        }
        if (auto pixels = ui->window().take_snapshot()) {
            char name[64];
            std::snprintf(name, sizeof name, "frame-%05d-%06ldms.ppm", frame++, ms);
            writePpm(*pixels, frames + "/" + name, underPixels);
        }
    });
    slint::run_event_loop();
    std::printf("recorded %d frames in %s\n", frame, frames.c_str());
    return 0;
}
