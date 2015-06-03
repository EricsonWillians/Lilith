import sys
import os
import re

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

class __statements__(dict):
	
	class __scope__(dict):
		def __init__(self):
			dict.__init__(self)
	
	class __statement__(list):
		def __init__(self):
			list.__init__(self)
		
	def __init__(self):
		dict.__init__(self)
		self['scope_statement'] = __statements__.__scope__()
		self['expression_statement'] = __statements__.__statement__()

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
		self.statements = __statements__()
		self.expressions = __expressions__()
		self.parse_scope_statements()

	def __str__(self):
		return self.data

	def parse_scope_statements(self):
		
		# Learning how to use regex...
		# It's going to be fucking necessary for this task...
		
		self.scope_list = re.search(self.tokens["open_block"], self.data).groups()
		print(self.scope_list)

if __name__ == "__main__":
	__parser__()
