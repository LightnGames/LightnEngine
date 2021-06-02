import yaml, sys, struct, os
WORK_ROOT = "L:\\LightnEngine\\work"
RESOURCE_ROOT = "L:\\LightnEngine\\resource"
MTO_EXT = ".sseto"

if __name__ == "__main__":
    args = sys.argv
    material_file_path = args[1]

    with open(material_file_path, 'r') as yml:
        material_data = yaml.load(yml,Loader=yaml.SafeLoader)
        mesh_shader_path = material_data['mesh_shader']
        pixel_shader_path = material_data['pixel_shader']

        material_file_name = os.path.basename(material_file_path)
        material_file_name, material_ext = os.path.splitext(material_file_name)
        work_dir = os.path.dirname(material_file_path)
        material_output_dir = work_dir.replace(WORK_ROOT, RESOURCE_ROOT)
        material_output_name = material_file_name + MTO_EXT
        material_output_path = os.path.join(material_output_dir, material_output_name)

        mesh_shader_path_length = len(mesh_shader_path)
        pixel_shader_path_length = len(pixel_shader_path)

        print("convert to : " + material_output_path)
        print('mesh shader : ' + mesh_shader_path)
        print('pixel shader : ' + pixel_shader_path)

        if not os.path.exists(material_output_dir):
            os.makedirs(material_output_dir)

        string_format = 'utf-8'
        with open(material_output_path, mode='wb') as fout:
            fout.write(struct.pack('<I', mesh_shader_path_length))
            fout.write(mesh_shader_path.encode(string_format))
            fout.write(struct.pack('<I', pixel_shader_path_length))
            fout.write(pixel_shader_path.encode(string_format))

