import bpy
from mathutils import Vector


def world_bounding_box_center_bottom(objects):
    corners = []
    for obj in objects:
        if obj.type not in {"MESH", "CURVE", "SURFACE", "FONT", "META"}:
            continue
        corners.extend(obj.matrix_world @ Vector(corner) for corner in obj.bound_box)

    if not corners:
        raise RuntimeError("No selected object with a bounding box.")

    min_x = min(corner.x for corner in corners)
    max_x = max(corner.x for corner in corners)
    min_y = min(corner.y for corner in corners)
    max_y = max(corner.y for corner in corners)
    min_z = min(corner.z for corner in corners)

    return Vector(((min_x + max_x) * 0.5, (min_y + max_y) * 0.5, min_z))


def set_origin_to_world_point(obj, origin_world):
    cursor = bpy.context.scene.cursor
    old_cursor_location = cursor.location.copy()
    old_active = bpy.context.view_layer.objects.active
    selected = list(bpy.context.selected_objects)

    try:
        bpy.ops.object.select_all(action="DESELECT")
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        cursor.location = origin_world
        bpy.ops.object.origin_set(type="ORIGIN_CURSOR", center="MEDIAN")
    finally:
        cursor.location = old_cursor_location
        bpy.ops.object.select_all(action="DESELECT")
        for selected_obj in selected:
            selected_obj.select_set(True)
        bpy.context.view_layer.objects.active = old_active


def main():
    selected_objects = list(bpy.context.selected_objects)
    if not selected_objects:
        raise RuntimeError("Select one or more objects first.")

    origin_world = world_bounding_box_center_bottom(selected_objects)
    for obj in selected_objects:
        set_origin_to_world_point(obj, origin_world)

    print(f"Set origin of {len(selected_objects)} object(s) to bottom center: {origin_world}")


if __name__ == "__main__":
    main()
