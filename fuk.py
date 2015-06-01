''' This file only works if you're on Windows and you've added the path to the 
folder where the Fukzen files are, to the system's Path environment variable.'''

import os
import sys

script_dir = os.path.dirname(os.path.abspath(__file__))
current_dir = os.getcwd()

if len(sys.argv) == 2:
	
	if script_dir == current_dir:
		os.system("python compiler.py {0:s}".format(sys.argv[1]))
	elif script_dir != current_dir:
		os.system("python {0:s}compiler.py {1:s}".format('"'+script_dir+'"\\', '"'+script_dir+'"\\'+sys.argv[1]))
else:
	raise RuntimeError("No fuk file has been specified, or it could not be found.")
