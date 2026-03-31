#include "plugin_manager.h"

#include <QDir>
#include <QDebug>
#include <QFileInfoList>

PluginManager& PluginManager::instance() {
    static PluginManager manager;
    return manager;
}

static AyuHostAPI g_hostApi = {
    AYUPLUGIN_API_VERSION,
    &PluginManager::hostLog,
    &PluginManager::hostSubscribe,
    &PluginManager::hostSendText,
    &PluginManager::hostCurrentChatId
};

void PluginManager::loadAll(const QString &pluginsDir) {
    QDir dir(pluginsDir);
    if (!dir.exists()) {
        qWarning() << "plugins dir does not exist:" << pluginsDir;
        return;
    }

#if defined(_WIN32)
    const auto files = dir.entryInfoList(QStringList() << "*.dll", QDir::Files);
#elif defined(__APPLE__)
    const auto files = dir.entryInfoList(QStringList() << "*.dylib", QDir::Files);
#else
    const auto files = dir.entryInfoList(QStringList() << "*.so", QDir::Files);
#endif

    for (const auto &fi : files) {
        LoadedPlugin lp;
        lp.path = fi.absoluteFilePath();
        lp.library.setFileName(lp.path);

        if (!lp.library.load()) {
            qWarning() << "Plugin load failed:" << lp.path << lp.library.errorString();
            continue;
        }

        lp.versionFn = reinterpret_cast<LoadedPlugin::VerFn>(
            lp.library.resolve("plugin_api_version"));
        lp.initFn = reinterpret_cast<LoadedPlugin::InitFn>(
            lp.library.resolve("plugin_init"));

        if (!lp.versionFn || !lp.initFn) {
            qWarning() << "Plugin exports missing:" << lp.path;
            lp.library.unload();
            continue;
        }

        if (lp.versionFn() != AYUPLUGIN_API_VERSION) {
            qWarning() << "Plugin API mismatch:" << lp.path;
            lp.library.unload();
            continue;
        }

        lp.plugin = lp.initFn();
        if (!lp.plugin || !lp.plugin->on_load) {
            qWarning() << "Plugin init failed:" << lp.path;
            lp.library.unload();
            continue;
        }

        if (lp.plugin->on_load(&g_hostApi) != 0) {
            qWarning() << "Plugin on_load returned error:" << lp.path;
            lp.library.unload();
            continue;
        }

        qInfo() << "Loaded plugin:" << lp.plugin->info.name << lp.plugin->info.version;
        _plugins.push_back(std::move(lp));
    }
}

void PluginManager::unloadAll() {
    for (auto &p : _plugins) {
        if (p.plugin && p.plugin->on_unload) {
            p.plugin->on_unload();
        }
        p.library.unload();
    }
    _plugins.clear();
    _subs.clear();
}

void PluginManager::emitEvent(const AyuEvent &event) {
    auto it = _subs.find((uint32_t)event.type);
    if (it == _subs.end()) return;

    for (const auto &sub : it->second) {
        if (sub.cb) {
            sub.cb(&event, sub.ud);
        }
    }
}

void PluginManager::hostLog(const char *utf8) {
    qInfo() << "[plugin]" << (utf8 ? utf8 : "");
}

int PluginManager::hostSubscribe(AyuEventType type, AyuEventCallback cb, void *ud) {
    if (!cb) return -1;
    instance()._subs[(uint32_t)type].push_back({cb, ud});
    return 0;
}

int64_t PluginManager::hostCurrentChatId() {
    return 0;
}

int PluginManager::hostSendText(int64_t chatId, const char *textUtf8) {
    if (!textUtf8 || !*textUtf8) return -1;
    qInfo() << "[plugin] SEND_TEXT stub:" << chatId << textUtf8;
    return 0;
}
