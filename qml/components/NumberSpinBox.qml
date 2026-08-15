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
        font: control.font
        color: Theme.text
        selectionColor: Theme.accent
        selectedTextColor: Theme.onAccent
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        leftPadding: 36
        rightPadding: 36
        inputMethodHints: Qt.ImhFormattedNumbersOnly
        selectByMouse: true
        readOnly: !control.editable
        validator: control.validator

        function commitValue() {
            const parsed = control.valueFromText(text, control.locale)
            control.value = Math.max(control.from, Math.min(control.to, parsed))
            text = control.textFromValue(control.value, control.locale)
        }

        onAccepted: commitValue()
        onEditingFinished: commitValue()
    }

    background: Rectangle {
        color: Theme.surfaceRaised
        radius: Theme.radiusSmall
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
    }
}
