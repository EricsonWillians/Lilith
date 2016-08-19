from ply import lex
from sys import exit

tokens = [
	"plus",
	"minus",
	"mult",
	"div",
	"mod",
	"pow",
	"open_parenthesis",
	"close_parenthesis",
	"identifier",
	"string",
	"number",
	"end",
	"simple_assignment_operator",
	"addition_assignment_operator",
	"subtraction_assignment_operator",
	"multiplication_assignment_operator",
	"division_assignment_operator",
	"modulo_assignment_operator",
	"input_operator",
	"output_operator",
	"send_operator",
	"greater_than_relational_operator",
	"greater_than_or_equal_to_relational_operator",
	"less_than_relational_operator",
	"less_than_or_equal_to_relational_operator",
	"equal_to_relational_operator",
	"not_equal_to_relational_operator",
	"alt_not_equal_to_relational_operator",
	"condition_specification_operator",
	"alternative_condition_specification_operator",
	"else_operator",
	"post_condition_evaluation_block_opening_operator",
	"post_condition_evaluation_block_closing_operator"
]

t_ignore = r""" 	
"""
t_identifier = r"[a-zA-Z_][a-zA-Z0-9_]*"
t_string = r"\*.*\*"
t_number = r"[+-]?(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?"
t_plus = r"\+"
t_minus = r"-"
t_mult = r"\*"
t_div = r"/"
t_mod = r"%"
t_pow = r"\^"
t_open_parenthesis = r"\("
t_close_parenthesis = r"\)"
t_end = r"->"
t_simple_assignment_operator = r'='
t_addition_assignment_operator = r'\+='
t_subtraction_assignment_operator = r'-='
t_multiplication_assignment_operator = r'\*='
t_division_assignment_operator = r'/='
t_modulo_assignment_operator = r'%='
t_input_operator = r'\[i\]'
t_output_operator = r'\[o\]'
t_send_operator = r'<-'
t_greater_than_relational_operator = r'>'
t_greater_than_or_equal_to_relational_operator = r'>='
t_less_than_relational_operator = r'<'
t_less_than_or_equal_to_relational_operator = r'<='
t_equal_to_relational_operator = r'=='
t_not_equal_to_relational_operator = r'<>'
t_alt_not_equal_to_relational_operator = r'!='
t_condition_specification_operator = r'\?'
t_alternative_condition_specification_operator = r'\?\?'
t_else_operator = r'\?\?\?'
t_post_condition_evaluation_block_opening_operator = r'<\?'
t_post_condition_evaluation_block_closing_operator = r'\?>'

def t_error(t):
	try:
		print("Illegal character '%s'" % t.value[0])
	except AttributeError as e:
		print("Error")

lex.lex()