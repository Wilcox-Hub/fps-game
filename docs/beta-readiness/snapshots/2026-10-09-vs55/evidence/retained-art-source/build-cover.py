"""Original metric cover-rock forms for the playable first-map layout.

Run in an isolated background Blender 5.2 process. These are refined blockout
meshes with matching convex hulls, not finished textured production rocks.
"""
from pathlib import Path
import bpy,bmesh,json,math,hashlib
base=Path(__file__).resolve().parent;out=base/'assets';out.mkdir(exist_ok=True)
assert not (out/'IronSun_FirstArenaCover_v55.blend').exists()
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
scene=bpy.context.scene;scene.unit_settings.scale_length=0.01
exporter=bpy.ops.export_scene.fbx;props=exporter.get_rna_type().properties
for name,value in [('axis_forward','-Y'),('axis_up','Z'),('mesh_smooth_type','FACE')]:
 assert value in {v.identifier for v in props[name].enum_items}
assets=[]
for variant,radii in [('A',[1,.94,1.02,.9,1,.97,.88,.95]),('B',[.92,1.04,.87,1,.94,1.02,.92,1])]:
 name='SM_IronSunCoverRock_'+variant;verts=[];faces=[]
 for ring,(z,scale,dx,dy) in enumerate([(0,1,0,0),(66,.9,6,-4),(100,.58,-12,7)]):
  for i,r in enumerate(radii):
   a=(i*45+15)*math.pi/180;verts.append((math.cos(a)*r*100*scale+dx,math.sin(a)*r*100*scale+dy,z))
 faces.append(tuple(reversed(range(8))))
 for ring in range(2):
  for i in range(8):
   j=(i+1)%8;faces.append((ring*8+i,ring*8+j,(ring+1)*8+j,(ring+1)*8+i))
 faces.append(tuple(range(16,24)))
 # Normalize the plan footprint for predictable engine scale values.
 for axis in (0,1):
  lo=min(v[axis] for v in verts);hi=max(v[axis] for v in verts)
  verts=[tuple((v[k]-(lo+hi)/2)*200/(hi-lo) if k==axis else v[k] for k in range(3)) for v in verts]
 mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);mesh.update()
 obj=bpy.data.objects.new(name,mesh);scene.collection.objects.link(obj)
 hull_mesh=mesh.copy();hull=bpy.data.objects.new('UCX_'+name+'_00',hull_mesh);scene.collection.objects.link(hull)
 bm=bmesh.new();bm.from_mesh(hull_mesh);result=bmesh.ops.convex_hull(bm,input=list(bm.verts),use_existing_faces=False)
 bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));bm.to_mesh(hull_mesh);bm.free();hull.hide_render=True
 bpy.ops.object.select_all(action='DESELECT');obj.select_set(True);hull.select_set(True);bpy.context.view_layer.objects.active=obj
 path=out/(name+'.fbx')
 exporter(filepath=str(path),use_selection=True,object_types={'MESH'},use_mesh_modifiers=True,mesh_smooth_type='FACE',apply_unit_scale=True,global_scale=1,axis_forward='-Y',axis_up='Z',bake_anim=False)
 mesh.calc_loop_triangles()
 assets.append({'name':name,'fbx':str(path),'dimensions_cm':[200,200,100],'render_triangles':len(mesh.loop_triangles),'convex_hull':hull.name,'sha256':hashlib.sha256(path.read_bytes()).hexdigest().upper()})
 for part in (obj,hull):part.hide_set(True)
bpy.ops.wm.save_as_mainfile(filepath=str(out/'IronSun_FirstArenaCover_v55.blend'))
(out/'blender-result.json').write_text(json.dumps({'passed':True,'blender_version':bpy.app.version_string,'assets':assets,'scope':'Original low-poly metric cover blockout meshes and matching convex hulls; not production texturing/art acceptance.'},indent=2)+'\n',encoding='utf-8')
print('IRON SUN VS55 COVER EXPORT PASS')
