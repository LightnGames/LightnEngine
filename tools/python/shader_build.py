import sys, os.path, subprocess, struct, xxhash, glob

# version 10.0.19628.0
DXC_PATH = "L:\\LightnEngine\\tools\\python\\dxc.exe"
WORK_ROOT = "L:\\LightnEngine\\work"
RESOURCE_ROOT = "L:\\LightnEngine\\resource"
ENTRY_POINT = 'main'
SHADER_MODEL  = {'.vsf': "_6_3", '.psf': "_6_3", '.csf': "_6_3", '.msf': "_6_5", '.asf': "_6_5"}
SHADER_EXT = {'.vsf': "vs", '.psf': "ps", '.csf': "cs", '.msf': "ms", '.asf': "as"}
SHADER_OUTPUT_EXT = {'.vsf': "vso", '.psf': "pso", '.csf': "cso", '.msf': "mso", '.asf': "aso"}
SHADER_INFO_EXT = {'.vsf': "vsinfo", '.psf': "psinfo", '.csf': "csinfo", '.msf': "msinfo", '.asf': "asinfo"}

def convert(shader_file_path):
    shader_file_name_ext = os.path.basename(shader_file_path)
    print("start shader build %s" % shader_file_path)

    shader_file_name, shader_ext = os.path.splitext(shader_file_name_ext)
    shader_type = SHADER_EXT[shader_ext]
    shader_output_ext = SHADER_OUTPUT_EXT[shader_ext]

    # with open(shader_file_path) as f:
    #     lines = f.readlines()
    #     for line in lines:
    #         print(line)

    work_dir = os.path.dirname(shader_file_path)
    shader_output_dir = work_dir.replace(WORK_ROOT, RESOURCE_ROOT)
    shader_output_name = shader_file_name + '.' + shader_output_ext
    shader_output_path = os.path.join(shader_output_dir, shader_output_name)

    if not os.path.exists(shader_output_dir):
        os.makedirs(shader_output_dir)
        print("mkdir %s" % shader_output_dir)


    # Shader Param Type
    # 0: float
    # 1: float2
    # 2: float3
    # 3: float4
    # 4: uint
    # 5: texture
    if shader_output_ext == 'pso':
        base_color_hash = xxhash.xxh32(b'BaseColor').intdigest()
        albedo_texture_index_hash = xxhash.xxh32(b'AlbedoTextureIndex').intdigest()
        shader_parameters = []
        shader_parameters.append([base_color_hash, 3])
        shader_parameters.append([albedo_texture_index_hash, 5])
        shader_info_output_ext = SHADER_INFO_EXT[shader_ext]
        shader_info_path = "%s\\%s.%s" % (shader_output_dir, shader_file_name, shader_info_output_ext)
        with open(shader_info_path, "wb") as fout:
            fout.write(struct.pack("H", len(shader_parameters)))
            for shader_parameter in shader_parameters:
                fout.write(struct.pack("IH", shader_parameter[0], shader_parameter[1]))

    # DXCシェーダーコンパイラでシェーダーをビルドする
    cmd = []
    cmd.append(DXC_PATH)
    cmd.append("/T")
    cmd.append("%s%s" % (shader_type, SHADER_MODEL[shader_ext]))
    cmd.append("/E")
    cmd.append("%s" % ENTRY_POINT)
    cmd.append("/Fo")
    cmd.append("%s" % shader_output_path)
    cmd.append("/enable_unbounded_descriptor_tables")
    # cmd.append("/Zi")
    # cmd.append("/Qembed_debug")
    # cmd.append("/Od")
    cmd.append("/nologo")
    cmd.append(shader_file_path)
    print(cmd)

    process_result = subprocess.call(cmd)
    if not process_result:
        print("shader build compleated!")
    else:
        print("shader build failed!")


if __name__ == "__main__":
    if len(sys.argv) == 1 :
        target_paths = glob.glob("L:\\LightnEngine\\work\\**\\*.*", recursive=True)
        for target_path in target_paths:
            shader_file_name, shader_ext = os.path.splitext(target_path)
            if shader_ext in SHADER_MODEL:
                convert(target_path)
    else:
        args = sys.argv
        shader_file_path = args[1]
        convert(shader_file_path)
    

