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
/* stringDup
makes a duplicate string

parameters: char*    // this would be the original string
returns: char*       // returns the copy
*/
char* stringDup(char* s);

/* handle_int_operations
this helper function handles various operations with int and int

parameters: enum OPERATORS, struct RAM_VALUE, struct RAM_VALUE, int
returns: bool
*/
bool handle_int_operations(enum OPERATORS op, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line);

/* handle_real_operations
this helper function handles various operations with real and real

parameters: enum OPERATORS, struct RAM_VALUE, struct RAM_VALUE, struct RAM_VALUE* (returned via pointer parameter), int
returns: bool
*/
bool handle_real_operations(enum OPERATORS op, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line);

/* handle_int_real_operations
this helper function handles various operations with int and real

parameters: enum OPERATORS, struct RAM_VALUE, struct RAM_VALUE, struct RAM_VALUE* (returned via pointer parameter), int
returns: bool
*/
bool handle_int_real_operations(enum OPERATORS op, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line);

/* handle_real_int_operations
this helper function handles various operations with real and int

parameters: enum OPERATORS, struct RAM_VALUE, struct RAM_VALUE, struct RAM_VALUE* (returned via pointer parameter), int
returns: bool
*/
bool handle_real_int_operations(enum OPERATORS op, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line);

/* handle_str_operations
this helper function handles various operations with real and int

parameters: enum OPERATORS, struct RAM_VALUE, struct RAM_VALUE, struct RAM_VALUE* (returned via pointer parameter), int
returns: bool
*/
bool handle_str_operations(enum OPERATORS op, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line);

//------------------------------------------------------------------------------------------------------------------------------------------------------------>

/* handle_print
implementation for print function call

parameters: struct ELEMENT*, struct RAM*, int
returns: bool
*/
bool handle_print(struct ELEMENT* parameter, struct RAM* memory, int line);


/* handle_int_conversion
implementation for int function call

parameters: char*, struct RAM*, struct RAM_VALUE* (pointer parameter that is returned), int
returns: struct RAM_VALUE*
*/
struct RAM_VALUE* handle_int_conversion(char* var_name, struct RAM* memory, int line);

/* handle_float_conversion
implementation for float function call

parameters: char*, struct RAM*, struct RAM_VALUE* (pointer parameter that is returned), int
returns: struct RAM_VALUE*
*/
struct RAM_VALUE* handle_float_conversion(char* var_name, struct RAM* memory, int line);

//----------------------------------------------------------------------------------------------------------------------------------------------------->

/* retrieve_value
helper function for execute_binary_expression
Retrieves the value of a unary expression, which can be an integer 
literal or a variable. Handles semantic errors for undefined variables
or unsupported element types.

parameters: struct UNARY_EXPR*, struct RAM*, struct RAM_VALUE* (for value), int (for line)
returns: bool
*/
bool retrieve_value(struct UNARY_EXPR* unary_expr, struct RAM* memory, struct RAM_VALUE* value, int line);

/* execute_binary_expression
this is a helper for if the rhs is a binary expression
Evaluates a binary expression by retrieving the values of the left-hand 
side (lhs) and right-hand side (rhs), and performing the specified operation.
Supports addition, subtraction, multiplication, division, modulus, and exponentiation.
Handles semantic errors such as division by zero or unsupported operators.

parameters: struct EXPR*, struct RAM*, struct RAM_VALUE* (for result), int (for line)
returns: bool
*/
bool execute_binary_expression(struct EXPR* expr, struct RAM* memory, struct RAM_VALUE* result, int line);

/*evaluate_expression
used in execute_assignment and execute_while_loop to evaluate an expression

parameters: struct EXPR*, struct RAM*, struct RAM_VALUE* (returned as a pointer parameter), int line
returns: bool
*/
bool evaluate_expression(struct EXPR* expression, struct RAM* memory, struct RAM_VALUE* result, int line);

//----------------------------------------------------------------------------------------------------------------------------------------------------->


/* execute_assignment_to_function_call
is an assignemnt but rhs is a function calll
this handles the additional input, int and float functions

parameters: struct FUNCTION_CALL, struct RAM*, int
returns: struct RAM_VALUE*
*/
struct RAM_VALUE* execute_assignment_to_function_call(struct FUNCTION_CALL* function_call, struct RAM* memory, int line);



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

//----------------------------------------------------------------------------------------------------------------------------------------------------->

/* execute_while_stmt
execute stmts within a while loop.

parameters: struct STMT*, struct RAM*, int
returns: bool
*/
bool execute_while_stmt(struct STMT* stmt, struct RAM* memory, int og_line);

/* execute_while_loop
called if the type is of while_loop

parameters: struct STMT*, struct RAM*, int
returns: bool
*/
bool execute_while_loop(struct STMT* stmt, struct RAM* memory, int og_line);

/* execute_statement
this is used to execute a statement 

parameters: struct STMT* stmt, struct RAM*
returns: bool
*/
bool execute_statement(struct STMT* stmt, struct RAM* memory);


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
