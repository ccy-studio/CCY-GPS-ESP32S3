import os
import fnmatch
import shutil

current_dir = os.path.dirname(os.path.abspath(__file__))
output_dir = os.path.normpath(os.path.join(current_dir, "output"))
images_dir = os.path.normpath(os.path.join(current_dir, "images"))
util_path = os.path.normpath(
    os.path.join(current_dir, "../../libs/lvgl/scripts/LVGLImage.py")
)

image_extensions = ["*.jpg", "*.jpeg", "*.png", "*.gif", "*.bmp", "*.tiff"]

config = {
    "fmt": "BIN",
    "cf": "RGB565",
}


def main():
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    for root, dirs, files in os.walk(images_dir):
        for filename in files:
            if any(fnmatch.fnmatch(filename, ext) for ext in image_extensions):
                path = os.path.join(root, filename)
                command = f"python {util_path} {path} --ofmt {config['fmt']} --cf {config['cf']} -o {output_dir}"
                os.system(command)

    print("执行全部完成")


if __name__ == "__main__":
    main()
