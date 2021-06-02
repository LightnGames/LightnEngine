import glob, os, subprocess

if __name__ == "__main__":
    target_paths = glob.glob("L:\\LightnEngine\\work\\**\\*.*", recursive=True)
    mesh_paths = []
    for target_path in target_paths:
        file_name, ext = os.path.splitext(target_path)
        if ext in ".fbx" or  ext in ".FBX":
            mesh_paths.append(target_path)
    
    batch_count_max = 32
    remaining_count = len(mesh_paths)
    print("Mesh count %d" % remaining_count)
    while remaining_count > 0:
        batch_count = min(remaining_count, batch_count_max)
        mesh_path_offset = remaining_count - batch_count
        cmd = []
        cmd.append("L:\\LightnEngine\\tools\\python\\FBXConverter.exe")
        for batch_index in range(batch_count):
            cmd.append(mesh_paths[mesh_path_offset + batch_index])

        proc_list = []
        for batch_index in range(batch_count):
            proc = subprocess.Popen(cmd)
            proc_list.append(proc)
            print(cmd)

        for proc in proc_list:
            proc.wait()
        remaining_count -= batch_count
    print("Compleated %d!" % len(mesh_paths))