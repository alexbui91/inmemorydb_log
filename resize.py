import os

from PIL import Image

from resizeimage import resizeimage


path = 'KITTI_dataset'
dst_path = 'dataset'

dirs = os.listdir(path)

for file in dirs:
    with open(os.path.join(path, file), 'r+b') as f:
        with Image.open(f) as image:
            cover = resizeimage.resize_cover(image, [696, 256])
            cover.save(os.path.join(dst_path, file), image.format)