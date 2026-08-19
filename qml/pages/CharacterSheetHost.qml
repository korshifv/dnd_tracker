import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCore
import DndTracker

Item {
    id: host
    required property string filePath
    signal backRequested()

    Settings {
        id: uiSettings
        category: "ui"
        property string characterSheetMode: "interactive"
    }

    function normalizedMode() {
        return uiSettings.characterSheetMode === "classic" ? "classic" : "interactive"
    }

    function switchMode() {
        if (normalizedMode() === "interactive") {
            interactivePage.save()
            classicPage.reload()
            uiSettings.characterSheetMode = "classic"
        } else {
            classicPage.save()
            interactivePage.reload()
            uiSettings.characterSheetMode = "interactive"
        }
        settingsDrawer.close()
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: host.normalizedMode() === "classic" ? 1 : 0

        CharacterSheetPage {
            id: interactivePage
            filePath: host.filePath
            onBackRequested: host.backRequested()
        }

        ClassicCharacterSheetBook {
            id: classicPage
            filePath: host.filePath
            onBackRequested: host.backRequested()
        }
    }

    // A narrow visible handle keeps the edge drawer discoverable without
    // occupying character-sheet toolbar space. The drawer itself is also
    // fully swipeable from the left edge on touch devices.
    Rectangle {
        id: drawerHandle
        z: 50
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: 7
        height: 78
        radius: 4
        color: Theme.accent
        opacity: handleMouse.containsMouse ? 1.0 : 0.62

        MouseArea {
            id: handleMouse
            anchors.centerIn: parent
            width: 32
            height: parent.height + 24
            hoverEnabled: true
            onClicked: settingsDrawer.open()
        }
    }

    Drawer {
        id: settingsDrawer
        edge: Qt.LeftEdge
        width: Math.min(390, Math.max(290, host.width * 0.86))
        height: host.height
        modal: true
        dim: true
        interactive: true
        padding: 0
        topPadding: Qt.platform.os === "android" ? 28 : 0
        bottomPadding: Qt.platform.os === "android" ? 24 : 0

        background: Rectangle {
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
        }

        contentItem: ColumnLayout {
            spacing: 14

            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 16
                Layout.topMargin: 18
                spacing: 8

                Label {
                    Layout.fillWidth: true
                    text: "Настройки"
                    color: Theme.text
                    font.pixelSize: 22
                    font.weight: Font.Bold
                }

                AppButton {
                    text: "×"
                    implicitWidth: 44
                    onClicked: settingsDrawer.close()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                implicitHeight: modeColumn.implicitHeight + 28
                radius: Theme.radius
                color: Theme.surfaceRaised
                border.width: 1
                border.color: Theme.border

                ColumnLayout {
                    id: modeColumn
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 8

                    Label {
                        text: "Вид листа персонажа"
                        color: Theme.text
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                    }

                    Label {
                        Layout.fillWidth: true
                        text: host.normalizedMode() === "classic"
                              ? "Сейчас: классический вид"
                              : "Сейчас: интерактивный вид"
                        color: Theme.textMuted
                        wrapMode: Text.WordWrap
                    }

                    AppButton {
                        Layout.fillWidth: true
                        primary: true
                        text: host.normalizedMode() === "classic"
                              ? "Переключить в интерактивный вид"
                              : "Переключить в классический вид"
                        onClicked: host.switchMode()
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                text: "Классический вид повторяет структуру трёхстраничного бумажного листа D&D 5e; интерактивный — текущий интерфейс приложения. Оба режима редактируют один и тот же файл персонажа."
                color: Theme.textMuted
                wrapMode: Text.WordWrap
                font.pixelSize: 12
            }

            Item { Layout.fillHeight: true }
        }
    }
}
