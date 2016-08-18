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
	"send_operator"
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

def t_error(t):
    print("Illegal character '%s'" % t.value[0])
    exit()

lex.lex()