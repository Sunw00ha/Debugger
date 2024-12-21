/*execute.h*/

//
// Executes nuPython program, given as a Program Graph.
// 
// modified by: Irene Ha
//
// Prof. Joe Hummel
// Northwestern University
// CS 211
//

#pragma once

#include "programgraph.h"
#include "ram.h"

//
// helper functions
//
/* execute_function_call
Executes a function call statement from the program graph.
Supports built-in functions like "print", handling string, integer, 
and variable outputs. Performs semantic error checks for undefined 
variables or unsupported function calls.

parameters: struct STMT*, struct RAM* 
returns: bool
*/
bool execute_function_call(struct STMT* stmt, struct RAM* memory);

/* execute_assignment
called if the type is of assignment
Executes an assignment statement by evaluating the right-hand side (rhs) 
expression and updating the corresponding variable in memory. Handles 
semantic errors for undefined variables, type mismatches, or unsupported expressions.

parameters: struct STMT*, struct RAM* 
returns: bool
*/
bool execute_assignment(struct STMT* stmt, struct RAM* memory);

/* retrieve_value
helper function for execute_binary_expression
Retrieves the value of a unary expression, which can be an integer 
literal or a variable. Handles semantic errors for undefined variables
or unsupported element types.

parameters: struct UNARY_EXPR*, struct RAM*, int* (for value), int (for line)
returns: bool
*/
bool retrieve_value(struct UNARY_EXPR* unary_expr, struct RAM* memory, int* value, int line);

/* execute_binary_expression
this is a helper for if the rhs is a binary expression
Evaluates a binary expression by retrieving the values of the left-hand 
side (lhs) and right-hand side (rhs), and performing the specified operation.
Supports addition, subtraction, multiplication, division, modulus, and exponentiation.
Handles semantic errors such as division by zero or unsupported operators.

parameters: struct EXPR*, struct RAM*, int* (for result), int (for line)
returns: bool
*/
bool execute_binary_expression(struct EXPR* expr, struct RAM* memory, int* result, int line);

//
// Public functions:
//

//
// execute
//
// Given a nuPython program graph and a memory, 
// executes the statements in the program graph.
// If a semantic error occurs (e.g. type error),
// and error message is output, execution stops,
// and the function returns.
//
void execute(struct STMT* program, struct RAM* memory);
