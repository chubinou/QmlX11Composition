#include "X11Utils.hpp"

bool findVisualFormat(xcb_connection_t* conn, xcb_visualid_t visual,
                  xcb_render_pictformat_t* formatOut = nullptr,
                  uint8_t* depthOut = nullptr
                  ) {
    xcb_render_query_pict_formats_cookie_t pictFormatC = xcb_render_query_pict_formats(conn);
    auto pictFormatR = wrap_cptr(xcb_render_query_pict_formats_reply(conn, pictFormatC, nullptr));

    if (!pictFormatR) {
        return false;
    }

    auto screenIt = xcb_render_query_pict_formats_screens_iterator(pictFormatR.get());
    for (; screenIt.rem > 0; xcb_render_pictscreen_next(&screenIt))
    {
        xcb_render_pictscreen_t* pictScreen = screenIt.data;
        auto depthIt = xcb_render_pictscreen_depths_iterator(pictScreen);
        for (; depthIt.rem > 0; xcb_render_pictdepth_next(&depthIt))
        {
            xcb_render_pictdepth_t* pictDepth = depthIt.data;
            auto visualIt = xcb_render_pictdepth_visuals_iterator(pictDepth);
            for (; visualIt.rem > 0; xcb_render_pictvisual_next(&visualIt))
            {
                xcb_render_pictvisual_t* pictVisual = visualIt.data;
                if (pictVisual->visual == visual)
                {
                    if (formatOut)
                        *formatOut = pictVisual->format;
                    if (depthOut)
                        *depthOut = pictDepth->depth;
                    return true;
                }
            }
        }
    }

    return false;
}
