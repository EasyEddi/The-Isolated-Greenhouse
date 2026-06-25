import math
import os

import unreal


PROJECT_MAP = "/Game/Maps/L_Greenhouse_MVP"
ACTOR_PREFIX = "MVP_Furniture_"
BED_LABEL = ACTOR_PREFIX + "Living_Bed"
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BRICK_WALL_SOURCE = os.path.join(PROJECT_ROOT, "SourceTextures", "Walls", "brick_wall.png")

HALL_LENGTH = 3000.0
HALL_WIDTH = 2000.0
WALL_HEIGHT = 600.0
WALL_THICKNESS = 120.0
FLOOR_THICKNESS = 20.0


FURNITURE = [
    {
        "key": "fridge",
        "asset": "/Game/models/furniture/fridge/fridge",
        "label": "Living_Fridge",
        "location": (-1285.0, -545.0, 0.0),
        "yaw": 0.0,
        "target_height": 266.0,
        "target_max_xy": 119.0,
    },
    {
        "key": "lower_kitchen_cabinet_01",
        "asset": "/Game/models/furniture/Lower_Kitchen_Cabinet/Lower_Kitchen_Cabinet",
        "label": "Living_Lower_Kitchen_Cabinet_01",
        "location": (-1285.0, -295.0, 0.0),
        "yaw": 0.0,
        "target_height": 129.0,
        "target_max_xy": 185.0,
    },
    {
        "key": "lower_kitchen_cabinet_02",
        "asset": "/Game/models/furniture/Lower_Kitchen_Cabinet/Lower_Kitchen_Cabinet",
        "label": "Living_Lower_Kitchen_Cabinet_02",
        "location": (-1285.0, -115.0, 0.0),
        "yaw": 0.0,
        "target_height": 129.0,
        "target_max_xy": 185.0,
    },
    {
        "key": "oven",
        "asset": "/Game/models/furniture/Oven/oven",
        "label": "Living_Oven",
        "location": (-1285.0, 65.0, 0.0),
        "yaw": 0.0,
        "target_height": 129.0,
        "target_max_xy": 129.0,
    },
    {
        "key": "microwave",
        "asset": "/Game/models/furniture/Microwave/microwave",
        "label": "Living_Microwave",
        "location": (-1285.0, -295.0, 157.0),
        "yaw": 0.0,
        "target_height": 59.0,
        "target_max_xy": 98.0,
        "snap_to_floor": False,
    },
    {
        "key": "desk_setup",
        "asset": "/Game/models/furniture/desk+setup/desk+setup",
        "label": "Desk_Office_Setup",
        "location": (-430.0, 720.0, 0.0),
        "yaw": 0.0,
        "target_height": 165.0,
        "target_max_xy": 364.0,
    },
    {
        "key": "storage_shelf_01",
        "asset": "/Game/models/furniture/Storage_Shelf/storge_shelf",
        "label": "Storage_Shelf_01",
        "location": (260.0, 360.0, 0.0),
        "yaw": 90.0,
        "target_height": 236.5,
        "target_max_xy": 253.0,
    },
]


def load_mesh(path):
    mesh = unreal.EditorAssetLibrary.load_asset(path)
    if not mesh:
        raise RuntimeError(f"Missing mesh asset: {path}")
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"Asset is not a StaticMesh: {path}")
    return mesh


def mesh_size(mesh):
    bounds = mesh.get_bounds()
    extent = bounds.box_extent
    return extent.x * 2.0, extent.y * 2.0, extent.z * 2.0


def scale_for_spec(mesh, spec):
    size_x, size_y, size_z = mesh_size(mesh)
    candidates = []

    if spec.get("target_height") and size_z > 0.0:
        candidates.append(spec["target_height"] / size_z)

    max_xy = max(size_x, size_y)
    if spec.get("target_max_xy") and max_xy > 0.0:
        candidates.append(spec["target_max_xy"] / max_xy)

    if not candidates:
        return spec.get("scale", 1.0)

    return min(candidates)


def snap_actor_to_floor(actor, floor_z=0.0):
    origin, extent = actor.get_actor_bounds(False)
    bottom_z = origin.z - extent.z
    actor.add_actor_world_offset(unreal.Vector(0.0, 0.0, floor_z - bottom_z), False, False)


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def set_editor_property_if_available(obj, property_name, value):
    try:
        obj.set_editor_property(property_name, value)
    except Exception:
        pass


def import_source_texture(name, source_path):
    ensure_folder("/Game/Art/Textures")
    if not os.path.exists(source_path):
        raise RuntimeError(f"Missing source texture: {source_path}")

    destination_path = f"/Game/Art/Textures/{name}"
    texture = unreal.EditorAssetLibrary.load_asset(destination_path)
    if not texture:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", source_path)
        task.set_editor_property("destination_path", "/Game/Art/Textures")
        task.set_editor_property("destination_name", name)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        texture = unreal.EditorAssetLibrary.load_asset(destination_path)

    if not texture:
        raise RuntimeError(f"Failed to import texture: {source_path}")

    set_editor_property_if_available(texture, "address_x", unreal.TextureAddress.TA_MIRROR)
    set_editor_property_if_available(texture, "address_y", unreal.TextureAddress.TA_CLAMP)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    return texture


def make_textured_material(name, texture, roughness, u_tiling, v_tiling):
    ensure_folder("/Game/Art/Materials")
    path = f"/Game/Art/Materials/{name}"
    existing = unreal.EditorAssetLibrary.load_asset(path)
    if existing:
        unreal.EditorAssetLibrary.delete_loaded_asset(existing)

    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name,
        "/Game/Art/Materials",
        unreal.Material,
        unreal.MaterialFactoryNew(),
    )

    coordinate_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureCoordinate, -680, 0
    )
    coordinate_expr.set_editor_property("u_tiling", u_tiling)
    coordinate_expr.set_editor_property("v_tiling", v_tiling)

    texture_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -450, 0
    )
    texture_expr.set_editor_property("texture", texture)
    unreal.MaterialEditingLibrary.connect_material_expressions(coordinate_expr, "", texture_expr, "UVs")
    unreal.MaterialEditingLibrary.connect_material_property(
        texture_expr,
        "",
        unreal.MaterialProperty.MP_BASE_COLOR,
    )

    roughness_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -400, 160
    )
    roughness_expr.set_editor_property("r", roughness)
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness_expr,
        "",
        unreal.MaterialProperty.MP_ROUGHNESS,
    )

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def resize_actor(label, location, scale, material=None):
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if actor.get_actor_label() != label:
            continue

        resize_static_mesh_actor(actor, location, scale, material)
        return True

    unreal.log_warning(f"Hall actor not found for resize: {label}")
    return False


def resize_static_mesh_actor(actor, location, scale, material=None):
    actor.set_actor_location(unreal.Vector(*location), False, False)
    actor.set_actor_scale3d(unreal.Vector(*scale))

    if material:
        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        if component:
            component.set_material(0, material)


def get_component_material_names(component):
    names = []
    if not component:
        return names

    for index in range(component.get_num_materials()):
        material = component.get_material(index)
        if material:
            names.append(material.get_path_name().lower())
    return names


def find_floor_actor():
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    label_matches = ("hall_rubber_floor", "hall_floor", "floor")

    for actor in actors:
        if actor.get_actor_label().lower() in label_matches:
            return actor

    for actor in actors:
        label = actor.get_actor_label().lower()
        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        material_names = get_component_material_names(component)
        if "floor" in label or any("hall_rubber_floor" in name for name in material_names):
            return actor

    for actor in actors:
        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        if not component:
            continue

        scale = actor.get_actor_scale3d()
        if scale.x >= 20.0 and scale.y >= 15.0 and scale.z <= 0.5:
            return actor

    return None


def resize_floor():
    floor = find_floor_actor()
    if not floor:
        unreal.log_warning("Hall floor actor not found for resize")
        return False

    resize_static_mesh_actor(
        floor,
        (0.0, 0.0, -FLOOR_THICKNESS / 2.0),
        (HALL_LENGTH / 100.0, HALL_WIDTH / 100.0, FLOOR_THICKNESS / 100.0),
    )
    unreal.log(f"Resized hall floor actor: {floor.get_actor_label()}")
    return True


def resize_hall_shell():
    texture = import_source_texture("T_Hall_Brick_Wall", BRICK_WALL_SOURCE)
    short_material = make_textured_material(
        "M_Hall_Brick_Wall_Short",
        texture,
        0.92,
        HALL_WIDTH / WALL_HEIGHT,
        1.0,
    )
    long_material = make_textured_material(
        "M_Hall_Brick_Wall_Long",
        texture,
        0.92,
        HALL_LENGTH / WALL_HEIGHT,
        1.0,
    )

    half_length = HALL_LENGTH / 2.0
    half_width = HALL_WIDTH / 2.0
    half_wall = WALL_THICKNESS / 2.0

    resized = 0
    resized += int(resize_floor())
    resized += int(resize_actor(
        "Hall_Back_wall_damaged_brick",
        (-half_length - half_wall, 0.0, WALL_HEIGHT / 2.0),
        (WALL_THICKNESS / 100.0, HALL_WIDTH / 100.0, WALL_HEIGHT / 100.0),
        short_material,
    ))
    resized += int(resize_actor(
        "Hall_Front_wall_damaged_brick",
        (half_length + half_wall, 0.0, WALL_HEIGHT / 2.0),
        (WALL_THICKNESS / 100.0, HALL_WIDTH / 100.0, WALL_HEIGHT / 100.0),
        short_material,
    ))
    resized += int(resize_actor(
        "Hall_Left_wall_damaged_brick",
        (0.0, -half_width - half_wall, WALL_HEIGHT / 2.0),
        (HALL_LENGTH / 100.0, WALL_THICKNESS / 100.0, WALL_HEIGHT / 100.0),
        long_material,
    ))
    resized += int(resize_actor(
        "Hall_Right_wall_damaged_brick",
        (0.0, half_width + half_wall, WALL_HEIGHT / 2.0),
        (HALL_LENGTH / 100.0, WALL_THICKNESS / 100.0, WALL_HEIGHT / 100.0),
        long_material,
    ))

    unreal.log(f"Resized hall shell actors: {resized}")


def yaw_rotation(yaw):
    rotation = unreal.Rotator()
    rotation.set_editor_property("pitch", 0.0)
    rotation.set_editor_property("yaw", yaw)
    rotation.set_editor_property("roll", 0.0)
    return rotation


def remove_previous_generated_furniture():
    removed = 0
    for actor in list(unreal.EditorLevelLibrary.get_all_level_actors()):
        if actor.get_actor_label().startswith(ACTOR_PREFIX) and actor.get_actor_label() != BED_LABEL:
            unreal.EditorLevelLibrary.destroy_actor(actor)
            removed += 1
    return removed


def place_actor(spec):
    mesh = load_mesh(spec["asset"])
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*spec["location"]),
        yaw_rotation(spec["yaw"]),
    )
    actor.set_actor_label(ACTOR_PREFIX + spec["label"])

    scale = scale_for_spec(mesh, spec)
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))

    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    component.set_static_mesh(mesh)
    component.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    component.set_collision_profile_name("BlockAll")
    actor.set_actor_enable_collision(True)

    if spec.get("snap_to_floor", True):
        snap_actor_to_floor(actor)

    origin, extent = actor.get_actor_bounds(False)
    unreal.log(
        f"PLACED_FURNITURE {actor.get_actor_label()} scale={scale:.5f} "
        f"bounds_cm=({extent.x * 2.0:.1f},{extent.y * 2.0:.1f},{extent.z * 2.0:.1f}) "
        f"loc=({actor.get_actor_location().x:.1f},{actor.get_actor_location().y:.1f},{actor.get_actor_location().z:.1f})"
    )
    return actor


def main():
    unreal.EditorLevelLibrary.load_level(PROJECT_MAP)

    resize_hall_shell()
    removed = remove_previous_generated_furniture()

    placed = 0
    for spec in FURNITURE:
        place_actor(spec)
        placed += 1

    actor_count = len(unreal.EditorLevelLibrary.get_all_level_actors())
    unreal.log(f"Removed previous generated furniture actors: {removed}")
    unreal.log(f"Placed furniture actors: {placed}")
    unreal.log(f"Map actor count after furniture placement: {actor_count}")

    unreal.EditorLevelLibrary.save_current_level()
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


if __name__ == "__main__":
    main()
