import math
import os
import sys

import bpy
from mathutils import Euler


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for material in bpy.data.materials:
        bpy.data.materials.remove(material)


def make_material(name, color, metallic, roughness, emission=None):
    material = bpy.data.materials.new(name)
    material.diffuse_color = (*color, 1.0)
    material.use_nodes = True
    shader = material.node_tree.nodes.get("Principled BSDF")
    shader.inputs["Base Color"].default_value = (*color, 1.0)
    shader.inputs["Metallic"].default_value = metallic
    shader.inputs["Roughness"].default_value = roughness
    if emission:
        shader.inputs["Emission"].default_value = (*emission, 1.0)
        shader.inputs["Emission Strength"].default_value = 1.2
    return material


def bevel_box(name, location, dimensions, material, bevel=0.01, rotation=(0.0, 0.0, 0.0)):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = dimensions
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(material)
    if bevel > 0.0:
        modifier = obj.modifiers.new("Machined edge radii", "BEVEL")
        modifier.width = bevel
        modifier.segments = 2
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.modifier_apply(modifier=modifier.name)
        obj.data.use_auto_smooth = True
        normal = obj.modifiers.new("Weighted face normals", "WEIGHTED_NORMAL")
        bpy.ops.object.modifier_apply(modifier=normal.name)
    return obj


def cylinder(name, location, radius, depth, material, rotation=(0.0, 0.0, 0.0), vertices=16):
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices, radius=radius, depth=depth, location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.data.materials.append(material)
    bevel = obj.modifiers.new("Turned edge chamfers", "BEVEL")
    bevel.width = min(radius * 0.16, depth * 0.08)
    bevel.segments = 2
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier=bevel.name)
    obj.data.use_auto_smooth = True
    normal = obj.modifiers.new("Weighted face normals", "WEIGHTED_NORMAL")
    bpy.ops.object.modifier_apply(modifier=normal.name)
    return obj


def uv_sphere(name, location, scale, material, segments=16, rings=8):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=rings, location=location)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(material)
    for polygon in obj.data.polygons:
        polygon.use_smooth = True
    return obj


def torus(name, location, major_radius, minor_radius, material, rotation=(0.0, 0.0, 0.0)):
    bpy.ops.mesh.primitive_torus_add(
        major_segments=24,
        minor_segments=8,
        location=location,
        rotation=rotation,
        major_radius=major_radius,
        minor_radius=minor_radius,
    )
    obj = bpy.context.object
    obj.name = name
    obj.data.materials.append(material)
    return obj


def join_meshes(name, objects):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    result = bpy.context.object
    result.name = name
    result.data.name = name + "_Mesh"
    result.location = (0.0, 0.0, 0.0)
    result.rotation_euler = Euler((0.0, 0.0, 0.0), "XYZ")
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    return result


def export_mesh(obj, path):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.fbx(
        filepath=path,
        use_selection=True,
        object_types={"MESH"},
        use_mesh_modifiers=True,
        mesh_smooth_type="FACE",
        apply_unit_scale=True,
        global_scale=1.0,
        axis_forward="-Z",
        axis_up="Y",
        add_leaf_bones=False,
        bake_anim=False,
        path_mode="AUTO",
        embed_textures=False,
    )
    print("RIDGEFIRE ART: exported {} ({:.2f} x {:.2f} x {:.2f} m)".format(
        os.path.basename(path), *obj.dimensions
    ))


def build_weapon(body, trim, glow):
    parts = []
    parts.append(bevel_box("Receiver", (0.0, 0.0, 0.01), (0.36, 0.076, 0.12), body, 0.018))
    parts.append(bevel_box("Rear stock", (-0.245, 0.0, 0.01), (0.18, 0.066, 0.078), body, 0.012))
    parts.append(bevel_box("Top rail", (0.015, 0.0, 0.079), (0.20, 0.045, 0.018), trim, 0.004))
    parts.append(bevel_box("Forward shroud", (0.17, 0.0, 0.012), (0.15, 0.094, 0.094), trim, 0.015))
    parts.append(bevel_box("Grip", (-0.045, 0.0, -0.104), (0.074, 0.055, 0.15), body, 0.012, (0.0, math.radians(-10.0), 0.0)))
    parts.append(bevel_box("Grip plate", (-0.045, -0.029, -0.108), (0.062, 0.008, 0.12), trim, 0.003, (0.0, math.radians(-10.0), 0.0)))
    parts.append(cylinder("Ion barrel", (0.34, 0.0, 0.018), 0.025, 0.30, body, (0.0, math.radians(90.0), 0.0)))
    parts.append(cylinder("Muzzle collar", (0.50, 0.0, 0.018), 0.043, 0.045, trim, (0.0, math.radians(90.0), 0.0)))
    parts.append(cylinder("Emitter aperture", (0.526, 0.0, 0.018), 0.026, 0.008, glow, (0.0, math.radians(90.0), 0.0)))
    parts.append(cylinder("Capacitor drum", (0.04, -0.052, -0.018), 0.041, 0.025, trim, (math.radians(90.0), 0.0, 0.0)))
    parts.append(cylinder("Capacitor core", (0.04, -0.067, -0.018), 0.021, 0.008, glow, (math.radians(90.0), 0.0, 0.0)))
    parts.append(bevel_box("Rear sight", (-0.045, 0.0, 0.108), (0.052, 0.044, 0.04), body, 0.006))
    parts.append(bevel_box("Sight glass", (-0.045, -0.024, 0.108), (0.025, 0.006, 0.018), glow, 0.003))
    parts.append(bevel_box("Side vent 1", (0.14, -0.049, 0.026), (0.065, 0.008, 0.012), glow, 0.003))
    parts.append(bevel_box("Side vent 2", (0.055, -0.049, 0.026), (0.045, 0.008, 0.012), trim, 0.003))
    return join_meshes("SM_Ridgefire_ViewModel", parts)


def build_sentinel(body, trim, glow, ember):
    parts = []
    parts.append(bevel_box("Armored chassis", (0.0, 0.0, 0.96), (0.56, 0.49, 0.3), body, 0.055))
    parts.append(bevel_box("Raised carapace", (-0.03, 0.0, 1.15), (0.43, 0.37, 0.12), trim, 0.026))
    parts.append(bevel_box("Forward visor", (0.29, 0.0, 1.0), (0.16, 0.33, 0.19), trim, 0.028))
    parts.append(uv_sphere("Targeting eye", (0.382, 0.0, 1.035), (0.045, 0.085, 0.055), glow))
    parts.append(cylinder("Optic bezel", (0.37, 0.0, 1.035), 0.092, 0.024, body, (0.0, math.radians(90.0), 0.0), 20))
    parts.append(cylinder("Lance housing", (0.25, -0.16, 0.89), 0.073, 0.12, trim, (0.0, math.radians(90.0), 0.0)))
    parts.append(cylinder("Energy lance", (0.45, -0.16, 0.89), 0.039, 0.31, body, (0.0, math.radians(90.0), 0.0)))
    parts.append(cylinder("Lance emitter", (0.615, -0.16, 0.89), 0.056, 0.028, ember, (0.0, math.radians(90.0), 0.0)))
    parts.append(torus("Rear power cell", (-0.295, 0.0, 0.99), 0.13, 0.016, glow, (0.0, math.radians(90.0), 0.0)))
    for side in (-1.0, 1.0):
        parts.append(bevel_box("Side armor", (0.0, side * 0.275, 1.0), (0.32, 0.08, 0.22), trim, 0.018))
        parts.append(bevel_box("Side signal", (0.07, side * 0.322, 1.035), (0.13, 0.01, 0.025), glow, 0.004))
        for fore in (-1.0, 1.0):
            hip_x = fore * 0.19
            hip_y = side * 0.25
            parts.append(uv_sphere("Hip joint", (hip_x, hip_y, 0.82), (0.095, 0.095, 0.085), trim))
            parts.append(bevel_box("Upper leg", (hip_x + fore * 0.07, hip_y + side * 0.12, 0.58), (0.085, 0.085, 0.47), body, 0.016))
            parts.append(uv_sphere("Knee joint", (hip_x + fore * 0.12, hip_y + side * 0.19, 0.39), (0.07, 0.07, 0.07), ember))
            parts.append(bevel_box("Lower leg", (hip_x + fore * 0.15, hip_y + side * 0.23, 0.22), (0.072, 0.072, 0.38), trim, 0.013))
            parts.append(bevel_box("Claw foot", (hip_x + fore * 0.2, hip_y + side * 0.27, 0.055), (0.19, 0.13, 0.09), body, 0.014))
    return join_meshes("SM_Ridgefire_Sentinel", parts)


def build_obelisk(body, trim, glow, ember):
    parts = []
    parts.append(cylinder("Foot plinth", (0.0, 0.0, 0.12), 0.82, 0.24, body, vertices=12))
    parts.append(cylinder("Machined foot ring", (0.0, 0.0, 0.28), 0.68, 0.09, trim, vertices=12))
    bpy.ops.mesh.primitive_cone_add(vertices=8, radius1=0.59, radius2=0.35, depth=3.85, location=(0.0, 0.0, 2.28))
    shaft = bpy.context.object
    shaft.name = "Tapered obsidian shaft"
    shaft.data.materials.append(body)
    parts.append(shaft)
    parts.append(cylinder("Crown collar", (0.0, 0.0, 4.25), 0.47, 0.14, trim, vertices=8))
    parts.append(bevel_box("Crown crownstone", (0.0, 0.0, 4.43), (0.70, 0.70, 0.22), body, 0.08))
    parts.append(cylinder("Crown signal ring", (0.0, 0.0, 4.58), 0.39, 0.055, ember, vertices=12))
    parts.append(uv_sphere("Crown beacon", (0.0, 0.0, 4.74), (0.16, 0.16, 0.19), glow, segments=12, rings=6))
    for face in range(4):
        angle = math.radians(face * 90.0)
        x = math.sin(angle) * 0.38
        y = -math.cos(angle) * 0.38
        parts.append(bevel_box(
            "Ion channel {:02d}".format(face),
            (x, y, 2.30),
            (0.095, 0.025, 3.25),
            glow,
            0.018,
            (0.0, 0.0, angle),
        ))
        parts.append(bevel_box(
            "Crown fin {:02d}".format(face),
            (math.sin(angle) * 0.31, math.cos(angle) * 0.31, 4.66),
            (0.12, 0.36, 0.32),
            trim,
            0.025,
            (0.0, 0.0, angle),
        ))
    return join_meshes("SM_Ridgefire_Obelisk", parts)


def main():
    script_path = os.path.abspath(__file__)
    project_root = os.path.abspath(os.path.join(os.path.dirname(script_path), ".."))
    output_dir = os.path.join(project_root, "SourceArt", "Meshes")
    os.makedirs(output_dir, exist_ok=True)

    clear_scene()
    alloy = make_material("Ridgefire_Alloy", (0.025, 0.07, 0.08), 0.74, 0.32)
    trim = make_material("Ridgefire_Trim", (0.24, 0.30, 0.34), 0.86, 0.24)
    glow = make_material("Ridgefire_Ion", (0.02, 0.35, 0.46), 0.3, 0.22, (0.03, 0.75, 1.0))

    if "--sentinel-only" not in sys.argv:
        weapon = build_weapon(alloy, trim, glow)
        export_mesh(weapon, os.path.join(output_dir, "SM_Ridgefire_ViewModel.fbx"))

    clear_scene()
    alloy = make_material("Ridgefire_Alloy", (0.025, 0.07, 0.08), 0.74, 0.32)
    trim = make_material("Ridgefire_Trim", (0.24, 0.30, 0.34), 0.86, 0.24)
    glow = make_material("Ridgefire_Ion", (0.02, 0.35, 0.46), 0.3, 0.22, (0.03, 0.75, 1.0))
    ember = make_material("Ridgefire_Ember", (0.55, 0.12, 0.025), 0.42, 0.28, (1.0, 0.22, 0.035))
    sentinel = build_sentinel(alloy, trim, glow, ember)
    export_mesh(sentinel, os.path.join(output_dir, "SM_Ridgefire_Sentinel.fbx"))

    if "--sentinel-only" not in sys.argv:
        clear_scene()
        alloy = make_material("Ridgefire_Alloy", (0.025, 0.07, 0.08), 0.74, 0.32)
        trim = make_material("Ridgefire_Trim", (0.24, 0.30, 0.34), 0.86, 0.24)
        glow = make_material("Ridgefire_Ion", (0.02, 0.35, 0.46), 0.3, 0.22, (0.03, 0.75, 1.0))
        ember = make_material("Ridgefire_Ember", (0.55, 0.12, 0.025), 0.42, 0.28, (1.0, 0.22, 0.035))
        obelisk = build_obelisk(alloy, trim, glow, ember)
        export_mesh(obelisk, os.path.join(output_dir, "SM_Ridgefire_Obelisk.fbx"))


if __name__ == "__main__":
    main()
