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
            selectByMouse: true
            leftPadding: 3
            rightPadding: 3
            topPadding: 0
            bottomPadding: 0
            background: Rectangle {
                color: "transparent"
                border.width: 0
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

    component PaperNumber: Item {
        id: numberField
        property string label: ""
        property int value: 0
        property int minimum: -999
        property int maximum: 9999
        property int textSize: 22
        signal edited(int value)

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            text: numberField.label
            color: book.faintInk
            font.pixelSize: 8
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
        TextField {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.height - 12
            text: String(numberField.value)
            color: book.inkColor
            font.pixelSize: numberField.textSize
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            selectByMouse: true
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0
            validator: IntValidator { bottom: numberField.minimum; top: numberField.maximum }
            background: Item {}
            onEditingFinished: {
                const parsed = Number(text)
                if (!Number.isNaN(parsed))
                    numberField.edited(Math.max(numberField.minimum, Math.min(numberField.maximum, parsed)))
            }
        }
    }

    component PaperArea: Rectangle {
        id: area
        property string label: ""
        property string value: ""
        property int textSize: 12
        signal edited(string value)

        color: "#fbf8f0"
        border.width: 1.4
        border.color: book.inkColor
        radius: 5

        Text {
            id: areaLabel
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 5
            text: area.label
            color: book.inkColor
            font.pixelSize: 9
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
        }
        TextArea {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: areaLabel.top
            anchors.margins: 6
            text: area.value
            color: book.inkColor
            font.pixelSize: area.textSize
            wrapMode: TextEdit.Wrap
            selectByMouse: true
            background: Item {}
            padding: 2
            onTextChanged: if (activeFocus) area.edited(text)
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
            label: "ИМЯ ПЕРСОНАЖА"
            value: book.nameValue
            textSize: 19
            onEdited: value => book.nameValue = value
        }
    }

    Rectangle {
        x: 374; y: 24; width: 558; height: 92
        color: "#fbf8f0"; border.width: 1.8; border.color: book.inkColor; radius: 5

        PaperField { x: 12; y: 7; width: 120; height: 37; label: "КЛАСС"; value: book.classValue; onEdited: value => book.classValue = value }
        PaperField { x: 138; y: 7; width: 120; height: 37; label: "ПОДКЛАСС"; value: book.subclassValue; onEdited: value => book.subclassValue = value }
        PaperNumber {
            x: 264; y: 7; width: 58; height: 37
            label: "УРОВЕНЬ"; value: book.levelValue; minimum: 1; maximum: 40; textSize: 16
            onEdited: value => book.levelValue = value
        }
        PaperField { x: 328; y: 7; width: 100; height: 37; label: "ПРЕДЫСТОРИЯ"; value: book.backgroundValue; textSize: 12; onEdited: value => book.backgroundValue = value }
        PaperField { x: 434; y: 7; width: 112; height: 37; label: "ИГРОК"; value: book.playerValue; textSize: 12; onEdited: value => book.playerValue = value }
        PaperField { x: 12; y: 49; width: 160; height: 35; label: "РАСА"; value: book.raceValue; onEdited: value => book.raceValue = value }
        PaperField { x: 178; y: 49; width: 242; height: 35; label: "МИРОВОЗЗРЕНИЕ"; value: book.alignmentValue; onEdited: value => book.alignmentValue = value }
        PaperField { x: 426; y: 49; width: 120; height: 35; label: "ОПЫТ"; value: book.experienceValue; onEdited: value => book.experienceValue = value }
    }

    Rectangle {
        x: 28; y: 140; width: 96; height: 664
        color: book.panelColor
        radius: 8
        border.width: 1
        border.color: "#d1c8bb"

        Repeater {
            model: 6
            delegate: Rectangle {
                required property int index
                x: 8
                y: 8 + index * 108
                width: 80
                height: 94
                color: "#fbf8f0"
                border.width: 1.6
                border.color: book.inkColor
                radius: 6

                Text {
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.topMargin: 7
                    width: parent.width - 8
                    text: book.statNames[index]
                    color: book.inkColor
                    font.pixelSize: index === 2 || index === 3 ? 8 : 9
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                }
                TextField {
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: 26
                    width: 58
                    height: 38
                    text: String(book.statValue(book.statCodes[index]))
                    color: book.inkColor
                    font.pixelSize: 24
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    topPadding: 0; bottomPadding: 0
                    validator: IntValidator { bottom: 1; top: 30 }
                    background: Item {}
                    onEditingFinished: {
                        const n = Number(text)
                        if (!Number.isNaN(n)) book.setStatValue(book.statCodes[index], Math.max(1, Math.min(30, n)))
                    }
                }
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: -12
                    width: 44; height: 25; radius: 13
                    color: book.paperColor
                    border.width: 1.4; border.color: book.inkColor
                    Text {
                        anchors.centerIn: parent
                        text: book.signed(book.statModifier(book.statCodes[index]))
                        color: book.inkColor
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                    }
                }
            }
        }
    }

    Rectangle {
        x: 142; y: 140; width: 212; height: 72
        color: "#fbf8f0"; border.width: 1.5; border.color: book.inkColor; radius: 5
        Rectangle {
            x: 8; y: 10; width: 28; height: 28; radius: 4
            color: book.inspirationValue ? book.inkColor : "transparent"
            border.width: 1.5; border.color: book.inkColor
            MouseArea { anchors.fill: parent; onClicked: book.inspirationValue = !book.inspirationValue }
        }
        Text { x: 46; y: 15; text: "ВДОХНОВЕНИЕ"; color: book.inkColor; font.pixelSize: 10; font.weight: Font.DemiBold }
        PaperNumber {
            x: 7; y: 40; width: 50; height: 28
            label: ""; value: book.proficiencyValue; minimum: 0; maximum: 12; textSize: 17
            onEdited: value => book.proficiencyValue = value
        }
        Text { x: 64; y: 49; text: "БОНУС МАСТЕРСТВА"; color: book.inkColor; font.pixelSize: 9; font.weight: Font.DemiBold }
    }

    Rectangle {
        x: 142; y: 224; width: 212; height: 220
        color: "#fbf8f0"; border.width: 1.5; border.color: book.inkColor; radius: 5
        Text { anchors.horizontalCenter: parent.horizontalCenter; y: 193; text: "СПАСБРОСКИ"; color: book.inkColor; font.pixelSize: 10; font.weight: Font.DemiBold }
        Repeater {
            model: 6
            delegate: Item {
                required property int index
                x: 12; y: 12 + index * 29; width: 188; height: 26
                Rectangle {
                    x: 0; y: 7; width: 12; height: 12; radius: 6
                    color: book.saves[book.statCodes[index]] ? book.inkColor : "transparent"
                    border.width: 1.2; border.color: book.inkColor
                    MouseArea { anchors.fill: parent; onClicked: book.setSave(book.statCodes[index], !book.saves[book.statCodes[index]]) }
                }
                Text {
                    x: 22; y: 3; width: 38
                    text: book.signed(book.statModifier(book.statCodes[index]) + (book.saves[book.statCodes[index]] ? book.proficiencyValue : 0))
                    color: book.inkColor; font.pixelSize: 12; font.weight: Font.DemiBold
                }
                Text { x: 62; y: 4; text: book.statNames[index]; color: book.inkColor; font.pixelSize: 10 }
            }
        }
    }

    Rectangle {
        x: 142; y: 456; width: 212; height: 394
        color: "#fbf8f0"; border.width: 1.5; border.color: book.inkColor; radius: 5
        Text { anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 6; text: "НАВЫКИ"; color: book.inkColor; font.pixelSize: 10; font.weight: Font.DemiBold }
        Repeater {
            model: book.skills.length
            delegate: Item {
                required property int index
                property var skill: book.skills[index]
                x: 9; y: 7 + index * 20; width: 194; height: 19
                visible: y < 365
                Rectangle {
                    x: 0; y: 4; width: 10; height: 10; radius: 5
                    color: (skill.profLevel || 0) > 0 ? book.inkColor : "transparent"
                    border.width: 1; border.color: book.inkColor
                    Rectangle {
                        anchors.centerIn: parent
                        width: 4; height: 4; radius: 2
                        color: book.paperColor
                        visible: (skill.profLevel || 0) > 1
                    }
                    MouseArea { anchors.fill: parent; onClicked: book.setSkillLevel(index, ((skill.profLevel || 0) + 1) % 3) }
                }
                Text { x: 16; y: 1; width: 30; text: book.signed(book.skillBonus(skill)); color: book.inkColor; font.pixelSize: 10; font.weight: Font.DemiBold }
                Text { x: 48; y: 1; width: 119; text: book.skillName(skill); color: book.inkColor; font.pixelSize: 9; elide: Text.ElideRight }
                Text { x: 168; y: 1; width: 25; text: book.shortStat(skill.baseStat); color: book.faintInk; font.pixelSize: 8; horizontalAlignment: Text.AlignRight }
            }
        }
    }

    Rectangle {
        x: 28; y: 866; width: 326; height: 48
        color: "#fbf8f0"; border.width: 1.5; border.color: book.inkColor; radius: 5
        Text {
            x: 12; y: 10; width: 48
            text: String(book.passivePerception())
            color: book.inkColor; font.pixelSize: 20; font.weight: Font.Bold
            horizontalAlignment: Text.AlignHCenter
        }
        Text { x: 68; y: 15; text: "ПАССИВНАЯ МУДРОСТЬ (ВНИМАТЕЛЬНОСТЬ)"; color: book.inkColor; font.pixelSize: 9; font.weight: Font.DemiBold }
    }

    PaperArea {
        x: 28; y: 930; width: 326; height: 318
        label: "ДРУГИЕ ВЛАДЕНИЯ И ЯЗЫКИ"
        value: book.proficienciesLanguagesValue
        onEdited: value => book.proficienciesLanguagesValue = value
    }

    Rectangle {
        x: 374; y: 140; width: 310; height: 710
        color: book.panelColor; radius: 8
        border.width: 1; border.color: "#d1c8bb"

        PaperNumber { x: 12; y: 12; width: 88; height: 78; label: "КЛАСС ДОСПЕХА"; value: book.acValue; minimum: 0; maximum: 99; onEdited: value => book.acValue = value }
        Rectangle {
            x: 111; y: 12; width: 88; height: 78; color: "#fbf8f0"; border.width: 1.5; border.color: book.inkColor; radius: 8
            Text { anchors.horizontalCenter: parent.horizontalCenter; y: 7; text: "ИНИЦИАТИВА"; color: book.faintInk; font.pixelSize: 8; font.weight: Font.DemiBold }
            Text { anchors.centerIn: parent; anchors.verticalCenterOffset: 7; text: book.signed(book.statModifier("dex")); color: book.inkColor; font.pixelSize: 25; font.weight: Font.Bold }
        }
        PaperField { x: 210; y: 12; width: 88; height: 78; label: "СКОРОСТЬ"; value: book.speedValue; textSize: 19; onEdited: value => book.speedValue = value }

        Rectangle {
            x: 12; y: 104; width: 286; height: 104; color: "#fbf8f0"; border.width: 1.5; border.color: book.inkColor; radius: 5
            PaperNumber { x: 8; y: 7; width: 92; height: 85; label: "ТЕКУЩИЕ HP"; value: book.hpValue; minimum: -999; maximum: 9999; onEdited: value => book.hpValue = value }
            PaperNumber { x: 108; y: 7; width: 78; height: 85; label: "МАКС. HP"; value: book.hpMaxValue; minimum: 0; maximum: 9999; onEdited: value => book.hpMaxValue = value }
            PaperNumber { x: 194; y: 7; width: 82; height: 85; label: "ВРЕМ. HP"; value: book.hpTempValue; minimum: 0; maximum: 9999; onEdited: value => book.hpTempValue = value }
        }

        Rectangle {
            x: 12; y: 220; width: 135; height: 86; color: "#fbf8f0"; border.width: 1.5; border.color: book.inkColor; radius: 5
            PaperField { x: 8; y: 7; width: 72; height: 55; label: "КОСТЬ ХИТОВ"; value: book.hitDieValue; textSize: 17; onEdited: value => book.hitDieValue = value }
            PaperField { x: 84; y: 7; width: 43; height: 55; label: "ИТОГО"; value: book.hitDiceTotalValue; textSize: 15; onEdited: value => book.hitDiceTotalValue = value }
            Text { anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 5; text: "КОСТИ ХИТОВ"; color: book.inkColor; font.pixelSize: 9; font.weight: Font.DemiBold }
        }

        Rectangle {
            x: 159; y: 220; width: 139; height: 86; color: "#fbf8f0"; border.width: 1.5; border.color: book.inkColor; radius: 5
            Text { x: 8; y: 9; text: "УСПЕХИ"; color: book.inkColor; font.pixelSize: 9 }
            Repeater {
                model: 3
                delegate: Rectangle {
                    required property int index
                    x: 66 + index * 21; y: 7; width: 15; height: 15; radius: 8
                    color: book.deathSuccess[index] ? book.inkColor : "transparent"; border.width: 1.1; border.color: book.inkColor
                    MouseArea { anchors.fill: parent; onClicked: book.toggleDeath("success", index) }
                }
            }
            Text { x: 8; y: 35; text: "ПРОВАЛЫ"; color: book.inkColor; font.pixelSize: 9 }
            Repeater {
                model: 3
                delegate: Rectangle {
                    required property int index
                    x: 66 + index * 21; y: 33; width: 15; height: 15; radius: 8
                    color: book.deathFail[index] ? book.inkColor : "transparent"; border.width: 1.1; border.color: book.inkColor
                    MouseArea { anchors.fill: parent; onClicked: book.toggleDeath("fail", index) }
                }
            }
            Text { anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 5; text: "СПАСБРОСКИ ОТ СМЕРТИ"; color: book.inkColor; font.pixelSize: 8; font.weight: Font.DemiBold }
        }

        Rectangle {
            x: 12; y: 320; width: 286; height: 374; color: "#fbf8f0"; border.width: 1.5; border.color: book.inkColor; radius: 5
            Text { x: 10; y: 7; text: "НАЗВАНИЕ"; color: book.faintInk; font.pixelSize: 8 }
            Text { x: 151; y: 7; text: "БОНУС"; color: book.faintInk; font.pixelSize: 8 }
            Text { x: 204; y: 7; text: "УРОН / ТИП"; color: book.faintInk; font.pixelSize: 8 }

            Repeater {
                model: 3
                delegate: Item {
                    required property int index
                    property var weapon: book.weaponAt(index)
                    x: 8; y: 24 + index * 45; width: 270; height: 39
                    TextField {
                        x: 0; y: 0; width: 137; height: 35
                        text: weapon.name || ""
                        color: book.inkColor; font.pixelSize: 11; background: Rectangle { color: "#eee9df"; radius: 2 }
                        onTextEdited: book.setWeaponField(index, "name", text)
                    }
                    Text {
                        x: 143; y: 8; width: 48
                        text: book.signed(book.weaponHit(weapon))
                        color: book.inkColor; font.pixelSize: 12; font.weight: Font.DemiBold; horizontalAlignment: Text.AlignHCenter
                    }
                    TextField {
                        x: 197; y: 0; width: 73; height: 35
                        text: weapon.damage || ""
                        color: book.inkColor; font.pixelSize: 10; background: Rectangle { color: "#eee9df"; radius: 2 }
                        onTextEdited: book.setWeaponField(index, "damage", text)
                    }
                }
            }

            TextArea {
                x: 8; y: 162; width: 270; height: 178
                text: book.attacksNotesValue
                color: book.inkColor
                font.pixelSize: 10
                wrapMode: TextEdit.Wrap
                selectByMouse: true
                background: Item {}
                placeholderText: "Дополнительные атаки, боеприпасы, свойства оружия, заметки о заклинаниях…"
                onTextChanged: if (activeFocus) book.attacksNotesValue = text
            }
            Text { anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 6; text: "АТАКИ И ЗАКЛИНАНИЯ"; color: book.inkColor; font.pixelSize: 10; font.weight: Font.DemiBold }
        }
    }

    Rectangle {
        x: 374; y: 866; width: 310; height: 382
        color: "#fbf8f0"; border.width: 1.4; border.color: book.inkColor; radius: 5
        Text { anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 5; text: "СНАРЯЖЕНИЕ"; color: book.inkColor; font.pixelSize: 9; font.weight: Font.DemiBold }

        Column {
            x: 8; y: 14; width: 58; spacing: 5
            PaperNumber { width: 58; height: 48; label: "ММ"; value: book.cpValue; minimum: 0; maximum: 999999; textSize: 14; onEdited: value => book.cpValue = value }
            PaperNumber { width: 58; height: 48; label: "СМ"; value: book.spValue; minimum: 0; maximum: 999999; textSize: 14; onEdited: value => book.spValue = value }
            PaperNumber { width: 58; height: 48; label: "ЭМ"; value: book.epValue; minimum: 0; maximum: 999999; textSize: 14; onEdited: value => book.epValue = value }
            PaperNumber { width: 58; height: 48; label: "ЗМ"; value: book.gpValue; minimum: 0; maximum: 999999; textSize: 14; onEdited: value => book.gpValue = value }
            PaperNumber { width: 58; height: 48; label: "ПМ"; value: book.ppValue; minimum: 0; maximum: 999999; textSize: 14; onEdited: value => book.ppValue = value }
        }
        TextArea {
            x: 74; y: 10; width: 226; height: 346
            text: book.equipmentValue
            color: book.inkColor; font.pixelSize: 11
            wrapMode: TextEdit.Wrap; selectByMouse: true
            background: Item {}; padding: 2
            onTextChanged: if (activeFocus) book.equipmentValue = text
        }
    }

    PaperArea { x: 704; y: 140; width: 228; height: 148; label: "ЧЕРТЫ ХАРАКТЕРА"; value: book.personalityValue; onEdited: value => book.personalityValue = value }
    PaperArea { x: 704; y: 300; width: 228; height: 126; label: "ИДЕАЛЫ"; value: book.idealsValue; onEdited: value => book.idealsValue = value }
    PaperArea { x: 704; y: 438; width: 228; height: 126; label: "ПРИВЯЗАННОСТИ"; value: book.bondsValue; onEdited: value => book.bondsValue = value }
    PaperArea { x: 704; y: 576; width: 228; height: 126; label: "СЛАБОСТИ"; value: book.flawsValue; onEdited: value => book.flawsValue = value }
    PaperArea { x: 704; y: 718; width: 228; height: 530; label: "УМЕНИЯ И ОСОБЕННОСТИ"; value: book.featuresValue; onEdited: value => book.featuresValue = value }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 7
        text: "DnD Tracker · классический вид · лист 1/3"
        color: "#aaa195"
        font.pixelSize: 8
    }
}
