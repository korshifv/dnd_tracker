import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DndTracker

Item {
    id: page
    signal openCharacter(string filePath)

    property var avatarPalette: [
        Theme.red, Theme.peach, Theme.yellow, Theme.green,
        Theme.teal, Theme.blue, Theme.mauve, Theme.pink
    ]

    function nextAvatarColor(current) {
        const normalized = String(current || "").toLowerCase()
        let currentIndex = -1
        for (let i = 0; i < avatarPalette.length; ++i) {
            if (String(avatarPalette[i]).toLowerCase() === normalized) {
                currentIndex = i
                break
            }
        }
        return avatarPalette[(currentIndex + 1) % avatarPalette.length]
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 14

        Surface {
            Layout.fillWidth: true
            implicitHeight: combatHeader.implicitHeight + 28

            ColumnLayout {
                id: combatHeader
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Label {
                            Layout.fillWidth: true
                            text: Initiative.currentTurnName.length
                                  ? "Сейчас ходит: " + Initiative.currentTurnName
                                  : "Бой не начат"
                            color: Theme.text
                            font.pixelSize: 18
                            font.weight: Font.Bold
                            elide: Text.ElideRight
                        }
                        Label { text: "Раунд " + Initiative.round; color: Theme.textMuted }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    AppButton {
                        Layout.fillWidth: true
                        text: "Следующий"
                        primary: true
                        enabled: Initiative.hasCombatants
                        onClicked: Initiative.nextTurn()
                    }
                    AppButton {
                        Layout.fillWidth: true
                        text: "Сброс"
                        onClicked: Initiative.resetCombat()
                    }
                }
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8
            AppButton { text: "+ Боец"; primary: true; onClicked: Initiative.addBlank("") }
            AppButton { text: "Сортировать"; onClicked: Initiative.sortByInitiative() }
            AppButton { text: "Очистить"; danger: true; enabled: Initiative.hasCombatants; onClicked: confirmClear.open() }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            clip: true
            model: Initiative
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            delegate: Surface {
                id: card
                required property int index
                required property string name
                required property int hp
                required property int armorClass
                required property int initiative
                required property string status
                required property string filePath
                required property string groupName
                required property string avatarColor
                required property bool activeTurn
                property bool compactCard: width < 820

                width: ListView.view.width
                implicitHeight: compactCard ? compactLayout.implicitHeight + 24 : wideLayout.implicitHeight + 24
                border.width: activeTurn ? 2 : 1
                border.color: activeTurn ? Theme.accent : Theme.border

                ColumnLayout {
                    id: compactLayout
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 9
                    visible: card.compactCard

                    RowLayout {
                        Layout.fillWidth: true
                        Rectangle {
                            width: 38
                            height: 38
                            radius: 19
                            color: avatarColor.length ? avatarColor : Theme.accent
                            border.width: 1
                            border.color: Theme.border
                            ToolTip.visible: avatarMouseCompact.containsMouse
                            ToolTip.text: "Нажми, чтобы сменить цвет"
                            MouseArea {
                                id: avatarMouseCompact
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: Initiative.setAvatarColor(index, page.nextAvatarColor(avatarColor))
                            }
                        }
                        TextField {
                            Layout.fillWidth: true
                            text: name
                            color: Theme.text
                            font.weight: Font.DemiBold
                            selectByMouse: true
                            onEditingFinished: Initiative.setName(index, text)
                            background: Rectangle {
                                color: Theme.surfaceRaised
                                radius: Theme.radiusSmall
                                border.color: activeFocus ? Theme.accent : Theme.border
                                border.width: activeFocus ? 2 : 1
                            }
                        }
                        AppButton { text: "×"; danger: true; implicitWidth: 44; onClicked: Initiative.removeAt(index) }
                    }

                    TextField {
                        Layout.fillWidth: true
                        text: groupName || "Основная группа"
                        placeholderText: "Группа"
                        color: Theme.text
                        selectByMouse: true
                        onEditingFinished: Initiative.setGroup(index, text)
                        background: Rectangle {
                            color: Theme.surfaceRaised
                            radius: Theme.radiusSmall
                            border.color: activeFocus ? Theme.accent : Theme.border
                            border.width: activeFocus ? 2 : 1
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: 10
                        rowSpacing: 8

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3
                            Label { text: "HP"; color: Theme.textMuted; font.pixelSize: 11 }
                            NumberSpinBox {
                                Layout.fillWidth: true
                                from: -999
                                to: 9999
                                value: hp
                                onValueChanged: if (value !== hp) Initiative.setHp(index, value)
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3
                            Label { text: "КД"; color: Theme.textMuted; font.pixelSize: 11 }
                            NumberSpinBox {
                                Layout.fillWidth: true
                                from: 0
                                to: 99
                                value: armorClass
                                onValueChanged: if (value !== armorClass) Initiative.setArmorClass(index, value)
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3
                            Label { text: "Инициатива"; color: Theme.textMuted; font.pixelSize: 11 }
                            NumberSpinBox {
                                Layout.fillWidth: true
                                from: -20
                                to: 99
                                value: initiative
                                onValueChanged: if (value !== initiative) Initiative.setInitiative(index, value)
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3
                            Label { text: "Урон / лечение"; color: Theme.textMuted; font.pixelSize: 11 }
                            NumberSpinBox {
                                id: compactEffectAmount
                                Layout.fillWidth: true
                                from: 1
                                to: 999
                                value: 1
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        AppButton {
                            Layout.fillWidth: true
                            text: "− Урон"
                            danger: true
                            onClicked: Initiative.applyDamage(index, compactEffectAmount.value)
                        }
                        AppButton {
                            Layout.fillWidth: true
                            text: "+ Лечение"
                            onClicked: Initiative.applyHeal(index, compactEffectAmount.value)
                        }
                    }

                    TextField {
                        Layout.fillWidth: true
                        text: status
                        placeholderText: "Статусы…"
                        color: Theme.text
                        selectByMouse: true
                        onEditingFinished: Initiative.setStatus(index, text)
                        background: Rectangle {
                            color: Theme.surfaceRaised
                            radius: Theme.radiusSmall
                            border.color: activeFocus ? Theme.accent : Theme.border
                            border.width: activeFocus ? 2 : 1
                        }
                    }

                    AppButton {
                        Layout.fillWidth: true
                        visible: filePath.length > 0
                        text: "Открыть чарник"
                        onClicked: page.openCharacter(filePath)
                    }
                }

                ColumnLayout {
                    id: wideLayout
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 9
                    visible: !card.compactCard

                    RowLayout {
                        Layout.fillWidth: true
                        Rectangle {
                            width: 38
                            height: 38
                            radius: 19
                            color: avatarColor.length ? avatarColor : Theme.accent
                            border.width: 1
                            border.color: Theme.border
                            ToolTip.visible: avatarMouseWide.containsMouse
                            ToolTip.text: "Нажми, чтобы сменить цвет"
                            MouseArea {
                                id: avatarMouseWide
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: Initiative.setAvatarColor(index, page.nextAvatarColor(avatarColor))
                            }
                        }
                        TextField {
                            Layout.fillWidth: true
                            text: name
                            color: Theme.text
                            font.weight: Font.DemiBold
                            onEditingFinished: Initiative.setName(index, text)
                            background: Rectangle { color: Theme.surfaceRaised; radius: Theme.radiusSmall; border.color: activeFocus ? Theme.accent : Theme.border; border.width: activeFocus ? 2 : 1 }
                        }
                        TextField {
                            Layout.preferredWidth: 180
                            text: groupName || "Основная группа"
                            placeholderText: "Группа"
                            color: Theme.text
                            onEditingFinished: Initiative.setGroup(index, text)
                            background: Rectangle { color: Theme.surfaceRaised; radius: Theme.radiusSmall; border.color: activeFocus ? Theme.accent : Theme.border; border.width: activeFocus ? 2 : 1 }
                        }
                        AppButton { text: "×"; danger: true; implicitWidth: 44; onClicked: Initiative.removeAt(index) }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        ColumnLayout {
                            Layout.preferredWidth: 100
                            spacing: 1
                            Label { text: "HP"; color: Theme.textMuted; font.pixelSize: 10 }
                            NumberSpinBox {
                                Layout.fillWidth: true
                                from: -999
                                to: 9999
                                value: hp
                                onValueChanged: if (value !== hp) Initiative.setHp(index, value)
                            }
                        }
                        ColumnLayout {
                            Layout.preferredWidth: 90
                            spacing: 1
                            Label { text: "КД"; color: Theme.textMuted; font.pixelSize: 10 }
                            NumberSpinBox {
                                Layout.fillWidth: true
                                from: 0
                                to: 99
                                value: armorClass
                                onValueChanged: if (value !== armorClass) Initiative.setArmorClass(index, value)
                            }
                        }
                        ColumnLayout {
                            Layout.preferredWidth: 110
                            spacing: 1
                            Label { text: "Инициатива"; color: Theme.textMuted; font.pixelSize: 10 }
                            NumberSpinBox {
                                Layout.fillWidth: true
                                from: -20
                                to: 99
                                value: initiative
                                onValueChanged: if (value !== initiative) Initiative.setInitiative(index, value)
                            }
                        }
                        ColumnLayout {
                            Layout.preferredWidth: 100
                            spacing: 1
                            Label { text: "Эффект"; color: Theme.textMuted; font.pixelSize: 10 }
                            NumberSpinBox { id: wideEffectAmount; Layout.fillWidth: true; from: 1; to: 999; value: 1 }
                        }
                        AppButton { text: "−"; danger: true; implicitWidth: 44; onClicked: Initiative.applyDamage(index, wideEffectAmount.value) }
                        AppButton { text: "+"; implicitWidth: 44; onClicked: Initiative.applyHeal(index, wideEffectAmount.value) }
                        TextField {
                            Layout.fillWidth: true
                            text: status
                            placeholderText: "Статусы…"
                            color: Theme.text
                            onEditingFinished: Initiative.setStatus(index, text)
                            background: Rectangle { color: Theme.surfaceRaised; radius: Theme.radiusSmall; border.color: activeFocus ? Theme.accent : Theme.border; border.width: activeFocus ? 2 : 1 }
                        }
                        AppButton {
                            visible: filePath.length > 0
                            text: "Чарник"
                            onClicked: page.openCharacter(filePath)
                        }
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: list.count === 0
                text: "Инициатива пустая\nДобавь бойца или персонажа"
                color: Theme.textMuted
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    Dialog {
        id: confirmClear
        title: "Очистить инициативу?"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Yes | Dialog.Cancel
        onAccepted: Initiative.clearAll()
    }
}
