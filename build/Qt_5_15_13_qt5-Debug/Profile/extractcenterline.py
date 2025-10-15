import vtk
import vmtk
from vmtk import vtkvmtk
import numpy as np
import os
import sys
import pathlib

def load_stl_surface(stl_filename):
    """加载STL文件作为血管表面"""
    
    print(f"1. 加载STL文件: {stl_filename}")
    
    if not os.path.exists(stl_filename):
        print(f"错误: 找不到文件 {stl_filename}")
        return None
    
    # 创建STL读取器
    reader = vtk.vtkSTLReader()
    reader.SetFileName(stl_filename)
    reader.Update()
    
    surface = reader.GetOutput()
    
    print(f"   血管表面点数: {surface.GetNumberOfPoints()}")
    print(f"   血管表面面片数: {surface.GetNumberOfCells()}")
    print(f"   血管表面边界框: {surface.GetBounds()}")
    
    return surface

def find_boundary_centers(surface, tolerance=0.01):
    """找到边界环的两个中心点，分别代表入口端和出口端的中心"""
    
    print("   正在计算边界环中心点...")
    
    # 使用vtkFeatureEdges来找到边界边
    feature_edges = vtk.vtkFeatureEdges()
    feature_edges.SetInputData(surface)
    feature_edges.BoundaryEdgesOn()
    feature_edges.FeatureEdgesOff()
    feature_edges.ManifoldEdgesOff()
    feature_edges.NonManifoldEdgesOff()
    feature_edges.Update()
    
    boundary_edges = feature_edges.GetOutput()
    
    if boundary_edges.GetNumberOfPoints() == 0:
        print("   没有发现边界点，可能是封闭表面")
        return [], []
    
    # 获取边界点坐标
    boundary_points = []
    points = boundary_edges.GetPoints()
    for i in range(points.GetNumberOfPoints()):
        point = points.GetPoint(i)
        boundary_points.append((i, point))
    
    if len(boundary_points) == 0:
        print("   没有边界点")
        return [], []
    
    print(f"   找到 {len(boundary_points)} 个边界点")
    
    # 如果边界点数量较少，直接返回边界点的中心
    if len(boundary_points) < 10:
        # 计算所有边界点的中心
        center_x = sum(p[1][0] for p in boundary_points) / len(boundary_points)
        center_y = sum(p[1][1] for p in boundary_points) / len(boundary_points)
        center_z = sum(p[1][2] for p in boundary_points) / len(boundary_points)
        
        center_point = (center_x, center_y, center_z)
        print(f"   边界中心点: ({center_point[0]:.2f}, {center_point[1]:.2f}, {center_point[2]:.2f})")
        return [center_point], [center_point]
    
    # 使用连通组件分析来分离不同的边界环
    # 首先获取边界边的连接信息
    edges = boundary_edges.GetLines()
    edges.InitTraversal()
    
    # 构建邻接表
    adjacency = {}
    for i in range(boundary_edges.GetNumberOfPoints()):
        adjacency[i] = set()
    
    while True:
        id_list = vtk.vtkIdList()
        status = edges.GetNextCell(id_list)
        if not status:
            break
        for i in range(id_list.GetNumberOfIds() - 1):
            p1 = id_list.GetId(i)
            p2 = id_list.GetId(i + 1)
            adjacency[p1].add(p2)
            adjacency[p2].add(p1)
    
    # 使用DFS找到连通组件（边界环）
    visited = set()
    boundary_rings = []
    
    for i in range(boundary_edges.GetNumberOfPoints()):
        if i not in visited:
            ring = []
            stack = [i]
            while stack:
                current = stack.pop()
                if current not in visited:
                    visited.add(current)
                    ring.append(current)
                    for neighbor in adjacency[current]:
                        if neighbor not in visited:
                            stack.append(neighbor)
            if len(ring) > 0:
                boundary_rings.append(ring)
    
    print(f"   发现 {len(boundary_rings)} 个边界环")
    
    if len(boundary_rings) == 0:
        return [], []
    elif len(boundary_rings) == 1:
        # 只有一个边界环，无法区分入口和出口，返回同一个中心
        ring = boundary_rings[0]
        center_x = sum(boundary_edges.GetPoint(i)[0] for i in ring) / len(ring)
        center_y = sum(boundary_edges.GetPoint(i)[1] for i in ring) / len(ring)
        center_z = sum(boundary_edges.GetPoint(i)[2] for i in ring) / len(ring)
        center_point = (center_x, center_y, center_z)
        print(f"   单一边界环中心点: ({center_point[0]:.2f}, {center_point[1]:.2f}, {center_point[2]:.2f})")
        return [center_point], [center_point]
    else:
        # 多个边界环，选择最大的两个作为入口和出口
        sorted_rings = sorted(boundary_rings, key=len, reverse=True)[:2]
        
        # 计算每个边界环的中心
        centers = []
        for ring in sorted_rings:
            center_x = sum(boundary_edges.GetPoint(i)[0] for i in ring) / len(ring)
            center_y = sum(boundary_edges.GetPoint(i)[1] for i in ring) / len(ring)
            center_z = sum(boundary_edges.GetPoint(i)[2] for i in ring) / len(ring)
            centers.append((center_x, center_y, center_z))
        
        entrance_center = centers[0]
        exit_center = centers[1] if len(centers) > 1 else centers[0]
        
        print(f"   入口端边界环中心: ({entrance_center[0]:.2f}, {entrance_center[1]:.2f}, {entrance_center[2]:.2f})")
        print(f"   出口端边界环中心: ({exit_center[0]:.2f}, {exit_center[1]:.2f}, {exit_center[2]:.2f})")
        
        return [entrance_center], [exit_center]

def find_closest_point_id_on_surface(surface, target_point):
    """找到表面中与目标点最近的点的ID"""
    points = surface.GetPoints()
    min_dist = float('inf')
    closest_id = 0
    
    for i in range(points.GetNumberOfPoints()):
        point = points.GetPoint(i)
        dist = np.sqrt(sum((point[j] - target_point[j])**2 for j in range(3)))
        if dist < min_dist:
            min_dist = dist
            closest_id = i
    
    return closest_id

def extend_centerline_to_boundaries(centerlines, entrance_center, exit_center):
    """将中心线的起点和终点直接连接到对应的边界环中心点"""
    
    print("   正在扩展中心线到边界环中心点...")
    
    if centerlines.GetNumberOfPoints() == 0:
        print("   中心线为空，无法扩展")
        return centerlines
    
    # 获取当前中心线的起点和终点
    original_first_point = centerlines.GetPoint(0)
    original_last_point = centerlines.GetPoint(centerlines.GetNumberOfPoints() - 1)
    
    # 计算距离以确定哪一端连接哪个边界中心
    dist_first_to_entrance = np.sqrt(sum((original_first_point[i] - entrance_center[i])**2 for i in range(3)))
    dist_first_to_exit = np.sqrt(sum((original_first_point[i] - exit_center[i])**2 for i in range(3)))
    dist_last_to_entrance = np.sqrt(sum((original_last_point[i] - entrance_center[i])**2 for i in range(3)))
    dist_last_to_exit = np.sqrt(sum((original_last_point[i] - exit_center[i])**2 for i in range(3)))
    
    # 确定连接关系：起点连接距离较近的边界中心，终点连接距离较近的边界中心
    if dist_first_to_entrance < dist_first_to_exit:
        # 起点连接入口，终点连接出口
        connection_entrance = entrance_center
        connection_exit = exit_center
    else:
        # 起点连接出口，终点连接入口
        connection_entrance = exit_center
        connection_exit = entrance_center
    
    print(f"   起点 ({original_first_point[0]:.2f}, {original_first_point[1]:.2f}, {original_first_point[2]:.2f}) "
          f"连接到边界中心 ({connection_entrance[0]:.2f}, {connection_entrance[1]:.2f}, {connection_entrance[2]:.2f})")
    print(f"   终点 ({original_last_point[0]:.2f}, {original_last_point[1]:.2f}, {original_last_point[2]:.2f}) "
          f"连接到边界中心 ({connection_exit[0]:.2f}, {connection_exit[1]:.2f}, {connection_exit[2]:.2f})")
    
    # 创建新的扩展中心线
    extended_points = vtk.vtkPoints()
    
    # 添加入口中心点
    extended_points.InsertNextPoint(connection_entrance)
    
    # 添加原始中心线点（不包括起点）
    for i in range(1, centerlines.GetNumberOfPoints()):
        point = centerlines.GetPoint(i)
        extended_points.InsertNextPoint(point)
    
    # 添加出口中心点
    extended_points.InsertNextPoint(connection_exit)
    
    # 创建线段
    lines = vtk.vtkCellArray()
    line = vtk.vtkPolyLine()
    line.GetPointIds().SetNumberOfIds(extended_points.GetNumberOfPoints())
    for i in range(extended_points.GetNumberOfPoints()):
        line.GetPointIds().SetId(i, i)
    lines.InsertNextCell(line)
    
    # 创建新的PolyData
    extended_centerlines = vtk.vtkPolyData()
    extended_centerlines.SetPoints(extended_points)
    extended_centerlines.SetLines(lines)
    
    print(f"   扩展后中心线点数: {extended_centerlines.GetNumberOfPoints()}")
    print(f"   扩展后中心线线段数: {extended_centerlines.GetNumberOfCells()}")
    
    return extended_centerlines

def create_seed_points_visualization_with_coordinates(surface, entrance_center, exit_center):
    """创建种子点的可视化模型，使用边界环中心坐标作为种子点"""
    
    # 创建一个点数据集来表示种子点
    seed_points = vtk.vtkPoints()
    seed_points.InsertNextPoint(entrance_center)
    seed_points.InsertNextPoint(exit_center)
    
    # 创建顶点单元
    vertices = vtk.vtkCellArray()
    for i in range(seed_points.GetNumberOfPoints()):
        vertex = vtk.vtkVertex()
        vertex.GetPointIds().SetId(0, i)
        vertices.InsertNextCell(vertex)
    
    # 创建PolyData
    seed_polydata = vtk.vtkPolyData()
    seed_polydata.SetPoints(seed_points)
    seed_polydata.SetVerts(vertices)
    
    # 添加颜色信息（区分源点和目标点）
    colors = vtk.vtkUnsignedCharArray()
    colors.SetNumberOfComponents(3)
    colors.SetName("Colors")
    
    # 源点用红色，目标点用蓝色
    colors.InsertNextTuple3(255, 0, 0)  # 红色 - 源点
    colors.InsertNextTuple3(0, 0, 255)  # 蓝色 - 目标点
    
    seed_polydata.GetPointData().SetScalars(colors)
    
    return seed_polydata

def create_all_boundary_points_visualization(surface):
    """创建所有边界点的可视化模型"""
    
    print("   创建所有边界点可视化...")
    
    # 使用vtkFeatureEdges来找到边界边
    feature_edges = vtk.vtkFeatureEdges()
    feature_edges.SetInputData(surface)
    feature_edges.BoundaryEdgesOn()
    feature_edges.FeatureEdgesOff()
    feature_edges.ManifoldEdgesOff()
    feature_edges.NonManifoldEdgesOff()
    feature_edges.Update()
    
    boundary_edges = feature_edges.GetOutput()
    
    if boundary_edges.GetNumberOfPoints() == 0:
        print("   没有找到边界点")
        return None
    
    # 创建边界点数据集
    boundary_points = vtk.vtkPoints()
    for i in range(boundary_edges.GetNumberOfPoints()):
        point = boundary_edges.GetPoints().GetPoint(i)
        boundary_points.InsertNextPoint(point)
    
    # 创建顶点单元
    vertices = vtk.vtkCellArray()
    for i in range(boundary_points.GetNumberOfPoints()):
        vertex = vtk.vtkVertex()
        vertex.GetPointIds().SetId(0, i)
        vertices.InsertNextCell(vertex)
    
    # 创建PolyData
    boundary_polydata = vtk.vtkPolyData()
    boundary_polydata.SetPoints(boundary_points)
    boundary_polydata.SetVerts(vertices)
    
    # 添加颜色信息（绿色）
    colors = vtk.vtkUnsignedCharArray()
    colors.SetNumberOfComponents(3)
    colors.SetName("Colors")
    
    # 所有边界点用绿色
    for i in range(boundary_points.GetNumberOfPoints()):
        colors.InsertNextTuple3(0, 255, 0)  # 绿色 - 边界点
    
    boundary_polydata.GetPointData().SetScalars(colors)
    
    print(f"   创建了 {boundary_points.GetNumberOfPoints()} 个边界点的可视化")
    
    return boundary_polydata

def create_boundary_centers_visualization(surface):
    """创建边界环中心点的可视化模型"""
    
    print("   创建边界环中心点可视化...")
    
    entrance_centers, exit_centers = find_boundary_centers(surface)
    
    if not entrance_centers and not exit_centers:
        print("   没有找到边界环中心点")
        return None
    
    # 创建边界中心点数据集
    centers_points = vtk.vtkPoints()
    
    # 添加入口中心点（黄色）
    if entrance_centers:
        for center in entrance_centers:
            centers_points.InsertNextPoint(center)
    
    # 添加出口中心点（紫色）
    if exit_centers:
        for center in exit_centers:
            centers_points.InsertNextPoint(center)
    
    if centers_points.GetNumberOfPoints() == 0:
        print("   没有边界环中心点")
        return None
    
    # 创建顶点单元
    vertices = vtk.vtkCellArray()
    for i in range(centers_points.GetNumberOfPoints()):
        vertex = vtk.vtkVertex()
        vertex.GetPointIds().SetId(0, i)
        vertices.InsertNextCell(vertex)
    
    # 创建PolyData
    centers_polydata = vtk.vtkPolyData()
    centers_polydata.SetPoints(centers_points)
    centers_polydata.SetVerts(vertices)
    
    # 添加颜色信息
    colors = vtk.vtkUnsignedCharArray()
    colors.SetNumberOfComponents(3)
    colors.SetName("Colors")
    
    # 入口中心点用黄色，出口中心点用紫色
    for i, center in enumerate(entrance_centers):
        colors.InsertNextTuple3(255, 255, 0)  # 黄色 - 入口中心点
    
    for i, center in enumerate(exit_centers):
        colors.InsertNextTuple3(128, 0, 128)  # 紫色 - 出口中心点
    
    centers_polydata.GetPointData().SetScalars(colors)
    
    print(f"   创建了 {len(entrance_centers)} 个入口中心点和 {len(exit_centers)} 个出口中心点的可视化")
    
    return centers_polydata

def extract_centerlines_from_stl(stl_filename):
    """从STL文件提取中心线"""
    
    print("=== 从STL文件提取 VMTK 中心线 (使用边界环中心坐标作为种子点，并扩展到边界) ===")
    
    # 创建输出目录
    output_dir = pathlib.Path("data/output-centerline")
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # 1. 加载STL表面
    surface = load_stl_surface(stl_filename)
    
    if surface is None or surface.GetNumberOfPoints() == 0:
        print("错误: 无法加载有效的表面数据")
        return False
    
    # 提取所有边界点并可视化
    all_boundary_points_viz = create_all_boundary_points_visualization(surface)
    
    # 提取边界环中心点并可视化
    boundary_centers_viz = create_boundary_centers_visualization(surface)
    
    # 2. 获取边界环中心坐标作为种子点
    print("2. 获取边界环中心坐标作为种子点...")
    
    entrance_centers, exit_centers = find_boundary_centers(surface)
    
    if not entrance_centers or not exit_centers:
        print("   错误: 无法找到边界环中心点")
        return False
    
    # 使用边界环中心坐标作为种子点
    source_point = entrance_centers[0]
    target_point = exit_centers[0] if len(exit_centers) > 0 else entrance_centers[0]
    
    # 找到表面上与边界环中心最近的点ID
    source_id = find_closest_point_id_on_surface(surface, source_point)
    target_id = find_closest_point_id_on_surface(surface, target_point)
    
    print(f"   使用边界环中心坐标作为种子点:")
    print(f"   源点坐标: ({source_point[0]:.2f}, {source_point[1]:.2f}, {source_point[2]:.2f})")
    print(f"   对应表面点ID: {source_id}")
    print(f"   目标点坐标: ({target_point[0]:.2f}, {target_point[1]:.2f}, {target_point[2]:.2f})")
    print(f"   对应表面点ID: {target_id}")
    
    # 3. 提取中心线
    print("3. 提取中心线...")
    
    try:
        # 创建中心线提取器
        centerlines_filter = vtkvmtk.vtkvmtkPolyDataCenterlines()
        centerlines_filter.SetInputData(surface)
        
        # 设置半径数组名称（这是关键！）
        centerlines_filter.SetRadiusArrayName("MaximumInscribedSphereRadius")
        
        # 设置种子点
        source_ids = vtk.vtkIdList()
        target_ids = vtk.vtkIdList()
        source_ids.InsertNextId(source_id)
        target_ids.InsertNextId(target_id)
        
        centerlines_filter.SetSourceSeedIds(source_ids)
        centerlines_filter.SetTargetSeedIds(target_ids)
        
        # 执行中心线提取
        print("   正在计算中心线...")
        centerlines_filter.Update()
        
        centerlines = centerlines_filter.GetOutput()
        
        print(f"   提取成功!")
        print(f"   中心线点数: {centerlines.GetNumberOfPoints()}")
        print(f"   中心线线段数: {centerlines.GetNumberOfCells()}")
        
        if centerlines.GetNumberOfPoints() == 0:
            print("   警告: 没有提取到中心线点!")
            return False
        
        # 检查是否只提取了一半（中心线点数远少于表面点数的一半可能表示不完整）
        if centerlines.GetNumberOfPoints() < surface.GetNumberOfPoints() * 0.1:
            print("   警告: 中心线点数较少，可能未完全提取")
        
        # 4. 扩展中心线到边界环中心点
        print("4. 扩展中心线到边界环中心点...")
        extended_centerlines = extend_centerline_to_boundaries(centerlines, source_point, target_point)
        
        # 5. 保存扩展后的中心线和表面到同一个文件
        print("5. 保存模型和扩展中心线到同一个文件...")
        
        # 生成输出文件名（写死的文件名）
        base_name = "vessel_centerline"
        
        # 保存单独的扩展中心线文件
        writer_vtp = vtk.vtkXMLPolyDataWriter()
        output_vtp = output_dir / f"{base_name}_extended_centerlines.vtp"
        writer_vtp.SetFileName(str(output_vtp))
        writer_vtp.SetInputData(extended_centerlines)
        writer_vtp.Write()
        
        # 保存单独的扩展中心线VTK文件
        writer_vtk = vtk.vtkPolyDataWriter()
        output_vtk = output_dir / f"{base_name}_extended_centerlines.vtk"
        writer_vtk.SetFileName(str(output_vtk))
        writer_vtk.SetInputData(extended_centerlines)
        writer_vtk.Write()
        
        # 合并表面和扩展中心线到同一个VTK文件
        append_filter = vtk.vtkAppendPolyData()
        append_filter.AddInputData(surface)
        append_filter.AddInputData(extended_centerlines)
        append_filter.Update()
        
        merged_output = append_filter.GetOutput()
        
        # 保存合并文件
        merged_writer = vtk.vtkPolyDataWriter()
        merged_filename = output_dir / f"{base_name}_surface_and_extended_centerlines.vtk"
        merged_writer.SetFileName(str(merged_filename))
        merged_writer.SetInputData(merged_output)
        merged_writer.Write()
        
        # 创建种子点可视化（使用边界环中心坐标）
        print("6. 创建种子点可视化...")
        seed_points_viz = create_seed_points_visualization_with_coordinates(surface, source_point, target_point)
        
        # 保存种子点可视化文件
        seed_writer = vtk.vtkPolyDataWriter()
        seed_filename = output_dir / f"{base_name}_seed_points.vtk"
        seed_writer.SetFileName(str(seed_filename))
        seed_writer.SetInputData(seed_points_viz)
        seed_writer.Write()
        
        # 保存所有边界点可视化文件
        if all_boundary_points_viz:
            boundary_writer = vtk.vtkPolyDataWriter()
            boundary_filename = output_dir / f"{base_name}_all_boundary_points.vtk"
            boundary_writer.SetFileName(str(boundary_filename))
            boundary_writer.SetInputData(all_boundary_points_viz)
            boundary_writer.Write()
            print(f"   所有边界点可视化文件已保存: {boundary_filename}")
        
        # 保存边界环中心点可视化文件
        if boundary_centers_viz:
            centers_writer = vtk.vtkPolyDataWriter()
            centers_filename = output_dir / f"{base_name}_boundary_centers.vtk"
            centers_writer.SetFileName(str(centers_filename))
            centers_writer.SetInputData(boundary_centers_viz)
            centers_writer.Write()
            print(f"   边界环中心点可视化文件已保存: {centers_filename}")
        
        # 创建包含表面、扩展中心线和种子点的完整文件
        complete_append = vtk.vtkAppendPolyData()
        complete_append.AddInputData(surface)
        complete_append.AddInputData(extended_centerlines)
        complete_append.AddInputData(seed_points_viz)
        if all_boundary_points_viz:
            complete_append.AddInputData(all_boundary_points_viz)
        if boundary_centers_viz:
            complete_append.AddInputData(boundary_centers_viz)
        complete_append.Update()
        
        complete_writer = vtk.vtkPolyDataWriter()
        complete_filename = output_dir / f"{base_name}_complete_model.vtk"
        complete_writer.SetFileName(str(complete_filename))
        complete_writer.SetInputData(complete_append.GetOutput())
        complete_writer.Write()
        
        print("   文件已保存:")
        print(f"     - {output_vtp} (扩展中心线，XML格式)")
        print(f"     - {output_vtk} (扩展中心线，传统格式)")
        print(f"     - {merged_filename} (表面和扩展中心线合并，VTK格式)")
        print(f"     - {seed_filename} (种子点可视化，VTK格式)")
        if all_boundary_points_viz:
            print(f"     - {boundary_filename} (所有边界点可视化，VTK格式)")
        if boundary_centers_viz:
            print(f"     - {centers_filename} (边界环中心点可视化，VTK格式)")
        print(f"     - {complete_filename} (完整模型：表面+扩展中心线+种子点+边界点+边界环中心点，VTK格式)")
        
        # 6. 输出详细信息
        print("7. 扩展中心线详细信息:")
        
        # 计算总长度
        total_length = 0.0
        for i in range(extended_centerlines.GetNumberOfCells()):
            cell = extended_centerlines.GetCell(i)
            if cell.GetCellType() == vtk.VTK_LINE:
                p1 = extended_centerlines.GetPoint(cell.GetPointId(0))
                p2 = extended_centerlines.GetPoint(cell.GetPointId(1))
                length = np.sqrt(sum((p1[j] - p2[j])**2 for j in range(3)))
                total_length += length
        
        print(f"   总长度: {total_length:.3f}")
        
        # 检查半径信息
        point_data = extended_centerlines.GetPointData()
        print(f"   点数据数组数: {point_data.GetNumberOfArrays()}")
        for i in range(point_data.GetNumberOfArrays()):
            array = point_data.GetArray(i)
            print(f"     数组 {i}: {array.GetName()}")
            if array.GetName() == "MaximumInscribedSphereRadius":
                print(f"       半径范围: [{array.GetRange()[0]:.3f}, {array.GetRange()[1]:.3f}]")
        
        # 额外检查：输出扩展中心线的边界框
        extended_centerlines_bounds = extended_centerlines.GetBounds()
        print(f"   扩展中心线边界框: [{extended_centerlines_bounds[0]:.2f}, {extended_centerlines_bounds[1]:.2f}, "
              f"{extended_centerlines_bounds[2]:.2f}, {extended_centerlines_bounds[3]:.2f}, "
              f"{extended_centerlines_bounds[4]:.2f}, {extended_centerlines_bounds[5]:.2f}]")
        
        print("\n可视化说明:")
        print(f"  - 红色点: 源点 - 血管入口 (坐标: {source_point[0]:.2f}, {source_point[1]:.2f}, {source_point[2]:.2f})")
        print(f"  - 蓝色点: 目标点 - 血管出口 (坐标: {target_point[0]:.2f}, {target_point[1]:.2f}, {target_point[2]:.2f})")
        print(f"  - 绿色点: 所有边界点")
        print(f"  - 黄色点: 入口端边界环中心点")
        print(f"  - 紫色点: 出口端边界环中心点")
        print(f"  - 可以在ParaView中打开 {seed_filename} 查看种子点位置")
        print(f"  - 可以在ParaView中打开 {boundary_filename} 查看所有边界点")
        print(f"  - 可以在ParaView中打开 {centers_filename} 查看边界环中心点")
        print(f"  - 或者打开 {complete_filename} 查看完整模型")
        
        # 检查扩展中心线完整性
        print("\n8. 检查扩展中心线完整性:")
        check_centerline_completeness(surface, extended_centerlines, source_point, target_point)
        
        return True
        
    except Exception as e:
        print(f"错误: {e}")
        import traceback
        traceback.print_exc()
        return False

def check_centerline_completeness(surface, centerlines, source_point, target_point):
    """检查中心线的完整性，确保从入口到出口完整连接"""
    
    if centerlines.GetNumberOfPoints() == 0:
        print("   中心线为空，无法检查完整性")
        return False
    
    # 获取中心线的起点和终点
    first_point = centerlines.GetPoint(0)
    last_point = centerlines.GetPoint(centerlines.GetNumberOfPoints() - 1)
    
    print(f"   扩展中心线起点: ({first_point[0]:.2f}, {first_point[1]:.2f}, {first_point[2]:.2f})")
    print(f"   扩展中心线终点: ({last_point[0]:.2f}, {last_point[1]:.2f}, {last_point[2]:.2f})")
    print(f"   设定的源点: ({source_point[0]:.2f}, {source_point[1]:.2f}, {source_point[2]:.2f})")
    print(f"   设定的目标点: ({target_point[0]:.2f}, {target_point[1]:.2f}, {target_point[2]:.2f})")
    
    # 计算距离
    dist_first_to_source = np.sqrt(sum((first_point[i] - source_point[i])**2 for i in range(3)))
    dist_last_to_target = np.sqrt(sum((last_point[i] - target_point[i])**2 for i in range(3)))
    dist_first_to_target = np.sqrt(sum((first_point[i] - target_point[i])**2 for i in range(3)))
    dist_last_to_source = np.sqrt(sum((last_point[i] - source_point[i])**2 for i in range(3)))
    
    print(f"   扩展中心线起点到源点距离: {dist_first_to_source:.3f}")
    print(f"   扩展中心线终点到目标点距离: {dist_last_to_target:.3f}")
    print(f"   扩展中心线起点到目标点距离: {dist_first_to_target:.3f}")
    print(f"   扩展中心线终点到源点距离: {dist_last_to_source:.3f}")
    
    # 检查是否正确连接
    tolerance = 5.0  # 距离容差，可根据需要调整
    correct_orientation = dist_first_to_source < tolerance and dist_last_to_target < tolerance
    reverse_orientation = dist_first_to_target < tolerance and dist_last_to_source < tolerance
    
    if correct_orientation:
        print(f"   ✓ 扩展中心线正确连接：起点接近源点，终点接近目标点")
        return True
    elif reverse_orientation:
        print(f"   ! 扩展中心线方向相反：起点接近目标点，终点接近源点")
        return True
    else:
        print(f"   ✗ 扩展中心线不完整：起点和终点都远离设定的源点和目标点")
        print(f"     建议重新检查种子点位置或STL文件完整性")
        return False

def save_centerlines_as_text(centerlines_filename, output_txt):
    """将中心线保存为文本格式"""
    try:
        # 尝试读取VTP文件，如果失败则尝试VTK文件
        reader = vtk.vtkXMLPolyDataReader()
        reader.SetFileName(centerlines_filename)
        reader.Update()
        
        if reader.GetOutput().GetNumberOfPoints() == 0:
            # 如果VTP读取失败，尝试VTK格式
            base_name = os.path.splitext(os.path.basename(centerlines_filename))[0]
            if base_name.endswith('_extended_centerlines'):
                vtk_filename = base_name + '.vtk'
            else:
                vtk_filename = centerlines_filename.replace('.vtp', '.vtk')
            
            vtk_reader = vtk.vtkPolyDataReader()
            vtk_reader.SetFileName(vtk_filename)
            vtk_reader.Update()
            centerlines = vtk_reader.GetOutput()
        else:
            centerlines = reader.GetOutput()
        
        with open(output_txt, "w") as f:
            f.write("# VMTK 中心线提取结果（扩展后）\n")
            f.write(f"# 源文件: {centerlines_filename}\n")
            f.write(f"# 点数: {centerlines.GetNumberOfPoints()}\n")
            f.write(f"# 线段数: {centerlines.GetNumberOfCells()}\n")
            f.write("# 格式: 点ID X Y Z [半径]\n")
            
            # 获取半径数组
            radius_array = centerlines.GetPointData().GetArray("MaximumInscribedSphereRadius")
            
            for i in range(centerlines.GetNumberOfPoints()):
                point = centerlines.GetPoint(i)
                if radius_array:
                    radius = radius_array.GetValue(i)
                    f.write(f"{i} {point[0]:.6f} {point[1]:.6f} {point[2]:.6f} {radius:.6f}\n")
                else:
                    f.write(f"{i} {point[0]:.6f} {point[1]:.6f} {point[2]:.6f}\n")
        
        print(f"   详细文本文件已保存: {output_txt}")
        
    except Exception as e:
        print(f"保存文本文件错误: {e}")

def main():
    # 从命令行参数获取STL文件名
    if len(sys.argv) < 2:
        print("错误: 请提供STL文件路径作为命令行参数")
        print("用法: python extractcenterline.py <stl_filename>")
        return
    
    stl_filename = sys.argv[1]  # 从命令行参数获取STL文件路径
    
    print(f"开始处理文件: {stl_filename}")
    
    success = extract_centerlines_from_stl(stl_filename)
    
    if success:
        print("\n=== 扩展中心线提取成功! ===")
        
        # 生成对应的输出文件名（写死的文件名）
        base_name = "vessel_centerline"
        output_dir = pathlib.Path("data/output-centerline")
        centerlines_vtp = output_dir / f"{base_name}_extended_centerlines.vtp"
        txt_filename = output_dir / f"{base_name}_extended_centerlines_detailed.txt"
        
        save_centerlines_as_text(str(centerlines_vtp), str(txt_filename))
        
    else:
        print("\n=== 扩展中心线提取失败 ===")
        print("可能的原因:")
        print("  1. STL文件损坏或格式不支持")
        print("  2. 模型不封闭（有孔洞）")
        print("  3. 模型太复杂或有自交")
        print("  4. VMTK库未正确安装")
        print("  5. 没有明显的边界点可用于确定入口和出口")

if __name__ == "__main__":
    main()




