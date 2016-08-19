from ply import yacc
from lexer import tokens as tk
from re import compile
from sys import argv, exit

tokens = tk
identifiers = {}
condition_blocks = {0: True}
current_condition_block = 0

def p_program(p):
	"""program : statement
			   | program statement"""
	p[0] = [x for x in p if x]

def p_statement(p):
	"""statement : condition_statement end
			     | expression end
		         | assignment end
		         | io_operation end"""
	p[0] = ("statement", p[1], p[2])

def p_compound_statement(p):
	"""compound_statement : statement
						  | statement compound_statement"""
	p[0] = ("compound_statement", p[1])

def p_expr(p):
	"""expression : expression plus term 
			      | expression minus term 
			      | expression greater_than_relational_operator term 
			      | expression greater_than_or_equal_to_relational_operator term 
			      | expression less_than_relational_operator term 
			      | expression less_than_or_equal_to_relational_operator term
			      | term
			      | string"""
	if len(p) == 4:
		if p[2] == '+':
			p[0] = p[1] + p[3]
		elif p[2] == '-':
			p[0] = p[1] - p[3]
		elif p[2] == '>':
			p[0] = p[1] > p[3]
		elif p[2] == '>=':
			p[0] = p[1] >= p[3]
		elif p[2] == '<':
			p[0] = p[1] <= p[3]
		elif p[2] == '<=':
			p[0] = p[1] <= p[3]
	else:
		p[0] = p[1]

def p_term(p):
	"""term : term mult factor 
			| term div factor 
			| term mod factor
			| term pow factor
			| term equal_to_relational_operator factor 
			| term not_equal_to_relational_operator factor 
			| term alt_not_equal_to_relational_operator factor 
			| factor"""
	if len(p) == 4:
		if p[2] == '*':
			p[0] = p[1] * p[3]
		elif p[2] == '/':
			p[0] = p[1] / p[3]
		elif p[2] == '%':
			p[0] = p[1] % p[3]
		elif p[2] == '^':
			p[0] = p[1] ** p[3]
		elif p[2] == '==':
			p[0] = p[1] == p[3]
		elif p[2] == '<>' or p[2] == '!=':
			p[0] = p[1] != p[3]
	else:
		p[0] = p[1]

def p_factor(p):
	"""factor : open_parenthesis expression close_parenthesis 
			  | name 
			  | number"""
	if len(p) == 4:
		p[0] = p[2]
	else:
		p[0] = p[1]

def parse_object(s):
	def isfloat(value):
		try:
			float(value)
			return True
		except ValueError:
			return False
	if isfloat(s):
		return float(s)
	else:
		if compile(r"\*.*\*").match(s):
			return s
		else:
			return "NIHIL"

def p_name(p):
	"""name : identifier"""
	if p[1] in identifiers:
		p[0] = parse_object(identifiers[p[1]])
	else:
		p[0] = parse_object(p[1])

def p_assignment_operator(p):
	"""assignment_operator : simple_assignment_operator
						   | addition_assignment_operator
						   | subtraction_assignment_operator
						   | multiplication_assignment_operator
						   | division_assignment_operator
						   | modulo_assignment_operator"""
	p[0] = p[1]

def p_assignment(p):
	"""assignment : identifier assignment_operator expression"""
	if len(p) == 4 and condition_blocks[current_condition_block]:
		if p[2] == '=':
			identifiers[p[1]] = parse_object(p[3])
		elif p[2] == "+=":
			identifiers[p[1]] += parse_object(p[3])
		elif p[2] == "-=":
			identifiers[p[1]] -= parse_object(p[3])
		elif p[2] == "*=":
			identifiers[p[1]] *= parse_object(p[3])
		elif p[2] == "/=":
			identifiers[p[1]] /= parse_object(p[3])
		p[0] = (p[1], p[2], p[3])

def p_io_operation(p):
	"""io_operation : input_operation
			 		| output_operation"""

def p_input_operation(p):
	"""input_operation : identifier send_operator input_operator
					   | identifier send_operator input_operator send_operator expression"""

def p_output_operation(p):
	"""output_operation : output_operator send_operator expression"""
	if len(p) == 4:
		print(p[3])

def p_condition_statement(p):
	"""condition_statement : condition_specification_operator expression post_condition_evaluation_block
						   | condition_specification_operator expression post_condition_evaluation_block condition_extension"""
	global current_condition_block
	current_condition_block += 1
	condition_blocks[current_condition_block] = p[2]
	print(condition_blocks)

def p_post_condition_evaluation_block(p):
	"""post_condition_evaluation_block : post_condition_evaluation_block_opening_operator compound_statement post_condition_evaluation_block_closing_operator"""
	
def p_condition_extension(p):
	"""condition_extension : alternative_condition_specification_operator expression post_condition_evaluation_block
						   | alternative_condition_specification_operator expression post_condition_evaluation_block condition_extension
					       | else"""

def p_else(p):
	"""else : else_operator post_condition_evaluation_block"""
	p[0] = (p[1], p[2], p[3])

def p_error(t):
	try:
		print("Syntax error at '%s'" % t.value)
	except AttributeError as e:
		pass

if __name__ == "__main__":
	parser = yacc.yacc()
	if len(argv) > 1:
		_file = argv[1]
		with open(_file) as f:
			data = f.read()
			parser.parse(data)
	else:
		print("Lilith cannot execute that which lieth in the void.")
		exit()
	# flatten = lambda lst: reduce(lambda l, i: l + flatten(i) if isinstance(i, (list, tuple)) else l + [i], lst, [])  