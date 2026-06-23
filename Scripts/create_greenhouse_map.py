import unreal


PROJECT_MAP = "/Game/Maps/L_Greenhouse_MVP"
CUBE = "/Engine/BasicShapes/Cube.Cube"


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def make_material(name, color, roughness=0.85, texture_strength=0.0, noise_scale=18.0):
    ensure_folder("/Game/Art/Materials")
    path = f"/Game/Art/Materials/{name}"
    existing = unreal.EditorAssetLibrary.load_asset(path)
    if existing:
        unreal.EditorAssetLibrary.delete_loaded_asset(existing)

    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(name, "/Game/Art/Materials", unreal.Material, unreal.MaterialFactoryNew())

    color_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant3Vector, -400, 0
    )
    color_expr.set_editor_property("constant", unreal.LinearColor(*color))

    if texture_strength > 0:
        noise_expr = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionNoise, -650, 120
        )
        noise_expr.set_editor_property("scale", noise_scale)
        noise_expr.set_editor_property("levels", 5)

        strength_expr = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionConstant, -650, 280
        )
        strength_expr.set_editor_property("r", texture_strength)

        multiply_expr = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionMultiply, -150, 20
        )
        unreal.MaterialEditingLibrary.connect_material_expressions(color_expr, "", multiply_expr, "A")
        unreal.MaterialEditingLibrary.connect_material_expressions(noise_expr, "", multiply_expr, "B")
        unreal.MaterialEditingLibrary.connect_material_property(
            multiply_expr, "", unreal.MaterialProperty.MP_BASE_COLOR
        )
    else:
        unreal.MaterialEditingLibrary.connect_material_property(
            color_expr, "", unreal.MaterialProperty.MP_BASE_COLOR
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
        "Hall_Floor_concrete_slab",
        (0, 0, -floor_thickness / 2),
        (hall_length / 100, hall_width / 100, floor_thickness / 100),
        "floor",
    )

    cube(
        "Hall_Back_wall_plaster",
        (-hall_length / 2, 0, wall_height / 2),
        (wall_thickness / 100, hall_width / 100, wall_height / 100),
        "wall",
    )
    cube(
        "Hall_Front_wall_plaster",
        (hall_length / 2, 0, wall_height / 2),
        (wall_thickness / 100, hall_width / 100, wall_height / 100),
        "wall",
    )
    cube(
        "Hall_Left_wall_plaster",
        (0, -hall_width / 2, wall_height / 2),
        (hall_length / 100, wall_thickness / 100, wall_height / 100),
        "wall",
    )
    cube(
        "Hall_Right_wall_plaster",
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
        "floor": make_material("M_Hall_Concrete_Floor", (0.55, 0.52, 0.46, 1), 0.94, 1.65, 26.0),
        "wall": make_material("M_Hall_Warm_Plaster_Wall", (0.80, 0.77, 0.68, 1), 0.9, 1.25, 38.0),
    }

    add_rectangular_hall()
    add_daylight()
    set_map_game_mode()

    unreal.EditorLevelLibrary.save_current_level()
    unreal.EditorAssetLibrary.save_directory("/Game/Art", only_if_is_dirty=False, recursive=True)
    unreal.log("Created playable hall map: textured floor/walls, daylight, spawn, and walking GameMode.")


if __name__ == "__main__":
    main()
