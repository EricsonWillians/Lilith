import sys
import os

class __tokens__(dict):
	
	def __init__(self):
		dict.__init__(self)
		self['shift_operator'] = '<-'
		self['addition_shift_operator'] = '<+>'
		self['subtraction_shift_operator'] = '<->'
		self['multiplication_shift_operator'] = '<*>'
		self['division_shift_operator'] = '</>'
		self['modulus_shift_operator'] = '<%>'
		self['open_function_identifier'] = '['
		self['close_function_identifier'] = ']'
		self['end_of_statement'] = '|'
		self['open_block'] = '|>'
		self['close_block'] = '<|'
		self['if'] = '?'
		self['then'] = '!>'
		self['else_if'] = '??'
		self['else'] = '???'
		self['negation'] = '!' 
		self['while'] = '@' 
		self['for'] = '<@>'
		self['increment'] = '++' 
		self['decrement'] = '--' 
		self['break'] = '<@'
		self['continue'] = '@>'
		self['separator'] = '{}'
		self['return'] = '/\\'

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
		
		self.tokens = __tokens__()
		self.expressions = __expressions__()

	def __str__(self):
		return self.data

if __name__ == "__main__":
	
	print(__parser__().tokens)
	
