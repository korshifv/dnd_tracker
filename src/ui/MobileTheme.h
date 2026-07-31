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
            font-size: 13px;
        }

        /* === TAB WIDGET (Bottom Bar / Top Bar) === */
        QTabWidget::pane {
            border: none;
            background-color: #121216;
        }

        QTabBar {
            background-color: #1A1A24;
            border-top: 1px solid #2A2A38;
            border-bottom: 1px solid #2A2A38;
        }

        QTabBar::tab {
            background-color: #1A1A24;
            color: #9A9AB0;
            padding: 10px 14px;
            font-weight: 600;
            font-size: 14px;
            border: none;
            min-height: 40px;
        }

        QTabBar::tab:selected {
            color: #A57BFF;
            background-color: #22222E;
            border-top: 3px solid #8C62FF;
            border-bottom: 3px solid #8C62FF;
        }

        QTabBar::tab:hover:!selected {
            color: #D0D0E0;
            background-color: #1E1E28;
        }

        /* === BUTTONS (Material Design Style) === */
        QPushButton {
            background-color: #262634;
            color: #FFFFFF;
            border: 1px solid #36364A;
            border-radius: 8px;
            padding: 8px 14px;
            font-weight: 600;
            font-size: 13px;
            min-height: 38px;
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

        /* === INPUT FIELDS & SPINS === */
        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QComboBox {
            background-color: #1A1A22;
            color: #F0F0F8;
            border: 1px solid #2E2E40;
            border-radius: 6px;
            padding: 8px 10px;
            selection-background-color: #8C62FF;
            min-height: 36px;
        }

        QLineEdit:focus, QTextEdit:focus, QSpinBox:focus, QComboBox:focus {
            border: 2px solid #8C62FF;
            background-color: #20202C;
        }

        /* === CARDS & FRAMES === */
        QFrame#cardFrame, QGroupBox {
            background-color: #1B1B24;
            border: 1px solid #2A2A3A;
            border-radius: 10px;
            padding: 10px;
            margin-top: 4px;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 4px;
            color: #A57BFF;
            font-weight: bold;
        }

        /* === SCROLLBARS === */
        QScrollBar:vertical, QScrollBar:horizontal {
            background: transparent;
            width: 0px;
            height: 0px;
        }

        /* === LISTS & TREES === */
        QListWidget, QListView, QTreeWidget, QTreeView {
            background-color: #16161E;
            border: 1px solid #262636;
            border-radius: 8px;
            padding: 4px;
        }

        QListWidget::item, QTreeView::item {
            padding: 10px;
            border-radius: 6px;
            margin-bottom: 2px;
            background-color: #1E1E28;
        }

        QListWidget::item:selected, QTreeView::item:selected {
            background-color: #8C62FF;
            color: #FFFFFF;
        }

        /* === HEADERS & LABELS === */
        QLabel {
            color: #E0E0E6;
        }
    )");
}

} // namespace MobileTheme

#endif // MOBILETHEME_H
