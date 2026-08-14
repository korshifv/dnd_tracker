import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DndTracker

Item {
    id: page
    property string currentPath: ""
    property string currentTitle: ""
    property string selectedFolder: ""
    property bool previewMode: false
    signal openCharacter(string filePath)

    function openNote(path, title) {
        if (currentPath.length)
            Notes.saveText(currentPath, editor.text)
        currentPath = path
        currentTitle = title
        editor.text = Notes.loadText(path)
        editor.enabled = true
    }

    function markdownWithWikiLinks(source) {
        return source.replace(/\[\[([^\]]+)\]\]/g, function(_, title) {
            return "[" + title + "](wiki:" + encodeURIComponent(title) + ")"
        })
    }

    function openWikiLink(link) {
        if (!link.startsWith("wiki:"))
            return
        const title = decodeURIComponent(link.substring(5))
        const notePath = Notes.pathByTitle(title)
        if (notePath.length) {
            page.openNote(notePath, title)
            return
        }
        const characterPath = App.characterPathByName(title)
        if (characterPath.length)
            page.openCharacter(characterPath)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Заметки"
                color: Theme.text
                font.pixelSize: 24
                font.weight: Font.Bold
                Layout.fillWidth: true
            }
            AppButton { text: "+ Папка"; onClicked: folderDialog.open() }
            AppButton { text: "+ Заметка"; primary: true; onClicked: noteDialog.open() }
        }

        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: width < 720 ? Qt.Vertical : Qt.Horizontal

            Surface {
                SplitView.preferredWidth: 300
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
                    ScrollBar.vertical: ScrollBar {}

                    delegate: Rectangle {
                        id: row
                        required property int index
                        required property string title
                        required property string relativePath
                        required property bool isFolder
                        required property int depth
                        width: ListView.view.width
                        height: 44
                        radius: Theme.radiusSmall
                        color: page.currentPath === relativePath || page.selectedFolder === relativePath
                               ? Theme.surfaceHover
                               : (hover.hovered ? Theme.surfaceHover : "transparent")

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 8 + depth * 14
                            anchors.rightMargin: 4
                            spacing: 4
                            Label {
                                text: isFolder ? "▸" : "•"
                                color: isFolder ? Theme.accent : Theme.textMuted
                            }
                            Label {
                                text: title
                                color: Theme.text
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                            AppButton {
                                text: "✎"
                                implicitWidth: 38
                                onClicked: {
                                    renameDialog.row = index
                                    renameDialog.oldPath = relativePath
                                    renameDialog.oldTitle = title
                                    renameDialog.folder = isFolder
                                    renameName.text = title
                                    renameDialog.open()
                                }
                            }
                            AppButton {
                                text: "×"
                                danger: true
                                implicitWidth: 38
                                onClicked: {
                                    removeDialog.row = index
                                    removeDialog.path = relativePath
                                    removeDialog.open()
                                }
                            }
                        }

                        HoverHandler { id: hover }
                        TapHandler {
                            onTapped: {
                                if (isFolder)
                                    page.selectedFolder = relativePath
                                else
                                    page.openNote(relativePath, title)
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
                            elide: Text.ElideRight
                        }
                        Label {
                            text: saveTimer.running ? "Изменения…" : ""
                            color: Theme.textMuted
                        }
                        AppButton {
                            text: page.previewMode ? "Редактор" : "Превью"
                            enabled: page.currentPath.length > 0
                            primary: page.previewMode
                            onClicked: page.previewMode = !page.previewMode
                        }
                    }

                    TextArea {
                        id: editor
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: !page.previewMode
                        enabled: false
                        color: Theme.text
                        placeholderText: "Markdown и [[wiki-ссылки]]"
                        placeholderTextColor: Theme.textMuted
                        wrapMode: TextArea.Wrap
                        selectByMouse: true
                        onTextChanged: if (enabled) saveTimer.restart()
                        background: Rectangle {
                            color: Theme.surfaceRaised
                            radius: Theme.radiusSmall
                            border.color: Theme.border
                            border.width: 1
                        }
                    }

                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: page.previewMode
                        clip: true

                        Text {
                            width: parent.width
                            padding: 12
                            text: page.markdownWithWikiLinks(editor.text)
                            textFormat: Text.MarkdownText
                            wrapMode: Text.WordWrap
                            color: Theme.text
                            linkColor: Theme.accentStrong
                            onLinkActivated: link => page.openWikiLink(link)
                        }
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
        onAccepted: {
            Notes.createNote(page.selectedFolder, noteName.text)
            noteName.clear()
        }
    }

    Dialog {
        id: folderDialog
        title: "Новая папка"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok | Dialog.Cancel
        contentItem: TextField { id: folderName; placeholderText: "Имя папки" }
        onAccepted: {
            Notes.createFolder(page.selectedFolder, folderName.text)
            folderName.clear()
        }
    }

    Dialog {
        id: renameDialog
        property int row: -1
        property string oldPath: ""
        property string oldTitle: ""
        property bool folder: false
        title: "Переименовать"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok | Dialog.Cancel
        contentItem: TextField { id: renameName; placeholderText: "Новое имя" }
        onAccepted: {
            if (Notes.renameAt(row, renameName.text)) {
                if (!folder && page.currentPath === oldPath) {
                    const newPath = Notes.pathByTitle(renameName.text)
                    if (newPath.length) {
                        page.currentPath = newPath
                        page.currentTitle = renameName.text
                    }
                }
                if (folder && page.selectedFolder === oldPath)
                    page.selectedFolder = ""
            }
        }
    }

    Dialog {
        id: removeDialog
        property int row: -1
        property string path: ""
        title: "Удалить?"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Yes | Dialog.Cancel
        onAccepted: {
            const wasCurrent = page.currentPath === path
            Notes.removeAt(row)
            if (wasCurrent) {
                page.currentPath = ""
                page.currentTitle = ""
                editor.text = ""
                editor.enabled = false
                page.previewMode = false
            }
        }
    }

    Component.onDestruction: if (page.currentPath.length) Notes.saveText(page.currentPath, editor.text)
}
