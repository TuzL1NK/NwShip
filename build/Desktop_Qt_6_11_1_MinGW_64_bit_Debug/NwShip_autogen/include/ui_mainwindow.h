/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <widgets/multiselectcombobox.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionclose;
    QAction *actionsave;
    QAction *actionsave_as_2;
    QAction *actionclose_2;
    QAction *actionLight_Dark;
    QAction *actionEnglish;
    QAction *actionChinese;
    QWidget *centralwidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *contentLayout;
    QGroupBox *groupBox_config;
    QVBoxLayout *configLayout;
    QLabel *label_level;
    QSpinBox *spinBox_level;
    QLabel *label_hull;
    QComboBox *comboBox_hull;
    QLabel *label_stern;
    QComboBox *comboBox_stern;
    QLabel *label_bow;
    QComboBox *comboBox_bow;
    QLabel *label_bridge;
    QComboBox *comboBox_bridge;
    QSpacerItem *configSpacer;
    QHBoxLayout *buttonLayout;
    QPushButton *pushButton_calculate;
    QPushButton *pushButton_reset;
    QVBoxLayout *rightLayout;
    QGroupBox *groupBox_filters;
    QHBoxLayout *filterLayout;
    QVBoxLayout *whitelistLayout;
    QLabel *label_whitelist_title;
    MultiSelectComboBox *widget_whitelist;
    QVBoxLayout *blacklistLayout;
    QLabel *label_blacklist_title;
    MultiSelectComboBox *widget_blacklist;
    QLabel *label_status;
    QTableView *tableView;
    QMenuBar *menubar;
    QMenu *menuoptions;
    QMenu *menuLanguages;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 800);
        MainWindow->setMinimumSize(QSize(960, 640));
        actionclose = new QAction(MainWindow);
        actionclose->setObjectName("actionclose");
        actionsave = new QAction(MainWindow);
        actionsave->setObjectName("actionsave");
        actionsave_as_2 = new QAction(MainWindow);
        actionsave_as_2->setObjectName("actionsave_as_2");
        actionclose_2 = new QAction(MainWindow);
        actionclose_2->setObjectName("actionclose_2");
        actionLight_Dark = new QAction(MainWindow);
        actionLight_Dark->setObjectName("actionLight_Dark");
        actionEnglish = new QAction(MainWindow);
        actionEnglish->setObjectName("actionEnglish");
        actionChinese = new QAction(MainWindow);
        actionChinese->setObjectName("actionChinese");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mainLayout = new QVBoxLayout(centralwidget);
        mainLayout->setSpacing(12);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(16, 12, 16, 12);
        contentLayout = new QHBoxLayout();
        contentLayout->setSpacing(16);
        contentLayout->setObjectName("contentLayout");
        groupBox_config = new QGroupBox(centralwidget);
        groupBox_config->setObjectName("groupBox_config");
        groupBox_config->setMinimumSize(QSize(240, 0));
        groupBox_config->setMaximumSize(QSize(280, 16777215));
        configLayout = new QVBoxLayout(groupBox_config);
        configLayout->setSpacing(10);
        configLayout->setObjectName("configLayout");
        label_level = new QLabel(groupBox_config);
        label_level->setObjectName("label_level");

        configLayout->addWidget(label_level);

        spinBox_level = new QSpinBox(groupBox_config);
        spinBox_level->setObjectName("spinBox_level");
        spinBox_level->setMinimum(1);
        spinBox_level->setMaximum(150);
        spinBox_level->setValue(1);

        configLayout->addWidget(spinBox_level);

        label_hull = new QLabel(groupBox_config);
        label_hull->setObjectName("label_hull");

        configLayout->addWidget(label_hull);

        comboBox_hull = new QComboBox(groupBox_config);
        comboBox_hull->setObjectName("comboBox_hull");

        configLayout->addWidget(comboBox_hull);

        label_stern = new QLabel(groupBox_config);
        label_stern->setObjectName("label_stern");

        configLayout->addWidget(label_stern);

        comboBox_stern = new QComboBox(groupBox_config);
        comboBox_stern->setObjectName("comboBox_stern");

        configLayout->addWidget(comboBox_stern);

        label_bow = new QLabel(groupBox_config);
        label_bow->setObjectName("label_bow");

        configLayout->addWidget(label_bow);

        comboBox_bow = new QComboBox(groupBox_config);
        comboBox_bow->setObjectName("comboBox_bow");

        configLayout->addWidget(comboBox_bow);

        label_bridge = new QLabel(groupBox_config);
        label_bridge->setObjectName("label_bridge");

        configLayout->addWidget(label_bridge);

        comboBox_bridge = new QComboBox(groupBox_config);
        comboBox_bridge->setObjectName("comboBox_bridge");

        configLayout->addWidget(comboBox_bridge);

        configSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        configLayout->addItem(configSpacer);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName("buttonLayout");
        pushButton_calculate = new QPushButton(groupBox_config);
        pushButton_calculate->setObjectName("pushButton_calculate");

        buttonLayout->addWidget(pushButton_calculate);

        pushButton_reset = new QPushButton(groupBox_config);
        pushButton_reset->setObjectName("pushButton_reset");

        buttonLayout->addWidget(pushButton_reset);


        configLayout->addLayout(buttonLayout);


        contentLayout->addWidget(groupBox_config);

        rightLayout = new QVBoxLayout();
        rightLayout->setSpacing(12);
        rightLayout->setObjectName("rightLayout");
        groupBox_filters = new QGroupBox(centralwidget);
        groupBox_filters->setObjectName("groupBox_filters");
        filterLayout = new QHBoxLayout(groupBox_filters);
        filterLayout->setSpacing(16);
        filterLayout->setObjectName("filterLayout");
        whitelistLayout = new QVBoxLayout();
        whitelistLayout->setSpacing(6);
        whitelistLayout->setObjectName("whitelistLayout");
        label_whitelist_title = new QLabel(groupBox_filters);
        label_whitelist_title->setObjectName("label_whitelist_title");

        whitelistLayout->addWidget(label_whitelist_title);

        widget_whitelist = new MultiSelectComboBox(groupBox_filters);
        widget_whitelist->setObjectName("widget_whitelist");

        whitelistLayout->addWidget(widget_whitelist);


        filterLayout->addLayout(whitelistLayout);

        blacklistLayout = new QVBoxLayout();
        blacklistLayout->setSpacing(6);
        blacklistLayout->setObjectName("blacklistLayout");
        label_blacklist_title = new QLabel(groupBox_filters);
        label_blacklist_title->setObjectName("label_blacklist_title");

        blacklistLayout->addWidget(label_blacklist_title);

        widget_blacklist = new MultiSelectComboBox(groupBox_filters);
        widget_blacklist->setObjectName("widget_blacklist");

        blacklistLayout->addWidget(widget_blacklist);


        filterLayout->addLayout(blacklistLayout);


        rightLayout->addWidget(groupBox_filters);

        label_status = new QLabel(centralwidget);
        label_status->setObjectName("label_status");
        label_status->setWordWrap(true);

        rightLayout->addWidget(label_status);

        tableView = new QTableView(centralwidget);
        tableView->setObjectName("tableView");
        tableView->setAlternatingRowColors(true);
        tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        tableView->setShowGrid(false);

        rightLayout->addWidget(tableView);


        contentLayout->addLayout(rightLayout);


        mainLayout->addLayout(contentLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1200, 17));
        menuoptions = new QMenu(menubar);
        menuoptions->setObjectName("menuoptions");
        menuLanguages = new QMenu(menubar);
        menuLanguages->setObjectName("menuLanguages");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuoptions->menuAction());
        menubar->addAction(menuLanguages->menuAction());
        menuoptions->addAction(actionLight_Dark);
        menuLanguages->addAction(actionEnglish);
        menuLanguages->addAction(actionChinese);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "NwShip", nullptr));
        actionclose->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200", nullptr));
        actionsave->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230", nullptr));
        actionsave_as_2->setText(QCoreApplication::translate("MainWindow", "\345\217\246\345\255\230\344\270\272", nullptr));
        actionclose_2->setText(QCoreApplication::translate("MainWindow", "\351\200\200\345\207\272", nullptr));
        actionLight_Dark->setText(QCoreApplication::translate("MainWindow", "\345\210\207\346\215\242\346\265\205\350\211\262\344\270\273\351\242\230", nullptr));
        actionEnglish->setText(QCoreApplication::translate("MainWindow", "English", nullptr));
        actionChinese->setText(QCoreApplication::translate("MainWindow", "\344\270\255\346\226\207", nullptr));
        groupBox_config->setTitle(QCoreApplication::translate("MainWindow", "\346\275\234\350\211\207\351\205\215\347\275\256", nullptr));
        label_level->setText(QCoreApplication::translate("MainWindow", "\346\275\234\346\260\264\350\211\207\347\255\211\347\272\247", nullptr));
        label_hull->setText(QCoreApplication::translate("MainWindow", "\350\210\271\344\275\223", nullptr));
        label_stern->setText(QCoreApplication::translate("MainWindow", "\350\210\271\345\260\276", nullptr));
        label_bow->setText(QCoreApplication::translate("MainWindow", "\350\210\271\351\246\226", nullptr));
        label_bridge->setText(QCoreApplication::translate("MainWindow", "\350\210\260\346\241\245", nullptr));
        pushButton_calculate->setText(QCoreApplication::translate("MainWindow", "\345\274\200\345\247\213\350\256\241\347\256\227", nullptr));
        pushButton_reset->setText(QCoreApplication::translate("MainWindow", "\351\207\215\347\275\256", nullptr));
        groupBox_filters->setTitle(QCoreApplication::translate("MainWindow", "\350\210\252\347\202\271\347\255\233\351\200\211", nullptr));
        label_whitelist_title->setText(QCoreApplication::translate("MainWindow", "\347\231\275\345\220\215\345\215\225\357\274\210\344\274\230\345\205\210\346\216\242\347\264\242\357\274\211", nullptr));
        label_blacklist_title->setText(QCoreApplication::translate("MainWindow", "\351\273\221\345\220\215\345\215\225\357\274\210\346\216\222\351\231\244\357\274\211", nullptr));
        label_status->setText(QCoreApplication::translate("MainWindow", "\347\231\275\345\220\215\345\215\225 0 \351\241\271  \302\267  \351\273\221\345\220\215\345\215\225 0 \351\241\271", nullptr));
        menuoptions->setTitle(QCoreApplication::translate("MainWindow", "\351\200\211\351\241\271", nullptr));
        menuLanguages->setTitle(QCoreApplication::translate("MainWindow", "\350\257\255\350\250\200", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
