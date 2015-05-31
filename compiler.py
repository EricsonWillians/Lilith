import sys
import os
						
if __name__ == "__main__":
	
	__keywords__ = ['[o]', '[i]', '[f]']
	__tokens__ = ['=', '<-', '->']
	__vars__ = {}
	
	if len(sys.argv) == 2:
		path = os.path.join(sys.argv[1])
		with open(path, "r") as yp:
			data = yp.read().replace('\n', '')
	else:
		raise RuntimeError("You must specify a path to a .fuk file.")

	statements = data.split('->')
	expressions = []
	for statement in statements:
		for token in __tokens__:
			if token in statement:
				expressions.append(statement.split(token))
				
	print(expressions)



