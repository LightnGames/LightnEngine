import yaml, sys, struct, os, xxhash
WORK_ROOT = "L:\\LightnEngine\\work"
RESOURCE_ROOT = "L:\\LightnEngine\\resource"
MTO_EXT = ".mto"

if __name__ == "__main__":
    args = sys.argv
    argc = len(args)

    for arg_index in range(1, argc):
        material_file_path = args[arg_index]

        with open(material_file_path, 'r') as yml:
            material_data = yaml.load(yml, Loader=yaml.SafeLoader)
            shader_set_path = material_data['shader_set']

            texture_parameters = dict()
            if 'textures' in material_data:
                texture_parameters = material_data['textures']

            material_file_name = os.path.basename(material_file_path)
            material_file_name, material_ext = os.path.splitext(material_file_name)
            work_dir = os.path.dirname(material_file_path)
            material_output_dir = work_dir.replace(WORK_ROOT, RESOURCE_ROOT)
            material_output_name = material_file_name + MTO_EXT
            material_output_path = os.path.join(material_output_dir, material_output_name)

            shader_set_path_length = len(shader_set_path)
            texture_parameter_count = len(texture_parameters)

            print("convert to : " + material_output_path)
            print('shader set path : ' + shader_set_path)
            print('texture count : %d' % texture_parameter_count)
            print(texture_parameters)

            if not os.path.exists(material_output_dir):
                os.makedirs(material_output_dir)

            string_format = 'utf-8'
            with open(material_output_path, mode='wb') as fout:
                shader_set_hash = xxhash.xxh64(shader_set_path.encode(string_format)).intdigest()
                fout.write(struct.pack('<Q', shader_set_hash))

                fout.write(struct.pack('<I', texture_parameter_count))
                for texture_parameter in texture_parameters.items():
                    key = texture_parameter[0]
                    texture_path = texture_parameter[1]
                    texture_path_hash = xxhash.xxh64(texture_path.encode(string_format)).intdigest()
                    fout.write(struct.pack('<Q', texture_path_hash))
