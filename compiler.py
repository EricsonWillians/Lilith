import sys
import os
						
if __name__ == "__main__":
	
	__keywords__ = ['[i]', '[o]', '[f]']
	__operators__ = ['<-', '->']
	__statements__ = {"Assignment": [], "Output": []}
	__vars__ = {}
	
	if len(sys.argv) == 2:
		path = os.path.join(sys.argv[1])
		with open(path, "r") as fuk:
			fukdata = fuk.read().replace('\n', '')
	else:
		raise RuntimeError("You must specify a path to a .fuk file.")

	def fetch_statements():
		lines = fukdata.split('->')
		for line in lines:
			for operator in __operators__:
				if operator in line and operator != '->':
					split_line = line.split(operator)
					left_side = split_line[0].replace(' ', '')
					if left_side not in __keywords__:
						__statements__["Assignment"].append(split_line)
					elif left_side in __keywords__:
						if left_side == '[o]':
							__statements__["Output"].append(split_line)
						elif left_side == '[i]':
							__statements__["Output"].append(split_line)
	
	def fetch_vars():
		for var in __statements__["Assignment"]:
			name = var[0].replace(' ', '')
			expression = var[1]
			__vars__[name] = eval(expression, globals(), __vars__)
			
	def execute_io_operations():
		for data in __statements__["Output"]:
			expression = data[1]
			print(eval(expression, globals(), __vars__))

	fetch_statements()
	fetch_vars()
	execute_io_operations()
