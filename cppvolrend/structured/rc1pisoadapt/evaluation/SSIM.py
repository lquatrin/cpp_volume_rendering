from pathlib import Path
import subprocess
import pandas as pd
import sys

GroundTruthFile = Path("img/0004.png")
CSVFileName = Path("eval.csv")
OutFileName = Path("EvalSSIM.csv")
ImgFolder = Path("img/")
OutImgFolder = Path("ssim/")

# Overwrite
if OutFileName.exists():
    print("Output will be overwritten: " + str(OutFileName))

if OutImgFolder.exists():
    print("Output will be overwritten: " + str(OutImgFolder))
else:
    OutImgFolder.mkdir()    

# Read as Pandas DataFrame
d = pd.read_csv(CSVFileName, quotechar='"')

# Compute SSIM for each image file
SSIM = list()
for row in d.iterrows():
    # if (row[0] > 5): break
    ImgFile = ImgFolder / Path(row[1]['ImageFile'])
    # print(ImgFile)
    OutFile = OutImgFolder / (GroundTruthFile.stem + "_" + ImgFile.stem + ".png")
    # print(OutFile)
    res = subprocess.run(["magick", "compare", "-metric", "SSIM",
                            str(GroundTruthFile),  str(ImgFile), str(OutFile)],
                            capture_output=True, text=True)
    print(res.stderr)
    SSIM.append(res.stderr)

# Add a new column to the DataFrame and save to disk as csv file
d["SSIM"] = SSIM
d.to_csv(OutFileName, index=False)
