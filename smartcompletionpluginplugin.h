#ifndef SMARTCOMPLETIONPLUGIN_H
#define SMARTCOMPLETIONPLUGIN_H

#include <extensionsystem/iplugin.h>

class QPlainTextEdit;

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
    void triggerAction() const;

private:
    /// completion macro:Q_PROPERTY
    void completionProperty(QPlainTextEdit *editor) const;
};

} // namespace Internal
} // namespace SmartCompletionPlugin

#endif // SMARTCOMPLETIONPLUGIN_H

