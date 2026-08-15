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

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: "Персонажи"
                    color: Theme.text
                    font.pixelSize: 24
                    font.weight: Font.Bold
                    elide: Text.ElideRight
                }
                Label {
                    Layout.fillWidth: true
                    text: "Локальное хранилище LSS / JSON"
                    color: Theme.textMuted
                    elide: Text.ElideRight
                }
            }

            Flow {
                Layout.fillWidth: true
                spacing: 8

                AppButton {
                    text: "Обновить"
                    onClicked: Characters.refresh()
                }
                AppButton {
                    text: "+ Импорт"
                    primary: true
                    onClicked: fileDialog.open()
                }
            }
        }

        GridView {
            id: grid
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: Characters
            cellWidth: width < 700
                       ? width
                       : width / Math.max(1, Math.floor(width / 360))
            cellHeight: width < 700 ? 236 : 198
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
                            spacing: 10

                            Label {
                                Layout.fillWidth: true
                                text: name
                                color: Theme.text
                                font.pixelSize: 18
                                font.weight: Font.Bold
                                elide: Text.ElideRight
                            }

                            Label {
                                text: armorClass > 0 ? "КД " + armorClass : "КД —"
                                color: Theme.accentStrong
                                font.weight: Font.DemiBold
                            }
                        }

                        Label {
                            Layout.fillWidth: true
                            text: (charClass || "Без класса") + (level > 0 ? " · ур. " + level : "")
                            color: Theme.textMuted
                            elide: Text.ElideRight
                        }

                        Flow {
                            Layout.fillWidth: true
                            spacing: 12

                            Label {
                                text: "HP " + hp + (hpMax > 0 ? "/" + hpMax : "")
                                color: Theme.text
                            }
                            Label {
                                text: "Иниц. " + (initiative >= 0 ? "+" : "") + initiative
                                color: Theme.text
                            }
                        }

                        Item { Layout.fillHeight: true }

                        Flow {
                            Layout.fillWidth: true
                            spacing: 8

                            AppButton {
                                text: "Открыть"
                                primary: true
                                onClicked: page.openCharacter(filePath)
                            }
                            AppButton {
                                text: "В бой"
                                onClicked: Initiative.addCharacter(filePath, "")
                            }
                            AppButton {
                                text: "Удалить"
                                danger: true
                                onClicked: {
                                    removeDialog.row = index
                                    removeDialog.open()
                                }
                            }
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
