import QtQuick
import QtQuick.Controls

Rectangle {
    id: sheet
    required property var book
    width: book.sheetWidth
    height: book.sheetHeight
    color: book.paperColor
    border.width: 1
    border.color: "#c7bfb2"

    component PaperField: Item {
        id: field
        property string label: ""
        property string value: ""
        property int textSize: 14
        signal edited(string value)

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            text: field.label
            color: book.faintInk
            font.pixelSize: 8
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }
        TextField {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.height - 12
            text: field.value
            color: book.inkColor
            font.pixelSize: field.textSize
            verticalAlignment: Text.AlignVCenter
            leftPadding: 3; rightPadding: 3; topPadding: 0; bottomPadding: 0
            selectByMouse: true
            background: Rectangle {
                color: "transparent"
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: "#8f887d"
                }
            }
            onTextEdited: field.edited(text)
        }
    }

    component ValueBox: Rectangle {
        id: box
        property string label: ""
        property string value: ""
        color: "#fbf8f0"
        border.width: 1.5
        border.color: book.inkColor
        radius: 7
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 7
            text: box.label
            color: book.faintInk
            font.pixelSize: 8
            font.weight: Font.DemiBold
        }
        Text {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: 8
            text: box.value
            color: book.inkColor
            font.pixelSize: 22
            font.weight: Font.Bold
        }
    }

    component SpellBlock: Rectangle {
        id: block
        required property int level
        required property int rows
        readonly property real headerHeight: level === 0 ? 52 : 62
        readonly property real rowHeight: Math.max(17, (height - headerHeight - 8) / rows)

        color: "#fbf8f0"
        border.width: 1.4
        border.color: book.inkColor
        radius: 6

        Rectangle {
            x: 7; y: 7; width: 34; height: 34
            color: book.paperColor
            border.width: 1.5
            border.color: book.inkColor
            radius: 6
            Text {
                anchors.centerIn: parent
                text: String(block.level)
                color: book.inkColor
                font.pixelSize: 17
                font.weight: Font.Bold
            }
        }

        Text {
            x: 50; y: 12
            text: block.level === 0 ? "ЗАГОВОРЫ" : "ЗАКЛИНАНИЯ " + block.level + " УРОВНЯ"
            color: book.inkColor
            font.pixelSize: 10
            font.weight: Font.DemiBold
        }

        Item {
            visible: block.level > 0
            x: 50; y: 28; width: block.width - 58; height: 28
            Text { x: 0; y: 2; text: "ЯЧЕЕК"; color: book.faintInk; font.pixelSize: 7 }
            TextField {
                x: 38; y: 0; width: 42; height: 24
                text: String(book.spellSlotCount(block.level))
                color: book.inkColor; font.pixelSize: 12; font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                topPadding: 0; bottomPadding: 0; leftPadding: 0; rightPadding: 0
                validator: IntValidator { bottom: 0; top: 99 }
                background: Rectangle { color: book.panelColor; radius: 3 }
                onEditingFinished: {
                    const n = Number(text)
                    if (!Number.isNaN(n)) book.setSpellSlotCount(block.level, n)
                }
            }
            Text { x: 90; y: 2; text: "ПОТРАЧЕНО"; color: book.faintInk; font.pixelSize: 7 }
            TextField {
                x: 158; y: 0; width: 42; height: 24
                text: String(book.spellExpendedCount(block.level))
                color: book.inkColor; font.pixelSize: 12; font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                topPadding: 0; bottomPadding: 0; leftPadding: 0; rightPadding: 0
                validator: IntValidator { bottom: 0; top: 99 }
                background: Rectangle { color: book.panelColor; radius: 3 }
                onEditingFinished: {
                    const n = Number(text)
                    if (!Number.isNaN(n)) book.setSpellExpendedCount(block.level, Math.max(0, n))
                }
            }
        }

        Repeater {
            model: block.rows
            delegate: Item {
                required property int index
                x: 8
                y: block.headerHeight + index * block.rowHeight
                width: block.width - 16
                height: block.rowHeight

                Rectangle {
                    visible: block.level > 0
                    x: 0
                    anchors.verticalCenter: parent.verticalCenter
                    width: 11; height: 11; radius: 6
                    color: book.spellPrepared(block.level, index) ? book.inkColor : "transparent"
                    border.width: 1
                    border.color: book.inkColor
                    MouseArea { anchors.fill: parent; onClicked: book.toggleSpellPrepared(block.level, index) }
                }

                TextField {
                    x: block.level > 0 ? 18 : 0
                    width: parent.width - (block.level > 0 ? 18 : 0)
                    height: parent.height
                    text: book.spellLine(block.level, index)
                    color: book.inkColor
                    font.pixelSize: 10
                    verticalAlignment: Text.AlignVCenter
                    topPadding: 0; bottomPadding: 0; leftPadding: 2; rightPadding: 2
                    selectByMouse: true
                    background: Rectangle {
                        color: "transparent"
                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 1
                            color: "#91897e"
                        }
                    }
                    onTextEdited: book.setSpellLine(block.level, index, text)
                }
            }
        }
    }

    Rectangle {
        x: 28; y: 24; width: 330; height: 92
        color: "#fbf8f0"; border.width: 1.8; border.color: book.inkColor; radius: 5
        Text {
            x: 14; y: 9
            text: "D&D 5e"
            color: book.inkColor
            font.pixelSize: 18
            font.weight: Font.Black
        }
        PaperField {
            x: 14; y: 39; width: 302; height: 43
            label: "КЛАСС ЗАКЛИНАТЕЛЯ"
            value: book.casterClassValue
            textSize: 18
            onEdited: value => book.casterClassValue = value
        }
    }

    Rectangle {
        x: 374; y: 24; width: 558; height: 92
        color: book.panelColor; border.width: 1.8; border.color: book.inkColor; radius: 5

        Rectangle {
            x: 14; y: 11; width: 174; height: 70
            color: "#fbf8f0"; border.width: 1.5; border.color: book.inkColor; radius: 7
            PaperField {
                anchors.fill: parent
                anchors.margins: 8
                label: "БАЗОВАЯ ХАРАКТЕРИСТИКА"
                value: book.spellAbilityName()
                textSize: 13
                onEdited: value => book.setSpellAbilityFromText(value)
            }
        }
        ValueBox { x: 202; y: 11; width: 160; height: 70; label: "СЛ ЗАКЛИНАНИЙ"; value: String(book.spellSaveDc()) }
        ValueBox { x: 376; y: 11; width: 168; height: 70; label: "БОНУС АТАКИ"; value: book.signed(book.spellAttackBonus()) }
    }

    SpellBlock { x: 28; y: 140; width: 286; height: 230; level: 0; rows: 8 }
    SpellBlock { x: 28; y: 382; width: 286; height: 410; level: 1; rows: 12 }
    SpellBlock { x: 28; y: 804; width: 286; height: 444; level: 2; rows: 12 }

    SpellBlock { x: 337; y: 140; width: 286; height: 390; level: 3; rows: 12 }
    SpellBlock { x: 337; y: 542; width: 286; height: 340; level: 4; rows: 10 }
    SpellBlock { x: 337; y: 894; width: 286; height: 354; level: 5; rows: 10 }

    SpellBlock { x: 646; y: 140; width: 286; height: 330; level: 6; rows: 9 }
    SpellBlock { x: 646; y: 482; width: 286; height: 290; level: 7; rows: 8 }
    SpellBlock { x: 646; y: 784; width: 286; height: 220; level: 8; rows: 6 }
    SpellBlock { x: 646; y: 1016; width: 286; height: 232; level: 9; rows: 6 }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 7
        text: "DnD Tracker · классический вид · лист 3/3"
        color: "#aaa195"
        font.pixelSize: 8
    }
}
