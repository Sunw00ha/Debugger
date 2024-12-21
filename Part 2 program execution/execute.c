/*execute.c*/

//
/*
Implementation of functions declared in execute.h.
This file is responsible for the execution of nuPython program graphs,
including the handling of assignment statements, function calls, 
expression evaluation, and while loops. It also incorporates helper
functions to manage different data types and operations.
Additionally, semantic error handling is performed during execution,
ensuring program correctness and meaningful error reporting.
*/
//
// Irene Ha
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
#include <ctype.h>

#include "programgraph.h"
#include "ram.h"
#include "execute.h"


//
// helper functions
//

/* stringDup
makes a duplicate string

parameters: char*    // this would be the original string
returns: char*       // returns the copy
*/
char* stringDup(char* s) {
  if (s == NULL) {
    return NULL;
  }
  
  size_t len = strlen(s) + 1;  // +1 for the null terminator

  char* copy = (char*)malloc(len * sizeof(char));
  if (copy == NULL) {
    //printf("ERROR: OUT OF MEMORY\n");
    exit(1);  // Exit or handle the error appropriately
  }

  strcpy(copy, s);
  return copy;
}

//----------------------------------------------------------------------------------------------------------------------------------

/* handle_int_operations
this helper function handles various operations with int and int

parameters: enum OPERATORS, struct RAM_VALUE, struct RAM_VALUE, struct RAM_VALUE* (returned via pointer parameter), int
returns: bool
*/
bool handle_int_operations(enum OPERATORS op, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line) {
  switch (op) {
    case OPERATOR_PLUS:
      result->value_type = RAM_TYPE_INT; // always set type to int for the regular operands
      result->types.i = lhs.types.i + rhs.types.i; // do + operation with the values
      break;

    case OPERATOR_MINUS:
      result->value_type = RAM_TYPE_INT;
      result->types.i = lhs.types.i - rhs.types.i; // do - operation with the values
      break;

    case OPERATOR_ASTERISK:
      result->value_type = RAM_TYPE_INT;
      result->types.i = lhs.types.i * rhs.types.i; // do * operation with the values
      break;

    case OPERATOR_DIV:
      if (rhs.types.i == 0) { // Check for division by zero
        printf("ZeroDivisionError: division by zero\n");
        return false;
      }
      result->value_type = RAM_TYPE_INT;
      result->types.i = lhs.types.i / rhs.types.i; // do / operation with the values
      break;

    case OPERATOR_MOD:
      if (rhs.types.i == 0) { // Check for modulus by zero
        printf("ZeroDivisionError: division by zero\n");
        return false;
      }
      result->value_type = RAM_TYPE_INT;
      result->types.i = lhs.types.i % rhs.types.i; // do % operation with the values
      break;

    case OPERATOR_POWER:
      result->value_type = RAM_TYPE_INT;
      result->types.i = pow(lhs.types.i, rhs.types.i); // do ** operation with the values
      break;

    case OPERATOR_EQUAL:
      result->value_type = RAM_TYPE_BOOLEAN; // always set type to bool for relational operands
      result->types.i = (lhs.types.i == rhs.types.i);
      break;

    case OPERATOR_NOT_EQUAL:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.i != rhs.types.i);
      break;

    case OPERATOR_LT:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.i < rhs.types.i);
      break;

    case OPERATOR_LTE:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.i <= rhs.types.i);
      break;

    case OPERATOR_GT:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.i > rhs.types.i);
      break;

    case OPERATOR_GTE:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.i >= rhs.types.i);
      break;

    default:
      printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
      return false;
  }

  return true; // Operation successful
}

/* handle_real_operations
this helper function handles various operations with real and real

parameters: enum OPERATORS, struct RAM_VALUE, struct RAM_VALUE, struct RAM_VALUE* (returned via pointer parameter), int
returns: bool
*/
bool handle_real_operations(enum OPERATORS op, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line) {
  switch (op) {
    case OPERATOR_PLUS:
      result->value_type = RAM_TYPE_REAL; // always set type to real for the regular operands
      result->types.d = lhs.types.d + rhs.types.d;
      break;

    case OPERATOR_MINUS:
      result->value_type = RAM_TYPE_REAL;
      result->types.d = lhs.types.d - rhs.types.d;
      break;

    case OPERATOR_ASTERISK:
      result->value_type = RAM_TYPE_REAL;
      result->types.d = lhs.types.d * rhs.types.d;
      break;

    case OPERATOR_DIV:
      if (rhs.types.d == 0.0) { // Check for division by zero
        printf("ZeroDivisionError: division by zero\n");
        return false;
      }
      result->value_type = RAM_TYPE_REAL;
      result->types.d = lhs.types.d / rhs.types.d;
      break;

    case OPERATOR_MOD:
      if (rhs.types.d == 0.0) { // Check for modulus by zero
        printf("ZeroDivisionError: modulus by zero\n");
        return false;
      }
      result->value_type = RAM_TYPE_REAL;
      result->types.d = fmod(lhs.types.d, rhs.types.d);
      break;

    case OPERATOR_POWER:
      result->value_type = RAM_TYPE_REAL;
      result->types.d = pow(lhs.types.d, rhs.types.d); // Use pow() from <math.h>
      break;

    case OPERATOR_EQUAL:
      result->value_type = RAM_TYPE_BOOLEAN; // always set type to bool for the relational operands
      result->types.i = (lhs.types.d == rhs.types.d);
      break;

    case OPERATOR_NOT_EQUAL:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.d != rhs.types.d);
      break;

    case OPERATOR_LT:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.d < rhs.types.d);
      break;

    case OPERATOR_LTE:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.d <= rhs.types.d);
      break;

    case OPERATOR_GT:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.d > rhs.types.d);
      break;

    case OPERATOR_GTE:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.d >= rhs.types.d);
      break;

    default:
      printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
      return false;
  }

  return true; // Operation successful
}

/* handle_int_real_operations
this helper function handles various operations with int and real

parameters: enum OPERATORS, struct RAM_VALUE, struct RAM_VALUE, struct RAM_VALUE* (returned via pointer parameter), int
returns: bool
*/
bool handle_int_real_operations(enum OPERATORS op, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line) {
  switch (op) {
    case OPERATOR_PLUS:
      result->value_type = RAM_TYPE_REAL; // always set type to real for the regular operands
      result->types.d = (double)lhs.types.i + rhs.types.d;
      break;

    case OPERATOR_MINUS:
      result->value_type = RAM_TYPE_REAL;
      result->types.d = (double)lhs.types.i - rhs.types.d;
      break;

    case OPERATOR_ASTERISK:
      result->value_type = RAM_TYPE_REAL;
      result->types.d = (double)lhs.types.i * rhs.types.d;
      break;

    case OPERATOR_DIV:
      if (rhs.types.d == 0.0) { // Check for division by zero
        printf("ZeroDivisionError: division by zero\n");
        return false;
      }
      result->value_type = RAM_TYPE_REAL;
      result->types.d = (double)lhs.types.i / rhs.types.d;
      break;

    case OPERATOR_POWER:
      result->value_type = RAM_TYPE_REAL;
      result->types.d = pow((double)lhs.types.i, rhs.types.d); // Convert int to double
      break;

    case OPERATOR_EQUAL:
      result->value_type = RAM_TYPE_BOOLEAN; // always set type to bool for the relational operands
      result->types.i = ((double)lhs.types.i == rhs.types.d);
      break;

    case OPERATOR_NOT_EQUAL:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = ((double)lhs.types.i != rhs.types.d);
      break;

    case OPERATOR_LT:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = ((double)lhs.types.i < rhs.types.d);
      break;

    case OPERATOR_LTE:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = ((double)lhs.types.i <= rhs.types.d);
      break;

    case OPERATOR_GT:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = ((double)lhs.types.i > rhs.types.d);
      break;

    case OPERATOR_GTE:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = ((double)lhs.types.i >= rhs.types.d);
      break;

    default:
      printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
      return false;
  }

  return true; // Operation successful
}

/* handle_real_int_operations
this helper function handles various operations with real and int

parameters: enum OPERATORS, struct RAM_VALUE, struct RAM_VALUE, struct RAM_VALUE* (returned via pointer parameter), int
returns: bool
*/
bool handle_real_int_operations(enum OPERATORS op, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line) {
  switch (op) {
    case OPERATOR_PLUS:
      result->value_type = RAM_TYPE_REAL; // always set type to real for the regular operands
      result->types.d = lhs.types.d + (double)rhs.types.i;
      break;

    case OPERATOR_MINUS:
      result->value_type = RAM_TYPE_REAL;
      result->types.d = lhs.types.d - (double)rhs.types.i;
      break;

    case OPERATOR_ASTERISK:
      result->value_type = RAM_TYPE_REAL;
      result->types.d = lhs.types.d * (double)rhs.types.i;
      break;

    case OPERATOR_DIV:
      if (rhs.types.i == 0) { // Check for division by zero
        printf("ZeroDivisionError: division by zero\n");
        return false;
      }
      result->value_type = RAM_TYPE_REAL;
      result->types.d = lhs.types.d / (double)rhs.types.i;
      break;

    case OPERATOR_MOD:
      if (rhs.types.i == 0) { // Check for modulus by zero
        printf("ZeroDivisionError: modulus by zero\n");
        return false;
      }
      result->value_type = RAM_TYPE_REAL;
      result->types.d = fmod(lhs.types.d, (double)rhs.types.i);
      break;

    case OPERATOR_POWER:
      result->value_type = RAM_TYPE_REAL;
      result->types.d = pow(lhs.types.d, (double)rhs.types.i); // Convert int to double
      break;

    case OPERATOR_EQUAL:
      result->value_type = RAM_TYPE_BOOLEAN; // always set type to bool for the relational operands
      result->types.i = (lhs.types.d == (double)rhs.types.i);
      break;

    case OPERATOR_NOT_EQUAL:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.d != (double)rhs.types.i);
      break;

    case OPERATOR_LT:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.d < (double)rhs.types.i);
      break;

    case OPERATOR_LTE:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.d <= (double)rhs.types.i);
      break;

    case OPERATOR_GT:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.d > (double)rhs.types.i);
      break;

    case OPERATOR_GTE:
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (lhs.types.d >= (double)rhs.types.i);
      break;

    default:
      printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
      return false;
  }

  return true; // Operation successful
}

/* handle_str_operations
this helper function handles various operations with real and int

parameters: enum OPERATORS, struct RAM_VALUE, struct RAM_VALUE, struct RAM_VALUE* (returned via pointer parameter), int
returns: bool
*/
bool handle_str_operations(enum OPERATORS op, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line) {
  switch (op) {
    case OPERATOR_PLUS: { // String concatenation
      size_t len = strlen(lhs.types.s) + strlen(rhs.types.s) + 1; // Calculate total length
      result->types.s = (char*)malloc(len * sizeof(char)); // Allocate memory

      if (result->types.s == NULL) { // out of mem
        return false;
      }

      strcpy(result->types.s, lhs.types.s); // Copy first string
      strcat(result->types.s, rhs.types.s); // Append second string
      result->value_type = RAM_TYPE_STR;
      break;
    }

    case OPERATOR_EQUAL: {
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (strcmp(lhs.types.s, rhs.types.s) == 0); // compare the string if it equals 0 that means its equal
      break;
    }

    case OPERATOR_NOT_EQUAL: {
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (strcmp(lhs.types.s, rhs.types.s) != 0);
      break;
    }

    case OPERATOR_LT: // <
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (strcmp(lhs.types.s, rhs.types.s) < 0); // strcmp being < 0 that means s1 < s2
      break;

    case OPERATOR_GT: // >
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (strcmp(lhs.types.s, rhs.types.s) > 0); // strcmp being > 0 that means s1 > s2
      break;

    case OPERATOR_LTE: // <=
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (strcmp(lhs.types.s, rhs.types.s) <= 0);
      break;

    case OPERATOR_GTE: // >=
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = (strcmp(lhs.types.s, rhs.types.s) >= 0);
      break;

    default:
      printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
      return false;
  }

  return true; // Operation successful
}

//----------------------------------------------------------------------------------------------------------------------------------

//
// functional call helper functions
//
/* handle_print
implementation for print function call

parameters: struct ELEMENT*, struct RAM*, int
returns: bool
*/
bool handle_print(struct ELEMENT* parameter, struct RAM* memory, int line) {
  // printf("hello\n"); // debug
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
    char* var_name = parameter->element_value; // Get variable name
    struct RAM_VALUE* value = ram_read_cell_by_name(memory, var_name); // Get value by calling this function

    if (value == NULL) {
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, line); // undefined variable
      // printf("hello\n"); // debug
      return false; // Stop execution
    }

    // printf("helloooooooooooOooOOOoOOOOOOOOOOOOO!!!!!!\n"); // debug
    // print based on the specific type
    if (value->value_type == RAM_TYPE_STR) {
      printf("%s\n", value->types.s); // Print string value
    } else if (value->value_type == RAM_TYPE_INT) {
      // printf("printing int\n"); // debug
      printf("%d\n", value->types.i); // Print integer value
    } else if (value->value_type == RAM_TYPE_REAL) {
      // printf("printing real\n"); // debug
      printf("%lf\n", value->types.d); // Print real value
    } else if (value->value_type == RAM_TYPE_BOOLEAN) {
      if (value->types.i == 1) {
        printf("True\n");
      } else if (value->types.i == 0) {
        printf("False\n");
      }
    } else {
      return false; // Unsupported type
    }
  } else if (parameter->element_type == ELEMENT_REAL_LITERAL) {
    double real_value = atof(parameter->element_value);
    printf("%lf\n", real_value); // Print the converted value
  } else if (parameter->element_type == ELEMENT_TRUE) { // bool type
    printf("True\n");
  } else if (parameter->element_type == ELEMENT_FALSE) { // bool type
    printf("False\n");
  }

  return true;
}



/* handle_int_conversion
implementation for int function call

parameters: char*, struct RAM*, struct RAM_VALUE* (pointer parameter that is returned), int
returns: struct RAM_VALUE*
*/
struct RAM_VALUE* handle_int_conversion(char* var_name, struct RAM* memory, int line) {
  // printf("hello from beginning of handle_int\n"); // debug
  struct RAM_VALUE* value = ram_read_cell_by_name(memory, var_name);
  // Check if the variable exists and is of type string
  if (value == NULL) {
    printf("variable is not defined\n");
    return NULL;
  }
  if (value->value_type != RAM_TYPE_STR) {
    printf("variable is not a string\n");
    return NULL;
  }
  if (value->types.s == NULL) {
    return NULL;
  }

  // printf("value->types.s: %s\n", value->types.s); // debug
  // printf("value->value_type: %d\n", value->value_type); // debug

  char* str = value->types.s; // loop through every character in the string to verify that each one is a 0
  char* temp = str;
  bool is_zero = true;
  while (*str != '\0') {
    if (*str != '0') {
      is_zero = false; // if this is set to false it means that we will get that atoi is 0 because our input is genuinly wrong
      break;
    }
    str++;
  }

  // Convert the string to an integer
  int int_value = atoi(value->types.s); // convert to int

  if (int_value == 0) {
    if (is_zero == false) {
      printf("**SEMANTIC ERROR: invalid string for int() (line %d)\n", line);
      return NULL;
    }
  }

  // update the result
  // struct RAM_VALUE* result = NULL;
  // Allocate memory for the result
  struct RAM_VALUE* result = (struct RAM_VALUE*)malloc(sizeof(struct RAM_VALUE));
  if (result == NULL) {
    printf("out of mem\n");
    return NULL;
  }

  result->value_type = RAM_TYPE_INT;
  result->types.i = int_value;
  // printf("hello from end of handle_int\n"); // debug
  //printf("%i", result->types.i); // debug

  return result; // Conversion successful
}

/* handle_float_conversion
implementation for float function call

parameters: char*, struct RAM*, struct RAM_VALUE* (pointer parameter that is returned), int
returns: struct RAM_VALUE*
*/
struct RAM_VALUE* handle_float_conversion(char* var_name, struct RAM* memory, int line) {
  struct RAM_VALUE* value = ram_read_cell_by_name(memory, var_name);
  // Check if the variable exists and is of type string
  if (value == NULL) {
    printf("variable is not defined\n");
    return false;
  }
  if (value->value_type != RAM_TYPE_STR) {
    printf("variable is not a string\n");
    return false;
  }
  if (value->types.s == NULL) {
    return NULL;
  }

  char* str = value->types.s; // loop through every character in the string to verify that each one is a 0 or .
  char* temp = str;
  bool is_zero = true;
  while (*str != '\0') {
    if (*str != '0') {
      if (*str != '.') { 
        is_zero = false; // if this is set to false it means that we will get that atof is 0 because our input is genuinly wrong
      break;
      }
    }
    str++;
  }


  // Convert the string to an integer
  double double_value = atof(value->types.s);

  if (double_value == 0) {
    if (is_zero == false) {
      printf("**SEMANTIC ERROR: invalid string for float() (line %d)\n", line);
      return NULL;
    }
  }

  // update the result
  // Allocate memory for the result
  struct RAM_VALUE* result = (struct RAM_VALUE*)malloc(sizeof(struct RAM_VALUE));
  if (result == NULL) {
    printf("out of mem\n");
    return NULL;
  }

  result->value_type = RAM_TYPE_REAL;
  result->types.d = double_value;

  return result; // Conversion successful
}


//----------------------------------------------------------------------------------------------------------------------------------

/* retrieve_value
helper function for execute_binary_expression
Retrieves the value of a unary expression, which can be an integer 
literal or a variable. Handles semantic errors for undefined variables
or unsupported element types.

parameters: struct UNARY_EXPR*, struct RAM*, struct RAM_VALUE* (for value), int (for line)
returns: bool
*/
bool retrieve_value(struct UNARY_EXPR* unary_expr, struct RAM* memory, struct RAM_VALUE* value, int line){
  // printf("entered retrieve value\n"); // debug
  if (unary_expr->element->element_type == ELEMENT_INT_LITERAL) {
    value->value_type = RAM_TYPE_INT;
    value->types.i = atoi(unary_expr->element->element_value);// convert from int literal to int
    return true;
  } else if (unary_expr->element->element_type == ELEMENT_REAL_LITERAL) {
    value->value_type = RAM_TYPE_REAL;
    value->types.d = atof(unary_expr->element->element_value); // convert to double
    return true;
  } else if (unary_expr->element->element_type == ELEMENT_STR_LITERAL) {
    value->value_type = RAM_TYPE_STR;
    // printf("hi im a string literal\n"); // debug

    value->types.s = (char*)malloc((strlen(unary_expr->element->element_value) + 1) * sizeof(char)); // Allocate memory
    if (value->types.s == NULL) { // Check for out of mem
      return false;
    }
    strcpy(value->types.s, unary_expr->element->element_value); // Copy the string into value->types.s
    // printf("%d\n", value->value_type); // debug
    // printf("%s\n", value->types.s); // debug

    return true;


  } else if (unary_expr->element->element_type == ELEMENT_TRUE) { // Boolean true
    value->value_type = RAM_TYPE_BOOLEAN;
    value->types.i = 1;
    return true;

  } else if (unary_expr->element->element_type == ELEMENT_FALSE) { // Boolean false
    value->value_type = RAM_TYPE_BOOLEAN;
    value->types.i = 0;
    return true;
  
  } else if (unary_expr->element->element_type == ELEMENT_IDENTIFIER) {
    
    char* var_name = unary_expr->element->element_value; // set var_name
    struct RAM_VALUE* var_value = ram_read_cell_by_name(memory, var_name); // set var_value with ram_read_by_cell_by_name

    if (var_value == NULL) { // Semantic error bc of an undefined variable
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, line);
      // printf("hello\n"); // debug
      printf("**done\n");
      ram_print(memory);
      exit(0); // debug
      return false;
    }

    *value = *var_value; // copy the RAM_VALUE
    free(var_value); // free mem allocated by readcellbyname
    return true;

  } else {
    printf("**SEMANTIC ERROR: invalid element types!!!!!!!!\n");
    return false;
  }
}

/* execute_binary_expression
this is a helper for if the rhs is a binary expression
Evaluates a binary expression by retrieving the values of the left-hand 
side (lhs) and right-hand side (rhs), and performing the specified operation.
Supports addition, subtraction, multiplication, division, modulus, and exponentiation.
Handles semantic errors such as division by zero or unsupported operators.

parameters: struct EXPR*, struct RAM*, struct RAM_VALUE* (for result), int (for line)
returns: bool
*/
bool execute_binary_expression(struct EXPR* expr, struct RAM* memory, struct RAM_VALUE* result, int line) {
  struct RAM_VALUE lhs_val;
  struct RAM_VALUE rhs_val;


  // CHECKS ------------>
  //get the lhs val
  if (expr->lhs == NULL || retrieve_value(expr->lhs, memory, &lhs_val, line) == false) {
    return false; // we ran into a semantic error, which is already handled by retrieve_value()
  }
  //get the rhs val
  if (expr->rhs == NULL || retrieve_value(expr->rhs, memory, &rhs_val, line) == false) {
    return false; // we ran into a semantic error, which is already handled by retrieve_value()
  }
  // CHECK DONE ------------->

  // each handle helper function is going to return true or false for success or failure
  if (lhs_val.value_type == RAM_TYPE_INT && rhs_val.value_type == RAM_TYPE_INT) {
    return handle_int_operations(expr->operator, lhs_val, rhs_val, result, line);
  
  } else if (lhs_val.value_type == RAM_TYPE_REAL && rhs_val.value_type == RAM_TYPE_REAL) {
    return handle_real_operations(expr->operator, lhs_val, rhs_val, result, line);
  
  } else if (lhs_val.value_type == RAM_TYPE_INT && rhs_val.value_type == RAM_TYPE_REAL) {
    return handle_int_real_operations(expr->operator, lhs_val, rhs_val, result, line);
  
  } else if (lhs_val.value_type == RAM_TYPE_REAL && rhs_val.value_type == RAM_TYPE_INT) {
    return handle_real_int_operations(expr->operator, lhs_val, rhs_val, result, line);
  
  } else if (lhs_val.value_type == RAM_TYPE_STR && rhs_val.value_type == RAM_TYPE_STR) {
    return handle_str_operations(expr->operator, lhs_val, rhs_val, result, line);
  
  } else { // The types are not valid
    printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
    return false;
  }
}



/*evaluate_expression
used in execute_assignment if its an expression and execute_while_loop to evaluate an expression which is the only option for while loops

parameters: struct EXPR*, struct RAM*, struct RAM_VALUE* (returned as a pointer parameter), int line
returns: bool
*/
bool evaluate_expression(struct EXPR* expression, struct RAM* memory, struct RAM_VALUE* result, int line) {
  // printf("%d", expression->isBinaryExpr); // debug
  // is a binary expression
  if (expression->isBinaryExpr == true) { 
    // printf("invalid read?\n"); // debug
    struct RAM_VALUE temp_result;
    if (execute_binary_expression(expression, memory, &temp_result, line) == false) {
      // printf("execute_binary entered\n"); // debug
      return false; // Stop execution if theres an error
    }

    *result = temp_result;
    return true;

  } 

  // is not a binary expression
  else if (expression->isBinaryExpr == false) { 
    if (expression == NULL) {
      printf("**SEMANTIC ERROR: invalid left-hand side in expression (line %d)\n", line);
      return false;
    }

    // printf("entered retrieve_value\n"); // debug
    struct RAM_VALUE temp_result;
    bool success = retrieve_value(expression->lhs, memory, &temp_result, line);
    
    //printf("%s", temp_result.types.s);
    // printf("lalala\n"); // debug
    if (success) {
      // printf("%s", temp_result.types.s); // debug
      // printf("lalala2\n"); // debug
      // printf("%d\n", temp_result.value_type); // debug
      // printf("%s\n", temp_result.types.s); // debug
      result->value_type = temp_result.value_type;
      result->types.s = temp_result.types.s;
      // *result = temp_result;
      return true;
    } else {
      return false; // eval expression didnt work
    }

  }

  printf("idk\n");
  return true;
}

//----------------------------------------------------------------------------------------------------------------------------------

/* execute_assignment_to_function_call
is an assignemnt but rhs is a function calll

parameters: struct FUNCTION_CALL, struct RAM*, int
returns: struct RAM_VALUE*
*/
struct RAM_VALUE* execute_assignment_to_function_call(struct FUNCTION_CALL* function_call, struct RAM* memory, int line) {

  // input function
  if (strcmp(function_call->function_name, "input") == 0) { // handle input
    char* message = function_call->parameter->element_value; // Get the message to display to the user

    printf("%s", message); // print message and receive input

    // read user input and allocate memory dynamically
    char input_line[256];
    //char* input_line = (char*) malloc (256 * sizeof(char));
    fgets(input_line, sizeof(input_line), stdin);
    input_line[strcspn(input_line, "\r\n")] = '\0';
    if (input_line == NULL) {
      printf("input_line is NULL uh oh\n");
    }
    

    struct RAM_VALUE* new_value = (struct RAM_VALUE*)malloc(sizeof(struct RAM_VALUE));

    new_value->value_type = RAM_TYPE_STR;
    new_value->types.s = stringDup(input_line); // make this a dupstring of input_line


    return new_value;
  
  // int function
  } else if (strcmp(function_call->function_name, "int") == 0) {

    struct RAM_VALUE* new_value = handle_int_conversion(function_call->parameter->element_value, memory, line);
    return new_value;
  }

  // float function
  else if (strcmp(function_call->function_name, "float") == 0) {
    struct RAM_VALUE* new_value = handle_float_conversion(function_call->parameter->element_value, memory, line);
    // printf("hello\n"); // debug
    return new_value;
  }

  return NULL; // in this case none of the valid function call names were called like input, int, float
}


/* execute_function_call
Executes a function call statement from the program graph.
Supports built-in functions like "print", handling string, integer, 
and variable outputs. Performs semantic error checks for undefined 
variables or unsupported function calls.

parameters: struct STMT*, struct RAM* 
returns: bool
*/
bool execute_function_call(struct STMT* stmt, struct RAM* memory) {
    // Get the function name and its parameter
    char* function_name = stmt->types.function_call->function_name;
    // printf("function name: %s\n", function_name); // debug
    // printf(function_name); // debug
    // printf("\n"); // debug
    struct ELEMENT* parameter = stmt->types.function_call->parameter;

    //print function
    if (strcmp(function_name, "print") == 0) {
      // printf("running handle_print"); // debug
      return handle_print(parameter, memory, stmt->line);
    } 

    else { // Unsupported function
      printf("**SEMANTIC ERROR: unsupported function '%s' (line %d)\n", function_name, stmt->line);
      // exit(0);// debug

      return false;
    }
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

    struct RAM_VALUE return_val;

    if (rhs->value_type == VALUE_FUNCTION_CALL) { // if it is a function call on the rhs...
      struct RAM_VALUE* new_value;

      struct FUNCTION_CALL* function_call = rhs->types.function_call; // get the function call on rhs
      //struct ELEMENT* parameter = stmt->types.function_call->parameter;
      new_value = execute_assignment_to_function_call(function_call, memory, stmt->line);
      
      if (new_value == NULL) {
        return false;
      }

      return_val = *new_value; 
    }

    if (rhs->value_type == VALUE_EXPR) { // if it is a expression on the rhs...
      struct RAM_VALUE new_value;

      // Evaluate the RHS expression using the helper evaluate_expression
      struct EXPR* expression = rhs->types.expr;
      if (!evaluate_expression(expression, memory, &new_value, stmt->line)) {
        return false;
      }

      return_val = new_value;
      
    }

    // otherwise... write evaluated value to memory
    // printf("%s", return_val.types.s); // debug
    if (!ram_write_cell_by_name(memory, return_val, var_name)) {
      // printf("HIT HERE\n"); // debug
      // exit(0); // debug
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
      return false;
    }

    return true; // assignment successful
  }

  return false; // in the case it is not of type assignment which should not be the case lol it will return false
}

//----------------------------------------------------------------------------------------------------------------------------------


/* execute_while_stmt
execute stmts within a while loop.

parameters: struct STMT*, struct RAM*, int
returns: bool
*/
bool execute_while_stmt(struct STMT* stmt, struct RAM* memory, int og_line) {
  
  if (stmt == NULL) {
    return true;
  }

  // assignment
  if (stmt->stmt_type == STMT_ASSIGNMENT) {
    return execute_assignment(stmt, memory);
  } 
  //function call
  else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
    // printf("stmt line: %d\n", stmt->line);
    // printf("stmt line: %s\n", stmt->types.function_call->function_name);
    return execute_function_call(stmt, memory);
  } 
  // pass
  else if (stmt->stmt_type == STMT_PASS) {
    return true;
  }   
  // while loop
  else if (stmt->stmt_type == STMT_WHILE_LOOP) { // this can either be original while loop or a nested loop
    // check this by seeing if the next stmt is the curr line is og line or not
    if (stmt->line == og_line) { // we know we finished one cycle of the og while loop
      return false;
    } else { // we know we hit a nested loop
      // return execute_while_loop(stmt, memory, stmt->line);
      struct EXPR* condition = stmt->types.while_loop->condition;
      struct STMT* body = stmt->types.while_loop->loop_body;

      while (true) {
        struct RAM_VALUE condition_result;

        // Evaluate the loop condition
        if (!evaluate_expression(condition, memory, &condition_result, stmt->line)) {
          return false; // Semantic error in condition
        }

        // Exit the loop if the condition evaluates to false
        if (condition_result.types.i == 0) {
          break;
        }

        // Recursively execute the body of the nested loop
        struct STMT* current_body_stmt = body;
        while (current_body_stmt != NULL) {
          if (!execute_while_stmt(current_body_stmt, memory, stmt->line)) {
            break;
            // return false; // Error in executing a statement
          }

          // Move to the next statement in the loop body
          switch (current_body_stmt->stmt_type) {
            case STMT_ASSIGNMENT:
              current_body_stmt = current_body_stmt->types.assignment->next_stmt;
              break;

            case STMT_FUNCTION_CALL:
              current_body_stmt = current_body_stmt->types.function_call->next_stmt;
              break;

            case STMT_PASS:
              current_body_stmt = current_body_stmt->types.pass->next_stmt;
              break;

            case STMT_WHILE_LOOP:
              current_body_stmt = current_body_stmt->types.while_loop->next_stmt;
              break;

            default:
              printf("**SEMANTIC ERROR: invalid stmt type in nested loop (line %d)\n", stmt->line);
              return false;
          }
        }
      }
      return true; // Nested loop executed successfully
    }
  } 

  // ERROR
  else {
    printf("**SEMANTIC ERROR: unsupported stmt type UH OH (line %d)\n", stmt->line);
    return false;
  }
}




/* execute_while_loop
called if the type is of while_loop

parameters: struct STMT*, struct RAM*, int
returns: bool
*/
bool execute_while_loop(struct STMT* stmt, struct RAM* memory, int og_line) {
  // Extract the while loop condition and body
  struct EXPR* condition = stmt->types.while_loop->condition;
  struct STMT* body = stmt->types.while_loop->loop_body;

  while(true) { // outer loop
    struct RAM_VALUE condition_result;
    // Evaluate the loop condition
    if (!evaluate_expression(condition, memory, &condition_result, stmt->line)) {
      return false; // Stop execution because condition is wrong
    }

    // Exit the loop if the condition is false
    if (condition_result.types.i == 0) {
      break;
    }

    struct STMT* current_body_stmt = body;

    while (current_body_stmt != NULL) { // run the stmts for the body
      if (!execute_while_stmt(current_body_stmt, memory, og_line)) {
        break;
        // return false;
      }

      // idkkkkkk
      if (current_body_stmt->stmt_type == STMT_ASSIGNMENT) {
        current_body_stmt = current_body_stmt->types.assignment->next_stmt;
      } else if (current_body_stmt->stmt_type == STMT_FUNCTION_CALL) {
        // printf("next line number: %d", current_body_stmt->line); // debug
        current_body_stmt = current_body_stmt->types.function_call->next_stmt;
        // printf("next line number: %d", current_body_stmt->line); // debug
      } else if (current_body_stmt->stmt_type == STMT_PASS) {
        current_body_stmt = current_body_stmt->types.pass->next_stmt;
      } else if (current_body_stmt->stmt_type == STMT_WHILE_LOOP) {
    
        current_body_stmt = current_body_stmt->types.while_loop->next_stmt;
      }
      
    }
    
  }
  return true; // while_loop executed successfully
}



/* execute_statement
this is used to execute a statement 

parameters: struct STMT* stmt, struct RAM*
returns: bool
*/
bool execute_statement(struct STMT* stmt, struct RAM* memory) {
  if (stmt == NULL) {
    return true;
  }

  // assignment
  if (stmt->stmt_type == STMT_ASSIGNMENT) {
    return execute_assignment(stmt, memory);
  } 
  //function call
  else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
    return execute_function_call(stmt, memory);
  } 
  // pass
  else if (stmt->stmt_type == STMT_PASS) {
    return true;
  }   
  // while loop
  else if (stmt->stmt_type == STMT_WHILE_LOOP) {
    return execute_while_loop(stmt, memory,stmt->line);
  } 

  // ERROR
  else {
    printf("**SEMANTIC ERROR: unsupported stmt type UH OH (line %d)\n", stmt->line);
    return false;
  }
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
    if (!execute_statement(stmt, memory)) {
      return;
    }

    // move to next stmt based on stmt type
    if (stmt->stmt_type == STMT_ASSIGNMENT) {
      stmt = stmt->types.assignment->next_stmt;
    }

    else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      stmt = stmt->types.function_call->next_stmt;
    }

    else if (stmt->stmt_type == STMT_PASS) {
      // printf("current line number: %i\n", stmt->line); // debug
      stmt = stmt->types.pass->next_stmt;
      // printf("next line number: %i\n", stmt->line); // debug
    }

    else if (stmt->stmt_type == STMT_WHILE_LOOP) {
      stmt = stmt->types.while_loop->next_stmt;
    }
  }

}