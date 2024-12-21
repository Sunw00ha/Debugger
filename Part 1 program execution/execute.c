
/*execute.c*/

//
/*Implementation of functions declared in execute.h
Handles the execution of nuPython program graphs, including assignment 
statements, function calls, and expression evaluation.
Also includes semantic error handling during execution.
*/
//
// Irene ha
// Northwestern University
// CS 211
// Fall 2024
// 
// Starter code: Prof. Joe Hummel
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>
#include <assert.h>
#include <math.h>

#include "programgraph.h"
#include "ram.h"
#include "execute.h"


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
bool execute_function_call(struct STMT* stmt, struct RAM* memory){
  char* function_name = stmt->types.function_call->function_name; // get the function name
  struct ELEMENT* parameter = stmt->types.function_call->parameter; // get the parameter

  if (strcmp(function_name, "print") == 0) { // if print is called...
    if (parameter == NULL) {
      printf("\n");
    } else if (parameter->element_type == ELEMENT_STR_LITERAL) { // for string literal
      printf("%s\n", parameter->element_value);
    } else if (parameter->element_type == ELEMENT_INT_LITERAL) { // for int literal
      // Convert string to integer and print
      char* int_literal_str = parameter->element_value;
      int int_value = atoi(int_literal_str);
      printf("%d\n", int_value);
    } else if (parameter->element_type == ELEMENT_IDENTIFIER) { // for identifier
      // if its a Variable
      char* var_name = parameter->element_value; // get variable name
      struct RAM_VALUE* value = ram_read_cell_by_name(memory, var_name); // get value by calling ram_read_cell_by_name function

      if (value == NULL) {
        // Semantic error: undefined variable
        printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
        return false; // Stop execution
      }

      printf("%d\n", value->types.i);
    }
  return true; 
  }
  return false; // unsupported function if it gets here
}


/* retrieve_value
helper function for execute_binary_expression
Retrieves the value of a unary expression, which can be an integer 
literal or a variable. Handles semantic errors for undefined variables
or unsupported element types.

parameters: struct UNARY_EXPR*, struct RAM*, int* (for value), int (for line)
returns: bool
*/
bool retrieve_value(struct UNARY_EXPR* unary_expr, struct RAM* memory, int* value, int line){
  if (unary_expr->element->element_type == ELEMENT_INT_LITERAL) {
    // convert from int literal to int
    *value = atoi(unary_expr->element->element_value);
    return true;
  } else if (unary_expr->element->element_type == ELEMENT_IDENTIFIER) {
    char* var_name = unary_expr->element->element_value; // set var_name
    struct RAM_VALUE* var_value = ram_read_cell_by_name(memory, var_name); // set var_value with ram_read_by_cell_by_name

    if (var_value == NULL) { // Semantic error bc of an undefined variable
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, line);
      return false;
    }

    if (var_value->value_type != RAM_TYPE_INT) { // Semantic error bc the var is not an int
      printf("**SEMANTIC ERROR: name '%s' is not an integer\n", var_name);
      return false;
    }

    *value = var_value->types.i;
    return true;
  }

  return false; // returns false if its an unsupported element type
}


/* execute_binary_expression
this is a helper for if the rhs is a binary expression
Evaluates a binary expression by retrieving the values of the left-hand 
side (lhs) and right-hand side (rhs), and performing the specified operation.
Supports addition, subtraction, multiplication, division, modulus, and exponentiation.
Handles semantic errors such as division by zero or unsupported operators.

parameters: struct EXPR*, struct RAM*, int* (for result), int (for line)
returns: bool
*/
bool execute_binary_expression(struct EXPR* expr, struct RAM* memory, int* result, int line) {
  int lhs_val = 0;
  int rhs_val = 0;

  //get the lhs val
  if (expr->lhs == NULL || retrieve_value(expr->lhs, memory, &lhs_val, line) == false) {
    return false; // we ran into a semantic error, which is already handled by retrieve_value()
  }

  //get the rhs val
  if (expr->rhs == NULL || !retrieve_value(expr->rhs, memory, &rhs_val, line)) {
    return false; // we ran into a semantic error, which is already handled by retrieve_value()
  }

  // handle the diff operators
  if (expr->operator == OPERATOR_PLUS) {
    *result = lhs_val + rhs_val;

    return true;
  } else if (expr->operator == OPERATOR_MINUS) {
    *result = lhs_val - rhs_val;

    return true;
  } else if (expr->operator == OPERATOR_ASTERISK) {
    *result = lhs_val * rhs_val;

    return true;
  } else if (expr->operator == OPERATOR_DIV) {
    if (rhs_val == 0) {
      printf("**SEMANTIC ERROR: division by zero\n"); // account for if its dividing by 0
      return false;
    } else { // rhs val is not 0
      *result = lhs_val / rhs_val;

      return true;
    }
  } else if (expr->operator == OPERATOR_MOD) {
    if (rhs_val == 0) {
      printf("**SEMANTIC ERROR: division by zero\n"); // account for if its dividing by 0
      return false;
    } else { // rhs_val is not 0
      *result = lhs_val % rhs_val;
      return true;
    }  
  } else if (expr->operator == OPERATOR_POWER) {
    *result = (int)pow(lhs_val, rhs_val);
    return true;

  } else { // unsupported operation
    printf("**SEMANTIC ERROR: unsupported operator\n");
    return false;
  }
return true; // operator executed successfully
}




/* execute_assignment
called if the type is of assignment
Executes an assignment statement by evaluating the right-hand side (rhs) 
expression and updating the corresponding variable in memory. Handles 
semantic errors for undefined variables, type mismatches, or unsupported expressions.

parameters: struct STMT*, struct RAM* 
returns: bool
*/
bool execute_assignment(struct STMT* stmt, struct RAM* memory) {
  if (stmt->stmt_type == STMT_ASSIGNMENT) { // assume this is going to be true
    // extract the fields of the assignment stmt
    char* var_name = stmt->types.assignment->var_name;
    bool isDeref = stmt->types.assignment->isPtrDeref;
    struct VALUE* rhs = stmt->types.assignment->rhs;

    int result;
    if (rhs->types.expr->isBinaryExpr == true) { // if this is a binary expression
      
      if (execute_binary_expression(rhs->types.expr, memory, &result, stmt->line) == false) {
        return false; // semantic error outputted by the helper function
        }
    } else {
      if (retrieve_value(rhs->types.expr->lhs, memory, &result, stmt->line) == false) {
        return false; 
      }
    }

    // if the output is true
    struct RAM_VALUE new_value;
    new_value.value_type = RAM_TYPE_INT;
    new_value.types.i = result;


    if (!ram_write_cell_by_name(memory, new_value, var_name)) {
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
      return false;
    }
  return true; // assignment successful


}
return false; // in the case it is not of type assignment which should not be the case lol it will return false
}


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
void execute(struct STMT* program, struct RAM* memory)
{
  struct STMT* stmt = program; // start from the first stmt

  while (stmt != NULL) {
    if (stmt->stmt_type == STMT_ASSIGNMENT) {
      if (execute_assignment(stmt, memory) == false) {
        return; // stop we ran into an error
      };
      stmt = stmt->types.assignment->next_stmt; // advance to next stmt
      
    } else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      if (execute_function_call(stmt, memory) == false) {
        return; // stop we ran into an error
      };
      stmt = stmt->types.function_call->next_stmt; // advance to next stmt

    } else if (stmt->stmt_type == STMT_PASS) {
      stmt = stmt->types.pass->next_stmt;
    }
  } // while
}