import os, glob
from PIL import Image

hd_path = 'some_large_imgs_folder_path' #'../iconsHD'
sd_path = 'some_folder_for_scaled_down_imgs' #'../icons'

if not os.path.exists(sd_path):
    os.makedirs(sd_path)

# Resize icon based on width and maintain aspect ratio
basewidth = 30
for filename in glob.glob(f'{hd_path}/*.png'): # Recursive scan the folder path of HD Icons
    img = Image.open(filename)
    wpercent = (basewidth/float(img.size[0]))
    hsize = int((float(img.size[1])*float(wpercent)))
    img = img.resize((basewidth,hsize), Image.Resampling.LANCZOS)
    # Save resized images with existing filename to sd_path folder 
    img.save(os.path.join(sd_path, os.path.basename(filename))) 