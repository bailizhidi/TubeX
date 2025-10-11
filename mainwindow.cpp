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

// ==========创建CAD模型==========
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





























// 文件菜单
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

//复用显示逻辑
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
        //clickCallback->SetCallback(OnLeftButtonDown);
        clickCallback->SetClientData(this); // 传递 this 指针

        // 添加观察者
        interactor->AddObserver(vtkCommand::LeftButtonPressEvent, clickCallback);

        //qDebug() << "模型已成功显示。";
        //qDebug() << "模型类型：" << m_currentShape.ShapeType();

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString("Display model failure: %1").arg(e.what()));
    }
}

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

void MainWindow::on_listWidget_Features_itemClicked(QListWidgetItem *item)
{
    if (item) {
        QString featureName = item->text();
        logMessage(QString("Feature selected: %1").arg(featureName));

        if (featureName == "Create Tube") {}
        else if (featureName == "Apply Bend") {}
        else if (featureName == "Generate Mesh") {}
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
