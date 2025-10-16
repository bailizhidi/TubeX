#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUI();
    setupConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // --- 初始化项目树 (QTreeView) 使用 QStandardItemModel ---
    // 创建空模型，不添加任何初始项目
    m_treeModel = new QStandardItemModel(this);
    ui->treeView_Project->setModel(m_treeModel);

    // 初始化文件系统监视器
    m_fileSystemWatcher = new QFileSystemWatcher(this);
    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged,
            this, &MainWindow::onDirectoryChanged);
    connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged,
            this, &MainWindow::onFileChanged);

    // 初始化防抖定时器
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(500); // 500ms 防抖延迟
    connect(m_debounceTimer, &QTimer::timeout, this, &MainWindow::updateProjectTree);

    // 设置表头标签
    //m_treeModel->setHorizontalHeaderLabels(QStringList() << "Project Tree");

    // --- 初始化属性表 ---
    // ui->tableWidget_Properties->setColumnCount(2);
    // ui->tableWidget_Properties->setHorizontalHeaderLabels(QStringList() << "Property" << "Value");
    // ui->tableWidget_Properties->horizontalHeader()->setStretchLastSection(true);

    // --- 初始化日志 ---
    logMessage("TubeForm CAE started.");
    logMessage("Ready for new project or file open.");

    // --- 初始化状态栏 ---
    QLabel *unitLabel = new QLabel("Units: mm");
    ui->statusbar->addPermanentWidget(unitLabel);

    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    ui->statusbar->addPermanentWidget(m_progressBar);

    // --- 初始化模拟进度定时器 ---
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &MainWindow::updateSimulationProgress);
}

void MainWindow::setupConnections()
{
    // 连接树视图点击信号
    connect(ui->treeView_Project, &QTreeView::clicked,this, &MainWindow::on_treeView_Project_clicked);

    // 连接参数化建模点击信号
    connect(ui->pushButton_confirm_CADmodel, &QPushButton::clicked, this, &MainWindow::on_CADmodel_clicked);

    // 连接网格划分点击信号
    connect(ui->pushButton_Mesh1, &QPushButton::clicked,this, &MainWindow::on_meshButton_clicked);

    // 连接提取外壁点击信号
    connect(ui->pushButton_extractFace, &QPushButton::clicked, this, &MainWindow::extractFace);

    // 连接提取中心线点击信号
    connect(ui->pushButton_ExtractCenterLine, &QPushButton::clicked, this, &MainWindow::on_extractCenterline_clicked);

    //过程可视化
    connect(ui->pushButton_initmodel, &QPushButton::clicked,this, &MainWindow::init_model);
    connect(ui->pushButton_meshmodel, &QPushButton::clicked,this, &MainWindow::mesh_model);
    connect(ui->pushButton_surfacemodel, &QPushButton::clicked,this, &MainWindow::surface_model);
    connect(ui->pushButton_centerlinemodel, &QPushButton::clicked,this, &MainWindow::centerline_model);
}

void MainWindow::logMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    ui->textEdit_Log->append(QString("[%1] %2").arg(timestamp).arg(message));
}

void MainWindow::updateStatusBar(const QString &message, int timeout)
{
    ui->statusbar->showMessage(message, timeout);
}

// ==========1.创建CAD模型==========
// VTK头文件
#include <vtkLight.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>

// OpenCASCADE头文件
// 抑制弃用警告
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <TopoDS_Face.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_Triangle.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax1.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
// 恢复警告设置
#pragma GCC diagnostic pop

// 文件系统生成
void MainWindow::createNewProject()
{
    // 弹出对话框让用户输入项目名称
    bool ok;
    QString projectName = QInputDialog::getText(this,
                                                "New Project",
                                                "Enter project name:",
                                                QLineEdit::Normal,
                                                "My Tube Project",
                                                &ok);

    if (!ok || projectName.isEmpty()) {
        // 用户取消了操作或输入了空名称
        logMessage("New project creation cancelled.");
        updateStatusBar("New project creation cancelled.");
        return;
    }

    // 弹出文件对话框让用户选择保存位置
    QString projectPath = QFileDialog::getExistingDirectory(this,
                                                            "Select Project Directory",
                                                            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (projectPath.isEmpty()) {
        // 用户取消了操作
        logMessage("Project directory selection cancelled.");
        updateStatusBar("Project directory selection cancelled.");
        return;
    }

    // 创建项目目录
    QDir projectDir(projectPath);
    QString fullProjectPath = projectDir.filePath(projectName);

    // 检查项目目录是否已存在
    if (projectDir.exists(fullProjectPath)) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                                  "Directory Exists",
                                                                  QString("Project directory '%1' already exists. Do you want to continue?").arg(fullProjectPath),
                                                                  QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            logMessage("User chose not to overwrite existing project directory.");
            updateStatusBar("Project creation cancelled.");
            return;
        }
    } else {
        // 创建项目目录
        if (!projectDir.mkpath(fullProjectPath)) {
            QMessageBox::critical(this,
                                  "Error",
                                  QString("Failed to create project directory: %1").arg(fullProjectPath));
            logMessage(QString("Failed to create project directory: %1").arg(fullProjectPath));
            updateStatusBar("Project creation failed.");
            return;
        }
    }

    // 清空模型数据
    m_treeModel->clear();
    m_treeModel->setHorizontalHeaderLabels(QStringList() << "Project Tree");

    // 添加根节点，显示项目名称和路径
    QStandardItem *rootItem = new QStandardItem(QString("%1 (%2)").arg(projectName, fullProjectPath));
    rootItem->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    m_treeModel->appendRow(rootItem);

    // 开始监视项目目录
    m_fileSystemWatcher->addPath(fullProjectPath);

    // 初始填充项目树
    populateProjectTree(rootItem, fullProjectPath);

    // 展开根节点
    ui->treeView_Project->expand(m_treeModel->indexFromItem(rootItem));

    // 记录项目信息
    m_currentProjectPath = fullProjectPath;
    m_currentProjectName = projectName;

    logMessage(QString("New project created: %1 at %2").arg(projectName, fullProjectPath));
    updateStatusBar(QString("Project '%1' created at %2").arg(projectName, QFileInfo(fullProjectPath).fileName()));
}

void MainWindow::populateProjectTree(QStandardItem *parentItem, const QString &path)
{
    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);

    for (const QFileInfo &entry : entries) {
        QStandardItem *item = new QStandardItem(entry.fileName());

        if (entry.isDir()) {
            item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
            parentItem->appendRow(item);

            // 递归填充子目录
            populateProjectTree(item, entry.absoluteFilePath());

            // 添加目录到监视器
            m_fileSystemWatcher->addPath(entry.absoluteFilePath());
        } else {
            item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
            parentItem->appendRow(item);
        }
    }
    logMessage(QString("Project tree updated."));
}

void MainWindow::onDirectoryChanged(const QString &path)
{
    // 防抖处理，避免频繁更新
    m_debounceTimer->start();
}

void MainWindow::onFileChanged(const QString &path)
{
    // 文件变化也触发更新
    m_debounceTimer->start();
}

void MainWindow::updateProjectTree()
{
    if (m_currentProjectPath.isEmpty()) {
        return;
    }

    // 获取当前展开状态
    QModelIndex rootIndex = m_treeModel->index(0, 0);
    QList<QPersistentModelIndex> expandedIndexes;

    // 递归收集所有展开的索引
    std::function<void(const QModelIndex&)> collectExpanded = [&](const QModelIndex &index) {
        if (ui->treeView_Project->isExpanded(index)) {
            expandedIndexes.append(QPersistentModelIndex(index));
        }
        for (int i = 0; i < m_treeModel->rowCount(index); ++i) {
            collectExpanded(m_treeModel->index(i, 0, index));
        }
    };
    collectExpanded(rootIndex);

    // 重新构建整个项目树
    m_treeModel->clear();
    m_treeModel->setHorizontalHeaderLabels(QStringList() << "Project Tree");

    QStandardItem *rootItem = new QStandardItem(QString("%1 (%2)").arg(m_currentProjectName, m_currentProjectPath));
    rootItem->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    m_treeModel->appendRow(rootItem);

    // 重建监视器
    QStringList paths = m_fileSystemWatcher->directories();
    for (const QString &path : paths) {
        m_fileSystemWatcher->removePath(path);
    }
    m_fileSystemWatcher->addPath(m_currentProjectPath);

    // 填充项目树
    populateProjectTree(rootItem, m_currentProjectPath);

    // 恢复之前的展开状态
    std::function<void(const QModelIndex&)> restoreExpanded = [&](const QModelIndex &index) {
        if (expandedIndexes.contains(QPersistentModelIndex(index))) {
            ui->treeView_Project->expand(index);
        }
        for (int i = 0; i < m_treeModel->rowCount(index); ++i) {
            restoreExpanded(m_treeModel->index(i, 0, index));
        }
    };
    restoreExpanded(rootIndex);

    // 展开根节点
    ui->treeView_Project->expand(m_treeModel->indexFromItem(rootItem));
}

// 参数化建模
void MainWindow::on_CADmodel_clicked() {
    bool ok[12];
    double tube_outer_radius        = ui->lineEdit_Rout->text().toDouble(&ok[0]);
    double tube_inner_radius         = ui->lineEdit_Rin->text().toDouble(&ok[1]);
    double tube_length       = ui->lineEdit_Length->text().toDouble(&ok[2]);
    double rotary_sleeve_thickness     = ui->lineEdit_SleeveThickness1->text().toDouble(&ok[3]);
    double rotary_sleeve_length     = ui->lineEdit_SleeveLength1->text().toDouble(&ok[4]);
    double fixed_sleeve_thickness     = ui->lineEdit_SleeveThickness->text().toDouble(&ok[5]);
    double fixed_sleeve_length     = ui->lineEdit_SleeveLength->text().toDouble(&ok[6]);
    double rotary_sleeve_pos   = ui->lineEdit_RotaryPos->text().toDouble(&ok[7]);
    double fixed_sleeve_pos    = ui->lineEdit_FixedPos->text().toDouble(&ok[8]);
    double arc_radius        = ui->lineEdit_ArcR->text().toDouble(&ok[9]);
    double arc_thickness        = ui->lineEdit_ArcThickness->text().toDouble(&ok[10]);
    double arc_angle_rad    = ui->lineEdit_ArcAngle->text().toDouble(&ok[11]);

    MakeElbowModel(
        tube_outer_radius, tube_inner_radius, tube_length,
        rotary_sleeve_thickness, rotary_sleeve_length, rotary_sleeve_pos,
        fixed_sleeve_thickness, fixed_sleeve_length, fixed_sleeve_pos,
        arc_radius, arc_thickness, arc_angle_rad
        );
}

void MainWindow::MakeElbowModel(
    double tube_outer_radius, double tube_inner_radius, double tube_length,
    double rotary_sleeve_thickness, double rotary_sleeve_length, double rotary_sleeve_pos,
    double fixed_sleeve_thickness, double fixed_sleeve_length, double fixed_sleeve_pos,
    double arc_radius, double arc_thickness, double arc_angle_rad) {

    const double tolerance = 1E-3;
    const double mesh_precision = 1.0;

    try {
        // 检查目标区域是否存在
        if (!ui->mdiArea) {
            QMessageBox::warning(this, "警告", "显示区域未找到！");
            return;
        }

        // 清理区域中的现有内容
        QLayout* layout = ui->mdiArea->layout();
        if (layout) {
            QLayoutItem* item;
            while ((item = layout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->setParent(nullptr);
                }
                delete item;
            }
        } else {
            // 如果没有布局，创建新的布局
            layout = new QVBoxLayout(ui->mdiArea);
            ui->mdiArea->setLayout(layout);
        }

        // Tube (管体) 参数
        struct tube_param {
            double R_out, R_in, length;
        } tp = {tube_outer_radius, tube_inner_radius, tube_length};

        // Rotary Sleeve (旋转套) 参数
        struct rotary_sleeve_param {
            double thickness, length, position;
            double R_out, R_in, ref[3], rot[3];
        } rsp = {rotary_sleeve_thickness, rotary_sleeve_length, rotary_sleeve_pos, 0.0, 0.0, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};

        // Fixed Sleeve (固定套) 参数
        struct fixed_sleeve_param {
            double thickness, length, position;
            double R_out, R_in, ref[3], rot[3];
        } fsp = {fixed_sleeve_thickness, fixed_sleeve_length, fixed_sleeve_pos, 0.0, 0.0, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};

        // Semi-circular Sleeve (半圆弧套) 参数
        struct semi_circular_sleeve_param {
            double R, thickness, angle, position, R_out, R_in, ref[3], rot[3];
        } scsp = {arc_radius, arc_thickness, arc_angle_rad, 0.0, 0.0, 0.0, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};

        rsp.position = tp.length - rsp.position;
        rsp.R_in     = tp.R_out + tolerance;
        rsp.R_out    = rsp.R_in + rsp.thickness;

        rsp.ref[0] = rsp.position + rsp.length/2;

        fsp.position = tp.length - fsp.position;
        fsp.R_in     = tp.R_out + tolerance;
        fsp.R_out    = rsp.R_in + rsp.thickness;

        // ref node
        fsp.ref[0] = fsp.position + fsp.length / 2;
        fsp.rot[0] = fsp.position + fsp.length / 2;

        // generate arc
        scsp.position = rsp.position - tolerance;
        scsp.R_in     = tp.R_out + tolerance;
        scsp.R_out    = scsp.R_in + scsp.thickness;

        // rot node
        scsp.rot[0] = scsp.position;
        scsp.rot[1] = -scsp.R;
        std::copy(scsp.rot, scsp.rot+3, scsp.ref);
        std::copy(scsp.rot, scsp.rot+3, rsp.rot);

        // ========== 写入 rigidbody.info ==========
        // 使用当前项目路径
        if (m_currentProjectPath.isEmpty()) {
            QMessageBox::warning(this, "警告", "当前项目路径未设置！请先创建或打开一个项目。");
            return;
        }

        QString filename = m_currentProjectPath + "/Profile/rigidbody.info";

        // 确保 Profile 目录存在
        QFileInfo fileInfo(filename);
        QDir dir;
        if (!dir.exists(fileInfo.absolutePath())) {
            if (!dir.mkpath(fileInfo.absolutePath())) {
                QMessageBox::warning(this, "警告", "无法创建 Profile 目录！");
                return;
            }
        }

        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "---\n";
            out << "Volume2:\n";

            out << "  ref: (" << scsp.ref[0] << "," << scsp.ref[1] << "," << scsp.ref[2] << ")\n";
            out << "  rot: (" << scsp.rot[0] << "," << scsp.rot[1] << "," << scsp.rot[2] << ")\n";

            out << "Volume3:\n";
            out << "  ref: (" << rsp.ref[0] << "," << rsp.ref[1] << "," << rsp.ref[2] << ")\n";
            out << "  rot: (" << rsp.rot[0] << "," << rsp.rot[1] << "," << rsp.rot[2] << ")\n";

            out << "Volume4:\n";
            out << "  ref: (" << fsp.ref[0] << "," << fsp.ref[1] << "," << fsp.ref[2] << ")\n";
            out << "  rot: (" << fsp.rot[0] << "," << fsp.rot[1] << "," << fsp.rot[2] << ")\n";
            file.close();
            qDebug() << "参考点写入 rigidbody.info 成功。";
        } else {
            qDebug() << "无法写入 rigidbody.info 文件！";
        }

        // 创建VTK渲染部件
        QVTKOpenGLNativeWidget *vtkWidget = new QVTKOpenGLNativeWidget(ui->mdiArea);
        vtkWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // 创建渲染器
        vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
        renderer->SetBackground(1,1,1); // 背景

        // 创建渲染窗口
        vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow =
            vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        vtkWidget->setRenderWindow(renderWindow);
        renderWindow->AddRenderer(renderer);

        logMessage(QString("Start creating the bend tube model."));

        // 定义统一的颜色（金属灰色）
        double metal_gray_r = 0.7;
        double metal_gray_g = 0.7;
        double metal_gray_b = 0.75;

        // 1. 创建管体
        logMessage(QString("Create tube."));
        gp_Ax2 tube_axis(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
        TopoDS_Shape outer_cylinder = BRepPrimAPI_MakeCylinder(tube_axis, tube_outer_radius, tube_length);
        TopoDS_Shape inner_cylinder = BRepPrimAPI_MakeCylinder(tube_axis, tube_inner_radius, tube_length);
        TopoDS_Shape tube = BRepAlgoAPI_Cut(outer_cylinder, inner_cylinder);

        // 保存管体到项目路径
        QString tubeFile = m_currentProjectPath + "/Models/tube.step";
        QDir modelsDir;
        if (!modelsDir.exists(QFileInfo(tubeFile).absolutePath())) {
            modelsDir.mkpath(QFileInfo(tubeFile).absolutePath());
        }

        // 使用STEP格式保存
        STEPControl_Writer step_writer;
        step_writer.Transfer(tube, STEPControl_AsIs);
        step_writer.Write(tubeFile.toStdString().c_str());

        vtkSmartPointer<vtkPolyData> tubePolyData = ConvertOCCShapeToVTKPolyData(tube, mesh_precision);
        vtkSmartPointer<vtkActor> tubeActor = CreateVTKActor(tubePolyData, metal_gray_r, metal_gray_g, metal_gray_b);
        renderer->AddActor(tubeActor);

        // 2. 创建旋转套筒
        logMessage(QString("Create a rotating sleeve."));
        double rotary_pos_from_left = tube_length - rotary_sleeve_pos;
        double rotary_inner_radius = tube_outer_radius + tolerance;
        double rotary_outer_radius = rotary_inner_radius + rotary_sleeve_thickness;

        gp_Ax2 rotary_axis(gp_Pnt(rotary_pos_from_left, 0, 0), gp_Dir(1, 0, 0));
        TopoDS_Shape rotary_inner = BRepPrimAPI_MakeCylinder(rotary_axis, rotary_inner_radius, rotary_sleeve_length);
        TopoDS_Shape rotary_outer = BRepPrimAPI_MakeCylinder(rotary_axis, rotary_outer_radius, rotary_sleeve_length);
        TopoDS_Shape rotary_sleeve = BRepAlgoAPI_Cut(rotary_outer, rotary_inner);

        // 保存旋转套筒到项目路径
        QString rotaryFile = m_currentProjectPath + "/Models/rotary_sleeve.step";
        STEPControl_Writer rotary_step_writer;
        rotary_step_writer.Transfer(rotary_sleeve, STEPControl_AsIs);
        rotary_step_writer.Write(rotaryFile.toStdString().c_str());

        vtkSmartPointer<vtkPolyData> rotaryPolyData = ConvertOCCShapeToVTKPolyData(rotary_sleeve, mesh_precision);
        vtkSmartPointer<vtkActor> rotaryActor = CreateVTKActor(rotaryPolyData, metal_gray_r, metal_gray_g, metal_gray_b);
        renderer->AddActor(rotaryActor);

        // 3. 创建固定套筒
        logMessage(QString("Create a fixed sleeve."));
        double fixed_pos_from_left = tube_length - fixed_sleeve_pos;
        double fixed_inner_radius = tube_outer_radius + tolerance;
        double fixed_outer_radius = fixed_inner_radius + fixed_sleeve_thickness;

        gp_Ax2 fixed_axis(gp_Pnt(fixed_pos_from_left, 0, 0), gp_Dir(1, 0, 0));
        TopoDS_Shape fixed_inner = BRepPrimAPI_MakeCylinder(fixed_axis, fixed_inner_radius, fixed_sleeve_length);
        TopoDS_Shape fixed_outer = BRepPrimAPI_MakeCylinder(fixed_axis, fixed_outer_radius, fixed_sleeve_length);
        TopoDS_Shape fixed_sleeve = BRepAlgoAPI_Cut(fixed_outer, fixed_inner);

        // 保存固定套筒到项目路径
        QString fixedFile = m_currentProjectPath + "/Models/fixed_sleeve.step";
        STEPControl_Writer fixed_step_writer;
        fixed_step_writer.Transfer(fixed_sleeve, STEPControl_AsIs);
        fixed_step_writer.Write(fixedFile.toStdString().c_str());

        vtkSmartPointer<vtkPolyData> fixedPolyData = ConvertOCCShapeToVTKPolyData(fixed_sleeve, mesh_precision);
        vtkSmartPointer<vtkActor> fixedActor = CreateVTKActor(fixedPolyData, metal_gray_r, metal_gray_g, metal_gray_b);
        renderer->AddActor(fixedActor);

        // 4. 创建圆弧段
        logMessage(QString("Create an arc segment."));
        double arc_position = rotary_pos_from_left - tolerance;
        double arc_inner_radius = tube_outer_radius + tolerance;
        double arc_outer_radius = arc_inner_radius + arc_thickness;

        // 创建圆弧截面
        Handle(Geom_TrimmedCurve) inner_arc = GC_MakeArcOfCircle(
            gp_Pnt(arc_position, 0, -arc_inner_radius),
            gp_Pnt(arc_position, -arc_inner_radius, 0),
            gp_Pnt(arc_position, 0, arc_inner_radius)
            );

        Handle(Geom_TrimmedCurve) outer_arc = GC_MakeArcOfCircle(
            gp_Pnt(arc_position, 0, -arc_outer_radius),
            gp_Pnt(arc_position, -arc_outer_radius, 0),
            gp_Pnt(arc_position, 0, arc_outer_radius)
            );

        // 创建连接边
        TopoDS_Edge left_edge = BRepBuilderAPI_MakeEdge(
            gp_Pnt(arc_position, 0, -arc_inner_radius),
            gp_Pnt(arc_position, 0, -arc_outer_radius)
            );

        TopoDS_Edge right_edge = BRepBuilderAPI_MakeEdge(
            gp_Pnt(arc_position, 0, arc_inner_radius),
            gp_Pnt(arc_position, 0, arc_outer_radius)
            );

        // 创建截面线框
        BRepBuilderAPI_MakeWire wire_builder;
        wire_builder.Add(BRepBuilderAPI_MakeEdge(outer_arc));
        wire_builder.Add(right_edge);
        wire_builder.Add(BRepBuilderAPI_MakeEdge(inner_arc));
        wire_builder.Add(left_edge);
        TopoDS_Wire section_wire = wire_builder.Wire();

        // 创建截面面
        TopoDS_Face section_face = BRepBuilderAPI_MakeFace(section_wire);

        // 旋转生成圆弧段
        gp_Ax1 rotation_axis(
            gp_Pnt(arc_position, -arc_radius, 0),
            gp_Dir(0, 0, 1)
            );
        TopoDS_Shape arc_sector = BRepPrimAPI_MakeRevol(section_face, rotation_axis, arc_angle_rad);

        // 保存圆弧段到项目路径
        QString arcFile = m_currentProjectPath + "/Models/arc_segment.step";
        STEPControl_Writer arc_step_writer;
        arc_step_writer.Transfer(arc_sector, STEPControl_AsIs);
        arc_step_writer.Write(arcFile.toStdString().c_str());

        vtkSmartPointer<vtkPolyData> arcPolyData = ConvertOCCShapeToVTKPolyData(arc_sector, mesh_precision * 0.5);
        vtkSmartPointer<vtkActor> arcActor = CreateVTKActor(arcPolyData, metal_gray_r, metal_gray_g, metal_gray_b);
        renderer->AddActor(arcActor);

        // 5. 写入参考点信息

        // 6. 设置相机并渲染
        renderer->ResetCamera();

        // 7. 将VTK部件添加到布局中
        layout->addWidget(vtkWidget);

        // 8. 强制刷新布局和渲染
        ui->mdiArea->layout()->activate();
        renderWindow->Render();

        // 9. 添加交互器
        vtkSmartPointer<vtkRenderWindowInteractor> interactor = renderWindow->GetInteractor();
        vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
            vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
        interactor->SetInteractorStyle(style);

        logMessage(QString("The creation of the bend tube model is complete."));
        logMessage(QString("The reference point has been successfully written into the rigidbody.info file."));
    } catch (const std::exception& e) {
        qDebug() << "错误:" << e.what();
        QMessageBox::critical(this, "错误", QString("模型创建失败: %1").arg(e.what()));
    }
}

// 将OCC形状转换为VTK PolyData
vtkSmartPointer<vtkPolyData> MainWindow::ConvertOCCShapeToVTKPolyData(const TopoDS_Shape& shape, double linearDeflection = 0.5) {
    // 生成三角网格
    BRepMesh_IncrementalMesh mesh(shape, linearDeflection, false, 0.1, true);
    mesh.Perform();

    if (!mesh.IsDone()) {
        throw std::runtime_error("网格生成失败！");
    }

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
    int pointIdOffset = 0;

    // 遍历所有面并提取三角网格
    for (TopExp_Explorer faceExp(shape, TopAbs_FACE); faceExp.More(); faceExp.Next()) {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());
        TopLoc_Location loc;
        Handle(Poly_Triangulation) triFace = BRep_Tool::Triangulation(face, loc);

        if (!triFace.IsNull()) {
            // 添加顶点
            for (Standard_Integer i = 1; i <= triFace->NbNodes(); i++) {
                gp_Pnt p = triFace->Node(i);
                if (!loc.IsIdentity()) {
                    p.Transform(loc.Transformation());
                }
                points->InsertNextPoint(p.X(), p.Y(), p.Z());
            }

            // 添加三角形
            for (Standard_Integer i = 1; i <= triFace->NbTriangles(); i++) {
                Poly_Triangle tri = triFace->Triangle(i);
                Standard_Integer n1, n2, n3;
                tri.Get(n1, n2, n3);

                vtkIdType ids[3] = {
                    n1 - 1 + pointIdOffset,
                    n2 - 1 + pointIdOffset,
                    n3 - 1 + pointIdOffset
                };
                triangles->InsertNextCell(3, ids);
            }

            pointIdOffset += triFace->NbNodes();
        }
    }

    // 构建VTK PolyData
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(triangles);

    return polyData;
}

// 移除网格显示，使用实体颜色
vtkSmartPointer<vtkActor> MainWindow::CreateVTKActor(vtkSmartPointer<vtkPolyData> polyData,
                                                     double r, double g, double b) {
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // 设置实体颜色
    actor->GetProperty()->SetColor(r, g, b);
    actor->GetProperty()->SetOpacity(1.0); // 完全不透明

    // 关闭网格显示
    actor->GetProperty()->EdgeVisibilityOff();

    // 设置光照和材质属性，使实体看起来更自然
    actor->GetProperty()->LightingOn();
    actor->GetProperty()->SetInterpolationToPhong();
    actor->GetProperty()->SetSpecular(0.4);     // 增加高光强度
    actor->GetProperty()->SetSpecularPower(30); // 高光范围
    actor->GetProperty()->SetAmbient(0.2);      // 环境光
    actor->GetProperty()->SetDiffuse(0.8);      // 漫反射

    return actor;
}

//显示函数
void MainWindow::ShowModelInMdiArea(vtkSmartPointer<vtkRenderer> renderer) {
    try {
        // 清空MDI区域的所有子窗口
        ui->mdiArea->closeAllSubWindows();

        // 创建VTK部件
        QVTKOpenGLNativeWidget *vtkWidget = new QVTKOpenGLNativeWidget();
        vtkWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // 创建渲染窗口
        vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow =
            vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        vtkWidget->setRenderWindow(renderWindow);
        renderWindow->AddRenderer(renderer);

        // 创建全屏子窗口（无边框）
        QMdiSubWindow *subWindow = new QMdiSubWindow(ui->mdiArea);
        subWindow->setWidget(vtkWidget);
        subWindow->setWindowFlags(Qt::FramelessWindowHint); // 无边框
        subWindow->setAttribute(Qt::WA_DeleteOnClose);

        // 添加到MDI区域并最大化
        ui->mdiArea->addSubWindow(subWindow);
        subWindow->showMaximized();

        // 渲染
        renderWindow->Render();

        // 设置交互器
        vtkSmartPointer<vtkRenderWindowInteractor> interactor = renderWindow->GetInteractor();
        vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
            vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
        interactor->SetInteractorStyle(style);

        qDebug() << "模型已全屏显示在MDI区域中";

    } catch (const std::exception& e) {
        qDebug() << "显示模型时出错:" << e.what();
        QMessageBox::critical(this, "错误", QString("显示模型失败: %1").arg(e.what()));
    }
}

// ==========2.数模显示==========

// 新建数模文件，与参数化建模匹配
void MainWindow::on_actionNew_triggered()
{
    logMessage("New project action triggered.");
    QMessageBox::StandardButton reply = QMessageBox::question(this, "New Project", "Start a new project? Any unsaved changes will be lost.", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        createNewProject();
        ui->tabWidget->setCurrentIndex(1);
        logMessage("Switched to Parametric Modeling tab after creating new project.");
    }
}

// 打开数模文件，支持stp和igs格式
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <TopoDS_Shape.hxx>
#include <BRepTools.hxx>
#include <STEPControl_Reader.hxx>
#include <Interface_Static.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <IGESControl_Reader.hxx>
#include <vtkCallbackCommand.h>
#pragma GCC diagnostic pop

//读取STP文件
TopoDS_Shape MainWindow::ReadSTEPFile(const QString& fileName)
{
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(fileName.toStdString().c_str());

    if (status != IFSelect_RetDone) {
        qWarning() << "读取STEP文件失败:" << fileName;
        return TopoDS_Shape();
    }

    // 将所有可转换的形状加载到模型
    reader.TransferRoots();
    TopoDS_Shape shape = reader.OneShape();

    if (shape.IsNull()) {
        qWarning() << "转换失败：文件中无有效几何体";
        return TopoDS_Shape();
    }

    return shape;
}

//读取IGS文件
TopoDS_Shape MainWindow::ReadIGESFile(const QString& fileName)
{
    IGESControl_Reader reader;

    IFSelect_ReturnStatus status = reader.ReadFile(fileName.toStdString().c_str());

    if (status != IFSelect_RetDone) {
        qWarning() << "读取IGES文件失败:" << fileName;
        return TopoDS_Shape();
    }

    // 设置精度模式
    Interface_Static::SetCVal("read.precision.mode", "1"); // 启用精度设置
    Interface_Static::SetRVal("read.precision.val", 1.0e-6);

    // 传输所有根实体
    reader.TransferRoots();

    // 获取合并后的整体形状
    TopoDS_Shape shape = reader.OneShape();

    if (shape.IsNull()) {
        qWarning() << "转换失败：IGES文件中无有效几何体";
        return TopoDS_Shape();
    }

    return shape;
}

//显示逻辑
void MainWindow::DisplayShape(const TopoDS_Shape& shape)
{
    try {
        if (shape.IsNull()) {
            QMessageBox::warning(this, "警告", "无效的几何体，无法显示！");
            return;
        }

        // 清理 ui->mdiArea 中的旧内容
        QLayout* layout = ui->mdiArea->layout();
        if (layout) {
            QLayoutItem* item;
            while ((item = layout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->setParent(nullptr);
                }
                delete item;
            }
        } else {
            layout = new QVBoxLayout(ui->mdiArea);
            ui->mdiArea->setLayout(layout);
        }

        // 创建 VTK 渲染部件
        QVTKOpenGLNativeWidget *vtkWidget = new QVTKOpenGLNativeWidget(ui->mdiArea);
        vtkWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // 创建渲染器
        vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
        renderer->SetBackground(1, 1, 1); // 白色背景

        // 创建渲染窗口
        vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow =
            vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        vtkWidget->setRenderWindow(renderWindow);
        renderWindow->AddRenderer(renderer);

        // 转换 OCC Shape 为 VTK PolyData
        BRepMesh_IncrementalMesh mesh(shape, 0.01);
        mesh.Perform();

        if (!mesh.IsDone()) {
            throw std::runtime_error("网格生成失败！");
        }

        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
        int pointIdOffset = 0;

        // 清空旧的映射
        m_faceMap.clear();

        // 准备建立新映射
        std::vector<TopoDS_Face> faceOrderList; // 按顺序存储所有 Face
        TopExp_Explorer tempExp(shape, TopAbs_FACE);
        for (; tempExp.More(); tempExp.Next()) {
            faceOrderList.push_back(TopoDS::Face(tempExp.Current()));
        }

        int currentVtkCellId = 0; // VTK 中 Cell 的全局 ID

        // 修改后的循环（带映射）
        int faceIndex = 0;
        for (const auto& face : faceOrderList) {
            TopLoc_Location loc;
            Handle(Poly_Triangulation) triFace = BRep_Tool::Triangulation(face, loc);

            if (!triFace.IsNull()) {
                int numNodes = triFace->NbNodes();
                int numTriangles = triFace->NbTriangles();

                // 添加顶点
                for (int i = 1; i <= numNodes; i++) {
                    gp_Pnt p = triFace->Node(i);
                    if (!loc.IsIdentity()) {
                        p.Transform(loc.Transformation());
                    }
                    points->InsertNextPoint(p.X(), p.Y(), p.Z());
                }

                // 添加三角形 & 建立映射
                for (int i = 1; i <= numTriangles; i++) {
                    Poly_Triangle tri = triFace->Triangle(i);
                    Standard_Integer n1, n2, n3;
                    tri.Get(n1, n2, n3);

                    vtkIdType ids[3] = {n1-1+pointIdOffset, n2-1+pointIdOffset, n3-1+pointIdOffset};
                    vtkIdType cellId = triangles->InsertNextCell(3, ids);

                    // 记录映射关系
                    m_faceMap[currentVtkCellId + i - 1] = face;
                }

                pointIdOffset += numNodes;
                currentVtkCellId += numTriangles; // 更新 Cell ID
            }
            faceIndex++;
        }

        // 创建 PolyData
        vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
        polyData->SetPoints(points);
        polyData->SetPolys(triangles);

        // 创建 Actor
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(polyData);

        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(0.8, 0.8, 0.8);     // 灰色
        actor->GetProperty()->SetOpacity(1.0);
        actor->GetProperty()->EdgeVisibilityOff();
        actor->GetProperty()->SetInterpolationToPhong();

        // 添加到渲染器
        renderer->AddActor(actor);
        renderer->ResetCamera();

        // 添加到布局
        layout->addWidget(vtkWidget);
        renderWindow->Render();

        // 设置交互器
        vtkSmartPointer<vtkRenderWindowInteractor> interactor = renderWindow->GetInteractor();
        vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
            vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
        interactor->SetInteractorStyle(style);

        // 创建回调命令对象
        vtkSmartPointer<vtkCallbackCommand> clickCallback =
            vtkSmartPointer<vtkCallbackCommand>::New();
        clickCallback->SetCallback(OnLeftButtonDown);
        clickCallback->SetClientData(this); // 传递 this 指针

        // 添加观察者
        interactor->AddObserver(vtkCommand::LeftButtonPressEvent, clickCallback);

        //qDebug() << "模型已成功显示。";
        //qDebug() << "模型类型：" << m_currentShape.ShapeType();

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString("Display model failure: %1").arg(e.what()));
    }
}

//打开数模文件并显示
void MainWindow::on_actionOpen_triggered()
{
    logMessage("Open project action triggered.");
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开数模文件",
        QDir::homePath(),
        "交换格式 (*.stp *.step *.igs *.iges);;STEP 文件 (*.stp *.step);;IGES 文件 (*.igs *.iges);;所有文件 (*)"
        );

    if (fileName.isEmpty()) return;

    // 获取文件名（不含扩展名）作为项目名
    QFileInfo fileInfo(fileName);
    QString baseName = fileInfo.baseName();
    QString suffix = fileInfo.suffix().toLower();

    // 弹出对话框让用户选择保存位置
    QString projectPath = QFileDialog::getExistingDirectory(this,
                                                            "选择项目目录",
                                                            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (projectPath.isEmpty()) {
        // 用户取消了操作
        return;
    }

    // 创建项目目录
    QDir projectDir(projectPath);
    QString fullProjectPath = projectDir.filePath(baseName);

    // 检查项目目录是否已存在
    if (projectDir.exists(fullProjectPath)) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                                  "目录已存在",
                                                                  QString("项目目录 '%1' 已存在。是否覆盖？").arg(fullProjectPath),
                                                                  QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    } else {
        // 创建项目目录
        if (!projectDir.mkpath(fullProjectPath)) {
            QMessageBox::critical(this,
                                  "错误",
                                  QString("无法创建项目目录: %1").arg(fullProjectPath));
            return;
        }
    }

    // 复制源文件到项目目录
    QString targetFilePath = QDir(fullProjectPath).filePath(fileInfo.fileName());
    if (!QFile::copy(fileName, targetFilePath)) {
        QMessageBox::critical(this,
                              "错误",
                              QString("无法复制文件到项目目录: %1").arg(targetFilePath));
        return;
    }

    // 读取模型文件
    TopoDS_Shape shape;
    if (suffix == "stp" || suffix == "step") {
        shape = ReadSTEPFile(targetFilePath);
    }
    else if (suffix == "igs" || suffix == "iges") {
        shape = ReadIGESFile(targetFilePath);
    }
    else {
        QMessageBox::warning(this, "不支持", "仅支持 .stp/.step/.igs/.iges");
        return;
    }

    if (!shape.IsNull()) {
        m_currentShape = shape;        // 保存当前模型
        DisplayShape(shape);

        // 清空模型数据
        m_treeModel->clear();
        m_treeModel->setHorizontalHeaderLabels(QStringList() << "Project Tree");

        // 添加根节点，显示项目名称和路径
        QStandardItem *rootItem = new QStandardItem(QString("%1 (%2)").arg(baseName, fullProjectPath));
        rootItem->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
        m_treeModel->appendRow(rootItem);

        // 添加文件节点
        QStandardItem *fileItem = new QStandardItem(fileInfo.fileName());
        fileItem->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
        rootItem->appendRow(fileItem);

        logMessage(QString("Successfully imported the mathematical model file: %1").arg(fileInfo.fileName()));
    } else {
        QMessageBox::warning(this, "错误", "无法读取或解析该文件！");
    }
}

//==========3.网格划分==========
#include <BRepBuilderAPI_Copy.hxx>

//网格模型显示
void MainWindow::DisplayMeshedShape(const TopoDS_Shape& meshedShape)
{
    try {
        if (meshedShape.IsNull()) {
            QMessageBox::warning(this, tr("警告"), tr("无效的几何体，无法显示！"));
            qDebug() << "DisplayMeshedShape: 传入的 meshedShape 为空。";
            return;
        }

        // --- 1. 清理 ui->mdiArea 中的旧内容 ---
        QLayout* layout = ui->mdiArea->layout();
        if (layout) {
            QLayoutItem* item;
            while ((item = layout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->setParent(nullptr);
                    delete item->widget();
                }
                delete item;
            }
        } else {
            layout = new QVBoxLayout(ui->mdiArea);
            ui->mdiArea->setLayout(layout);
        }

        // --- 2. 创建 VTK 渲染部件 ---
        QVTKOpenGLNativeWidget *vtkWidget = new QVTKOpenGLNativeWidget(ui->mdiArea);
        vtkWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // --- 3. 创建渲染器和渲染窗口 ---
        vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
        renderer->SetBackground(1, 1, 1);

        vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow =
            vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        vtkWidget->setRenderWindow(renderWindow);
        renderWindow->AddRenderer(renderer);

        // --- 4. 从已划分的 Shape 提取 VTK PolyData ---
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
        int pointIdOffset = 0;
        int totalPointsAdded = 0;
        int totalCellsAdded = 0;

        m_faceMap.clear(); // 清空旧的面映射
        std::vector<TopoDS_Face> faceOrderList;
        TopExp_Explorer tempExp(meshedShape, TopAbs_FACE);
        int faceCount = 0;
        for (; tempExp.More(); tempExp.Next()) {
            faceOrderList.push_back(TopoDS::Face(tempExp.Current()));
            faceCount++;
        }
        qDebug() << "DisplayMeshedShape: 找到 " << faceCount << " 个面。";

        int currentVtkCellId = 0;
        for (const auto& face : faceOrderList) {
            TopLoc_Location loc;
            Handle(Poly_Triangulation) triFace = BRep_Tool::Triangulation(face, loc);

            if (!triFace.IsNull()) {
                int numNodes = triFace->NbNodes();
                int numTriangles = triFace->NbTriangles();

                for (int i = 1; i <= numNodes; i++) {
                    gp_Pnt p = triFace->Node(i);
                    if (!loc.IsIdentity()) {
                        p.Transform(loc.Transformation());
                    }
                    points->InsertNextPoint(p.X(), p.Y(), p.Z());
                }
                totalPointsAdded += numNodes;

                for (int i = 1; i <= numTriangles; i++) {
                    Poly_Triangle tri = triFace->Triangle(i);
                    Standard_Integer n1, n2, n3;
                    tri.Get(n1, n2, n3);

                    vtkIdType ids[3] = {n1 - 1 + pointIdOffset, n2 - 1 + pointIdOffset, n3 - 1 + pointIdOffset};
                    vtkIdType cellId = triangles->InsertNextCell(3, ids);

                    m_faceMap[currentVtkCellId + i - 1] = face;
                }
                totalCellsAdded += numTriangles;
                pointIdOffset += numNodes;
                currentVtkCellId += numTriangles;
            }
        }

        qDebug() << "DisplayMeshedShape: 总共添加了 " << totalPointsAdded << " 个顶点, "
                 << totalCellsAdded << " 个三角形。";

        if (totalCellsAdded == 0) {
            QString warnMsg = tr("模型似乎没有有效的网格数据（0 个三角形），无法显示。");
            qDebug() << "DisplayMeshedShape: " << warnMsg;
            QMessageBox::warning(this, tr("警告"), warnMsg);
            delete vtkWidget;
            return;
        }

        // --- 5. 创建 VTK PolyData 对象 ---
        vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
        polyData->SetPoints(points);
        polyData->SetPolys(triangles);

        // --- 6. 创建 Mapper 和 Actor ---
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(polyData);

        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);

        // --- 7. 设置透明度和网格线显示 ---
        // 设置面颜色
        actor->GetProperty()->SetColor(0.8, 0.8, 0.8); // 灰色
        // 设置不透明度 (0.0 完全透明, 1.0 完全不透明)
        actor->GetProperty()->SetOpacity(0.3); // 例如，30% 不透明度
        // 开启边缘/网格线显示
        actor->GetProperty()->EdgeVisibilityOn();
        // 设置边缘/网格线颜色 (例如，黑色)
        actor->GetProperty()->SetEdgeColor(0, 0, 0);
        // 设置表面表示为表面 (SURFACE) 或线框 (WIREFRAME)
        // WIREFRAME 只显示网格线，SURFACE 显示面和线
        // 如果只想显示线，用 WIREFRAME；如果想显示透明面+线，用 SURFACE
        actor->GetProperty()->SetRepresentationToSurface();

        // --- 8. 添加 Actor 到渲染器并重置相机 ---
        renderer->AddActor(actor);
        renderer->ResetCamera();

        // --- 9. 将 VTK 部件添加到 UI 布局 ---
        layout->addWidget(vtkWidget);

        // --- 10. 触发渲染 ---
        renderWindow->Render();

        // --- 11. 设置交互器和样式 ---
        vtkSmartPointer<vtkRenderWindowInteractor> interactor = renderWindow->GetInteractor();
        if (interactor) {
            vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
                vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
            interactor->SetInteractorStyle(style);
        } else {
            qWarning() << "DisplayMeshedShape: 获取 VTK 渲染窗口交互器失败。";
        }

        qDebug() << "DisplayMeshedShape: 透明网格模型显示成功。";
    } catch (const std::exception& e) {
        QString errorMsg = QString("DisplayMeshedShape: 显示透明网格模型失败: %1").arg(e.what());
        QMessageBox::critical(this, tr("错误"), errorMsg);
        qDebug() << errorMsg;
    } catch (...) {
        QString errorMsg = QString("DisplayMeshedShape: 显示透明网格模型时发生未知异常。");
        QMessageBox::critical(this, tr("错误"), errorMsg);
        qDebug() << errorMsg;
    }
}

//网格划分功能
void MainWindow::on_meshButton_clicked()
{
    // 1. 检查是否有原始模型加载
    if (m_currentShape.IsNull()) {
        QMessageBox::warning(this, tr("网格划分"), tr("请先加载一个模型！"));
        qDebug() << "网格划分失败：未加载原始模型。";
        return;
    }

    // 2. 从界面获取网格大小参数
    double linearDeflection = ui->doubleSpinBox->value();
    // 确保值是正数
    if (linearDeflection <= 0.0) {
        QMessageBox::warning(this, tr("网格划分"), tr("网格大小必须大于 0！"));
        qDebug() << "网格划分失败：网格大小无效 (" << linearDeflection << ")。";
        return;
    }

    // 3. 创建原始模型的深拷贝，避免修改 m_currentShape 本身
    // BRepMesh_IncrementalMesh 会修改 Shape 内部的三角剖分数据
    BRepBuilderAPI_Copy copier(m_currentShape);
    TopoDS_Shape shapeToMesh = copier.Shape();

    if (shapeToMesh.IsNull()) {
        QString errorMsg = QString("创建模型副本失败！");
        QMessageBox::critical(this, tr("网格划分错误"), errorMsg);
        qDebug() << errorMsg;
        return;
    }

    // 4. 对副本执行网格划分
    try {
        // BRepMesh_IncrementalMesh 构造函数会直接修改传入的 shapeToMesh
        BRepMesh_IncrementalMesh mesher(shapeToMesh, linearDeflection, Standard_False, 0.5);

        // 执行网格划分
        mesher.Perform();

        // 检查是否成功
        if (!mesher.IsDone()) {
            QString errorMsg = QString("网格划分失败！OpenCASCADE Mesher 返回 IsDone() 为 false。");
            QMessageBox::critical(this, tr("网格划分错误"), errorMsg);
            qDebug() << errorMsg;
            return;
        }

        // 5. 将划分后的模型存储到新成员变量 ---
        m_meshedShape = shapeToMesh;
        qDebug() << "划分后的模型已存储到 m_meshedShape。";

        // 6. 显示划分后的模型
        DisplayMeshedShape(m_meshedShape);
        //QMessageBox::information(this, tr("Mesh Generation"), tr("Grid division completed, new model stored and displayed!"));
        logMessage(QString("Grid division completed, new model stored and displayed!"));
    } catch (const std::exception& e) {
        QString errorMsg = QString("网格划分过程中发生异常: %1").arg(e.what());
        //QMessageBox::critical(this, tr("网格划分错误"), errorMsg);
        qDebug() << errorMsg;
        // 发生错误时，清空存储的划分后模型
        m_meshedShape = TopoDS_Shape();
    } catch (...) {
        QString errorMsg = QString("网格划分过程中发生未知异常。");
        //QMessageBox::critical(this, tr("网格划分错误"), errorMsg);
        qDebug() << errorMsg;
        // 发生错误时，清空存储的划分后模型
        m_meshedShape = TopoDS_Shape();
    }
}

//==========4.提取外表面==========、
#include <vtkRendererCollection.h>
#include <unordered_set>
#include <BRepAdaptor_Surface.hxx>
#include <STEPControl_Writer.hxx>

//鼠标点击事件响应函数
void MainWindow::OnLeftButtonDown(vtkObject* obj, unsigned long eid, void* clientdata, void* calldata)
{
    MainWindow* self = static_cast<MainWindow*>(clientdata);
    vtkRenderWindowInteractor* interactor = vtkRenderWindowInteractor::SafeDownCast(obj);
    int* pos = interactor->GetEventPosition();

    vtkRenderer* renderer = self->ui->mdiArea->findChild<QVTKOpenGLNativeWidget*>()->renderWindow()->GetRenderers()->GetFirstRenderer();

    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.0005);
    picker->Pick(pos[0], pos[1], 0, renderer);

    if (picker->GetCellId() != -1) {
        vtkIdType cellId = picker->GetCellId();
        auto it = self->m_faceMap.find(cellId);
        if (it != self->m_faceMap.end()) {
            TopoDS_Face clickedFace = it->second;
            TopoDS_Shape outerSurface = self->FindConnectedOuterSurface(self->m_currentShape, clickedFace);
            if (!outerSurface.IsNull()) {
                self->m_extractedOuterSurface = outerSurface; //保存结果
                self->DisplayShape(outerSurface);
            }
        }
    }
    //logMessage(QString("The outer surface of the model has been successfully extracted."));
}

//BFS 算法提取相连的圆柱/环面/B样条面，组成外壳
TopoDS_Shape MainWindow::FindConnectedOuterSurface(const TopoDS_Shape& shape, const TopoDS_Face& seedFace)
{
    // 1. 判断种子面是否是圆柱面、环面或 B-Spline 面
    BRepAdaptor_Surface surf(seedFace);
    GeomAbs_SurfaceType type = surf.GetType();

    // 支持三种类型：圆柱、环面、B样条（自由曲面）
    if (type != GeomAbs_Cylinder && type != GeomAbs_Torus && type != GeomAbs_BSplineSurface) {
        qDebug() << "【警告】点击的面不是圆柱/环面/B样条面，无法提取外壁";
        qDebug() << "         面类型代码：" << type; // 打印类型编号，便于调试
        return TopoDS_Shape();
    }

    qDebug() << "【调试】种子面类型：" << type;

    // 2. 初始化 BFS
    BRep_Builder builder;
    TopoDS_Compound result;
    builder.MakeCompound(result);

    std::unordered_set<TopoDS_Shape, ShapeHash, ShapeEqual> visited;
    std::queue<TopoDS_Face> toVisit;
    toVisit.push(seedFace);

    int processedCount = 0;
    while (!toVisit.empty()) {
        TopoDS_Face currentFace = toVisit.front();
        toVisit.pop();

        if (visited.count(currentFace) > 0) continue;
        visited.insert(currentFace);
        builder.Add(result, currentFace);
        processedCount++;

        // 3. 查找相邻面
        TopExp_Explorer edgeExp(currentFace, TopAbs_EDGE);
        for (; edgeExp.More(); edgeExp.Next()) {
            TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());
            TopTools_ListOfShape faceList;
            GetFacesSharingEdge(shape, edge, faceList);

            for (const auto& adjFaceShape : faceList) {
                TopoDS_Face adjFace = TopoDS::Face(adjFaceShape);
                if (visited.count(adjFace) > 0) continue;

                // 4. 判断相邻面是否是圆柱/环面/B样条
                BRepAdaptor_Surface adjSurf(adjFace);
                GeomAbs_SurfaceType adjType = adjSurf.GetType();
                if (adjType != GeomAbs_Cylinder && adjType != GeomAbs_Torus && adjType != GeomAbs_BSplineSurface) {
                    continue; // 不是这三种类型，跳过
                }

                toVisit.push(adjFace);
            }
        }
    }

    qDebug() << "【提取外壁】成功提取" << processedCount << "个面";
    return result;
}

//判断两条边是否相同，几何上重合
bool AreEdgesSame(const TopoDS_Edge& e1, const TopoDS_Edge& e2)
{
    if (e1.IsNull() || e2.IsNull()) return false;

    // 正确声明参数变量
    Standard_Real first1, last1;
    Handle(Geom_Curve) curve1 = BRep_Tool::Curve(e1, first1, last1);

    Standard_Real first2, last2;
    Handle(Geom_Curve) curve2 = BRep_Tool::Curve(e2, first2, last2);

    // 检查曲线是否有效
    if (curve1.IsNull() || curve2.IsNull()) {
        return false;
    }

    // 检查曲线类型是否相同
    if (curve1->DynamicType() != curve2->DynamicType()) {
        return false;
    }

    // 比较端点位置
    gp_Pnt p1_start = BRep_Tool::Pnt(TopExp::FirstVertex(e1, Standard_True));
    gp_Pnt p1_end   = BRep_Tool::Pnt(TopExp::LastVertex(e1, Standard_True));
    gp_Pnt p2_start = BRep_Tool::Pnt(TopExp::FirstVertex(e2, Standard_True));
    gp_Pnt p2_end   = BRep_Tool::Pnt(TopExp::LastVertex(e2, Standard_True));

    Standard_Real tol = Precision::Confusion();
    bool sameOrder   = (p1_start.Distance(p2_start) < tol && p1_end.Distance(p2_end) < tol);
    bool reverseOrder = (p1_start.Distance(p2_end) < tol && p1_end.Distance(p2_start) < tol);

    return sameOrder || reverseOrder;
}

//查找与给定边共享的所有面。
void MainWindow::GetFacesSharingEdge(const TopoDS_Shape& shape, const TopoDS_Edge& edge, TopTools_ListOfShape& faceList)
{
    TopTools_MapOfShape uniqueFaces;
    TopExp_Explorer exp(shape, TopAbs_FACE);
    for (; exp.More(); exp.Next()) {
        TopoDS_Face face = TopoDS::Face(exp.Current());
        TopExp_Explorer edgeExp(face, TopAbs_EDGE);
        for (; edgeExp.More(); edgeExp.Next()) {
            TopoDS_Edge currentEdge = TopoDS::Edge(edgeExp.Current());
            // 使用自定义函数判断边是否相同
            if (AreEdgesSame(currentEdge, edge)) {
                if (!uniqueFaces.Contains(face)) {
                    uniqueFaces.Add(face);
                    faceList.Append(face);
                }
                break;
            }
        }
    }
}

#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <StlAPI_Writer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Interface_Static.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_StepModelType.hxx>

void MainWindow::extractFace()
{
    if (m_extractedOuterSurface.IsNull()) {
        QMessageBox::warning(this, "Warning", "No outer wall model was extracted!");
        return;
    }

    // 创建Profile文件夹路径
    QString profileDir = QDir::currentPath() + "/Profile";
    QDir().mkpath(profileDir); // 确保目录存在

    QString fileName = profileDir + "/surface.stl";

    // 创建网格化对象，设置精度
    // 精度值越小，网格越精细，但文件越大
    Standard_Real linearDeflection = 0.1; // 可根据需要调整
    Standard_Real angularDeflection = 0.5; // 角度偏差，弧度

    BRepMesh_IncrementalMesh mesh(m_extractedOuterSurface, linearDeflection, false, angularDeflection, true);
    mesh.Perform();

    if (!mesh.IsDone()) {
        QMessageBox::critical(this, "Error", "Mesh generation failed! Cannot save as STL.");
        return;
    }

    // 创建STL写入器
    StlAPI_Writer stlWriter;

    // 设置二进制或ASCII格式
    // 默认为ASCII格式，如果要二进制格式，取消下面这行的注释
    // stlWriter.SetASCIIMode(false);

    // 写入模型
    bool success = stlWriter.Write(m_extractedOuterSurface, fileName.toStdString().c_str());

    if (success) {
        QMessageBox::information(this, "Success", "The outer wall model has been saved as: " + fileName);
        logMessage(QString("The outer wall model has been saved as STL to: ") + fileName);
    } else {
        QMessageBox::critical(this, "Error", "Save failed!");
    }
}

//==========5.提取中心线==========
#include <QProcess>
#include <vtkPolyDataReader.h>

//用Python脚本提取中心线，生成vtk文件，并显示
void MainWindow::on_extractCenterline_clicked()
{
    // 获取应用程序目录
    QString appDirPath = QCoreApplication::applicationDirPath();

    // 使用 QDir 来拼接路径，避免重复拼接
    QDir scriptDir(appDirPath);
    QString scriptPath1 = scriptDir.filePath("Profile/extractcenterline.py");
    QString fileName = scriptDir.filePath("Profile/surface.stl");

    // 确定 Python 解释器的路径
    QString pythonCommand = "/home/ysy/anaconda3/envs/Extract-midline/bin/python3";  // 使用绝对路径来确保正确的解释器

    // 检查Python脚本是否存在
    if (!QFile::exists(scriptPath1)) {
        logMessage("Error,Python script not found: " + scriptPath1);
        return;
    }

    // 检查Python解释器是否存在
    if (!QFile::exists(pythonCommand)) {
        logMessage("Error,Python interpreter not found: " + pythonCommand);
        return;
    }

    // 检查输入文件是否存在（fileName应该是从其他地方获取的STL文件路径）
    if (fileName.isEmpty() || !QFile::exists(fileName)) {
        logMessage("Error,Input file not found: " + fileName);
        return;
    }

    // 创建进程并执行Python脚本，将fileName作为参数传递
    QProcess process1;

    // 连接信号以捕获输出（可选，用于调试）
    QObject::connect(&process1, &QProcess::readyReadStandardOutput, [&process1]() {
        QString output = process1.readAllStandardOutput();
        qDebug() << "Python script output:" << output;
    });

    QObject::connect(&process1, &QProcess::readyReadStandardError, [&process1]() {
        QString error = process1.readAllStandardError();
        qDebug() << "Python script error:" << error;
    });

    // 启动进程，将fileName作为参数传递给Python脚本
    process1.start(pythonCommand, QStringList() << scriptPath1 << fileName);

    // 等待进程完成
    if (!process1.waitForStarted()) {
        logMessage("Error,Failed to start Python process");
        return;
    }

    if (!process1.waitForFinished()) {
        logMessage("Error,Python process did not finish properly");
        return;
    }

    // 检查进程退出状态
    if (process1.exitCode() != 0) {
        QString error = process1.readAllStandardError();
        logMessage("Error,Python script execution failed with error: " + error);
        return;
    }

    // 生成的VTK文件路径
    QString vtkFilePath = scriptDir.filePath("data/output-centerline/vessel_centerline_surface_and_extended_centerlines.vtk");

    // 显示VTK文件
    showVTKFile(vtkFilePath);

    // 生成的中心线vtk文件路径
    QString CenterlineFilePath = scriptDir.filePath("data/output-centerline/vessel_centerline_extended_centerlines.vtk");

    // 计算弯曲参数
    Calculate_bendingparameters(CenterlineFilePath);

    // 成功执行
    logMessage("Centerline extraction completed successfully");
}

// 显示vtk文件
void MainWindow::showVTKFile(const QString &vtkFilePath)
{
    // 清空MDI区域的所有子窗口
    //ui->mdiArea->closeAllSubWindows();
    // 清理 ui->mdiArea 中的旧内容
    QLayout* layout = ui->mdiArea->layout();
    if (layout) {
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->setParent(nullptr);
            }
            delete item;
        }
    } else {
        layout = new QVBoxLayout(ui->mdiArea);
        ui->mdiArea->setLayout(layout);
    }

    // 创建VTK渲染器
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow =
        vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderWindow->AddRenderer(renderer);

    // 读取VTK文件
    vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName(vtkFilePath.toStdString().c_str());

    reader->Update();

    // 创建Mapper
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(reader->GetOutputPort());

    // 创建Actor
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // 设置属性
    actor->GetProperty()->SetColor(0.5, 0.5, 0.5);     // 灰色 - 面的颜色
    actor->GetProperty()->SetOpacity(0.5);             // 透明度（0.0完全透明，1.0完全不透明）
    actor->GetProperty()->SetLineWidth(0.1);           // 线宽

    // 添加Actor到渲染器
    renderer->AddActor(actor);
    renderer->SetBackground(1, 1, 1); // 背景颜色

    // 创建VTK部件
    QVTKOpenGLNativeWidget *vtkWidget = new QVTKOpenGLNativeWidget();
    vtkWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    vtkWidget->setRenderWindow(renderWindow);

    // 创建全屏子窗口（无边框）
    QMdiSubWindow *subWindow = new QMdiSubWindow(ui->mdiArea);
    subWindow->setWidget(vtkWidget);
    subWindow->setWindowFlags(Qt::FramelessWindowHint); // 无边框
    subWindow->setAttribute(Qt::WA_DeleteOnClose);

    // 添加到MDI区域并最大化
    ui->mdiArea->addSubWindow(subWindow);
    subWindow->showMaximized();

    // 设置交互器
    vtkSmartPointer<vtkRenderWindowInteractor> interactor = renderWindow->GetInteractor();
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    interactor->SetInteractorStyle(style);

    logMessage("VTK file displayed successfully in MDI area: " + vtkFilePath);
}

// 计算弯曲参数
// 定义三维点结构
struct Point3D {
    double x, y, z;
    Point3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
    Point3D operator-(const Point3D& other) const {
        return Point3D(x - other.x, y - other.y, z - other.z);
    }
    Point3D operator+(const Point3D& other) const {
        return Point3D(x + other.x, y + other.y, z + other.z);
    }
    Point3D operator*(double scalar) const {
        return Point3D(x * scalar, y * scalar, z * scalar);
    }
    double magnitude() const {
        return std::sqrt(x*x + y*y + z*z);
    }
    Point3D normalized() const {
        double mag = magnitude();
        if (mag > 1e-10) {
            return Point3D(x/mag, y/mag, z/mag);
        }
        return Point3D(0, 0, 0);
    }

    // 计算叉积
    Point3D cross(const Point3D& other) const {
        return Point3D(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
            );
    }
};

// 计算两点间距离
double distance(const Point3D& p1, const Point3D& p2) {
    return (p1 - p2).magnitude();
}

// 计算向量夹角 (弧度)
double angleBetweenVectors(const Point3D& v1, const Point3D& v2) {
    double dot = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
    double mag1 = v1.magnitude();
    double mag2 = v2.magnitude();
    if (mag1 == 0 || mag2 == 0) return 0.0;
    double cosTheta = dot / (mag1 * mag2);
    cosTheta = std::max(-1.0, std::min(1.0, cosTheta)); // 防止浮点误差导致的无效值
    return std::acos(cosTheta);
}

// 三点确定的圆的圆心和半径
bool fitCircle(const Point3D& p1, const Point3D& p2, const Point3D& p3, Point3D& center, double& radius) {
    // 使用三点确定圆的几何方法
    Point3D v1 = p2 - p1;
    Point3D v2 = p3 - p1;

    double v1v1 = v1.x*v1.x + v1.y*v1.y + v1.z*v1.z;
    double v2v2 = v2.x*v2.x + v2.y*v2.y + v2.z*v2.z;
    double v1v2 = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;

    double denominator = 2.0 * (v1v1 * v2v2 - v1v2 * v1v2);
    if (std::abs(denominator) < 1e-10) return false; // 三点共线

    double a = v2v2 * (v1v1 - v1v2) / denominator;
    double b = v1v1 * (v2v2 - v1v2) / denominator;

    center.x = p1.x + a * v1.x + b * v2.x;
    center.y = p1.y + a * v1.y + b * v2.y;
    center.z = p1.z + a * v1.z + b * v2.z;

    radius = distance(center, p1);
    return true;
}

// 计算弯曲参数（支持多段弯曲）
void MainWindow::Calculate_bendingparameters(const QString &CenterlineFilePath)
{
    // 将QString转换为std::string
    std::string vtkFilePath = CenterlineFilePath.toStdString();

    // 读取VTK文件
    vtkSmartPointer<vtkPolyDataReader> reader =
        vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName(vtkFilePath.c_str());
    reader->Update();

    vtkPolyData* polyData = reader->GetOutput();
    if (!polyData) {
        qDebug() << "错误: 无法读取VTK文件或文件为空";
        return;
    }

    vtkPoints* points = polyData->GetPoints();
    if (!points || points->GetNumberOfPoints() < 3) {
        qDebug() << "错误: VTK文件中点的数量不足，至少需要3个点进行分析";
        return;
    }

    int numPoints = points->GetNumberOfPoints();
    qDebug() << "读取到" << numPoints << "个点";

    // 将VTK点转换为自定义Point3D结构
    std::vector<Point3D> centerlinePoints;
    for (int i = 0; i < numPoints; ++i) {
        double p[3];
        points->GetPoint(i, p);
        centerlinePoints.push_back(Point3D(p[0], p[1], p[2]));
    }

    // 参数设置
    const double curvatureThreshold = 1e-6; // 曲率阈值，认为0的阈值
    const int windowSize = 3; // 用于计算局部曲率的窗口大小
    const int minBendLength = 5; // 弯曲段最小长度

    // 存储每个点的曲率
    std::vector<double> curvatures(numPoints, 0.0);

    // 计算所有点的曲率
    for (int i = windowSize; i < numPoints - windowSize; ++i) {
        Point3D p_prev = centerlinePoints[i - windowSize];
        Point3D p_curr = centerlinePoints[i];
        Point3D p_next = centerlinePoints[i + windowSize];

        Point3D v1 = p_curr - p_prev;
        Point3D v2 = p_next - p_curr;

        double angle = angleBetweenVectors(v1, v2);

        // 计算近似曲率
        double chord_length = distance(p_prev, p_next);
        if (chord_length > 1e-6) {
            curvatures[i] = 2.0 * std::sin(angle / 2.0) / chord_length;
        } else {
            curvatures[i] = 0.0;
        }
    }

    // 清空之前的弯曲段数据
    m_bendSegments.clear();

    // 检测所有弯曲段
    int currentIndex = windowSize;
    while (currentIndex < numPoints - windowSize) {
        // 寻找弯曲开始点：找到第一个曲率不为0的地方
        int bendStartIndex = -1;
        for (int i = currentIndex; i < numPoints - windowSize; ++i) {
            if (curvatures[i] > curvatureThreshold) { // 曲率不为0
                // 检查后续连续几个点是否都保持非零曲率
                bool isBendStart = true;
                for (int j = i; j < std::min(i + minBendLength, numPoints - windowSize); ++j) {
                    if (curvatures[j] <= curvatureThreshold) {
                        isBendStart = false;
                        break;
                    }
                }

                if (isBendStart) {
                    bendStartIndex = i;
                    break;
                }
            }
        }

        if (bendStartIndex == -1) {
            break; // 没有更多弯曲段
        }

        // 从弯曲开始点继续寻找弯曲结束点：找到第一个曲率为0的地方
        int bendEndIndex = -1;
        for (int i = bendStartIndex + minBendLength; i < numPoints - windowSize; ++i) {
            if (curvatures[i] <= curvatureThreshold) { // 曲率为0
                // 检查后续连续几个点是否都保持曲率为0
                bool isBendEnd = true;
                for (int j = i; j < std::min(i + minBendLength, numPoints - windowSize); ++j) {
                    if (curvatures[j] > curvatureThreshold) {
                        isBendEnd = false;
                        break;
                    }
                }

                if (isBendEnd) {
                    bendEndIndex = i;
                    break;
                }
            }
        }

        // 如果没有找到明确的结束点，就使用最后的点
        if (bendEndIndex == -1) {
            bendEndIndex = numPoints - windowSize - 1;
        }

        // 确保弯曲段有足够长度
        if (bendEndIndex - bendStartIndex < minBendLength) {
            qDebug() << "检测到的弯曲段太短，长度为: " << (bendEndIndex - bendStartIndex);
            currentIndex = bendStartIndex + 1; // 继续寻找下一个可能的弯曲段
            continue;
        }

        // 计算从起点到弯曲开始点的距离
        double bendStartDistance = 0.0;
        for (int i = 0; i < bendStartIndex && i < numPoints - 1; ++i) {
            bendStartDistance += distance(centerlinePoints[i], centerlinePoints[i+1]);
        }

        // 计算从起点到弯曲结束点的距离
        double bendEndDistance = 0.0;
        for (int i = 0; i < bendEndIndex && i < numPoints - 1; ++i) {
            bendEndDistance += distance(centerlinePoints[i], centerlinePoints[i+1]);
        }

        // 使用弯曲开始点、中间点和结束点进行三点法计算
        int bendMidIndex = (bendStartIndex + bendEndIndex) / 2;
        Point3D p_start = centerlinePoints[bendStartIndex];
        Point3D p_mid = centerlinePoints[bendMidIndex];
        Point3D p_end = centerlinePoints[bendEndIndex];

        // 计算弯曲角度（使用切线方向）
        // 估算起始点和结束点的切线方向
        Point3D tangent_start, tangent_end;
        if (bendStartIndex >= 1 && bendStartIndex < numPoints - 1) {
            tangent_start = centerlinePoints[bendStartIndex + 1] - centerlinePoints[bendStartIndex - 1];
        } else if (bendStartIndex == 0) {
            tangent_start = centerlinePoints[1] - centerlinePoints[0];
        } else {
            tangent_start = centerlinePoints[numPoints - 1] - centerlinePoints[numPoints - 2];
        }

        if (bendEndIndex >= 1 && bendEndIndex < numPoints - 1) {
            tangent_end = centerlinePoints[bendEndIndex + 1] - centerlinePoints[bendEndIndex - 1];
        } else if (bendEndIndex == 0) {
            tangent_end = centerlinePoints[1] - centerlinePoints[0];
        } else {
            tangent_end = centerlinePoints[numPoints - 1] - centerlinePoints[numPoints - 2];
        }

        double bendAngle = angleBetweenVectors(tangent_start, tangent_end);

        // 使用三点法计算弯曲半径
        Point3D circleCenter;
        double bendRadius = 0.0;
        bool circleFitted = fitCircle(p_start, p_mid, p_end, circleCenter, bendRadius);

        if (!circleFitted) {
            // 如果三点共线，使用弦长和矢高估算
            double chord_len = distance(p_start, p_end);
            double mid_to_chord_dist = 0.0;

            // 计算中点到弦的距离（矢高）
            Point3D chord_vec = p_end - p_start;
            double chord_mag = chord_vec.magnitude();
            if (chord_mag > 1e-6) {
                Point3D unit_chord = Point3D(chord_vec.x/chord_mag, chord_vec.y/chord_mag, chord_vec.z/chord_mag);
                Point3D from_start_to_mid = p_mid - p_start;
                double proj_len = from_start_to_mid.x*unit_chord.x + from_start_to_mid.y*unit_chord.y + from_start_to_mid.z*unit_chord.z;
                Point3D proj_point = Point3D(p_start.x + proj_len*unit_chord.x,
                                             p_start.y + proj_len*unit_chord.y,
                                             p_start.z + proj_len*unit_chord.z);
                mid_to_chord_dist = distance(p_mid, proj_point);
            }

            if (mid_to_chord_dist > 1e-6) {
                bendRadius = (chord_len * chord_len) / (8.0 * mid_to_chord_dist) + mid_to_chord_dist / 2.0;
            } else {
                bendRadius = chord_len / 2.0; // 如果三点几乎共线，估算为弦长的一半
            }
        }

        // 创建弯曲段信息
        BendSegment segment;
        segment.startIndex = bendStartIndex;
        segment.endIndex = bendEndIndex;
        segment.midIndex = bendMidIndex;
        segment.startDistance = bendStartDistance;
        segment.endDistance = bendEndDistance;
        segment.angle = bendAngle * 180.0 / M_PI; // 转换为度
        segment.radius = bendRadius;
        segment.length = bendEndIndex - bendStartIndex + 1;

        // 添加到弯曲段列表
        m_bendSegments.push_back(segment);

        // 输出当前弯曲段结果
        qDebug() << "\n--- 弯曲段" << m_bendSegments.size() << "---";
        qDebug() << "弯曲起始距离:" << bendStartDistance << "(从中心线起点算起)";
        qDebug() << "弯曲结束距离:" << bendEndDistance << "(从中心线起点算起)";
        qDebug() << "弯曲起始点索引:" << bendStartIndex;
        qDebug() << "弯曲结束点索引:" << bendEndIndex;
        qDebug() << "弯曲中点索引:" << bendMidIndex;
        qDebug() << "弯曲段长度:" << (bendEndIndex - bendStartIndex + 1) << "个点";
        qDebug() << "弯曲角度:" << (bendAngle * 180.0 / M_PI) << "度";
        qDebug() << "弯曲半径:" << bendRadius;

        // 更新搜索起始位置
        currentIndex = bendEndIndex + minBendLength; // 确保跳过当前弯曲段，避免重复检测
    }

    // 更新UI显示
    showBendingResults();

    qDebug() << "\n=== 总计检测到" << m_bendSegments.size() << "个弯曲段 ===";
    qDebug() << "弯曲参数计算完成";
}

void MainWindow::showBendingResults()
{
    ui->resultsTable->setColumnCount(4);

    // 设置表头
    ui->resultsTable->setHorizontalHeaderLabels(QStringList()
                                            << "  Start  "
                                            << "  End  "
                                            << "  Angle(°)  "
                                            << "  Radius  ");

    // 清空表格
    ui->resultsTable->setRowCount(0);

    // 添加所有弯曲段信息到表格
    for (size_t i = 0; i < m_bendSegments.size(); ++i) {
        const BendSegment& segment = m_bendSegments[i];

        int row = ui->resultsTable->rowCount();
        ui->resultsTable->insertRow(row);

        // 设置单元格内容
        ui->resultsTable->setItem(row, 0, new QTableWidgetItem(QString::number(segment.startDistance, 'f', 1)));
        ui->resultsTable->setItem(row, 1, new QTableWidgetItem(QString::number(segment.endDistance, 'f', 1)));
        ui->resultsTable->setItem(row, 2, new QTableWidgetItem(QString::number(segment.angle, 'f', 2)));
        ui->resultsTable->setItem(row, 3, new QTableWidgetItem(QString::number(segment.radius, 'f', 1)));
    }

    // 调整行高和列宽
    ui->resultsTable->resizeRowsToContents();
    ui->resultsTable->resizeColumnsToContents();
}

//==========6.过程可视化==========
//原始模型
void MainWindow::init_model(){
    if (!m_currentShape.IsNull()) {
        DisplayShape(m_currentShape);
        logMessage("The original model was successfully displayed.");
    } else {
        QMessageBox::warning(this, "error", "No available models!");
    }
}

//网格模型
void MainWindow::mesh_model(){
    if (!m_meshedShape.IsNull()) {
        DisplayMeshedShape(m_meshedShape);
        logMessage("The grid model was successfully displayed.");
    } else {
        QMessageBox::warning(this, "error", "No available models!");
    }
}

//表面模型
void MainWindow::surface_model(){
    if (!m_extractedOuterSurface.IsNull()) {
        DisplayShape(m_extractedOuterSurface);
        logMessage("The surface model was successfully displayed.");
    } else {
        QMessageBox::warning(this, "error", "No available models!");
    }
}

//中线模型
void MainWindow::centerline_model(){
    // 获取应用程序目录
    QString appDirPath = QCoreApplication::applicationDirPath();
    // 使用 QDir 来拼接路径，避免重复拼接
    QDir scriptDir(appDirPath);
    // 生成的VTK文件路径
    QString vtkFilePath = scriptDir.filePath("data/output-centerline/vessel_centerline_surface_and_extended_centerlines.vtk");
    // 显示VTK文件
    showVTKFile(vtkFilePath);
}

//==========7.结果可视化==========


















void MainWindow::on_actionSave_triggered()
{
    logMessage("Save project action triggered.");
    // TODO: 实现保存项目的逻辑
    QMessageBox::information(this, "Save", "Save functionality would save the project here.");
    logMessage("Project saved.");
    updateStatusBar("Project saved.");
}

void MainWindow::on_actionSaveAs_triggered()
{
    logMessage("Save As project action triggered.");
    QString fileName = QFileDialog::getSaveFileName(this, "Save Project As", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "Project Files (*.prj);;All Files (*)");
    if (!fileName.isEmpty()) {
        logMessage(QString("Saving file as: %1").arg(QDir::toNativeSeparators(fileName)));
        // TODO: 实现另存为项目的逻辑
        QMessageBox::information(this, "Save As", QString("Project saved as %1").arg(QDir::toNativeSeparators(fileName)));
        logMessage("Project saved as new file.");
        updateStatusBar(QString("Project saved as %1").arg(QFileInfo(fileName).fileName()));
    }
}

void MainWindow::on_actionExit_triggered()
{
    logMessage("Exit action triggered. Quitting...");
    QApplication::quit();
}

// 编辑菜单
void MainWindow::on_actionUndo_triggered()
{
    logMessage("Undo action triggered.");
    QMessageBox::information(this, "Undo", "Undo functionality goes here.");
}

void MainWindow::on_actionRedo_triggered()
{
    logMessage("Redo action triggered.");
    QMessageBox::information(this, "Redo", "Redo functionality goes here.");
}

void MainWindow::on_actionPreferences_triggered()
{
    logMessage("Preferences action triggered.");
    QMessageBox::information(this, "Preferences", "Preferences dialog goes here.");
}

// 视图菜单
void MainWindow::on_actionToggleProjectTree_triggered()
{
    ui->dockWidget_ProjectTree->setVisible(!ui->dockWidget_ProjectTree->isVisible());
}

void MainWindow::on_actionToggleProperties_triggered()
{
    ui->dockWidget_Properties->setVisible(!ui->dockWidget_Properties->isVisible());
}

void MainWindow::on_actionToggleLog_triggered()
{
    ui->dockWidget_Log->setVisible(!ui->dockWidget_Log->isVisible());
}

void MainWindow::on_actionResetLayout_triggered()
{
    logMessage("Reset layout action triggered.");
    ui->dockWidget_ProjectTree->setVisible(true);
    ui->dockWidget_Properties->setVisible(true);
    ui->dockWidget_Features->setVisible(true);
    ui->dockWidget_Log->setVisible(true);
    updateStatusBar("Layout reset.", 2000);
}

// 工具菜单
void MainWindow::on_actionRunSimulation_triggered()
{
    if (m_simulationRunning) {
        logMessage("Simulation is already running, ignoring start request.");
        return;
    }

    logMessage("Run Simulation action triggered.");
    updateStatusBar("Simulation starting...", 0);
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_simulationRunning = true;

    m_progressTimer->start(200); // 每200ms更新一次进度
    logMessage("Simulation started (simulated).");
}

void MainWindow::on_actionPostProcessing_triggered()
{
    logMessage("Post Processing action triggered.");
    QMessageBox::information(this, "Post-Processing", "Post-processing view goes here.");
}

void MainWindow::on_actionMeshGeneration_triggered()
{
    logMessage("Mesh Generation action triggered.");
    QMessageBox::information(this, "Mesh Generation", "Mesh generation process goes here.");
    logMessage("Mesh generation completed (simulated).");
    updateStatusBar("Mesh generation completed.", 3000);
}

// 帮助菜单
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About", "TubeForm CAE\nVersion 0.1\nA simple tube forming simulation software.");
}

void MainWindow::on_actionDocumentation_triggered()
{
    logMessage("Documentation action triggered.");
    QMessageBox::information(this, "Documentation", "Documentation link or file viewer goes here.");
}

// 其他交互
void MainWindow::on_treeView_Project_clicked(const QModelIndex &index)
{

}

// 功能与tab栏的切换
void MainWindow::on_listWidget_Features_itemClicked(QListWidgetItem *item)
{
    if (item) {
        QString featureName = item->text();
        logMessage(QString("Feature selected: %1").arg(featureName));

        if (featureName == "Parametric  Modeling") {ui->tabWidget->setCurrentIndex(1);}
        else if (featureName == "Mesh Generation") {ui->tabWidget->setCurrentIndex(2);}
        else if (featureName == "Extract Surface") {ui->tabWidget->setCurrentIndex(3);}
        else if (featureName == "Extract Centerline") {ui->tabWidget->setCurrentIndex(4);}
        else {}
    }
}

void MainWindow::updateSimulationProgress()
{
    if (m_simulationRunning) {
        int val = m_progressBar->value();
        if (val < 100) {
            m_progressBar->setValue(val + 5);
        } else {
            m_progressTimer->stop();
            m_simulationRunning = false;
            m_progressBar->setVisible(false);
            logMessage("Simulation completed.");
            updateStatusBar("Simulation finished.", 3000);
        }
    }
}
