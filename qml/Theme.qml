pragma Singleton
import QtCore
import QtQuick

QtObject {
    readonly property bool darkMode: Application.styleHints.colorScheme === Qt.Dark
    readonly property string flavor: darkMode ? "mocha" : "frappe"

    // Catppuccin Mocha / Frappé palettes.
    readonly property color rosewater: darkMode ? "#F5E0DC" : "#F2D5CF"
    readonly property color flamingo: darkMode ? "#F2CDCD" : "#EEBEBE"
    readonly property color pink: darkMode ? "#F5C2E7" : "#F4B8E4"
    readonly property color mauve: darkMode ? "#CBA6F7" : "#CA9EE6"
    readonly property color red: darkMode ? "#F38BA8" : "#E78284"
    readonly property color maroon: darkMode ? "#EBA0AC" : "#EA999C"
    readonly property color peach: darkMode ? "#FAB387" : "#EF9F76"
    readonly property color yellow: darkMode ? "#F9E2AF" : "#E5C890"
    readonly property color green: darkMode ? "#A6E3A1" : "#A6D189"
    readonly property color teal: darkMode ? "#94E2D5" : "#81C8BE"
    readonly property color sky: darkMode ? "#89DCEB" : "#99D1DB"
    readonly property color sapphire: darkMode ? "#74C7EC" : "#85C1DC"
    readonly property color blue: darkMode ? "#89B4FA" : "#8CAAEE"
    readonly property color lavender: darkMode ? "#B4BEFE" : "#BABBF1"
    readonly property color textBase: darkMode ? "#CDD6F4" : "#C6D0F5"
    readonly property color subtext1: darkMode ? "#BAC2DE" : "#B5BFE2"
    readonly property color subtext0: darkMode ? "#A6ADC8" : "#A5ADCE"
    readonly property color overlay2: darkMode ? "#9399B2" : "#949CBB"
    readonly property color overlay1: darkMode ? "#7F849C" : "#838BA7"
    readonly property color overlay0: darkMode ? "#6C7086" : "#737994"
    readonly property color surface2: darkMode ? "#585B70" : "#626880"
    readonly property color surface1: darkMode ? "#45475A" : "#51576D"
    readonly property color surface0: darkMode ? "#313244" : "#414559"
    readonly property color base: darkMode ? "#1E1E2E" : "#303446"
    readonly property color mantle: darkMode ? "#181825" : "#292C3C"
    readonly property color crust: darkMode ? "#11111B" : "#232634"

    // Semantic application roles. Keep controls on these rather than on raw colors.
    readonly property color background: base
    readonly property color surface: mantle
    readonly property color surfaceRaised: surface0
    readonly property color surfaceHover: surface1
    readonly property color border: surface2
    readonly property color text: textBase
    readonly property color textMuted: subtext0
    readonly property color textSubtle: overlay1
    readonly property color accent: mauve
    readonly property color accentStrong: lavender
    readonly property color onAccent: base
    readonly property color danger: red
    readonly property color success: green
    readonly property color warning: yellow
    readonly property color info: blue
    readonly property color link: sapphire

    readonly property int radiusSmall: 8
    readonly property int radius: 14
    readonly property int radiusLarge: 20
    readonly property int gap: 12
    readonly property int touchTarget: 44
}
