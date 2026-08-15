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
        structureDrawer.close()
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

    function moveEntry(sourceIndex, sourcePath, sourceTitle, sourceIsFolder, targetFolder) {
        if (page.currentPath.length)
            Notes.saveText(page.currentPath, editor.text)
        const containsCurrent = page.currentPath === sourcePath
                || (sourceIsFolder && page.currentPath.startsWith(sourcePath + "/"))
        if (!Notes.moveAt(sourceIndex, targetFolder))
            return

        if (containsCurrent && page.currentTitle.length) {
            const movedPath = Notes.pathByTitle(page.currentTitle)
            if (movedPath.length)
                page.currentPath = movedPath
        }
        if (page.selectedFolder === sourcePath || page.selectedFolder.startsWith(sourcePath + "/"))
            page.selectedFolder = ""
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            AppButton {
                text: "☰"
                implicitWidth: 46
                ToolTip.visible: hovered
                ToolTip.text: "Структура заметок"
                onClicked: structureDrawer.open()
            }

            Label {
                text: "Заметки"
                color: Theme.text
                font.pixelSize: 24
                font.weight: Font.Bold
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            AppButton {
                text: "+ Папка"
                onClicked: folderDialog.open()
            }
            AppButton {
                text: "+ Заметка"
                primary: true
                onClicked: noteDialog.open()
            }
        }

        Surface {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Label {
                            Layout.fillWidth: true
                            text: page.currentTitle.length ? page.currentTitle : "Выберите заметку"
                            color: Theme.text
                            font.pixelSize: 19
                            font.weight: Font.Bold
                            elide: Text.ElideRight
                        }
                        Label {
                            Layout.fillWidth: true
                            visible: page.currentPath.length > 0
                            text: page.currentPath
                            color: Theme.textMuted
                            font.pixelSize: 11
                            elide: Text.ElideMiddle
                        }
                    }

                    Label {
                        text: saveTimer.running ? "Изменения…" : ""
                        color: Theme.textMuted
                        visible: width > 0
                    }
                    AppButton {
                        text: page.previewMode ? "Редактор" : "Превью"
                        enabled: page.currentPath.length > 0
                        primary: page.previewMode
                        onClicked: {
                            page.previewMode = !page.previewMode
                            if (page.previewMode)
                                editor.focus = false
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 1
                    color: Theme.border
                }

                AppTextArea {
                    id: editor
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: !page.previewMode && page.currentPath.length > 0
                    enabled: page.currentPath.length > 0
                    placeholderText: "Markdown и [[wiki-ссылки]]"
                    onTextChanged: if (enabled) saveTimer.restart()
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: page.previewMode && page.currentPath.length > 0
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

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: page.currentPath.length === 0

                    Column {
                        anchors.centerIn: parent
                        spacing: 8
                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "☰"
                            color: Theme.accent
                            font.pixelSize: 34
                        }
                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Открой структуру и выбери заметку"
                            color: Theme.textMuted
                        }
                    }
                }
            }
        }
    }

    Drawer {
        id: structureDrawer
        edge: Qt.LeftEdge
        width: Math.min(380, Math.max(280, page.width * 0.86))
        height: page.height
        modal: true
        dim: true
        interactive: true
        padding: 0

        background: Rectangle {
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
        }

        contentItem: ColumnLayout {
            spacing: 6

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 14
                Layout.rightMargin: 10
                Layout.topMargin: 12
                Layout.bottomMargin: 6

                Label {
                    text: "Структура"
                    color: Theme.text
                    font.pixelSize: 20
                    font.weight: Font.Bold
                    Layout.fillWidth: true
                }
                AppButton {
                    text: "×"
                    implicitWidth: 42
                    onClicked: structureDrawer.close()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 1
                color: Theme.border
            }

            Button {
                id: rootButton
                Layout.fillWidth: true
                Layout.leftMargin: 8
                Layout.rightMargin: 8
                implicitHeight: 44
                leftPadding: 12
                rightPadding: 12
                onClicked: page.selectedFolder = ""

                contentItem: Label {
                    text: "Корень заметок"
                    color: page.selectedFolder.length === 0 ? Theme.accentStrong : Theme.text
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                background: Rectangle {
                    radius: Theme.radiusSmall
                    color: rootButton.down || rootButton.hovered || page.selectedFolder.length === 0
                           ? Theme.surfaceHover : "transparent"
                }

                DropArea {
                    anchors.fill: parent
                    onDropped: function(drop) {
                        const source = drop.source
                        if (!source) return
                        page.moveEntry(source.sourceIndex, source.sourcePath,
                                       source.sourceTitle, source.sourceIsFolder, "")
                        drop.acceptProposedAction()
                    }
                }
            }

            ListView {
                id: tree
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: 8
                Layout.rightMargin: 8
                Layout.bottomMargin: 10
                clip: true
                model: Notes
                spacing: 3
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {}

                delegate: Rectangle {
                    id: row
                    required property int index
                    required property string title
                    required property string relativePath
                    required property bool isFolder
                    required property int depth
                    property int sourceIndex: index
                    property string sourcePath: relativePath
                    property string sourceTitle: title
                    property bool sourceIsFolder: isFolder

                    width: ListView.view.width
                    height: 46
                    radius: Theme.radiusSmall
                    color: folderDrop.containsDrag ? Theme.surfaceHover : "transparent"
                    border.width: folderDrop.containsDrag ? 1 : 0
                    border.color: Theme.accent

                    Drag.active: dragHandler.active
                    Drag.source: row
                    Drag.supportedActions: Qt.MoveAction

                    RowLayout {
                        anchors.fill: parent
                        spacing: 4

                        Button {
                            id: entryButton
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            leftPadding: 10 + row.depth * 14
                            rightPadding: 8
                            onClicked: {
                                if (row.isFolder) {
                                    page.selectedFolder = row.relativePath
                                } else {
                                    page.openNote(row.relativePath, row.title)
                                }
                            }

                            contentItem: RowLayout {
                                spacing: 7
                                Label {
                                    text: row.isFolder ? ">" : "•"
                                    color: row.isFolder ? Theme.accent : Theme.textMuted
                                    font.weight: row.isFolder ? Font.Bold : Font.Normal
                                }
                                Label {
                                    Layout.fillWidth: true
                                    text: row.title
                                    color: Theme.text
                                    elide: Text.ElideRight
                                }
                            }
                            background: Rectangle {
                                radius: Theme.radiusSmall
                                color: entryButton.down || entryButton.hovered
                                       || page.currentPath === row.relativePath
                                       || page.selectedFolder === row.relativePath
                                       ? Theme.surfaceHover : "transparent"
                            }
                        }

                        AppButton {
                            text: "..."
                            implicitWidth: 42
                            ToolTip.visible: hovered
                            ToolTip.text: "Переименовать"
                            onClicked: {
                                renameDialog.row = row.index
                                renameDialog.oldPath = row.relativePath
                                renameDialog.oldTitle = row.title
                                renameDialog.folder = row.isFolder
                                renameName.text = row.title
                                renameDialog.open()
                            }
                        }
                        AppButton {
                            text: "×"
                            danger: true
                            implicitWidth: 38
                            onClicked: {
                                removeDialog.row = row.index
                                removeDialog.path = row.relativePath
                                removeDialog.open()
                            }
                        }
                    }

                    DragHandler { id: dragHandler; target: null }
                    DropArea {
                        id: folderDrop
                        anchors.fill: parent
                        enabled: row.isFolder && !dragHandler.active
                        onDropped: function(drop) {
                            const source = drop.source
                            if (!source || source === row) return
                            page.moveEntry(source.sourceIndex, source.sourcePath,
                                           source.sourceTitle, source.sourceIsFolder,
                                           row.relativePath)
                            drop.acceptProposedAction()
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
                page.previewMode = false
            }
        }
    }

    Component.onDestruction: if (page.currentPath.length) Notes.saveText(page.currentPath, editor.text)
}
