import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import DndTracker

Item {
    id: page
    signal openCharacter(string filePath)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                Label { text: "Персонажи"; color: Theme.text; font.pixelSize: 24; font.weight: Font.Bold }
                Label { text: "Локальное хранилище LSS / JSON"; color: Theme.textMuted }
            }
            AppButton { text: "Обновить"; onClicked: Characters.refresh() }
            AppButton { text: "+ Импорт"; primary: true; onClicked: fileDialog.open() }
        }

        GridView {
            id: grid
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: Characters
            cellWidth: Math.max(280, width < 680 ? width : width / Math.floor(width / 320))
            cellHeight: 184
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            delegate: Item {
                required property int index
                required property string filePath
                required property string name
                required property string charClass
                required property int level
                required property int hp
                required property int hpMax
                required property int armorClass
                required property int initiative
                width: grid.cellWidth
                height: grid.cellHeight

                Surface {
                    anchors.fill: parent
                    anchors.margins: 6

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            ColumnLayout {
                                Layout.fillWidth: true
                                Label {
                                    Layout.fillWidth: true
                                    text: name
                                    color: Theme.text
                                    font.pixelSize: 18
                                    font.weight: Font.Bold
                                    elide: Text.ElideRight
                                }
                                Label {
                                    text: (charClass || "Без класса") + (level > 0 ? " · ур. " + level : "")
                                    color: Theme.textMuted
                                }
                            }
                            Label {
                                text: armorClass > 0 ? "КД " + armorClass : "КД —"
                                color: Theme.accentStrong
                                font.weight: Font.DemiBold
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: "HP " + hp + (hpMax > 0 ? "/" + hpMax : ""); color: Theme.text }
                            Label { text: "Иниц. " + (initiative >= 0 ? "+" : "") + initiative; color: Theme.text }
                            Item { Layout.fillWidth: true }
                        }

                        Item { Layout.fillHeight: true }

                        RowLayout {
                            Layout.fillWidth: true
                            AppButton { text: "Открыть"; primary: true; onClicked: page.openCharacter(filePath) }
                            AppButton { text: "В бой"; onClicked: Initiative.addCharacter(filePath, "") }
                            Item { Layout.fillWidth: true }
                            AppButton { text: "Удалить"; danger: true; onClicked: { removeDialog.row = index; removeDialog.open() } }
                        }
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: grid.count === 0
                text: "Персонажей пока нет\nИмпортируй JSON-файл"
                color: Theme.textMuted
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Импорт персонажа"
        nameFilters: ["JSON (*.json)"]
        onAccepted: App.importCharacter(selectedFile)
    }

    Dialog {
        id: removeDialog
        property int row: -1
        title: "Удалить персонажа?"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Yes | Dialog.Cancel
        onAccepted: Characters.removeAt(row)
    }
}
