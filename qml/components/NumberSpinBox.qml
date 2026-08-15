import QtQuick
import QtQuick.Controls
import DndTracker

SpinBox {
    id: control
    editable: true
    implicitHeight: Theme.touchTarget

    contentItem: TextInput {
        z: 2
        text: control.textFromValue(control.value, control.locale)
        color: Theme.text
        selectionColor: Theme.accent
        selectedTextColor: Theme.onAccent
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        inputMethodHints: Qt.ImhFormattedNumbersOnly
        selectByMouse: true
        validator: control.validator

        onEditingFinished: {
            const parsed = control.valueFromText(text, control.locale)
            control.value = Math.max(control.from, Math.min(control.to, parsed))
            text = control.textFromValue(control.value, control.locale)
        }
    }

    background: Rectangle {
        color: Theme.surfaceRaised
        radius: Theme.radiusSmall
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
    }
}
