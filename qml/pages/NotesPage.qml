import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DndTracker

Item {
    id: page
    property string currentPath: ""
    property string currentTitle: ""
    property string selectedFolder: ""

    function openNote(path, title) {
        if (currentPath.length) Notes.saveText(currentPath, editor.text)
        currentPath = path
        currentTitle = title
        editor.text = Notes.loadText(path)
        editor.enabled = true
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Label { text: "Заметки"; color: Theme.text; font.pixelSize: 24; font.weight: Font.Bold; Layout.fillWidth: true }
            AppButton { text: "+ Папка"; onClicked: folderDialog.open() }
            AppButton { text: "+ Заметка"; primary: true; onClicked: noteDialog.open() }
        }

        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: width < 720 ? Qt.Vertical : Qt.Horizontal

            Surface {
                SplitView.preferredWidth: 290
                SplitView.preferredHeight: 230
                SplitView.minimumWidth: 220
                SplitView.minimumHeight: 160

                ListView {
                    id: tree
                    anchors.fill: parent
                    anchors.margins: 8
                    clip: true
                    model: Notes
                    spacing: 3

                    delegate: Rectangle {
                        required property int index
                        required property string title
                        required property string relativePath
                        required property bool isFolder
                        required property int depth
                        width: ListView.view.width
                        height: 42
                        radius: Theme.radiusSmall
                        color: hover.hovered ? Theme.surfaceHover : "transparent"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 8 + depth * 14
                            anchors.rightMargin: 6
                            Label { text: isFolder ? "▸" : "•"; color: isFolder ? Theme.accent : Theme.textMuted }
                            Label { text: title; color: Theme.text; Layout.fillWidth: true; elide: Text.ElideRight }
                            AppButton { text: "×"; danger: true; implicitWidth: 42; visible: hover.hovered; onClicked: { removeDialog.row = index; removeDialog.open() } }
                        }

                        HoverHandler { id: hover }
                        TapHandler {
                            onTapped: {
                                if (isFolder) page.selectedFolder = relativePath
                                else page.openNote(relativePath, title)
                            }
                        }
                    }
                }
            }

            Surface {
                SplitView.fillWidth: true
                SplitView.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            Layout.fillWidth: true
                            text: page.currentTitle.length ? page.currentTitle : "Выберите заметку"
                            color: Theme.text
                            font.pixelSize: 18
                            font.weight: Font.Bold
                        }
                        Label { text: saveTimer.running ? "Изменения…" : ""; color: Theme.textMuted }
                    }

                    TextArea {
                        id: editor
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        enabled: false
                        color: Theme.text
                        placeholderText: "Markdown и [[wiki-ссылки]]"
                        placeholderTextColor: Theme.textMuted
                        wrapMode: TextArea.Wrap
                        selectByMouse: true
                        onTextChanged: if (enabled) saveTimer.restart()
                        background: Rectangle { color: Theme.surfaceRaised; radius: Theme.radiusSmall; border.color: Theme.border; border.width: 1 }
                    }
                }
            }
        }
    }

    Timer {
        id: saveTimer
        interval: 900
        repeat: false
        onTriggered: if (page.currentPath.length) Notes.saveText(page.currentPath, editor.text)
    }

    Dialog {
        id: noteDialog
        title: "Новая заметка"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok | Dialog.Cancel
        contentItem: TextField { id: noteName; placeholderText: "Имя заметки" }
        onAccepted: { Notes.createNote(page.selectedFolder, noteName.text); noteName.clear() }
    }

    Dialog {
        id: folderDialog
        title: "Новая папка"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok | Dialog.Cancel
        contentItem: TextField { id: folderName; placeholderText: "Имя папки" }
        onAccepted: { Notes.createFolder(page.selectedFolder, folderName.text); folderName.clear() }
    }

    Dialog {
        id: removeDialog
        property int row: -1
        title: "Удалить?"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Yes | Dialog.Cancel
        onAccepted: {
            Notes.removeAt(row)
            page.currentPath = ""
            page.currentTitle = ""
            editor.text = ""
            editor.enabled = false
        }
    }

    Component.onDestruction: if (page.currentPath.length) Notes.saveText(page.currentPath, editor.text)
}
