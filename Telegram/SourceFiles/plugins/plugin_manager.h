#pragma once

#include <QObject>
#include <QLibrary>
#include <QVector>
#include <QString>
#include <unordered_map>
#include <vector>

#include "plugin_api.h"

class PluginManager final : public QObject {
    Q_OBJECT

public:
    static PluginManager& instance();

    void loadAll(const QString &pluginsDir);
    void unloadAll();
    void emitEvent(const AyuEvent &event);

    static void hostLog(const char *utf8);
    static int hostSubscribe(AyuEventType type, AyuEventCallback cb, void *ud);
    static int hostSendText(int64_t chatId, const char *textUtf8);
    static int64_t hostCurrentChatId();

private:
    PluginManager() = default;

    struct LoadedPlugin {
        QString path;
        QLibrary library;
        AyuPlugin *plugin = nullptr;
        using InitFn = AyuPlugin* (*)();
        using VerFn = uint32_t (*)();
        VerFn versionFn = nullptr;
        InitFn initFn = nullptr;
    };

    struct Subscription {
        AyuEventCallback cb = nullptr;
        void *ud = nullptr;
    };

    QVector<LoadedPlugin> _plugins;
    std::unordered_map<uint32_t, std::vector<Subscription>> _subs;
};
