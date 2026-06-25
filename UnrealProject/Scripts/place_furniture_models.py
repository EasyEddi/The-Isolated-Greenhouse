import math

import unreal


PROJECT_MAP = "/Game/Maps/L_Greenhouse_MVP"
ACTOR_PREFIX = "MVP_Furniture_"


FURNITURE = [
    {
        "key": "bed",
        "asset": "/Game/models/furniture/Bed/bed",
        "label": "Living_Bed",
        "location": (-1380.0, -520.0, 0.0),
        "yaw": 90.0,
        "target_height": 90.0,
        "target_max_xy": 235.0,
    },
    {
        "key": "fridge",
        "asset": "/Game/models/furniture/fridge/fridge",
        "label": "Living_Fridge",
        "location": (-1570.0, 430.0, 0.0),
        "yaw": 90.0,
        "target_height": 190.0,
        "target_max_xy": 85.0,
    },
    {
        "key": "lower_kitchen_cabinet_01",
        "asset": "/Game/models/furniture/Lower_Kitchen_Cabinet/Lower_Kitchen_Cabinet",
        "label": "Living_Lower_Kitchen_Cabinet_01",
        "location": (-1540.0, 45.0, 0.0),
        "yaw": 90.0,
        "target_height": 92.0,
        "target_max_xy": 132.0,
    },
    {
        "key": "lower_kitchen_cabinet_02",
        "asset": "/Game/models/furniture/Lower_Kitchen_Cabinet/Lower_Kitchen_Cabinet",
        "label": "Living_Lower_Kitchen_Cabinet_02",
        "location": (-1540.0, -95.0, 0.0),
        "yaw": 90.0,
        "target_height": 92.0,
        "target_max_xy": 132.0,
    },
    {
        "key": "oven",
        "asset": "/Game/models/furniture/Oven/oven",
        "label": "Living_Oven",
        "location": (-1540.0, -235.0, 0.0),
        "yaw": 90.0,
        "target_height": 92.0,
        "target_max_xy": 92.0,
    },
    {
        "key": "microwave",
        "asset": "/Game/models/furniture/Microwave/microwave",
        "label": "Living_Microwave",
        "location": (-1540.0, 45.0, 112.0),
        "yaw": 90.0,
        "target_height": 42.0,
        "target_max_xy": 70.0,
        "snap_to_floor": False,
    },
    {
        "key": "desk_setup",
        "asset": "/Game/models/furniture/desk+setup/desk+setup",
        "label": "Desk_Office_Setup",
        "location": (-555.0, -790.0, 0.0),
        "yaw": 0.0,
        "target_height": 118.0,
        "target_max_xy": 260.0,
    },
    {
        "key": "storage_shelf_01",
        "asset": "/Game/models/furniture/Storage_Shelf/storge_shelf",
        "label": "Storage_Shelf_01",
        "location": (355.0, -285.0, 0.0),
        "yaw": 0.0,
        "target_height": 215.0,
        "target_max_xy": 230.0,
    },
    {
        "key": "storage_shelf_02",
        "asset": "/Game/models/furniture/Storage_Shelf/storge_shelf",
        "label": "Storage_Shelf_02",
        "location": (665.0, -285.0, 0.0),
        "yaw": 0.0,
        "target_height": 215.0,
        "target_max_xy": 230.0,
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


def yaw_rotation(yaw):
    rotation = unreal.Rotator()
    rotation.set_editor_property("pitch", 0.0)
    rotation.set_editor_property("yaw", yaw)
    rotation.set_editor_property("roll", 0.0)
    return rotation


def remove_previous_generated_furniture():
    removed = 0
    for actor in list(unreal.EditorLevelLibrary.get_all_level_actors()):
        if actor.get_actor_label().startswith(ACTOR_PREFIX):
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
