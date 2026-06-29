import unreal


GREENHOUSE_MESH_PATH = "/Game/models/map/Greenhouse/greenhouse_5x3m.greenhouse_5x3m"


def set_property_if_available(obj, property_name, value):
    try:
        obj.set_editor_property(property_name, value)
        return True
    except Exception as exc:
        unreal.log_warning(f"Could not set {property_name}: {exc}")
        return False


def main():
    mesh = unreal.EditorAssetLibrary.load_asset(GREENHOUSE_MESH_PATH)
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"Expected StaticMesh at {GREENHOUSE_MESH_PATH}, got {mesh}")

    body_setup = mesh.get_editor_property("body_setup")
    if not body_setup:
        raise RuntimeError(f"StaticMesh has no BodySetup: {GREENHOUSE_MESH_PATH}")

    mesh.modify()
    body_setup.modify()

    # The greenhouse door is open in the model. Using complex collision keeps the
    # visible doorway walkable without changing the FBX/base mesh.
    set_property_if_available(
        body_setup,
        "collision_trace_flag",
        unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE,
    )

    unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
    unreal.log(f"Greenhouse collision set to Use Complex As Simple: {GREENHOUSE_MESH_PATH}")


if __name__ == "__main__":
    main()
