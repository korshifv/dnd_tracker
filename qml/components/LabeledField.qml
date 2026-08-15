import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DndTracker

ColumnLayout {
    id: root
    property alias label: labelItem.text
    property alias text: field.text
    property alias placeholderText: field.placeholderText
    property alias inputMethodHints: field.inputMethodHints
    property alias readOnly: field.readOnly
    signal editingFinished()
    spacing: 5

    Label {
        id: labelItem
        color: Theme.textMuted
        font.pixelSize: 11
        font.weight: Font.DemiBold
    }

    TextField {
        id: field
        Layout.fillWidth: true
        implicitHeight: Theme.touchTarget
        color: Theme.text
        placeholderTextColor: Theme.textMuted
        selectionColor: Theme.accent
        selectedTextColor: "#0D0F14"
        onEditingFinished: root.editingFinished()
        background: Rectangle {
            color: Theme.surfaceRaised
            radius: Theme.radiusSmall
            border.width: field.activeFocus ? 2 : 1
            border.color: field.activeFocus ? Theme.accent : Theme.border
        }
    }
}
