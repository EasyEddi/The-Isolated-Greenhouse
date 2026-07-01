import unreal


DESTINATION_PATH = "/Game/models/equipment/fertilizer/ornament_plants"
MATERIAL_PATH = f"{DESTINATION_PATH}/Materials"
FERTILIZER_MESH_PATH = f"{DESTINATION_PATH}/ornament_plants_fertilizer.ornament_plants_fertilizer"


MATERIAL_SPECS = {
    "kraft_paper": ("M_FertilizerBag_KraftPaper_Clean", (0.46, 0.31, 0.17, 1.0), 0.88, 0.12),
    "label_cream": ("M_FertilizerBag_LabelCream_Clean", (0.86, 0.76, 0.54, 1.0), 0.82, 0.10),
    "label_green": ("M_FertilizerBag_LabelGreen_Clean", (0.13, 0.42, 0.16, 1.0), 0.80, 0.12),
    "dark_green": ("M_FertilizerBag_DarkGreen_Clean", (0.055, 0.18, 0.075, 1.0), 0.88, 0.10),
    "label_text_dark": ("M_FertilizerBag_LabelTextDark_Clean", (0.035, 0.055, 0.030, 1.0), 0.86, 0.08),
    "seam_dark": ("M_FertilizerBag_SeamDark_Clean", (0.075, 0.050, 0.030, 1.0), 0.90, 0.08),
    "seam_highlight": ("M_FertilizerBag_SeamHighlight_Clean", (0.68, 0.50, 0.28, 1.0), 0.82, 0.10),
}


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def create_clean_material(name, color, roughness, specular):
    ensure_folder(MATERIAL_PATH)
    path = f"{MATERIAL_PATH}/{name}"
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
    return {key: create_clean_material(*spec) for key, spec in MATERIAL_SPECS.items()}


def material_for_slot(slot_name, materials):
    name = slot_name.lower()
    if "kraft" in name or "paper" in name:
        return materials["kraft_paper"]
    if "cream" in name:
        return materials["label_cream"]
    if "label_green" in name or ("label" in name and "green" in name):
        return materials["label_green"]
    if "dark_green" in name or ("dark" in name and "green" in name):
        return materials["dark_green"]
    if "text" in name:
        return materials["label_text_dark"]
    if "highlight" in name:
        return materials["seam_highlight"]
    if "seam" in name:
        return materials["seam_dark"]
    return materials["kraft_paper"]


def assign_clean_materials(mesh, materials):
    for index, slot in enumerate(mesh.get_editor_property("static_materials")):
        slot_name = str(slot.material_slot_name)
        material = material_for_slot(slot_name, materials)
        mesh.set_material(index, material)
        unreal.log(f"FERTILIZER_BAG_SLOT {index}: {slot_name} -> {material.get_name()}")


def main():
    mesh = unreal.EditorAssetLibrary.load_asset(FERTILIZER_MESH_PATH)
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"Missing StaticMesh: {FERTILIZER_MESH_PATH}")

    materials = build_materials()
    assign_clean_materials(mesh, materials)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
    unreal.EditorAssetLibrary.save_directory(DESTINATION_PATH, only_if_is_dirty=False, recursive=True)
    unreal.log(f"Rebuilt clean fertilizer bag materials: {FERTILIZER_MESH_PATH}")


if __name__ == "__main__":
    main()
