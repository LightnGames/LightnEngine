import yaml, sys, struct, os, xxhash
WORK_ROOT = "L:\\LightnEngine\\work"
RESOURCE_ROOT = "L:\\LightnEngine\\resource"
LEVEL_EXT = ".level"

if __name__ == "__main__":
    args = sys.argv
    level_file_path = args[1]

    with open(level_file_path, 'r') as yml:
        level_data = yaml.load(yml, Loader=yaml.SafeLoader)
        mesh_paths = level_data['meshes']
        texture_parameters = level_data['textures']
        materials = level_data['material_instances']
        shader_sets = level_data['shader_sets']
        mesh_instances = level_data['mesh_instances']

        level_file_name = os.path.basename(level_file_path)
        level_file_name, level_ext = os.path.splitext(level_file_name)
        work_dir = os.path.dirname(level_file_path)
        level_output_dir = work_dir.replace(WORK_ROOT, RESOURCE_ROOT)
        level_output_name = level_file_name + LEVEL_EXT
        level_output_path = os.path.join(level_output_dir, level_output_name)

        mesh_path_count = len(mesh_paths)
        texture_parameter_count = len(texture_parameters)
        shader_set_count = len(shader_sets)
        material_instance_count = len(materials)
        mesh_instance_count = len(mesh_instances)

        print("convert to : " + level_output_path)
        print('mesh count : %d' % mesh_path_count)
        print('texture count : %d' % texture_parameter_count)
        print('shader set count : %d' % texture_parameter_count)
        print('material instance count : %d' % material_instance_count)
        print('mesh instance count : %d' % mesh_instance_count)

        if not os.path.exists(level_output_dir):
            os.mkdir(level_output_dir)

        string_format = 'utf-8'
        with open(level_output_path, mode='wb') as fout:
            fout.write('RESH'.encode(string_format))
            fout.write(struct.pack('<IIIII', mesh_path_count, texture_parameter_count, shader_set_count, material_instance_count, mesh_instance_count))

            # meshes
            fout.write('MESH'.encode(string_format))
            for mesh_path in mesh_paths:
                str_length = len(mesh_path)
                fout.write(struct.pack('<I', str_length))
                fout.write(mesh_path.encode(string_format))

            # textures
            fout.write('TEX '.encode(string_format))
            for texture_parameter in texture_parameters:
                str_length = len(texture_parameter)
                fout.write(struct.pack('<I', str_length))
                fout.write(texture_parameter.encode(string_format))

            # shader sets
            fout.write('SSET'.encode(string_format))
            for shader_set in shader_sets:
                str_length = len(shader_set)
                fout.write(struct.pack('<I', str_length))
                fout.write(shader_set.encode(string_format))
            
            # material instances
            fout.write('MATI'.encode(string_format))
            for material_instance in materials:
                str_length = len(material_instance)
                fout.write(struct.pack('<I', str_length))
                fout.write(material_instance.encode(string_format))

            # mesh instances
            fout.write('MESI'.encode(string_format))
            for mesh_instance_index in range(mesh_instance_count):
                mesh_instance = mesh_instances[mesh_instance_index]
                
                # mesh path
                mesh_index = mesh_paths.index(mesh_instance['mesh_path'])
                fout.write(struct.pack('<I', mesh_index))

                # world matrix
                world_matrix_colmns = mesh_instance['world_matrix']
                for colmn in world_matrix_colmns:
                    fout.write(struct.pack('<ffff', colmn[0], colmn[1], colmn[2], colmn[3]))

                # material instances
                material_instances = mesh_instance['materials']
                material_count = len(material_instances)
                fout.write(struct.pack('<I', material_count))
                for material_instance in material_instances:
                    material_index = materials.index(material_instance)
                    fout.write(struct.pack('<I', material_index))
