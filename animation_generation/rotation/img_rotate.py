# import the Python Image
# processing Library
from PIL import Image
import subprocess
import os
import re
 
degrees = 15
bitmap_height = 36

# Giving The Original image Directory
# Specified
Original_Image = Image.open("./logo_0.png")
 
# Create images in 5 degree increments
j = 0
for x in range(0, 360, degrees):
   rotated_image1 = Original_Image.rotate(-x)
   rotated_image1.save("logo_{}.png".format(x))
   # create .c file of bitmapping
   subprocess.run(["UTFTConverter_w.exe", "logo_{}.png".format(x), "/c"])
   j+=1

pre_final_file = []
files = []

# combine all c files into 1
for file in os.listdir():
    if file.endswith('.c'):
        files.append(file)
files.sort(key=lambda f: int(re.sub('\D', '', f)))

for file in files:
    with open(file, 'r') as f:
        for line in f:
            if line[:2] == "0x":
                pre_final_file.append(line)

final_file = []
# replace all pixels with white pixels
for line in pre_final_file:
    final_file.append(line.replace("0x0000", "0xFFFF"))


# write include & nested array declaration
with open('mchp_rotations.h', 'w') as f:
    f.write(f"#include <avr/pgmspace.h>\n")
    f.write(f"const uint16_t mchp_rotate[{int(360/degrees)}][576] PROGMEM={{\n{{")
    i = 0
    count = 0
    for line in final_file:
        if(i < bitmap_height):
            f.write(f"{line}")
            i+=1
        else:
            f.write(f"}}")
            i = 0
            count+=1
            f.write(f",\n{{") # another entry is coming
            f.write(f"{line}")
            i+=1
            # if(count <= int(360/degrees)):
            #     f.write(f",\n{{") # another entry is coming
            #     f.write(f"{line}")
            #     i+=1
            # else:
            #     f.write(f"}}\n") # no more entries
    f.write(f"}}\n}};\n")

