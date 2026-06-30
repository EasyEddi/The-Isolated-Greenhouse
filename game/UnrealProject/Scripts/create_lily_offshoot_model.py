import math
import os

import bpy
from mathutils import Vector


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUTPUTS = [
    os.path.join(PROJECT_ROOT, "models", "Plants", "LilyOffshoot", "lily_offshoot.fbx"),
    os.path.join(PROJECT_ROOT, "models", "Plants", "Lily", "offshoot", "lily_offshoot.fbx"),
    os.path.join(PROJECT_ROOT, "Content", "models", "Plants", "LilyOffshoot", "lily_offshoot.fbx"),
]


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def make_material(name, color, roughness=0.86):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    bsdf = material.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = color
    bsdf.inputs["Roughness"].default_value = roughness
    return material


def curve_to_mesh(obj):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.convert(target="MESH")
    return bpy.context.object


def create_stem_curve(name, points, radius, material):
    curve = bpy.data.curves.new(name, "CURVE")
    curve.dimensions = "3D"
    curve.resolution_u = 18
    curve.bevel_depth = radius
    curve.bevel_resolution = 5

    spline = curve.splines.new("BEZIER")
    spline.bezier_points.add(len(points) - 1)
    for point, co in zip(spline.bezier_points, points):
        point.co = Vector(co)
        point.handle_left_type = "AUTO"
        point.handle_right_type = "AUTO"

    obj = bpy.data.objects.new(name, curve)
    bpy.context.collection.objects.link(obj)
    obj.data.materials.append(material)
    mesh_obj = curve_to_mesh(obj)
    mesh_obj.name = name
    mesh_obj.data.name = name
    return mesh_obj


def create_cylinder(name, radius, depth, location, direction, material, vertices=24):
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=vertices,
        radius=radius,
        depth=depth,
        location=location,
    )
    obj = bpy.context.object
    obj.name = name
    obj.data.name = name
    obj.rotation_euler = Vector(direction).to_track_quat("Z", "Y").to_euler()
    obj.data.materials.append(material)
    return obj


def create_node(name, location, scale, material):
    bpy.ops.mesh.primitive_uv_sphere_add(
        segments=24,
        ring_count=12,
        radius=1.0,
        location=location,
    )
    obj = bpy.context.object
    obj.name = name
    obj.data.name = name
    obj.scale = scale
    obj.data.materials.append(material)
    return obj


def create_leaf(name, location, length, width, lift, angle_deg, pitch_deg, material):
    verts = [
        (0.0, 0.0, 0.0),
        (-width * 0.42, length * 0.22, lift * 0.12),
        (-width * 0.54, length * 0.48, lift * 0.38),
        (-width * 0.32, length * 0.75, lift * 0.72),
        (0.0, length, lift),
        (width * 0.32, length * 0.75, lift * 0.72),
        (width * 0.54, length * 0.48, lift * 0.38),
        (width * 0.42, length * 0.22, lift * 0.12),
    ]
    faces = [(0, 1, 2, 3, 4, 5, 6, 7)]
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, [], faces)
    mesh.update()

    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    obj.location = location
    obj.rotation_euler = (math.radians(pitch_deg), 0.0, math.radians(angle_deg))
    obj.data.materials.append(material)

    solidify = obj.modifiers.new(name="LeafThickness", type="SOLIDIFY")
    solidify.thickness = 0.32
    bevel = obj.modifiers.new(name="SoftLeafEdge", type="BEVEL")
    bevel.width = 0.11
    bevel.segments = 1

    return obj


def create_leaf_vein(name, location, length, lift, angle_deg, pitch_deg, material):
    curve = bpy.data.curves.new(name, "CURVE")
    curve.dimensions = "3D"
    curve.resolution_u = 4
    curve.bevel_depth = 0.08
    curve.bevel_resolution = 2

    spline = curve.splines.new("BEZIER")
    spline.bezier_points.add(2)
    coords = [
        (0.0, 1.2, 0.08),
        (0.0, length * 0.52, lift * 0.45),
        (0.0, length * 0.92, lift * 0.92),
    ]
    for point, co in zip(spline.bezier_points, coords):
        point.co = Vector(co)
        point.handle_left_type = "AUTO"
        point.handle_right_type = "AUTO"

    obj = bpy.data.objects.new(name, curve)
    bpy.context.collection.objects.link(obj)
    obj.location = location
    obj.rotation_euler = (math.radians(pitch_deg), 0.0, math.radians(angle_deg))
    obj.data.materials.append(material)
    mesh_obj = curve_to_mesh(obj)
    mesh_obj.name = name
    mesh_obj.data.name = name
    return mesh_obj


def shade_smooth(objects):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        try:
            bpy.ops.object.shade_smooth()
        finally:
            obj.select_set(False)


def apply_modifiers(objects):
    for obj in objects:
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        for modifier in list(obj.modifiers):
            bpy.ops.object.modifier_apply(modifier=modifier.name)
        obj.select_set(False)


def normalize_origin(objects):
    min_z = min((obj.matrix_world @ Vector(corner)).z for obj in objects for corner in obj.bound_box)
    for obj in objects:
        obj.location.z -= min_z

    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.context.scene.cursor.location = (0.0, 0.0, 0.0)
    bpy.ops.object.origin_set(type="ORIGIN_CURSOR", center="MEDIAN")


def build_model():
    stem = make_material("stem_green", (0.18, 0.36, 0.15, 1.0))
    node = make_material("node_deep_green", (0.10, 0.27, 0.10, 1.0))
    leaf = make_material("leaf_green", (0.13, 0.31, 0.12, 1.0))
    leaf_light = make_material("young_leaf_green", (0.30, 0.48, 0.18, 1.0))
    vein = make_material("leaf_vein", (0.07, 0.19, 0.08, 1.0))
    cut = make_material("cut_stem_pale", (0.62, 0.72, 0.43, 1.0))

    objects = []

    main_points = [
        (0.0, 0.0, 0.0),
        (4.5, 2.0, 11.0),
        (9.0, -1.5, 24.0),
        (17.0, 4.0, 39.0),
        (28.0, 2.0, 55.0),
    ]
    side_points = [
        (9.5, -1.1, 24.0),
        (-2.0, 9.0, 31.0),
        (-15.5, 14.0, 42.0),
    ]
    small_side_points = [
        (18.0, 3.8, 39.0),
        (25.5, -10.5, 44.0),
        (34.0, -14.0, 49.0),
    ]

    objects.append(create_stem_curve("LilyOffshoot_MainCutStem", main_points, 1.6, stem))
    objects.append(create_stem_curve("LilyOffshoot_SideStem_A", side_points, 1.08, stem))
    objects.append(create_stem_curve("LilyOffshoot_SideStem_B", small_side_points, 0.86, stem))

    objects.append(create_cylinder("LilyOffshoot_BaseCut", 2.05, 0.62, main_points[0], (1.0, 0.4, 2.2), cut))
    objects.append(create_cylinder("LilyOffshoot_TopCut", 1.72, 0.56, main_points[-1], (1.0, 0.0, 1.45), cut))
    objects.append(create_cylinder("LilyOffshoot_SideCut_A", 1.20, 0.48, side_points[-1], (-1.2, 0.3, 0.8), cut))
    objects.append(create_cylinder("LilyOffshoot_SideCut_B", 1.03, 0.44, small_side_points[-1], (0.8, -0.5, 0.35), cut))

    objects.append(create_node("LilyOffshoot_Node_Lower", (4.5, 2.0, 11.0), (2.0, 1.7, 1.35), node))
    objects.append(create_node("LilyOffshoot_Node_Middle", (9.5, -1.1, 24.0), (2.35, 1.85, 1.55), node))
    objects.append(create_node("LilyOffshoot_Node_Upper", (18.0, 3.8, 39.0), (2.15, 1.65, 1.38), node))

    leaves = [
        ("LilyOffshoot_Leaf_Lower_A", (3.2, 2.8, 9.6), 28.0, 6.2, 3.2, 126.0, -24.0, leaf),
        ("LilyOffshoot_Leaf_Lower_B", (5.0, 1.0, 12.2), 23.0, 5.2, 2.5, -62.0, -18.0, leaf_light),
        ("LilyOffshoot_Leaf_Middle_A", (9.0, -1.0, 24.0), 33.0, 7.3, 4.0, 62.0, -20.0, leaf),
        ("LilyOffshoot_Leaf_Middle_B", (8.4, -1.5, 25.2), 26.0, 5.8, 3.0, 205.0, -16.0, leaf),
        ("LilyOffshoot_Leaf_Upper_A", (17.0, 4.0, 39.4), 26.5, 5.7, 2.8, -24.0, -18.0, leaf_light),
        ("LilyOffshoot_Leaf_Side_A", (-5.0, 10.0, 33.6), 24.5, 5.5, 2.7, 154.0, -18.0, leaf),
    ]
    for leaf_spec in leaves:
        name, location, length, width, lift, angle, pitch, material = leaf_spec
        objects.append(create_leaf(name, location, length, width, lift, angle, pitch, material))
        objects.append(create_leaf_vein(f"{name}_Vein", location, length, lift, angle, pitch, vein))

    apply_modifiers(objects)
    shade_smooth(objects)
    normalize_origin(objects)
    return objects


def export_fbx(path):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    bpy.ops.export_scene.fbx(
        filepath=path,
        use_selection=False,
        apply_unit_scale=True,
        object_types={"MESH"},
        mesh_smooth_type="FACE",
        add_leaf_bones=False,
        bake_space_transform=False,
    )


def main():
    clear_scene()
    build_model()
    for output in OUTPUTS:
        export_fbx(output)


if __name__ == "__main__":
    main()
