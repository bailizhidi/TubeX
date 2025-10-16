/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionNew;
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionSaveAs;
    QAction *actionExit;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionPreferences;
    QAction *actionToggleProjectTree;
    QAction *actionToggleProperties;
    QAction *actionToggleLog;
    QAction *actionResetLayout;
    QAction *actionRunSimulation;
    QAction *actionPostProcessing;
    QAction *actionMeshGeneration;
    QAction *actionAbout;
    QAction *actionDocumentation;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QMdiArea *mdiArea;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuTools;
    QMenu *menuHelp;
    QToolBar *toolBar;
    QStatusBar *statusbar;
    QDockWidget *dockWidget_ProjectTree;
    QWidget *dockWidgetContents_ProjectTree;
    QVBoxLayout *verticalLayout_ProjectTree;
    QTreeView *treeView_Project;
    QDockWidget *dockWidget_Properties;
    QWidget *dockWidgetContents_Properties;
    QVBoxLayout *verticalLayout_Properties;
    QTabWidget *tabWidget;
    QWidget *tab_5;
    QPushButton *pushButton_initmodel;
    QLabel *label_27;
    QLabel *label_28;
    QPushButton *pushButton_surfacemodel;
    QPushButton *pushButton_centerlinemodel;
    QPushButton *pushButton_meshmodel;
    QWidget *tab;
    QLabel *label_Tube;
    QListWidget *listWidget;
    QWidget *formLayoutWidget;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *lineEdit_Rout;
    QLabel *label_2;
    QLineEdit *lineEdit_Rin;
    QLabel *label_3;
    QLineEdit *lineEdit_Length;
    QLabel *label_Tube_2;
    QListWidget *listWidget_2;
    QWidget *formLayoutWidget_2;
    QFormLayout *formLayout_2;
    QLabel *label_4;
    QLineEdit *lineEdit_SleeveThickness1;
    QLabel *label_5;
    QLineEdit *lineEdit_SleeveLength1;
    QLabel *label_6;
    QLineEdit *lineEdit_RotaryPos;
    QLabel *label_Tube_3;
    QListWidget *listWidget_3;
    QWidget *formLayoutWidget_3;
    QFormLayout *formLayout_3;
    QLabel *label_7;
    QLineEdit *lineEdit_SleeveThickness;
    QLabel *label_8;
    QLineEdit *lineEdit_SleeveLength;
    QLabel *label_9;
    QLineEdit *lineEdit_FixedPos;
    QLabel *label_Tube_4;
    QListWidget *listWidget_4;
    QWidget *formLayoutWidget_4;
    QFormLayout *formLayout_4;
    QLabel *label_10;
    QLineEdit *lineEdit_ArcR;
    QLabel *label_11;
    QLineEdit *lineEdit_ArcThickness;
    QLabel *label_12;
    QLineEdit *lineEdit_ArcAngle;
    QPushButton *pushButton_confirm_CADmodel;
    QWidget *tab_2;
    QLabel *label_25;
    QDoubleSpinBox *doubleSpinBox;
    QPushButton *pushButton_Mesh1;
    QWidget *tab_3;
    QLabel *label_26;
    QPushButton *pushButton_extractFace;
    QWidget *tab_4;
    QLabel *label_29;
    QPushButton *pushButton_ExtractCenterLine;
    QTableWidget *resultsTable;
    QDockWidget *dockWidget_Features;
    QWidget *dockWidgetContents_Features;
    QVBoxLayout *verticalLayout_Features;
    QListWidget *listWidget_Features;
    QDockWidget *dockWidget_Log;
    QWidget *dockWidgetContents_Log;
    QGridLayout *gridLayout;
    QTextEdit *textEdit_Log;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1204, 807);
        actionNew = new QAction(MainWindow);
        actionNew->setObjectName(QString::fromUtf8("actionNew"));
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        actionSaveAs = new QAction(MainWindow);
        actionSaveAs->setObjectName(QString::fromUtf8("actionSaveAs"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionUndo = new QAction(MainWindow);
        actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
        actionRedo = new QAction(MainWindow);
        actionRedo->setObjectName(QString::fromUtf8("actionRedo"));
        actionPreferences = new QAction(MainWindow);
        actionPreferences->setObjectName(QString::fromUtf8("actionPreferences"));
        actionToggleProjectTree = new QAction(MainWindow);
        actionToggleProjectTree->setObjectName(QString::fromUtf8("actionToggleProjectTree"));
        actionToggleProperties = new QAction(MainWindow);
        actionToggleProperties->setObjectName(QString::fromUtf8("actionToggleProperties"));
        actionToggleLog = new QAction(MainWindow);
        actionToggleLog->setObjectName(QString::fromUtf8("actionToggleLog"));
        actionResetLayout = new QAction(MainWindow);
        actionResetLayout->setObjectName(QString::fromUtf8("actionResetLayout"));
        actionRunSimulation = new QAction(MainWindow);
        actionRunSimulation->setObjectName(QString::fromUtf8("actionRunSimulation"));
        actionPostProcessing = new QAction(MainWindow);
        actionPostProcessing->setObjectName(QString::fromUtf8("actionPostProcessing"));
        actionMeshGeneration = new QAction(MainWindow);
        actionMeshGeneration->setObjectName(QString::fromUtf8("actionMeshGeneration"));
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionDocumentation = new QAction(MainWindow);
        actionDocumentation->setObjectName(QString::fromUtf8("actionDocumentation"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        mdiArea = new QMdiArea(centralwidget);
        mdiArea->setObjectName(QString::fromUtf8("mdiArea"));
        mdiArea->setStyleSheet(QString::fromUtf8(""));

        verticalLayout->addWidget(mdiArea);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1204, 27));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuView = new QMenu(menubar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuTools = new QMenu(menubar);
        menuTools->setObjectName(QString::fromUtf8("menuTools"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        MainWindow->setMenuBar(menubar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);
        dockWidget_ProjectTree = new QDockWidget(MainWindow);
        dockWidget_ProjectTree->setObjectName(QString::fromUtf8("dockWidget_ProjectTree"));
        dockWidgetContents_ProjectTree = new QWidget();
        dockWidgetContents_ProjectTree->setObjectName(QString::fromUtf8("dockWidgetContents_ProjectTree"));
        verticalLayout_ProjectTree = new QVBoxLayout(dockWidgetContents_ProjectTree);
        verticalLayout_ProjectTree->setObjectName(QString::fromUtf8("verticalLayout_ProjectTree"));
        treeView_Project = new QTreeView(dockWidgetContents_ProjectTree);
        treeView_Project->setObjectName(QString::fromUtf8("treeView_Project"));

        verticalLayout_ProjectTree->addWidget(treeView_Project);

        dockWidget_ProjectTree->setWidget(dockWidgetContents_ProjectTree);
        MainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockWidget_ProjectTree);
        dockWidget_Properties = new QDockWidget(MainWindow);
        dockWidget_Properties->setObjectName(QString::fromUtf8("dockWidget_Properties"));
        dockWidget_Properties->setMinimumSize(QSize(274, 493));
        dockWidgetContents_Properties = new QWidget();
        dockWidgetContents_Properties->setObjectName(QString::fromUtf8("dockWidgetContents_Properties"));
        verticalLayout_Properties = new QVBoxLayout(dockWidgetContents_Properties);
        verticalLayout_Properties->setObjectName(QString::fromUtf8("verticalLayout_Properties"));
        tabWidget = new QTabWidget(dockWidgetContents_Properties);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setMinimumSize(QSize(256, 455));
        tabWidget->setStyleSheet(QString::fromUtf8("QTabBar::tab {\n"
"	background-color: rgb(222, 221, 218);\n"
"    color: #303030; \n"
"}\n"
"QTabBar::tab:selected {\n"
"   background-color: rgb(94, 92, 100);\n"
"	color: rgb(0, 0, 0);\n"
"}\n"
""));
        tabWidget->setElideMode(Qt::ElideNone);
        tab_5 = new QWidget();
        tab_5->setObjectName(QString::fromUtf8("tab_5"));
        pushButton_initmodel = new QPushButton(tab_5);
        pushButton_initmodel->setObjectName(QString::fromUtf8("pushButton_initmodel"));
        pushButton_initmodel->setGeometry(QRect(10, 30, 110, 25));
        pushButton_initmodel->setMinimumSize(QSize(110, 25));
        pushButton_initmodel->setMaximumSize(QSize(110, 25));
        pushButton_initmodel->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        label_27 = new QLabel(tab_5);
        label_27->setObjectName(QString::fromUtf8("label_27"));
        label_27->setGeometry(QRect(0, 0, 251, 30));
        label_27->setMinimumSize(QSize(0, 30));
        label_27->setMaximumSize(QSize(100000, 25));
        label_27->setStyleSheet(QString::fromUtf8(""));
        label_28 = new QLabel(tab_5);
        label_28->setObjectName(QString::fromUtf8("label_28"));
        label_28->setGeometry(QRect(0, 100, 251, 30));
        label_28->setMinimumSize(QSize(0, 30));
        label_28->setMaximumSize(QSize(100000, 25));
        label_28->setStyleSheet(QString::fromUtf8(""));
        pushButton_surfacemodel = new QPushButton(tab_5);
        pushButton_surfacemodel->setObjectName(QString::fromUtf8("pushButton_surfacemodel"));
        pushButton_surfacemodel->setGeometry(QRect(130, 30, 110, 25));
        pushButton_surfacemodel->setMinimumSize(QSize(110, 25));
        pushButton_surfacemodel->setMaximumSize(QSize(110, 25));
        pushButton_surfacemodel->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        pushButton_centerlinemodel = new QPushButton(tab_5);
        pushButton_centerlinemodel->setObjectName(QString::fromUtf8("pushButton_centerlinemodel"));
        pushButton_centerlinemodel->setGeometry(QRect(130, 70, 110, 25));
        pushButton_centerlinemodel->setMinimumSize(QSize(110, 25));
        pushButton_centerlinemodel->setMaximumSize(QSize(110, 25));
        pushButton_centerlinemodel->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        pushButton_meshmodel = new QPushButton(tab_5);
        pushButton_meshmodel->setObjectName(QString::fromUtf8("pushButton_meshmodel"));
        pushButton_meshmodel->setGeometry(QRect(10, 70, 110, 25));
        pushButton_meshmodel->setMinimumSize(QSize(110, 25));
        pushButton_meshmodel->setMaximumSize(QSize(110, 25));
        pushButton_meshmodel->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        tabWidget->addTab(tab_5, QString());
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        tab->setMinimumSize(QSize(75, 0));
        label_Tube = new QLabel(tab);
        label_Tube->setObjectName(QString::fromUtf8("label_Tube"));
        label_Tube->setGeometry(QRect(0, 0, 331, 20));
        label_Tube->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
""));
        listWidget = new QListWidget(tab);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        listWidget->setGeometry(QRect(0, 20, 256, 81));
        listWidget->setStyleSheet(QString::fromUtf8(""));
        formLayoutWidget = new QWidget(tab);
        formLayoutWidget->setObjectName(QString::fromUtf8("formLayoutWidget"));
        formLayoutWidget->setGeometry(QRect(-1, 20, 241, 81));
        formLayout = new QFormLayout(formLayoutWidget);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(formLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMinimumSize(QSize(90, 20));
        label->setMaximumSize(QSize(90, 20));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        lineEdit_Rout = new QLineEdit(formLayoutWidget);
        lineEdit_Rout->setObjectName(QString::fromUtf8("lineEdit_Rout"));
        lineEdit_Rout->setMinimumSize(QSize(100, 20));
        lineEdit_Rout->setMaximumSize(QSize(100, 20));

        formLayout->setWidget(0, QFormLayout::FieldRole, lineEdit_Rout);

        label_2 = new QLabel(formLayoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMinimumSize(QSize(90, 20));
        label_2->setMaximumSize(QSize(90, 20));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        lineEdit_Rin = new QLineEdit(formLayoutWidget);
        lineEdit_Rin->setObjectName(QString::fromUtf8("lineEdit_Rin"));
        lineEdit_Rin->setMinimumSize(QSize(100, 20));
        lineEdit_Rin->setMaximumSize(QSize(100, 20));

        formLayout->setWidget(1, QFormLayout::FieldRole, lineEdit_Rin);

        label_3 = new QLabel(formLayoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setMinimumSize(QSize(90, 20));
        label_3->setMaximumSize(QSize(90, 20));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_3);

        lineEdit_Length = new QLineEdit(formLayoutWidget);
        lineEdit_Length->setObjectName(QString::fromUtf8("lineEdit_Length"));
        lineEdit_Length->setMinimumSize(QSize(100, 20));
        lineEdit_Length->setMaximumSize(QSize(100, 20));

        formLayout->setWidget(2, QFormLayout::FieldRole, lineEdit_Length);

        label_Tube_2 = new QLabel(tab);
        label_Tube_2->setObjectName(QString::fromUtf8("label_Tube_2"));
        label_Tube_2->setGeometry(QRect(0, 100, 331, 20));
        label_Tube_2->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
""));
        listWidget_2 = new QListWidget(tab);
        listWidget_2->setObjectName(QString::fromUtf8("listWidget_2"));
        listWidget_2->setGeometry(QRect(0, 120, 256, 81));
        listWidget_2->setStyleSheet(QString::fromUtf8(""));
        formLayoutWidget_2 = new QWidget(tab);
        formLayoutWidget_2->setObjectName(QString::fromUtf8("formLayoutWidget_2"));
        formLayoutWidget_2->setGeometry(QRect(0, 120, 241, 81));
        formLayout_2 = new QFormLayout(formLayoutWidget_2);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        formLayout_2->setContentsMargins(0, 0, 0, 0);
        label_4 = new QLabel(formLayoutWidget_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setMinimumSize(QSize(90, 20));
        label_4->setMaximumSize(QSize(90, 20));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label_4);

        lineEdit_SleeveThickness1 = new QLineEdit(formLayoutWidget_2);
        lineEdit_SleeveThickness1->setObjectName(QString::fromUtf8("lineEdit_SleeveThickness1"));
        lineEdit_SleeveThickness1->setMinimumSize(QSize(100, 20));
        lineEdit_SleeveThickness1->setMaximumSize(QSize(100, 20));

        formLayout_2->setWidget(0, QFormLayout::FieldRole, lineEdit_SleeveThickness1);

        label_5 = new QLabel(formLayoutWidget_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setMinimumSize(QSize(90, 20));
        label_5->setMaximumSize(QSize(90, 20));

        formLayout_2->setWidget(1, QFormLayout::LabelRole, label_5);

        lineEdit_SleeveLength1 = new QLineEdit(formLayoutWidget_2);
        lineEdit_SleeveLength1->setObjectName(QString::fromUtf8("lineEdit_SleeveLength1"));
        lineEdit_SleeveLength1->setMinimumSize(QSize(100, 20));
        lineEdit_SleeveLength1->setMaximumSize(QSize(100, 20));

        formLayout_2->setWidget(1, QFormLayout::FieldRole, lineEdit_SleeveLength1);

        label_6 = new QLabel(formLayoutWidget_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setMinimumSize(QSize(90, 20));
        label_6->setMaximumSize(QSize(90, 20));

        formLayout_2->setWidget(2, QFormLayout::LabelRole, label_6);

        lineEdit_RotaryPos = new QLineEdit(formLayoutWidget_2);
        lineEdit_RotaryPos->setObjectName(QString::fromUtf8("lineEdit_RotaryPos"));
        lineEdit_RotaryPos->setMinimumSize(QSize(100, 20));
        lineEdit_RotaryPos->setMaximumSize(QSize(100, 20));

        formLayout_2->setWidget(2, QFormLayout::FieldRole, lineEdit_RotaryPos);

        label_Tube_3 = new QLabel(tab);
        label_Tube_3->setObjectName(QString::fromUtf8("label_Tube_3"));
        label_Tube_3->setGeometry(QRect(0, 200, 331, 20));
        label_Tube_3->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
""));
        listWidget_3 = new QListWidget(tab);
        listWidget_3->setObjectName(QString::fromUtf8("listWidget_3"));
        listWidget_3->setGeometry(QRect(0, 220, 256, 81));
        listWidget_3->setStyleSheet(QString::fromUtf8(""));
        formLayoutWidget_3 = new QWidget(tab);
        formLayoutWidget_3->setObjectName(QString::fromUtf8("formLayoutWidget_3"));
        formLayoutWidget_3->setGeometry(QRect(0, 220, 241, 81));
        formLayout_3 = new QFormLayout(formLayoutWidget_3);
        formLayout_3->setObjectName(QString::fromUtf8("formLayout_3"));
        formLayout_3->setContentsMargins(0, 0, 0, 0);
        label_7 = new QLabel(formLayoutWidget_3);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setMinimumSize(QSize(90, 20));
        label_7->setMaximumSize(QSize(90, 20));

        formLayout_3->setWidget(0, QFormLayout::LabelRole, label_7);

        lineEdit_SleeveThickness = new QLineEdit(formLayoutWidget_3);
        lineEdit_SleeveThickness->setObjectName(QString::fromUtf8("lineEdit_SleeveThickness"));
        lineEdit_SleeveThickness->setMinimumSize(QSize(100, 20));
        lineEdit_SleeveThickness->setMaximumSize(QSize(100, 20));

        formLayout_3->setWidget(0, QFormLayout::FieldRole, lineEdit_SleeveThickness);

        label_8 = new QLabel(formLayoutWidget_3);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setMinimumSize(QSize(90, 20));
        label_8->setMaximumSize(QSize(90, 20));

        formLayout_3->setWidget(1, QFormLayout::LabelRole, label_8);

        lineEdit_SleeveLength = new QLineEdit(formLayoutWidget_3);
        lineEdit_SleeveLength->setObjectName(QString::fromUtf8("lineEdit_SleeveLength"));
        lineEdit_SleeveLength->setMinimumSize(QSize(100, 20));
        lineEdit_SleeveLength->setMaximumSize(QSize(100, 20));

        formLayout_3->setWidget(1, QFormLayout::FieldRole, lineEdit_SleeveLength);

        label_9 = new QLabel(formLayoutWidget_3);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setMinimumSize(QSize(90, 20));
        label_9->setMaximumSize(QSize(90, 20));

        formLayout_3->setWidget(2, QFormLayout::LabelRole, label_9);

        lineEdit_FixedPos = new QLineEdit(formLayoutWidget_3);
        lineEdit_FixedPos->setObjectName(QString::fromUtf8("lineEdit_FixedPos"));
        lineEdit_FixedPos->setMinimumSize(QSize(100, 20));
        lineEdit_FixedPos->setMaximumSize(QSize(100, 20));

        formLayout_3->setWidget(2, QFormLayout::FieldRole, lineEdit_FixedPos);

        label_Tube_4 = new QLabel(tab);
        label_Tube_4->setObjectName(QString::fromUtf8("label_Tube_4"));
        label_Tube_4->setGeometry(QRect(0, 300, 331, 20));
        label_Tube_4->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
""));
        listWidget_4 = new QListWidget(tab);
        listWidget_4->setObjectName(QString::fromUtf8("listWidget_4"));
        listWidget_4->setGeometry(QRect(0, 320, 256, 81));
        listWidget_4->setStyleSheet(QString::fromUtf8(""));
        formLayoutWidget_4 = new QWidget(tab);
        formLayoutWidget_4->setObjectName(QString::fromUtf8("formLayoutWidget_4"));
        formLayoutWidget_4->setGeometry(QRect(0, 320, 241, 81));
        formLayout_4 = new QFormLayout(formLayoutWidget_4);
        formLayout_4->setObjectName(QString::fromUtf8("formLayout_4"));
        formLayout_4->setContentsMargins(0, 0, 0, 0);
        label_10 = new QLabel(formLayoutWidget_4);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setMinimumSize(QSize(90, 20));
        label_10->setMaximumSize(QSize(90, 20));

        formLayout_4->setWidget(0, QFormLayout::LabelRole, label_10);

        lineEdit_ArcR = new QLineEdit(formLayoutWidget_4);
        lineEdit_ArcR->setObjectName(QString::fromUtf8("lineEdit_ArcR"));
        lineEdit_ArcR->setMinimumSize(QSize(100, 20));
        lineEdit_ArcR->setMaximumSize(QSize(100, 20));

        formLayout_4->setWidget(0, QFormLayout::FieldRole, lineEdit_ArcR);

        label_11 = new QLabel(formLayoutWidget_4);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setMinimumSize(QSize(90, 20));
        label_11->setMaximumSize(QSize(90, 20));

        formLayout_4->setWidget(1, QFormLayout::LabelRole, label_11);

        lineEdit_ArcThickness = new QLineEdit(formLayoutWidget_4);
        lineEdit_ArcThickness->setObjectName(QString::fromUtf8("lineEdit_ArcThickness"));
        lineEdit_ArcThickness->setMinimumSize(QSize(100, 20));
        lineEdit_ArcThickness->setMaximumSize(QSize(100, 20));

        formLayout_4->setWidget(1, QFormLayout::FieldRole, lineEdit_ArcThickness);

        label_12 = new QLabel(formLayoutWidget_4);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setMinimumSize(QSize(90, 20));
        label_12->setMaximumSize(QSize(90, 20));

        formLayout_4->setWidget(2, QFormLayout::LabelRole, label_12);

        lineEdit_ArcAngle = new QLineEdit(formLayoutWidget_4);
        lineEdit_ArcAngle->setObjectName(QString::fromUtf8("lineEdit_ArcAngle"));
        lineEdit_ArcAngle->setMinimumSize(QSize(100, 20));
        lineEdit_ArcAngle->setMaximumSize(QSize(100, 20));

        formLayout_4->setWidget(2, QFormLayout::FieldRole, lineEdit_ArcAngle);

        pushButton_confirm_CADmodel = new QPushButton(tab);
        pushButton_confirm_CADmodel->setObjectName(QString::fromUtf8("pushButton_confirm_CADmodel"));
        pushButton_confirm_CADmodel->setGeometry(QRect(180, 400, 75, 25));
        pushButton_confirm_CADmodel->setMinimumSize(QSize(75, 25));
        pushButton_confirm_CADmodel->setMaximumSize(QSize(75, 25));
        pushButton_confirm_CADmodel->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        label_25 = new QLabel(tab_2);
        label_25->setObjectName(QString::fromUtf8("label_25"));
        label_25->setGeometry(QRect(0, 0, 60, 25));
        label_25->setMinimumSize(QSize(60, 25));
        label_25->setMaximumSize(QSize(60, 25));
        label_25->setStyleSheet(QString::fromUtf8(""));
        doubleSpinBox = new QDoubleSpinBox(tab_2);
        doubleSpinBox->setObjectName(QString::fromUtf8("doubleSpinBox"));
        doubleSpinBox->setGeometry(QRect(70, 0, 65, 27));
        pushButton_Mesh1 = new QPushButton(tab_2);
        pushButton_Mesh1->setObjectName(QString::fromUtf8("pushButton_Mesh1"));
        pushButton_Mesh1->setGeometry(QRect(180, 30, 75, 25));
        pushButton_Mesh1->setMinimumSize(QSize(75, 25));
        pushButton_Mesh1->setMaximumSize(QSize(75, 25));
        pushButton_Mesh1->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        label_26 = new QLabel(tab_3);
        label_26->setObjectName(QString::fromUtf8("label_26"));
        label_26->setGeometry(QRect(0, 0, 251, 30));
        label_26->setMinimumSize(QSize(0, 30));
        label_26->setMaximumSize(QSize(100000, 25));
        label_26->setStyleSheet(QString::fromUtf8(""));
        pushButton_extractFace = new QPushButton(tab_3);
        pushButton_extractFace->setObjectName(QString::fromUtf8("pushButton_extractFace"));
        pushButton_extractFace->setGeometry(QRect(160, 30, 90, 25));
        pushButton_extractFace->setMinimumSize(QSize(90, 25));
        pushButton_extractFace->setMaximumSize(QSize(90, 25));
        pushButton_extractFace->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        tabWidget->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        label_29 = new QLabel(tab_4);
        label_29->setObjectName(QString::fromUtf8("label_29"));
        label_29->setGeometry(QRect(0, 0, 251, 30));
        label_29->setMinimumSize(QSize(0, 30));
        label_29->setMaximumSize(QSize(100000, 25));
        label_29->setStyleSheet(QString::fromUtf8(""));
        pushButton_ExtractCenterLine = new QPushButton(tab_4);
        pushButton_ExtractCenterLine->setObjectName(QString::fromUtf8("pushButton_ExtractCenterLine"));
        pushButton_ExtractCenterLine->setGeometry(QRect(180, 230, 75, 25));
        pushButton_ExtractCenterLine->setMinimumSize(QSize(75, 25));
        pushButton_ExtractCenterLine->setMaximumSize(QSize(75, 25));
        pushButton_ExtractCenterLine->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        resultsTable = new QTableWidget(tab_4);
        resultsTable->setObjectName(QString::fromUtf8("resultsTable"));
        resultsTable->setGeometry(QRect(0, 30, 256, 192));
        tabWidget->addTab(tab_4, QString());

        verticalLayout_Properties->addWidget(tabWidget);

        dockWidget_Properties->setWidget(dockWidgetContents_Properties);
        MainWindow->addDockWidget(Qt::RightDockWidgetArea, dockWidget_Properties);
        dockWidget_Features = new QDockWidget(MainWindow);
        dockWidget_Features->setObjectName(QString::fromUtf8("dockWidget_Features"));
        dockWidgetContents_Features = new QWidget();
        dockWidgetContents_Features->setObjectName(QString::fromUtf8("dockWidgetContents_Features"));
        verticalLayout_Features = new QVBoxLayout(dockWidgetContents_Features);
        verticalLayout_Features->setObjectName(QString::fromUtf8("verticalLayout_Features"));
        listWidget_Features = new QListWidget(dockWidgetContents_Features);
        new QListWidgetItem(listWidget_Features);
        new QListWidgetItem(listWidget_Features);
        new QListWidgetItem(listWidget_Features);
        new QListWidgetItem(listWidget_Features);
        new QListWidgetItem(listWidget_Features);
        new QListWidgetItem(listWidget_Features);
        listWidget_Features->setObjectName(QString::fromUtf8("listWidget_Features"));

        verticalLayout_Features->addWidget(listWidget_Features);

        dockWidget_Features->setWidget(dockWidgetContents_Features);
        MainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockWidget_Features);
        dockWidget_Log = new QDockWidget(MainWindow);
        dockWidget_Log->setObjectName(QString::fromUtf8("dockWidget_Log"));
        dockWidgetContents_Log = new QWidget();
        dockWidgetContents_Log->setObjectName(QString::fromUtf8("dockWidgetContents_Log"));
        gridLayout = new QGridLayout(dockWidgetContents_Log);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        textEdit_Log = new QTextEdit(dockWidgetContents_Log);
        textEdit_Log->setObjectName(QString::fromUtf8("textEdit_Log"));
        textEdit_Log->setReadOnly(true);

        gridLayout->addWidget(textEdit_Log, 0, 0, 1, 1);

        dockWidget_Log->setWidget(dockWidgetContents_Log);
        MainWindow->addDockWidget(Qt::BottomDockWidgetArea, dockWidget_Log);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuView->menuAction());
        menubar->addAction(menuTools->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSaveAs);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionRedo);
        menuEdit->addSeparator();
        menuEdit->addAction(actionPreferences);
        menuView->addAction(actionToggleProjectTree);
        menuView->addAction(actionToggleProperties);
        menuView->addAction(actionToggleLog);
        menuView->addSeparator();
        menuView->addAction(actionResetLayout);
        menuTools->addAction(actionRunSimulation);
        menuTools->addAction(actionPostProcessing);
        menuTools->addAction(actionMeshGeneration);
        menuHelp->addAction(actionAbout);
        menuHelp->addAction(actionDocumentation);
        toolBar->addAction(actionNew);
        toolBar->addAction(actionOpen);
        toolBar->addAction(actionSave);
        toolBar->addSeparator();
        toolBar->addAction(actionRunSimulation);
        toolBar->addAction(actionPostProcessing);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(4);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "TubeForm CAE", nullptr));
        actionNew->setText(QCoreApplication::translate("MainWindow", "&New", nullptr));
#if QT_CONFIG(shortcut)
        actionNew->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionOpen->setText(QCoreApplication::translate("MainWindow", "&Open...", nullptr));
#if QT_CONFIG(shortcut)
        actionOpen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave->setText(QCoreApplication::translate("MainWindow", "&Save", nullptr));
#if QT_CONFIG(shortcut)
        actionSave->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSaveAs->setText(QCoreApplication::translate("MainWindow", "Save &As...", nullptr));
        actionExit->setText(QCoreApplication::translate("MainWindow", "E&xit", nullptr));
#if QT_CONFIG(shortcut)
        actionExit->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actionUndo->setText(QCoreApplication::translate("MainWindow", "&Undo", nullptr));
#if QT_CONFIG(shortcut)
        actionUndo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Z", nullptr));
#endif // QT_CONFIG(shortcut)
        actionRedo->setText(QCoreApplication::translate("MainWindow", "&Redo", nullptr));
#if QT_CONFIG(shortcut)
        actionRedo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Y", nullptr));
#endif // QT_CONFIG(shortcut)
        actionPreferences->setText(QCoreApplication::translate("MainWindow", "&Preferences", nullptr));
        actionToggleProjectTree->setText(QCoreApplication::translate("MainWindow", "Toggle &Project Tree", nullptr));
        actionToggleProperties->setText(QCoreApplication::translate("MainWindow", "Toggle &Properties", nullptr));
        actionToggleLog->setText(QCoreApplication::translate("MainWindow", "Toggle &Log", nullptr));
        actionResetLayout->setText(QCoreApplication::translate("MainWindow", "&Reset Layout", nullptr));
        actionRunSimulation->setText(QCoreApplication::translate("MainWindow", "&Run Simulation", nullptr));
#if QT_CONFIG(tooltip)
        actionRunSimulation->setToolTip(QCoreApplication::translate("MainWindow", "Start the simulation process", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionRunSimulation->setShortcut(QCoreApplication::translate("MainWindow", "F5", nullptr));
#endif // QT_CONFIG(shortcut)
        actionPostProcessing->setText(QCoreApplication::translate("MainWindow", "&Post Processing", nullptr));
#if QT_CONFIG(tooltip)
        actionPostProcessing->setToolTip(QCoreApplication::translate("MainWindow", "View and analyze simulation results", nullptr));
#endif // QT_CONFIG(tooltip)
        actionMeshGeneration->setText(QCoreApplication::translate("MainWindow", "&Mesh Generation", nullptr));
#if QT_CONFIG(tooltip)
        actionMeshGeneration->setToolTip(QCoreApplication::translate("MainWindow", "Generate the mesh for the model", nullptr));
#endif // QT_CONFIG(tooltip)
        actionAbout->setText(QCoreApplication::translate("MainWindow", "&About", nullptr));
        actionDocumentation->setText(QCoreApplication::translate("MainWindow", "&Documentation", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "&File", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("MainWindow", "&Edit", nullptr));
        menuView->setTitle(QCoreApplication::translate("MainWindow", "&View", nullptr));
        menuTools->setTitle(QCoreApplication::translate("MainWindow", "&Tools", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "&Help", nullptr));
        toolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "Standard Toolbar", nullptr));
        dockWidget_ProjectTree->setWindowTitle(QCoreApplication::translate("MainWindow", "&Project Tree", nullptr));
        dockWidget_Properties->setWindowTitle(QCoreApplication::translate("MainWindow", "&Properties", nullptr));
        pushButton_initmodel->setText(QCoreApplication::translate("MainWindow", "Original model", nullptr));
        label_27->setText(QCoreApplication::translate("MainWindow", "Process visualization", nullptr));
        label_28->setText(QCoreApplication::translate("MainWindow", "Result visualization", nullptr));
        pushButton_surfacemodel->setText(QCoreApplication::translate("MainWindow", "Surface model", nullptr));
        pushButton_centerlinemodel->setText(QCoreApplication::translate("MainWindow", "Midline model", nullptr));
        pushButton_meshmodel->setText(QCoreApplication::translate("MainWindow", "Grid model", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_5), QCoreApplication::translate("MainWindow", "DV", nullptr));
        label_Tube->setText(QCoreApplication::translate("MainWindow", "Tube", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Outer radius:", nullptr));
        lineEdit_Rout->setText(QCoreApplication::translate("MainWindow", "10", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Inner radius:", nullptr));
        lineEdit_Rin->setText(QCoreApplication::translate("MainWindow", "8", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Length:", nullptr));
        lineEdit_Length->setText(QCoreApplication::translate("MainWindow", "200", nullptr));
        label_Tube_2->setText(QCoreApplication::translate("MainWindow", "Rotating sleeve", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Thickness:", nullptr));
        lineEdit_SleeveThickness1->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "Length:", nullptr));
        lineEdit_SleeveLength1->setText(QCoreApplication::translate("MainWindow", "20", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "Position:", nullptr));
        lineEdit_RotaryPos->setText(QCoreApplication::translate("MainWindow", "30", nullptr));
        label_Tube_3->setText(QCoreApplication::translate("MainWindow", "Fixed sleeve", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "Thickness:", nullptr));
        lineEdit_SleeveThickness->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "Length:", nullptr));
        lineEdit_SleeveLength->setText(QCoreApplication::translate("MainWindow", "50", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "Position:", nullptr));
        lineEdit_FixedPos->setText(QCoreApplication::translate("MainWindow", "150", nullptr));
        label_Tube_4->setText(QCoreApplication::translate("MainWindow", "Semi-circular sleeve", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow", "Radius:", nullptr));
        lineEdit_ArcR->setText(QCoreApplication::translate("MainWindow", "50", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow", "Thickness:", nullptr));
        lineEdit_ArcThickness->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
        label_12->setText(QCoreApplication::translate("MainWindow", "Radian:", nullptr));
        lineEdit_ArcAngle->setText(QCoreApplication::translate("MainWindow", "1.57", nullptr));
        pushButton_confirm_CADmodel->setText(QCoreApplication::translate("MainWindow", "Apply", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("MainWindow", "PM", nullptr));
        label_25->setText(QCoreApplication::translate("MainWindow", "Grid size:", nullptr));
        pushButton_Mesh1->setText(QCoreApplication::translate("MainWindow", "Apply", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("MainWindow", "MG", nullptr));
        label_26->setText(QCoreApplication::translate("MainWindow", "Click the model to extract its surface!", nullptr));
        pushButton_extractFace->setText(QCoreApplication::translate("MainWindow", "Save Surface", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QCoreApplication::translate("MainWindow", "ES", nullptr));
        label_29->setText(QCoreApplication::translate("MainWindow", "Bending parameters", nullptr));
        pushButton_ExtractCenterLine->setText(QCoreApplication::translate("MainWindow", "Apply", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QCoreApplication::translate("MainWindow", "EC", nullptr));
        dockWidget_Features->setWindowTitle(QCoreApplication::translate("MainWindow", "&Features", nullptr));

        const bool __sortingEnabled = listWidget_Features->isSortingEnabled();
        listWidget_Features->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = listWidget_Features->item(0);
        ___qlistwidgetitem->setText(QCoreApplication::translate("MainWindow", "Parametric  Modeling", nullptr));
        QListWidgetItem *___qlistwidgetitem1 = listWidget_Features->item(1);
        ___qlistwidgetitem1->setText(QCoreApplication::translate("MainWindow", "Mesh Generation", nullptr));
        QListWidgetItem *___qlistwidgetitem2 = listWidget_Features->item(2);
        ___qlistwidgetitem2->setText(QCoreApplication::translate("MainWindow", "Extract Surface", nullptr));
        QListWidgetItem *___qlistwidgetitem3 = listWidget_Features->item(3);
        ___qlistwidgetitem3->setText(QCoreApplication::translate("MainWindow", "Extract Centerline", nullptr));
        QListWidgetItem *___qlistwidgetitem4 = listWidget_Features->item(4);
        ___qlistwidgetitem4->setText(QCoreApplication::translate("MainWindow", "Define Material", nullptr));
        QListWidgetItem *___qlistwidgetitem5 = listWidget_Features->item(5);
        ___qlistwidgetitem5->setText(QCoreApplication::translate("MainWindow", "Generate Mesh", nullptr));
        listWidget_Features->setSortingEnabled(__sortingEnabled);

        dockWidget_Log->setWindowTitle(QCoreApplication::translate("MainWindow", "&Log", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
