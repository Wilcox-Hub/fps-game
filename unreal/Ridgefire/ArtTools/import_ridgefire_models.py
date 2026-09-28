import os

import unreal

project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
source_dir = os.path.join(project_root, "SourceArt", "Meshes")
destination = "/Game/Variant_Shooter/Meshes"
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

mesh_names = ("SM_Ridgefire_Sentinel",) if os.environ.get("RIDGEFIRE_SENTINEL_ONLY") == "1" else ("SM_Ridgefire_ViewModel", "SM_Ridgefire_Sentinel", "SM_Ridgefire_Obelisk")
for mesh_name in mesh_names:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", os.path.join(source_dir, mesh_name + ".fbx"))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", mesh_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)

    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", False)
    options.static_mesh_import_data.set_editor_property("combine_meshes", True)
    options.static_mesh_import_data.set_editor_property("auto_generate_collision", False)
    task.set_editor_property("options", options)
    asset_tools.import_asset_tasks([task])

    asset_path = destination + "/" + mesh_name
    mesh = unreal.load_asset(asset_path)
    if not mesh:
        raise RuntimeError("Failed to import " + asset_path)
    unreal.log("RIDGEFIRE IMPORT: {} materials={} bounds={}".format(
        mesh_name,
        len(mesh.get_editor_property("static_materials")),
        mesh.get_bounds().box_extent,
    ))

if not unreal.EditorAssetLibrary.save_directory(destination, only_if_is_dirty=False, recursive=True):
    raise RuntimeError("Failed to save Ridgefire meshes")

unreal.log("RIDGEFIRE IMPORT: all meshes saved")
unreal.SystemLibrary.quit_editor()
