import os
import json

current_dir = os.path.dirname(os.path.abspath(__file__))


def main():
    with open(os.path.join(current_dir, "config.json"), "r", encoding="utf-8") as jsonf:
        data = json.load(jsonf)
    # print(data)
    outDir = os.path.normpath(os.path.join(current_dir, data["outputPath"]))
    print(f"output: {outDir}")
    # 循环获取字体配置信息
    fonts = data["fontFileConfigs"]
    for font_config in fonts:
        font_path = os.path.normcase(os.path.join(current_dir, font_config["fontPath"]))
        gen_files = font_config["generators"]
        # 循环此字体要配置生成项目
        for item in gen_files:
            size = item["size"]
            fileName = item["fileName"]
            bpp = item["bpp"]
            symbols = ''.join(set(item["symbols"]))
            r = item["range"] or "0x20-0x7f"
            os.system(
                f"lv_font_conv --no-compress --no-prefilter --bpp {bpp}   --size {size} --font {font_path} -r {r} --format lvgl --force-fast-kern-format -o {os.path.normpath(os.path.join(outDir,fileName))} {'--symbols ' + symbols.encode('utf-8').decode('utf-8') if symbols else ''}"
            )
            print(f"{fileName} 生成成功!")
    print("\n字体生成全部完成.....")


if __name__ == "__main__":
    main()
