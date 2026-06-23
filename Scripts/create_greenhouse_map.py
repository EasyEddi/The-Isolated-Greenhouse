import math
import os
import random
import struct
import zlib

import unreal


PROJECT_MAP = "/Game/Maps/L_Greenhouse_MVP"
CUBE = "/Engine/BasicShapes/Cube.Cube"
GENERATED_TEXTURE_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "Intermediate",
    "GeneratedTextures",
)


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def write_png(path, width, height, pixels):
    def chunk(tag, data):
        return (
            struct.pack(">I", len(data))
            + tag
            + data
            + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
        )

    raw_rows = []
    for y in range(height):
        row = bytearray([0])
        for x in range(width):
            row.extend(pixels[y * width + x])
        raw_rows.append(bytes(row))

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as handle:
        handle.write(b"\x89PNG\r\n\x1a\n")
        handle.write(chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)))
        handle.write(chunk(b"IDAT", zlib.compress(b"".join(raw_rows), 9)))
        handle.write(chunk(b"IEND", b""))


def clamp_channel(value):
    return max(0, min(255, int(value)))


def make_rubber_floor_png(path, size=768):
    rng = random.Random(42)
    pixels = []
    for y in range(size):
        for x in range(size):
            seam = min(x % 192, y % 192)
            seam_line = 22 if seam < 3 else 0
            scuff = 10 * math.sin((x * 0.071) + (y * 0.037)) + 6 * math.sin((x - y) * 0.019)
            speck = rng.randint(-10, 12)
            base = 55 + scuff + speck - seam_line
            pixels.append(
                (
                    clamp_channel(base),
                    clamp_channel(base + 4),
                    clamp_channel(base + 3),
                    255,
                )
            )
    write_png(path, size, size, pixels)


def make_damaged_brick_png(path, size=1024):
    rng = random.Random(73)
    brick_w = 160
    brick_h = 70
    mortar = 8
    damage_centers = [(210, 240, 130), (760, 350, 105), (510, 720, 150), (875, 825, 90)]
    pixels = []

    for y in range(size):
        row = y // brick_h
        offset = (brick_w // 2) if row % 2 else 0
        for x in range(size):
            local_x = (x + offset) % brick_w
            local_y = y % brick_h
            is_mortar = local_x < mortar or local_y < mortar

            chip_strength = 0.0
            for cx, cy, radius in damage_centers:
                distance = math.hypot(x - cx, y - cy)
                if distance < radius:
                    edge_noise = 0.72 + 0.22 * math.sin(x * 0.081) + 0.18 * math.sin(y * 0.067)
                    chip_strength = max(chip_strength, max(0.0, (1.0 - distance / radius) * edge_noise))

            if is_mortar:
                base = 88 + rng.randint(-10, 8)
                color = (base, base - 2, base - 7)
            else:
                variation = 18 * math.sin((x // 9) * 0.8) + 10 * math.sin((y // 11) * 0.7) + rng.randint(-14, 14)
                color = (145 + variation, 68 + variation * 0.35, 45 + variation * 0.25)

            if chip_strength > 0.18 and not is_mortar:
                plaster = 150 + rng.randint(-18, 20)
                color = (
                    color[0] * (1 - chip_strength) + plaster * chip_strength,
                    color[1] * (1 - chip_strength) + (plaster - 8) * chip_strength,
                    color[2] * (1 - chip_strength) + (plaster - 18) * chip_strength,
                )

            pixels.append(
                (
                    clamp_channel(color[0]),
                    clamp_channel(color[1]),
                    clamp_channel(color[2]),
                    255,
                )
            )

    write_png(path, size, size, pixels)


def import_texture(name, generator):
    ensure_folder("/Game/Art/Textures")
    source_path = os.path.join(GENERATED_TEXTURE_DIR, f"{name}.png")
    generator(source_path)

    destination_path = f"/Game/Art/Textures/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(destination_path):
        unreal.EditorAssetLibrary.delete_asset(destination_path)

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
    return texture


def make_textured_material(name, texture, roughness=0.85):
    ensure_folder("/Game/Art/Materials")
    path = f"/Game/Art/Materials/{name}"
    existing = unreal.EditorAssetLibrary.load_asset(path)
    if existing:
        unreal.EditorAssetLibrary.delete_loaded_asset(existing)

    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(name, "/Game/Art/Materials", unreal.Material, unreal.MaterialFactoryNew())

    texture_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -450, 0
    )
    texture_expr.set_editor_property("texture", texture)
    unreal.MaterialEditingLibrary.connect_material_property(
        texture_expr, "", unreal.MaterialProperty.MP_BASE_COLOR
    )

    roughness_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -400, 160
    )
    roughness_expr.set_editor_property("r", roughness)
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness_expr, "", unreal.MaterialProperty.MP_ROUGHNESS
    )

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


MATERIALS = {}


def material(name):
    return MATERIALS[name]


def cube(label, location, scale, mat_name):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(0, 0, 0),
    )
    actor.set_actor_label(label)
    actor.set_actor_scale3d(unreal.Vector(*scale))
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_static_mesh(load_asset(CUBE))
    comp.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    comp.set_material(0, material(mat_name))
    return actor


def set_map_game_mode():
    game_mode_class = unreal.EditorAssetLibrary.load_blueprint_class(
        "/Game/FirstPerson/Blueprints/BP_FirstPersonGameMode"
    )
    if not game_mode_class:
        unreal.log_warning("First-person GameMode not found; map will still open, but PIE may use the default pawn.")
        return

    world = unreal.EditorLevelLibrary.get_editor_world()
    world_settings = world.get_world_settings()
    world_settings.set_editor_property("default_game_mode", game_mode_class)


def add_rectangular_hall():
    # Unreal cube base size is 100cm. This hall is 24m x 16m with 4m high walls.
    hall_length = 2400
    hall_width = 1600
    wall_thickness = 30
    wall_height = 400
    floor_thickness = 20

    cube(
        "Hall_Floor_rubber_surface",
        (0, 0, -floor_thickness / 2),
        (hall_length / 100, hall_width / 100, floor_thickness / 100),
        "floor",
    )

    cube(
        "Hall_Back_wall_damaged_brick",
        (-hall_length / 2, 0, wall_height / 2),
        (wall_thickness / 100, hall_width / 100, wall_height / 100),
        "wall",
    )
    cube(
        "Hall_Front_wall_damaged_brick",
        (hall_length / 2, 0, wall_height / 2),
        (wall_thickness / 100, hall_width / 100, wall_height / 100),
        "wall",
    )
    cube(
        "Hall_Left_wall_damaged_brick",
        (0, -hall_width / 2, wall_height / 2),
        (hall_length / 100, wall_thickness / 100, wall_height / 100),
        "wall",
    )
    cube(
        "Hall_Right_wall_damaged_brick",
        (0, hall_width / 2, wall_height / 2),
        (hall_length / 100, wall_thickness / 100, wall_height / 100),
        "wall",
    )


def add_daylight():
    atmosphere = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyAtmosphere,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0),
    )
    atmosphere.set_actor_label("Lighting_Daytime_SkyAtmosphere")

    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 600),
        unreal.Rotator(0, 0, 0),
    )
    sky.set_actor_label("Lighting_Daytime_SkyLight")
    sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_comp.set_editor_property("intensity", 1.4)

    sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 800),
        unreal.Rotator(-45, -35, 0),
    )
    sun.set_actor_label("Lighting_Sun_Daylight")
    sun_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_comp.set_editor_property("intensity", 6.0)
    sun_comp.set_editor_property("light_color", unreal.Color(255, 246, 226, 255))

    start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(-520, 65, 95),
        unreal.Rotator(0, 0, 0),
    )
    start.set_actor_label("PlayerStart_Main_Hall")


def main():
    global MATERIALS

    ensure_folder("/Game/Maps")
    ensure_folder("/Game/Art")

    if unreal.EditorAssetLibrary.does_asset_exist(PROJECT_MAP):
        unreal.EditorLevelLibrary.load_level(PROJECT_MAP)
    else:
        unreal.EditorLevelLibrary.new_level(PROJECT_MAP)

    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        unreal.EditorLevelLibrary.destroy_actor(actor)

    MATERIALS = {
        "floor": make_textured_material(
            "M_Hall_Rubber_Floor",
            import_texture("T_Hall_Rubber_Floor", make_rubber_floor_png),
            0.96,
        ),
        "wall": make_textured_material(
            "M_Hall_Damaged_Brick_Wall",
            import_texture("T_Hall_Damaged_Brick_Wall", make_damaged_brick_png),
            0.91,
        ),
    }

    add_rectangular_hall()
    add_daylight()
    set_map_game_mode()

    unreal.EditorLevelLibrary.save_current_level()
    unreal.EditorAssetLibrary.save_directory("/Game/Art", only_if_is_dirty=False, recursive=True)
    unreal.log("Created playable hall map: textured floor/walls, daylight, spawn, and walking GameMode.")


if __name__ == "__main__":
    main()
