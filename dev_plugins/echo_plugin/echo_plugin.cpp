#include "../../Telegram/SourceFiles/plugins/plugin_api.h"
#include <cstring>

static const AyuHostAPI *g_host = nullptr;

static void onMessage(const AyuEvent *event, void *) {
    if (!event || event->type != AYU_EVENT_MESSAGE_RECEIVED) return;
    const auto &m = event->message;

    if (!m.text_utf8) return;

    if (std::strcmp(m.text_utf8, "/ping") == 0) {
        g_host->send_text(m.chat_id, "pong");
    }
}

static int onLoad(const AyuHostAPI *host) {
    g_host = host;
    g_host->log("echo plugin loaded");
    return g_host->subscribe(AYU_EVENT_MESSAGE_RECEIVED, &onMessage, nullptr);
}

static void onUnload() {
    if (g_host) g_host->log("echo plugin unloaded");
}

static AyuPlugin g_plugin = {
    { "echo", "Echo Plugin", "0.1.0" },
    &onLoad,
    &onUnload
};

AYUPLUGIN_EXPORT uint32_t plugin_api_version() {
    return AYUPLUGIN_API_VERSION;
}

AYUPLUGIN_EXPORT AyuPlugin* plugin_init() {
    return &g_plugin;
}
