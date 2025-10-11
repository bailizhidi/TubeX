#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel> // 用于QTreeView的数据模型
#include <QProgressBar>
#include <QLabel>
#include <QListWidgetItem>
#include <QTimer>
#include <QMdiArea>
#include <QFileSystemWatcher>

#include <QMainWindow>
#include <QMdiArea>

#include <QMainWindow>
#include <AIS_InteractiveContext.hxx>
#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>

// 在包含 OpenCASCADE 头文件之前，抑制弃用警告
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
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
#include <BRepAdaptor_Surface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <gp_Lin.hxx>
#include <gp_Circ.hxx>
#include <STEPControl_Writer.hxx>
#include <Interface_Static.hxx>

#include <map>
#include <vtkCellPicker.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <TopTools_ListOfShape.hxx>
#include <NCollection_DataMap.hxx>
// 在包含完 OpenCASCADE 头文件之后，恢复警告设置
#pragma GCC diagnostic pop

#include <vtkCylinderSource.h>
#include <vtkCubeSource.h>
#include <vtkSphereSource.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkDataSetMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkScalarBarActor.h>
#include <vtkLookupTable.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>

#include <functional>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 定义一个 DataMap 类型
typedef NCollection_DataMap<TopoDS_Shape, Handle(AIS_Shape), TopTools_ShapeMapHasher> AIS_ShapeMap;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 文件菜单
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExit_triggered();

    // 编辑菜单
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionPreferences_triggered();

    // 视图菜单
    void on_actionToggleProjectTree_triggered();
    void on_actionToggleProperties_triggered();
    void on_actionToggleLog_triggered();
    void on_actionResetLayout_triggered();

    // 工具菜单
    void on_actionRunSimulation_triggered();
    void on_actionPostProcessing_triggered();
    void on_actionMeshGeneration_triggered();

    // 帮助菜单
    void on_actionAbout_triggered();
    void on_actionDocumentation_triggered();

    // 其他交互
    void on_treeView_Project_clicked(const QModelIndex &index); // 点击QTreeView项时触发
    void on_listWidget_Features_itemClicked(QListWidgetItem *item);

    // 定时器槽，用于模拟进度
    void updateSimulationProgress();

private:
    void setupUI();
    void setupConnections();
    void populateProjectTree(QStandardItem *parentItem, const QString &path);
    void logMessage(const QString &message);
    void createNewProject();
    void updateStatusBar(const QString &message, int timeout = 2000);
    void onDirectoryChanged(const QString &path);
    void onFileChanged(const QString &path);
    void updateProjectTree(); // 更新项目树函数

    // 主要功能实现函数
    // 1.参数化建模以及显示
    void on_CADmodel_clicked();
    void MakeElbowModel(
        double R_out, double R_in, double length,        // 管体外半径、内半径、长度
        double sleeve_thickness1, double sleeve_length1,   // 旋转套
        double sleeve_thickness, double sleeve_length,   // 固定套
        double rotary_pos, double fixed_pos,             // 套筒位置（距管体右端的距离）
        double arc_R, double arc_t, double arc_angle // 半圆弧套筒半径、厚度、角度（单位：弧度）
        );
    vtkSmartPointer<vtkPolyData> ConvertOCCShapeToVTKPolyData(const TopoDS_Shape& shape, double linearDeflection);
    vtkSmartPointer<vtkActor> CreateVTKActor(vtkSmartPointer<vtkPolyData> polyData, double r, double g, double b);
    void ShowModelInMdiArea(vtkSmartPointer<vtkRenderer> renderer);

    // 2.数模导入以及显示
    // 读取STEP文件
    TopoDS_Shape ReadSTEPFile(const QString& fileName);
    // 读取IGES文件
    TopoDS_Shape ReadIGESFile(const QString& fileName);
    // 将OCC形状显示到ui->mdiArea（复用渲染逻辑）
    void DisplayShape(const TopoDS_Shape& shape);
    // 存储 OCC Face 和 VTK CellId 的映射
    std::map<vtkIdType, TopoDS_Face> m_faceMap;
    static void OnLeftButtonDown(vtkObject* caller, unsigned long eventId, void* clientData, void* callData);

private:
    Ui::MainWindow *ui;
    QStandardItemModel *m_treeModel; // QTreeView的数据模型
    QFileSystemWatcher *m_fileSystemWatcher; // 文件系统监视器
    QTimer *m_debounceTimer; // 防抖定时器
    QProgressBar *m_progressBar = nullptr;
    bool m_simulationRunning = false;
    QTimer *m_progressTimer = nullptr;

    QString m_currentProjectPath;  // 当前项目路径
    QString m_currentProjectName;  // 当前项目名称

    TopoDS_Shape m_currentShape; // 保存当前加载的模型
    TopoDS_Shape m_extractedOuterSurface; // 存储提取的外壁
    TopoDS_Shape m_meshedShape; // 保存网格模型
};
#endif // MAINWINDOW_H
