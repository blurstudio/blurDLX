
import os
from blur.build import *

path = os.path.dirname(os.path.abspath(__file__))

# Replace revision numbers in the nsi template
svnnsi = WCRevTarget("blurdlx_svnrevnsi",path,".","blurdlx-svnrev-template.nsi","blurdlx-svnrev.nsi")

# Create the nsi installer
All_Targets.append( NSISTarget( "blurdlx", path, "blurdlx.nsi", [svnnsi] ) )

#runs itself if double-clicked the py file
if __name__ == "__main__":
	build()