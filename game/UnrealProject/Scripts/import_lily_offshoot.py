import os

import unreal


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SOURCE_FBX = os.path.join(PROJECT_ROOT, "models", "Plants", "LilyOffshoot", "lily_offshoot.fbx")
DESTINATION_PATH = "/Game/models/Plants/LilyOffshoot"


def import_static_mesh():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = False
    options.import_materials = True
    options.import_textures = False
    options.static_mesh_import_data.combine_meshes = True
    options.static_mesh_import_data.generate_lightmap_u_vs = True
    options.static_mesh_import_data.auto_generate_collision = True

    task = unreal.AssetImportTask()
    task.filename = SOURCE_FBX
    task.destination_path = DESTINATION_PATH
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.options = options

    asset_tools.import_asset_tasks([task])
    if task.imported_object_paths:
        unreal.log("Imported Lily Offshoot assets: {}".format(", ".join(task.imported_object_paths)))
    else:
        unreal.log_error("Lily Offshoot import did not produce any assets.")


if __name__ == "__main__":
    import_static_mesh()
