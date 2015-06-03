import sys
import os

class __expressions__(list):
	
	def __init__(self):
		list.__init__(self)
		
	def __getitem__(self, index=0):
		return self.pop

class __parser__():
	
	def __init__(self):
		if len(sys.argv) == 2:
			self.path = os.path.join(sys.argv[1])
			with open(self.path, "r") as fuk:
				self.data = fuk.read().replace('\n', '')
		else:
			raise RuntimeError("You must specify a path to a .fuk file.")

	def __str__(self):
		return self.data

if __name__ == "__main__":
	
	print(__parser__())
	
