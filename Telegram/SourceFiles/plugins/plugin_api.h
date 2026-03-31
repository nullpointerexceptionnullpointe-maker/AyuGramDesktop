#pragma once
#include <stdint.h>

#if defined(_WIN32)
  #define AYUPLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
  #define AYUPLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#define AYUPLUGIN_API_VERSION 1

enum AyuEventType : uint32_t {
    AYU_EVENT_MESSAGE_RECEIVED = 1,
    AYU_EVENT_MESSAGE_EDITED   = 2,
    AYU_EVENT_MESSAGE_DELETED  = 3,
    AYU_EVENT_SELF_SENT        = 4,
    AYU_EVENT_CHAT_OPENED      = 5
};

struct AyuMessageEvent {
    int64_t chat_id;
    int64_t sender_id;
    int64_t message_id;
    const char *text_utf8;
    int32_t is_outgoing;
    int32_t is_edited;
};

struct AyuChatEvent {
    int64_t chat_id;
};

struct AyuEvent {
    AyuEventType type;
    union {
        AyuMessageEvent message;
        AyuChatEvent chat;
    };
};

typedef void (*AyuEventCallback)(const AyuEvent *event, void *user_data);

struct AyuHostAPI {
    uint32_t api_version;
    void (*log)(const char *utf8);
    int (*subscribe)(AyuEventType type, AyuEventCallback callback, void *user_data);
    int (*send_text)(int64_t chat_id, const char *text_utf8);
    int64_t (*current_chat_id)();
};

struct AyuPluginInfo {
    const char *id;
    const char *name;
    const char *version;
};

struct AyuPlugin {
    AyuPluginInfo info;
    int (*on_load)(const AyuHostAPI *host);
    void (*on_unload)();
};

AYUPLUGIN_EXPORT uint32_t plugin_api_version();
AYUPLUGIN_EXPORT AyuPlugin* plugin_init();
