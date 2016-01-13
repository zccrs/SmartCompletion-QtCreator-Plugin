#include "smartcompletionpluginplugin.h"
#include "smartcompletionpluginconstants.h"
#include "smartcompletionplugin_global.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QRegularExpression>

#include <QtPlugin>

using namespace SmartCompletionPlugin::Internal;

SmartCompletionPluginPlugin::SmartCompletionPluginPlugin()
{
    // Create your members
}

SmartCompletionPluginPlugin::~SmartCompletionPluginPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
}

bool SmartCompletionPluginPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    QAction *action = new QAction(tr("SmartCompletionPlugin action"), this);
    Core::Command *cmd = Core::ActionManager::registerAction(action, Constants::ACTION_ID,
                                                             Core::Context(Core::Constants::C_GLOBAL));
    cmd->setDefaultKeySequence(QKeySequence(tr("Meta+Return")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));

    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("SmartCompletionPlugin"));
    menu->addAction(cmd);
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    return true;
}

void SmartCompletionPluginPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag SmartCompletionPluginPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

void SmartCompletionPluginPlugin::triggerAction() const
{
    const Core::EditorManager *editorManager = Core::EditorManager::instance();

    if (!editorManager)
        return;

    Core::IEditor *editor = editorManager->currentEditor();

    if (!editor)
        return;

    Core::IDocument *idoc = editor->document();

    if (!idoc)
        return;

    QPlainTextEdit *textEditor = qobject_cast<QPlainTextEdit *>(editor->widget());

    if (!textEditor)
        return;

    const QString &text = textEditor->toPlainText();
    int cursor_position = textEditor->textCursor().position();

    Global::CodeInfo symbol = Global::codeParse(text, cursor_position);

    switch (symbol.type) {
    case Global::PropertyType:
        completionProperty(textEditor);
        break;
    default:
        break;
    }

    QMessageBox::information(Core::ICore::mainWindow(),
                             tr("word type: %1").arg(symbol.type),
                             symbol.word + LS(" ") + QString::number(symbol.word.length()));
}

void SmartCompletionPluginPlugin::completionProperty(QPlainTextEdit *editor) const
{
    QRegularExpression rx(LS(".+"));

    const QRegularExpressionMatch &match = rx.match(editor->toPlainText(),
                                                    editor->textCursor().position());

    if(match.isValid()) {
        Global::Property property;

        Global::propertyParse(match.captured(), property);

        qDebug() << property;
    }
}
