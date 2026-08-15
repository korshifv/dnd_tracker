import QtQuick
import QtQuick.Controls
import DndTracker

TextArea {
    id: control
    activeFocusOnPress: false
    color: Theme.text
    placeholderTextColor: Theme.textMuted
    selectionColor: Theme.accent
    selectedTextColor: Theme.onAccent
    wrapMode: TextArea.Wrap
    selectByMouse: true
    padding: 10

    // A Flickable may steal a drag without the editor taking focus. A real tap
    // still focuses the field, so scrolling no longer summons the keyboard.
    onReleased: function(event) {
        if (!activeFocus)
            forceActiveFocus()
    }

    background: Rectangle {
        color: Theme.surfaceRaised
        radius: Theme.radiusSmall
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
    }
}
