import unreal


MOUSE_LOOK_CONTEXT = "/Game/Input/IMC_MouseLook"
MOUSE_SENSITIVITY = 0.08


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


if __name__ == "__main__":
    configure_mouse_sensitivity()
