import sys, os.path, subprocess, shutil
TEXCONV_PATH = "L:\\LightnEngine\\tools\\python\\texconv.exe"
WORK_ROOT = "L:\\LightnEngine\\work"
RESOURCE_ROOT = "L:\\LightnEngine\\resource"
DDS_EXT = ".dds"

if __name__ == "__main__":
    max_process = 64
    args = sys.argv
    argc = len(args)
    remaining_count = argc - 1
    loop_count = int(argc / max_process) + 1

    for target_index in range(0, loop_count):
        target_count = min(max_process, remaining_count)
        remaining_count -= target_count
        print("target count: %d" % target_count)
        proc_list = []
        for arg_index in range(remaining_count, remaining_count + target_count):
            texture_file_path = args[arg_index + 1]

            cmd = []
            cmd.append(TEXCONV_PATH)
            cmd.append("-f")
            cmd.append("BC7_UNORM_SRGB")
            cmd.append("%s" % (texture_file_path.replace('\\','/')))
            cmd.append("-m")
            cmd.append("0")
            cmd.append("-y")
            # cmd.append("-nogpu")
            print(cmd)

            proc = subprocess.Popen(cmd)
            proc_list.append(proc)

        for proc in proc_list:
            proc.wait()

    print("texture convert compleated!")
    for arg_index in range(1, argc):
        texture_file_path = args[arg_index]
        texture_file_name_ext = os.path.basename(texture_file_path)
        texture_file_name, texture_ext = os.path.splitext(texture_file_name_ext)
        texture_work_dds_path = texture_file_path.replace(texture_ext, DDS_EXT)
        texture_resource_dds_path = texture_work_dds_path.replace(WORK_ROOT, RESOURCE_ROOT)

        dst_directory_file_name = os.path.split(texture_resource_dds_path)
        dst_directory = dst_directory_file_name[0]
        if not os.path.exists(dst_directory):
            os.makedirs(dst_directory)
            print("mkdir %s" % dst_directory)

        copy_path = shutil.move(texture_work_dds_path, texture_resource_dds_path)
        print("move to %s" % copy_path)
        
    #os.system("pause")

    