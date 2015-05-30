"""
[[GNU]

[[[[[
[[[[NASTY
[[[[OVERWHELMING
[[[[-
[[[[PANDEMONIUM
[[[[INFERNAL
[[[[THOUGHTLESS
[[[[YGRITTE: YOU KNOW NOTHING, PYTHON!
[[[[-
[[[[OVERNASTY
[[[[NONSENSE
[[[[]

[GNU]]
"""

import sys
import os

if __name__ == "__main__":
	
	if len(sys.argv) == 2:
		PATH = sys.argv[1]
	else:
		raise RuntimeError("[DO YOU REALLY THINK YOU CAN NOMPILE SOMETHING WITHOUT A PATH??? [G][E][N][I][U][S][!]]")
		
	class Nonpiler():
		
		def __init__(self, path):
			self.path = os.path.join(path)
			self.nonp = open(self.path, "r")
			self.lines = self.nonp.readlines()
			
	np = Nonpiler(PATH)
	print(np.lines)
