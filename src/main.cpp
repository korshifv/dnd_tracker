#include "AppController.h"
#include "CharacterRepositoryModel.h"
#include "ClassicSheetStore.h"
#include "InitiativeModel.h"
#include "NotesModel.h"
#include "Storage.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication::setApplicationName("dnd_tracker");
    QCoreApplication::setOrganizationName("dnd_tracker");
    QCoreApplication::setApplicationVersion("2.0.0");

    QGuiApplication app(argc, argv);

    // Keep Qt Quick Controls visually consistent and palette-driven on every
    // target. Platform defaults (Material on Android, Windows style, Fusion)
    // do not all consume the same palette roles.
    QQuickStyle::setStyle("Basic");

    if (!Storage::ensureDirs()) {
        qCritical() << "Unable to create application data directory:"
                    << Storage::appDataDir();
        return 1;
    }

    CharacterRepositoryModel characters;
    InitiativeModel initiative;
    NotesModel notes;
    AppController controller(&characters, &initiative, &notes);
    ClassicSheetStore classicSheetStore;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("App", &controller);
    engine.rootContext()->setContextProperty("Characters", &characters);
    engine.rootContext()->setContextProperty("Initiative", &initiative);
    engine.rootContext()->setContextProperty("Notes", &notes);
    engine.rootContext()->setContextProperty("ClassicStore", &classicSheetStore);

    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     &initiative, &InitiativeModel::flush);

    engine.loadFromModule("DndTracker", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;

    return app.exec();
}
