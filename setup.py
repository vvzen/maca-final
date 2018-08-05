# check all addons inside openframeworks folder and download them if required

import os
import re
import subprocess

OF_ROOT="/Volumes/LaCie3TBYas/Code/c++/of_v0.10.0_osx_release"

addons_path = "{}/addons".format(OF_ROOT)

target_addons = { 
    "ofxIo": "https://github.com/bakercp/ofxIO",
    "ofxSerial" : "https://github.com/bakercp/ofxSerial",
    "ofxCv" : "https://github.com/kylemcdonald/ofxCv",
    "ofxPS3EyeGrabber" : "https://github.com/bakercp/ofxPS3EyeGrabber",
    "ofxFaceTracker" : "https://github.com/kylemcdonald/ofxFaceTracker"
}

user_addons = [f for f in os.listdir(addons_path) if re.match("ofx*", f)]

for addon in target_addons.keys():
    num_tabs = 2 if len(addon) > 10 else 3
    # check if addons are downloaded
    if addon not in user_addons:
        print("-- {}{}addon not found, downloading it ---> git clone {}".format(addon, num_tabs*"\t", target_addons[addon]))
        # TODO: download addon using git
        #subprocess.call(["git", "clone", "{}".format(target_addons[addon]))
        # print('calling : subprocess.call(["git", "clone", "{}"])'.format(target_addons[addon]))
    else:
        print("-- {}{}downloaded, skipping".format(addon, num_tabs*"\t"))

print("finished")