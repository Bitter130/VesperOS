#pragma once

#include "ui/window.hpp"
#include "drivers/display.hpp"
#include "system/info.hpp"

namespace Vesper::UI {

class SysInfoWindow : public Window {
public:
    SysInfoWindow(int id, INT32 x, INT32 y, UINT32 w, UINT32 h);

    void OnRenderContent(Graphics::Framebuffer& fb) override;
};

} // namespace Vesper::UI
