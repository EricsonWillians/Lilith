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
import string

class Nonpiler():
		
	def __init__(self, path):
		self.path = os.path.join(path)
		self.nonp = open(self.path, "r")
		self.lines = [x.strip() for x in self.nonp.readlines()]
		
		self.keywords = ['[o]', '[i]', '[f]']
		self.variables = {}
		self.analyze_syntax()
		
	def analyze_syntax(self):
		if self.lines[0] != '[' or self.lines[len(self.lines)-1] != ']':
			raise SyntaxError("[Bad structure... [:(]]")
		else:
			for i in range(1, len(self.lines)-2):
				if self.lines[i] not in self.keywords:
					self.separate = self.lines[i].replace(' ', '').split('=')
					self.variables[self.separate[0]] = eval(self.separate[1], globals(), self.variables)
			
if __name__ == "__main__":
	
	if len(sys.argv) == 2:
		PATH = sys.argv[1]
		np = Nonpiler(PATH)
		print(np.variables)
	else:
		raise RuntimeError("[Do you really think you can nonpile something without a path??? [G][E][N][I][U][S][!]]")
	
	
