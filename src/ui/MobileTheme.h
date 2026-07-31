#ifndef MOBILETHEME_H
#define MOBILETHEME_H

#include <QString>

namespace MobileTheme {

inline QString getStylesheet() {
    return QStringLiteral(R"(
        /* === GLOBAL MODERN DARK THEME === */
        QWidget {
            background-color: #121216;
            color: #E0E0E6;
            font-family: 'Segoe UI', 'Roboto', 'Inter', sans-serif;
            font-size: 14px;
        }

        /* === TAB WIDGET (Mobile Bottom Bar & Desktop Tabs) === */
        QTabWidget::pane {
            border: none;
            background-color: #121216;
        }

        QTabBar {
            background-color: #1A1A22;
            border-top: 1px solid #282836;
        }

        QTabBar::tab {
            background-color: #1A1A22;
            color: #9A9AB0;
            padding: 12px 18px;
            font-weight: 600;
            font-size: 15px;
            border: none;
            min-height: 44px;
        }

        QTabBar::tab:selected {
            color: #A57BFF;
            background-color: #22222E;
            border-top: 3px solid #8C62FF;
        }

        QTabBar::tab:hover:!selected {
            color: #D0D0E0;
            background-color: #1E1E28;
        }

        /* Hide close buttons on non-closable fixed tabs */
        QTabBar::tab:first, QTabBar::tab:nth-child(2), QTabBar::tab:nth-child(3) {
            /* Clean tab style */
        }

        /* === BUTTONS (Material Design 3 Style) === */
        QPushButton {
            background-color: #262634;
            color: #FFFFFF;
            border: 1px solid #36364A;
            border-radius: 10px;
            padding: 10px 18px;
            font-weight: 600;
            font-size: 14px;
            min-height: 44px;
        }

        QPushButton:hover {
            background-color: #323246;
            border-color: #8C62FF;
        }

        QPushButton:pressed {
            background-color: #8C62FF;
            color: #FFFFFF;
        }

        QPushButton:disabled {
            background-color: #181820;
            color: #555566;
            border-color: #22222E;
        }

        /* Primary Accent Button */
        QPushButton#primaryBtn, QPushButton[accent="true"] {
            background-color: #8C62FF;
            color: #FFFFFF;
            border: none;
        }

        QPushButton#primaryBtn:hover, QPushButton[accent="true"]:hover {
            background-color: #9D75FF;
        }

        /* === INPUT FIELDS & SPINS === */
        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QComboBox {
            background-color: #1A1A22;
            color: #F0F0F8;
            border: 1px solid #2E2E40;
            border-radius: 8px;
            padding: 10px 12px;
            selection-background-color: #8C62FF;
            min-height: 44px;
        }

        QLineEdit:focus, QTextEdit:focus, QSpinBox:focus, QComboBox:focus {
            border: 2px solid #8C62FF;
            background-color: #20202C;
        }

        QSpinBox::up-button, QSpinBox::down-button {
            width: 24px;
            background-color: #2A2A3A;
            border-radius: 4px;
        }

        /* === CARDS & FRAMES === */
        QFrame#cardFrame, QGroupBox {
            background-color: #1B1B24;
            border: 1px solid #2A2A3A;
            border-radius: 12px;
            padding: 12px;
            margin-top: 6px;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 14px;
            padding: 0 6px;
            color: #A57BFF;
            font-weight: bold;
        }

        /* === SCROLLBARS === */
        QScrollBar:vertical {
            background: #121216;
            width: 10px;
            margin: 0px;
        }

        QScrollBar::handle:vertical {
            background: #323246;
            min-height: 30px;
            border-radius: 5px;
        }

        QScrollBar::handle:vertical:hover {
            background: #8C62FF;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QScrollBar:horizontal {
            background: #121216;
            height: 10px;
            margin: 0px;
        }

        QScrollBar::handle:horizontal {
            background: #323246;
            min-width: 30px;
            border-radius: 5px;
        }

        QScrollBar::handle:horizontal:hover {
            background: #8C62FF;
        }

        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }

        /* === LISTS & TABLES === */
        QListWidget, QListView, QTreeWidget {
            background-color: #16161E;
            border: 1px solid #262636;
            border-radius: 10px;
            padding: 6px;
        }

        QListWidget::item {
            padding: 12px;
            border-radius: 8px;
            margin-bottom: 4px;
            background-color: #1E1E28;
        }

        QListWidget::item:selected {
            background-color: #8C62FF;
            color: #FFFFFF;
        }

        QListWidget::item:hover:!selected {
            background-color: #2A2A38;
        }

        /* === HEADERS & LABELS === */
        QLabel {
            color: #E0E0E6;
        }

        QLabel#headerLabel {
            font-size: 18px;
            font-weight: bold;
            color: #FFFFFF;
        }
    )");
}

} // namespace MobileTheme

#endif // MOBILETHEME_H
