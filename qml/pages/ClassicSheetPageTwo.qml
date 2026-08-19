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

    component PaperArea: Rectangle {
        id: area
        property string label: ""
        property string value: ""
        property int textSize: 12
        signal edited(string value)

        color: "#fbf8f0"
        border.width: 1.5
        border.color: book.inkColor
        radius: 6

        Text {
            id: areaLabel
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 6
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
            anchors.margins: 8
            text: area.value
            color: book.inkColor
            font.pixelSize: area.textSize
            wrapMode: TextEdit.Wrap
            selectByMouse: true
            padding: 2
            background: Item {}
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

        PaperField { x: 12; y: 7; width: 168; height: 37; label: "ВОЗРАСТ"; value: book.ageValue; onEdited: value => book.ageValue = value }
        PaperField { x: 190; y: 7; width: 168; height: 37; label: "РОСТ"; value: book.heightValue; onEdited: value => book.heightValue = value }
        PaperField { x: 368; y: 7; width: 178; height: 37; label: "ВЕС"; value: book.weightValue; onEdited: value => book.weightValue = value }
        PaperField { x: 12; y: 49; width: 168; height: 35; label: "ГЛАЗА"; value: book.eyesValue; onEdited: value => book.eyesValue = value }
        PaperField { x: 190; y: 49; width: 168; height: 35; label: "КОЖА"; value: book.skinValue; onEdited: value => book.skinValue = value }
        PaperField { x: 368; y: 49; width: 178; height: 35; label: "ВОЛОСЫ"; value: book.hairValue; onEdited: value => book.hairValue = value }
    }

    PaperArea {
        x: 28; y: 140; width: 300; height: 386
        label: "ВНЕШНОСТЬ ПЕРСОНАЖА"
        value: book.appearanceValue
        textSize: 12
        onEdited: value => book.appearanceValue = value
    }

    Rectangle {
        x: 348; y: 140; width: 584; height: 386
        color: "#fbf8f0"; border.width: 1.5; border.color: book.inkColor; radius: 6

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 6
            text: "СОЮЗНИКИ И ОРГАНИЗАЦИИ"
            color: book.inkColor
            font.pixelSize: 9
            font.weight: Font.DemiBold
        }

        TextArea {
            x: 10; y: 10; width: 360; height: 340
            text: book.alliesOrganizationsValue
            color: book.inkColor
            font.pixelSize: 12
            wrapMode: TextEdit.Wrap
            selectByMouse: true
            padding: 2
            background: Item {}
            onTextChanged: if (activeFocus) book.alliesOrganizationsValue = text
        }

        Rectangle {
            x: 382; y: 22; width: 188; height: 270
            color: book.panelColor
            border.width: 1.5
            border.color: book.inkColor
            radius: 6

            TextArea {
                anchors.fill: parent
                anchors.margins: 8
                anchors.bottomMargin: 28
                text: book.alliesSymbolValue
                color: book.inkColor
                font.pixelSize: 12
                wrapMode: TextEdit.Wrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                selectByMouse: true
                background: Item {}
                placeholderText: "Название, описание или знак организации"
                onTextChanged: if (activeFocus) book.alliesSymbolValue = text
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 7
                text: "СИМВОЛ"
                color: book.inkColor
                font.pixelSize: 9
                font.weight: Font.DemiBold
            }
        }
    }

    PaperArea {
        x: 28; y: 546; width: 300; height: 702
        label: "ПРЕДЫСТОРИЯ ПЕРСОНАЖА"
        value: book.backstoryValue
        textSize: 12
        onEdited: value => book.backstoryValue = value
    }

    PaperArea {
        x: 348; y: 546; width: 584; height: 350
        label: "ДОПОЛНИТЕЛЬНЫЕ УМЕНИЯ И ОСОБЕННОСТИ"
        value: book.additionalFeaturesValue
        textSize: 12
        onEdited: value => book.additionalFeaturesValue = value
    }

    PaperArea {
        x: 348; y: 916; width: 584; height: 332
        label: "СОКРОВИЩА"
        value: book.treasureValue
        textSize: 12
        onEdited: value => book.treasureValue = value
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 7
        text: "DnD Tracker · классический вид · лист 2/3"
        color: "#aaa195"
        font.pixelSize: 8
    }
}
