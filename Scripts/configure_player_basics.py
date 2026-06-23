import unreal


MOUSE_LOOK_CONTEXT = "/Game/Input/IMC_MouseLook"
FIRST_PERSON_CHARACTER = "/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"
CHARACTER_MESH = "/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"
MOUSE_SENSITIVITY = 0.12


def configure_mouse_sensitivity():
    context = unreal.EditorAssetLibrary.load_asset(MOUSE_LOOK_CONTEXT)
    if not context:
        raise RuntimeError(f"Missing input mapping context: {MOUSE_LOOK_CONTEXT}")

    mapping_data = context.get_editor_property("default_key_mappings")
    mappings = list(mapping_data.get_editor_property("mappings"))
    if not mappings:
        raise RuntimeError("IMC_MouseLook has no key mappings.")

    mapping = mappings[0]
    modifiers = list(mapping.get_editor_property("modifiers"))

    scalar_modifier = None
    for modifier in modifiers:
        if modifier.get_class().get_name() == "InputModifierScalar":
            scalar_modifier = modifier
            break

    if not scalar_modifier:
        scalar_modifier = unreal.new_object(
            unreal.InputModifierScalar,
            outer=context,
            name="InputModifierScalar_MouseSensitivity",
        )
        modifiers.append(scalar_modifier)

    scalar_modifier.set_editor_property(
        "scalar",
        unreal.Vector(MOUSE_SENSITIVITY, MOUSE_SENSITIVITY, 1.0),
    )
    mapping.set_editor_property("modifiers", modifiers)
    mappings[0] = mapping
    mapping_data.set_editor_property("mappings", mappings)
    context.set_editor_property("default_key_mappings", mapping_data)

    unreal.EditorAssetLibrary.save_loaded_asset(context)
    unreal.log(f"Mouse look sensitivity scaled to {MOUSE_SENSITIVITY}.")


def configure_character_mesh():
    character_bp = unreal.EditorAssetLibrary.load_asset(FIRST_PERSON_CHARACTER)
    if not character_bp:
        raise RuntimeError(f"Missing first person character blueprint: {FIRST_PERSON_CHARACTER}")

    character_mesh = unreal.EditorAssetLibrary.load_asset(CHARACTER_MESH)
    if not character_mesh:
        raise RuntimeError(f"Missing character mesh: {CHARACTER_MESH}")

    generated_class = character_bp.generated_class()
    cdo = unreal.get_default_object(generated_class)
    changed = 0

    for component in cdo.get_components_by_class(unreal.SkeletalMeshComponent):
        component.set_editor_property("skeletal_mesh", character_mesh)
        component.set_editor_property("visible", True)
        component.set_editor_property("hidden_in_game", False)
        component.set_editor_property("cast_shadow", False)
        component.set_editor_property("cast_dynamic_shadow", False)
        component.set_editor_property("cast_static_shadow", False)
        changed += 1

    unreal.EditorAssetLibrary.save_loaded_asset(character_bp)
    unreal.log(f"Configured {changed} character skeletal mesh component(s) with Quinn mesh and shadows disabled.")


if __name__ == "__main__":
    configure_mouse_sensitivity()
    configure_character_mesh()
