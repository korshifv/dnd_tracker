import QtQuick
import QtQuick.Layouts
import QtCore
import DndTracker

Item {
    id: host
    required property string filePath
    signal backRequested()

    // One setting, one pair of QML views, on every platform. Layout differences
    // inside the views are responsive to available width rather than split into
    // separate mobile/desktop implementations.
    Settings {
        id: uiSettings
        category: "ui"
        property string characterSheetMode: "classic"
    }

    function normalizedMode() {
        return uiSettings.characterSheetMode === "interactive"
                ? "interactive"
                : "classic"
    }

    function switchMode() {
        if (normalizedMode() === "classic") {
            if (!classicPage.save())
                return
            interactivePage.reload()
            uiSettings.characterSheetMode = "interactive"
        } else {
            if (!interactivePage.save())
                return
            classicPage.reload()
            uiSettings.characterSheetMode = "classic"
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: host.normalizedMode() === "classic" ? 0 : 1

        ClassicCharacterSheetBook {
            id: classicPage
            filePath: host.filePath
            onBackRequested: host.backRequested()
            onModeSwitchRequested: host.switchMode()
        }

        CharacterSheetPage {
            id: interactivePage
            filePath: host.filePath
            onBackRequested: host.backRequested()
            onModeSwitchRequested: host.switchMode()
        }
    }
}
