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
			for i in range(1, len(self.lines)-1):
				if '=' in self.lines[i]:
					self.separate = self.lines[i].replace(' ', '').split('=')
				elif '<-' in self.lines[i]:
					self.separate = self.lines[i].replace(' ', '').split('<-')
				if self.separate[0] not in self.keywords:
					self.variables[self.separate[0]] = eval(self.separate[1], globals(), self.variables)
				elif self.separate[0] in self.keywords:
					if self.separate[0] == '[o]':
						print(eval(self.separate[1], globals(), self.variables))
						
if __name__ == "__main__":
	
	if len(sys.argv) == 2:
		PATH = sys.argv[1]
		np = Nonpiler(PATH)
	else:
		raise RuntimeError("[Do you really think you can nonpile something without a path??? [G][E][N][I][U][S][!]]")
