#ifndef SMARTCOMPLETIONPLUGIN_H
#define SMARTCOMPLETIONPLUGIN_H

#include <extensionsystem/iplugin.h>

namespace SmartCompletionPlugin {
namespace Internal {

class SmartCompletionPluginPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "SmartCompletionPlugin.json")

public:
    SmartCompletionPluginPlugin();
    ~SmartCompletionPluginPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();

private slots:
    void triggerAction();
};

} // namespace Internal
} // namespace SmartCompletionPlugin

#endif // SMARTCOMPLETIONPLUGIN_H

