import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DndTracker

ApplicationWindow {
    id: window
    visible: true
    width: 1180
    height: 780
    minimumWidth: 360
    minimumHeight: 560
    title: "DnD Tracker"
    color: Theme.background

    property int section: 0
    readonly property bool compact: width < 760

    function showCharacter(path) {
        contentStack.push(characterSheetComponent, { filePath: path })
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            visible: !window.compact
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            color: Theme.surface
            border.width: 1
            border.color: Theme.border

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10

                Label {
                    text: "DnD Tracker"
                    color: Theme.text
                    font.pixelSize: 21
                    font.weight: Font.Bold
                    Layout.bottomMargin: 10
                }

                Repeater {
                    model: [
                        { text: "Инициатива", index: 0 },
                        { text: "Персонажи", index: 1 },
                        { text: "Заметки", index: 2 }
                    ]
                    delegate: AppButton {
                        required property var modelData
                        Layout.fillWidth: true
                        text: modelData.text
                        primary: window.section === modelData.index
                        onClicked: {
                            window.section = modelData.index
                            while (contentStack.depth > 1) contentStack.pop()
                        }
                    }
                }

                Item { Layout.fillHeight: true }
                Label {
                    Layout.fillWidth: true
                    text: "Qt Quick · QML"
                    color: Theme.textMuted
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        StackView {
            id: contentStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            initialItem: Item {
                StackLayout {
                    anchors.fill: parent
                    currentIndex: window.section
                    InitiativePage { onOpenCharacter: path => window.showCharacter(path) }
                    CharactersPage { onOpenCharacter: path => window.showCharacter(path) }
                    NotesPage {}
                }
            }
        }
    }

    footer: TabBar {
        visible: window.compact && contentStack.depth === 1
        currentIndex: window.section
        height: visible ? 62 : 0
        background: Rectangle {
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
        }
        onCurrentIndexChanged: if (currentIndex >= 0) window.section = currentIndex
        TabButton { text: "Инициатива" }
        TabButton { text: "Персонажи" }
        TabButton { text: "Заметки" }
    }

    Component {
        id: characterSheetComponent
        CharacterSheetPage { onBackRequested: contentStack.pop() }
    }

    Popup {
        id: errorPopup
        width: Math.min(window.width - 32, 520)
        x: (window.width - width) / 2
        y: 18
        padding: 14
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            color: Theme.surfaceRaised
            radius: Theme.radius
            border.width: 1
            border.color: Theme.danger
        }
        contentItem: Label {
            text: App.lastError
            color: Theme.text
            wrapMode: Text.WordWrap
        }
    }

    Connections {
        target: App
        function onLastErrorChanged() {
            if (App.lastError.length > 0) errorPopup.open()
        }
    }
}
