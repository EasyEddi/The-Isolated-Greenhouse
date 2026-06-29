import os

import unreal


PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GREENHOUSE_FBX = os.path.join(PROJECT_ROOT, "models", "map", "Greenhouse", "greenhouse_5x3m.fbx")
DESTINATION_PATH = "/Game/models/map/Greenhouse"
MATERIAL_PATH = f"{DESTINATION_PATH}/Materials"
GREENHOUSE_MESH_PATH = f"{DESTINATION_PATH}/greenhouse_5x3m.greenhouse_5x3m"


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def set_property_if_available(obj, property_name, value):
    try:
        obj.set_editor_property(property_name, value)
        return True
    except Exception:
        return False


def material_asset_path(name):
    return f"{MATERIAL_PATH}/{name}"


def create_base_material(name, color, roughness=0.75, specular=0.25, opacity=None, two_sided=True):
    ensure_folder(MATERIAL_PATH)
    path = material_asset_path(name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        existing = unreal.EditorAssetLibrary.load_asset(path)
        unreal.EditorAssetLibrary.delete_loaded_asset(existing)

    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name,
        MATERIAL_PATH,
        unreal.Material,
        unreal.MaterialFactoryNew(),
    )
    material.set_editor_property("two_sided", two_sided)

    if opacity is not None:
        set_property_if_available(material, "blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
        set_property_if_available(material, "translucency_lighting_mode", unreal.TranslucencyLightingMode.TLM_SURFACE)

    color_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant3Vector, -420, 0
    )
    color_expr.set_editor_property("constant", unreal.LinearColor(*color))
    unreal.MaterialEditingLibrary.connect_material_property(
        color_expr, "", unreal.MaterialProperty.MP_BASE_COLOR
    )

    roughness_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -420, 170
    )
    roughness_expr.set_editor_property("r", roughness)
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness_expr, "", unreal.MaterialProperty.MP_ROUGHNESS
    )

    specular_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -420, 310
    )
    specular_expr.set_editor_property("r", specular)
    unreal.MaterialEditingLibrary.connect_material_property(
        specular_expr, "", unreal.MaterialProperty.MP_SPECULAR
    )

    if opacity is not None:
        opacity_expr = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionConstant, -420, 450
        )
        opacity_expr.set_editor_property("r", opacity)
        unreal.MaterialEditingLibrary.connect_material_property(
            opacity_expr, "", unreal.MaterialProperty.MP_OPACITY
        )

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
    return material


def import_greenhouse_mesh():
    if not os.path.exists(GREENHOUSE_FBX):
        raise RuntimeError(f"Missing source FBX: {GREENHOUSE_FBX}")

    ensure_folder(DESTINATION_PATH)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", GREENHOUSE_FBX)
    task.set_editor_property("destination_path", DESTINATION_PATH)
    task.set_editor_property("destination_name", "greenhouse_5x3m")
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)

    import_ui = unreal.FbxImportUI()
    set_property_if_available(import_ui, "import_mesh", True)
    set_property_if_available(import_ui, "import_as_skeletal", False)
    set_property_if_available(import_ui, "mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    set_property_if_available(import_ui, "automated_import_should_detect_type", False)
    set_property_if_available(import_ui, "import_materials", False)
    set_property_if_available(import_ui, "import_textures", False)

    if import_ui.static_mesh_import_data:
        data = import_ui.static_mesh_import_data
        set_property_if_available(data, "combine_meshes", True)
        set_property_if_available(data, "generate_lightmap_u_vs", True)
        set_property_if_available(data, "import_uniform_scale", 1.0)
        set_property_if_available(data, "normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS)
        set_property_if_available(data, "normal_generation_method", unreal.FBXNormalGenerationMethod.MIKK_T_SPACE)
        set_property_if_available(data, "use_mikk_t_space", False)
        set_property_if_available(data, "auto_generate_collision", False)

    task.set_editor_property("options", import_ui)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    mesh = unreal.EditorAssetLibrary.load_asset(GREENHOUSE_MESH_PATH)
    if isinstance(mesh, unreal.StaticMesh):
        return mesh

    for object_path in task.imported_object_paths:
        asset = unreal.EditorAssetLibrary.load_asset(object_path)
        if isinstance(asset, unreal.StaticMesh):
            return asset

    raise RuntimeError(f"Failed to import greenhouse mesh from {GREENHOUSE_FBX}")


def assign_clean_materials(mesh):
    materials = {
        "base": create_base_material("M_Greenhouse_Base_Clean", (0.10, 0.15, 0.11, 1.0), 0.82, 0.18),
        "frame": create_base_material("M_Greenhouse_Frame_Clean", (0.02, 0.18, 0.10, 1.0), 0.68, 0.30),
        "glass": create_base_material("M_Greenhouse_Glass_Clean", (0.58, 0.80, 0.86, 1.0), 0.04, 0.75, 0.34),
        "wood": create_base_material("M_Greenhouse_Wood_Clean", (0.34, 0.20, 0.11, 1.0), 0.88, 0.15),
        "handle": create_base_material("M_Greenhouse_Handle_Clean", (0.30, 0.29, 0.25, 1.0), 0.48, 0.45),
        "plant": create_base_material("M_Greenhouse_Plant_Spots_Clean", (0.13, 0.16, 0.10, 1.0), 0.92, 0.10),
    }

    for index, slot in enumerate(mesh.get_editor_property("static_materials")):
        slot_name = str(slot.material_slot_name).lower()
        if "glass" in slot_name:
            mesh.set_material(index, materials["glass"])
        elif "wood" in slot_name or "shel" in slot_name:
            mesh.set_material(index, materials["wood"])
        elif "handle" in slot_name:
            mesh.set_material(index, materials["handle"])
        elif "plant" in slot_name or "spot" in slot_name:
            mesh.set_material(index, materials["plant"])
        elif "frame" in slot_name:
            mesh.set_material(index, materials["frame"])
        else:
            mesh.set_material(index, materials["base"])


def fix_collision(mesh):
    body_setup = mesh.get_editor_property("body_setup")
    if body_setup:
        set_property_if_available(
            body_setup,
            "collision_trace_flag",
            unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE,
        )
    set_property_if_available(mesh, "allow_cpu_access", True)


def main():
    mesh = import_greenhouse_mesh()
    assign_clean_materials(mesh)
    fix_collision(mesh)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
    unreal.EditorAssetLibrary.save_directory(DESTINATION_PATH, only_if_is_dirty=False, recursive=True)
    unreal.log(f"Reimported clean greenhouse mesh and materials: {GREENHOUSE_MESH_PATH}")


if __name__ == "__main__":
    main()
