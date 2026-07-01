import unreal


DESTINATION_PATH = "/Game/models/furniture/Storage_Shelf"
MATERIAL_PATH = DESTINATION_PATH
SHELF_MESH_PATH = f"{DESTINATION_PATH}/storage_shelf.storage_shelf"


MATERIAL_SPECS = {
    "dark_metal": ("Storage_Shelf_Dark_Metal", (0.055, 0.055, 0.050, 1.0), 0.58, 0.48),
    "wood": ("Storage_Shelf_Wooden_Boards", (0.360, 0.235, 0.135, 1.0), 0.82, 0.16),
    "tool": ("Storage_Shelf_Slot_Tool", (0.075, 0.125, 0.150, 1.0), 0.78, 0.20),
    "pot": ("Storage_Shelf_Slot_Pot", (0.430, 0.195, 0.115, 1.0), 0.84, 0.12),
    "soil": ("Storage_Shelf_Slot_Soil_Bag", (0.145, 0.090, 0.060, 1.0), 0.92, 0.08),
    "fertilizer": ("Storage_Shelf_Slot_Fertilizer", (0.115, 0.220, 0.120, 1.0), 0.88, 0.10),
}


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


def create_clean_material(name, color, roughness, specular):
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
    material.set_editor_property("two_sided", True)

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

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
    return material


def build_materials():
    materials = {}
    for key, spec in MATERIAL_SPECS.items():
        materials[key] = create_clean_material(*spec)
    return materials


def material_for_slot(slot_name, materials):
    name = slot_name.lower()
    if "tool" in name:
        return materials["tool"]
    if "pot" in name:
        return materials["pot"]
    if "soil" in name:
        return materials["soil"]
    if "fertilizer" in name:
        return materials["fertilizer"]
    if "metal" in name or "frame" in name or "dark" in name:
        return materials["dark_metal"]
    if "wood" in name or "board" in name or "shelf" in name:
        return materials["wood"]
    return materials["wood"]


def assign_clean_materials(mesh, materials):
    static_materials = mesh.get_editor_property("static_materials")
    for index, slot in enumerate(static_materials):
        slot_name = str(slot.material_slot_name)
        material = material_for_slot(slot_name, materials)
        mesh.set_material(index, material)
        unreal.log(f"STORAGE_SHELF_SLOT {index}: {slot_name} -> {material.get_name()}")


def main():
    mesh = unreal.EditorAssetLibrary.load_asset(SHELF_MESH_PATH)
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"Missing StaticMesh: {SHELF_MESH_PATH}")

    materials = build_materials()
    assign_clean_materials(mesh, materials)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
    unreal.EditorAssetLibrary.save_directory(DESTINATION_PATH, only_if_is_dirty=False, recursive=True)
    unreal.log(f"Rebuilt clean storage shelf materials: {SHELF_MESH_PATH}")


if __name__ == "__main__":
    main()
