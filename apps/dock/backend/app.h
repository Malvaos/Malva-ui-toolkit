// The glue: holds the dock's model and answers the window's callbacks by changing it. All the look
// and all the motion are in the .slint files; nothing here draws.
#pragma once

#include "dock_model.h"

#include "dock.h" // generated from ui/dock.slint

#include <slint.h>

#include <memory>
#include <string>

namespace dockui {

struct Options {
    bool dark = false;
    std::string edge = "bottom"; // "bottom", "left" or "right"
    std::string accent;          // "#rrggbb", or empty for the default
    float iconSize = 56;
    float magnification = 1.65f;
    bool launcher = true;
    bool autoHide = false;
    bool animateEntrance = true;
    // How long a launching app hops before it is running. Real launches take as long as they take.
    int launchMs = 1400;
    // Leave the dock empty until `sync()` is called. A dock filled after its window is on the screen
    // gets its entrance seen; one filled before it opens already arrived.
    bool deferFill = false;
    // What shows where the dock is not (a stand-in desktop): "#rrggbb", or empty for clear.
    std::string desk;
};

class App
{
public:
    explicit App(const Options& options);

    slint::ComponentHandle<DockWindow> window() const { return m_ui; }
    Model& model() { return m_model; }
    // The window's rows follow the model. Call after changing it from outside.
    void sync();

    // What the backend does when the dock says so; the window's callbacks call these, and so can a test.
    void launch(int index);
    void openMenu(const std::string& target, int index);

private:
    void closeMenu();
    void chooseMenu(int row);
    void scheduleRemoval();
    void checkAutoHide();

    slint::ComponentHandle<DockWindow> m_ui;
    Model m_model;
    Options m_options;
    std::shared_ptr<slint::VectorModel<DockApp>> m_apps;
    std::shared_ptr<slint::VectorModel<MinimizedWindow>> m_minimized;
    std::shared_ptr<slint::VectorModel<MenuEntry>> m_menu;
    std::vector<MenuItem> m_menuItems;
    std::string m_menuTarget; // "app", "window" or "trash"
    int m_menuIndex = -1;
    slint::Timer m_removalTimer;
    slint::Timer m_hideTimer;
    int m_outsideTicks = 0;
};

} // namespace dockui
