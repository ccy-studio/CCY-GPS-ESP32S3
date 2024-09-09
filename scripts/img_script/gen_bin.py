import os

current_dir = os.path.dirname(os.path.abspath(__file__))
output_dir = os.path.normpath(os.path.join(current_dir, "output"))
merge_dir = os.path.normpath(os.path.join(current_dir, "merge_output"))


def main():
    total_size = 0
    file_infos = []
    if not os.path.exists(merge_dir):
        os.makedirs(merge_dir)
    with open(os.path.join(merge_dir, "merge.bin"), "wb") as outout:
        for root, dirs, files in os.walk(output_dir):
            for filename in files:
                if filename.endswith("bin"):
                    path = os.path.join(root, filename)
                    # 读取数据
                    with open(path, "rb") as fb:
                        start_address = total_size
                        content = fb.read()
                        outout.write(content)
                        end_address = total_size + len(content)
                        total_size += len(content)
                        # 记录文件信息
                        file_info = {
                            "file_name": filename,
                            "start_address": start_address,
                            "end_address": end_address,
                        }
                        file_infos.append(file_info)
    # 打印合并文件的信息
    for info in file_infos:
        print(
            f"文件名: {info['file_name']}, 起始地址: {info['start_address']}, 结束地址: {info['end_address']},长度: {info['end_address']-info['start_address']}"
        )
    output_c(file_infos)


def output_c(file_infos):
    with open(os.path.join(merge_dir, "mergin_bin.c"), "w") as c_file:
        c_file.write('#include "mergin_bin.h"\n\n')
        c_file.write("struct mergin_bin_t bin_infos[] = {\n")

        for info in file_infos:
            c_file.write(
                f'\t{{ \n\t\t.item_name = "{info["file_name"][:30]}",\n\t\t.start_address = {info["start_address"]},\n\t\t.end_address = {info["end_address"]} \n\t}},\n'
            )
        c_file.write("};\n")
        c_file.write(f"size_t bin_infos_len = {len(file_infos)};")


if __name__ == "__main__":
    main()
