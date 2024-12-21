/*ram.c*/

//
// this file holds the implementation for various ram functions
//
// Irene Ha
//
// Template: Prof. Joe Hummel
// Northwestern University
// CS 211
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // true, false
#include <string.h>
#include <assert.h>

#include "ram.h"


// 
// EXTRA FUNCTIONS I WROTE
//

/* dupString
makes a duplicate string

parameters: char*    // this would be the original string
returns: char*       // returns the copy
*/
char* dupString(char* s) {
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



//
// Public functions:
//

//
// ram_init
//
// Returns a pointer to a dynamically-allocated memory
// for storing nuPython variables and their values. All
// memory cells are initialized to the value None.
//
struct RAM* ram_init(void)
{
  // Step 1: Allocate memory for the RAM structure
  struct RAM* memory = (struct RAM*) malloc(sizeof(struct RAM));
  if (memory == NULL) {
    //printf("ERROR: allocation failed and NULL was returned\n");
    return NULL;
  }
  
  // Step 2: Allocate memory for 4 RAM_CELLS and set the intial capacity
  // cells is a dynamically allocated array of RAM_CELL struct objects
  memory->cells = (struct RAM_CELL*) malloc(4 * sizeof(struct RAM_CELL));
  if (memory-> cells == NULL) {
    //printf("RAM freed because cell allocation failed and NULL is returned\n");
    free(memory);
    return NULL;
  }

  // Step 3: Initialize num_values to 0 and capacity to 4
  // each ram_cell struct has an identifier and a ram_value
  memory -> num_values = 0;
  memory -> capacity = 4;

  // Step 4: Initialize each cell’s identifier to NULL and set value type to RAM_TYPE_NONE
  for (int i = 0; i < 4; i++) {
    memory->cells[i].identifier = NULL;
    memory->cells[i].value.value_type = RAM_TYPE_NONE;
  }

  return memory;
}


//
// ram_destroy
//
// Frees the dynamically-allocated memory associated with
// the given memory. After the call returns, you cannot
// use the memory.
//
void ram_destroy(struct RAM* memory)
{
  if (memory == NULL) {
    return;  // there is nothing to destroy if memory is NULL
  }

  // Step 1: Free each cell's identifier and value
  for (int i = 0; i < memory->num_values; i++) {
    // Free the identifier if it was dynamically allocated
    free(memory->cells[i].identifier);

    // free string memory if it is of the correct type
    if (memory->cells[i].value.value_type == RAM_TYPE_STR) {
      free(memory->cells[i].value.types.s);
    }
  }

  // Step 2: Free the cells array itself
  free(memory->cells);

  // Step 3: Free the RAM structure
  free(memory);
  return;
}


//
// ram_get_addr
// 
// If the given identifier (e.g. "x") has been written to 
// memory, returns the address of this value --- an integer
// in the range 0..N-1 where N is the number of values currently 
// stored in memory. Returns -1 if no such identifier exists 
// in memory. 
// 
// NOTE: a variable has to be written to memory before you can
// get its address. Once a variable is written to memory, its
// address never changes.
//
int ram_get_addr(struct RAM* memory, char* identifier)
{
  // Step 1: loop through all stored values in memory right now
  for (int i = 0; i < memory->num_values; i++) {
    // check if the cell's identifier matches the given identifier by the parameter
    if (strcmp(memory->cells[i].identifier, identifier) == 0) {
      return i; // return the cell's index if a match is found
    }
  }
  return -1; // if not found return -1
}


//
// ram_read_cell_by_addr
//
// Given a memory address (an integer in the range 0..N-1), 
// returns a COPY of the value contained in that memory cell.
// Returns NULL if the address is not valid.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and 
// must eventually free this memory via ram_free_value().
//
// NOTE: a variable has to be written to memory before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
struct RAM_VALUE* ram_read_cell_by_addr(struct RAM* memory, int address)
{
  //Step 1: make sure address is valid
  if (address < 0 || address >= memory->num_values) {
    //printf("this is a invalid address and NULL was returned\n");;
    return NULL; // invalid address
  }

  // Step 2: Allocate memory for copy of RAM_VALUE
  struct RAM_VALUE* copy = (struct RAM_VALUE*)malloc(sizeof(struct RAM_VALUE));
  if (copy == NULL) {
    //printf("ERROR: OUT OF MEMORY\n");
    return NULL;
  }

  // Step 3: Copy the value type
  copy->value_type = memory->cells[address].value.value_type;

  // Step 4: Copy the actual data based on the type
  if (copy->value_type == RAM_TYPE_STR) {
    // Duplicate the string
    copy->types.s = dupString(memory->cells[address].value.types.s);
  } else {
    // it is not a string type so just copy the data directly
    copy->types = memory->cells[address].value.types;
  }

  return copy;
}


// 
// ram_read_cell_by_name
//
// If the given name (e.g. "x") has been written to 
// memory, returns a COPY of the value contained in memory.
// Returns NULL if no such name exists in memory.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and 
// must eventually free this memory via ram_free_value().
//
struct RAM_VALUE* ram_read_cell_by_name(struct RAM* memory, char* name)
{
  // Step 1: find he address of the variable by name
  int addr = ram_get_addr(memory, name);
  if (addr == -1) { // variable was not found
    //printf("variable was not found and NULL was returned");
    return NULL;
  }

  //Step 2: allocate memory for the copy of RAM_VALUE
  struct RAM_VALUE* copy = (struct RAM_VALUE*)malloc(sizeof(struct RAM_VALUE));
  if (copy == NULL) {
    //printf("ERROR: OUT OF MEMORY\n");
    return NULL;
  }

  // Step 3: copy the value type 
  copy->value_type = memory->cells[addr].value.value_type;

  // step 4: copy the data based on the type
  if (copy->value_type == RAM_TYPE_STR) {
    // duplicate the string
    copy->types.s = dupString(memory->cells[addr].value.types.s);
  } else {
    // if its not a string copy the data directly
    copy->types = memory->cells[addr].value.types;
  }

  return copy;
}


//
// ram_free_value
//
// Frees the memory value returned by ram_read_cell_by_name and
// ram_read_cell_by_addr.
//
void ram_free_value(struct RAM_VALUE* value)
{
  // case 1: there is nothing to free because memory is null
  if (value == NULL) {
    return;
  }
  
  // Free the string if the value is of type RAM_TYPE_STR
  if (value->value_type == RAM_TYPE_STR) {
    //printf("Freeing string: %s\n", value->types.s); // debug
    free(value->types.s);
  }

  // Free the RAM_VALUE struct itself
  //printf("Freeing RAM_VALUE struct\n"); // debug
  free(value);

}


//
// ram_write_cell_by_addr
//
// Writes the given value to the memory cell at the given 
// address. If a value already exists at this address, that
// value is overwritten by this new value. Returns true if 
// the value was successfully written, false if not (which 
// implies the memory address is invalid).
// 
// NOTE: if the value being written is a string, it will
// be duplicated and stored.
// 
// NOTE: a variable has to be written to memory before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
bool ram_write_cell_by_addr(struct RAM* memory, struct RAM_VALUE value, int address)
{
  // Step 1.5: make sure num_values is > 0 so that we dont produce a seg fault
  if (memory == NULL || memory->cells == NULL) {
    return false;  // Invalid memory structure
  }
  
  // Step 1: make sure the address is valid
  if (address < 0 || address >= memory->num_values) {
        return false;  // Invalid address
    }

  // Step 2: address is valid. Lets free the old value if its a string
  if (memory->cells[address].value.value_type == RAM_TYPE_STR) {
    free(memory->cells[address].value.types.s);
  }

  // extra addition: ensure the string value is not NULL if the type is RAM_TYPE_STR
  if (value.value_type == RAM_TYPE_STR && value.types.s == NULL) {
    return false; // Avoid crashing on invalid input
  }

  // // Step 2.5: free the identifier and duplicate it
  // if (memory->cells[address].identifier != NULL) {
  //   char* new_identifier = dupString(memory->cells[address].identifier);
  //   free(memory->cells[address].identifier);
  //   memory->cells[address].identifier = new_identifier;
  // } else { // we must account for the fact that the identifier could be NULL
  //   memory->cells[address].identifier = NULL;
  // }

  // Step 3: Write the new value
  memory->cells[address].value = value;
  
  // Step 4: If the new value is a string, duplicate it and assign to types.s
  if (value.value_type == RAM_TYPE_STR) {
    // if (value.types.s != NULL) {
      memory->cells[address].value.types.s = dupString(value.types.s);  // Duplicate the string
    // }
  // } else { // handle the case where string is NULL
  //   memory->cells[address].value.types.s = NULL;  
  }

  return true;
}


//
// ram_write_cell_by_name
//
// Writes the given value to a memory cell named by the given
// name. If a memory cell already exists with this name, the
// existing value is overwritten by the given value. Returns
// true since this operation always succeeds.
// 
// NOTE: if the value being written is a string, it will
// be duplicated and stored.
// 
// NOTE: a variable has to be written to memory before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
bool ram_write_cell_by_name(struct RAM* memory, struct RAM_VALUE value, char* name)
{
  // Check for invalid inputs
  if (memory == NULL || name == NULL) {
    return false; // Avoid crashing on invalid inputs
  }

  // Step 1: check if the variable exists
  int address = ram_get_addr(memory, name);
  if (address != -1) {
    // Free the old value if it is a string
    if (memory->cells[address].value.value_type == RAM_TYPE_STR) {
      free(memory->cells[address].value.types.s);
    }

    // Handle the new value
    if (value.value_type == RAM_TYPE_STR && value.types.s != NULL) {
      memory->cells[address].value.types.s = dupString(value.types.s);
    } else {
      memory->cells[address].value = value; // Copy non-string types
    }

    // Update the value type
    memory->cells[address].value.value_type = value.value_type;

    return true;
  }

  // Step 2: the variable doesn't exist, let's see if we need to expand capacity
  if (memory->num_values == memory->capacity) {
    // double the capacity
    memory->capacity *= 2;
    memory->cells = (struct RAM_CELL*) realloc(memory->cells, memory->capacity * sizeof(struct RAM_CELL));
    if (memory->cells == NULL) { // memory allocation failed
      return false;
    }
  }

  // Step 3: Add new variable
  int new_index = memory->num_values;
  memory->cells[new_index].identifier = dupString(name);  // Copy the name
  memory->cells[new_index].value = value;

  // If the new value is a string, duplicate it
  if (value.value_type == RAM_TYPE_STR) {
    memory->cells[new_index].value.types.s = dupString(value.types.s);
  }

  memory->num_values++;  // Update the count of stored variables
  return true;
}




//
// ram_print
//
// Prints the contents of memory to the console.
//
void ram_print(struct RAM* memory)
{
  printf("**MEMORY PRINT**\n");

  printf("Capacity: %d\n", memory->capacity);
  printf("Num values: %d\n", memory->num_values);
  printf("Contents:\n");

  for (int i = 0; i < memory->num_values; i++)
  {
    printf(" %d: %s, ", i, memory->cells[i].identifier);

    if (memory->cells[i].value.value_type == RAM_TYPE_INT){
      printf("int, %d", memory->cells[i].value.types.i);
    } else if (memory->cells[i].value.value_type == RAM_TYPE_REAL) {
      printf("real, %lf", memory->cells[i].value.types.d);
    } else if (memory->cells[i].value.value_type == RAM_TYPE_STR) {
      printf("str, '%s'", memory->cells[i].value.types.s);
    } else if (memory->cells[i].value.value_type == RAM_TYPE_PTR) {
      printf("ptr, %d", memory->cells[i].value.types.i);
    } else if (memory->cells[i].value.value_type == RAM_TYPE_BOOLEAN) {
      if (memory->cells[i].value.types.i == false) {
        printf("boolean, False");
      } else if (memory->cells[i].value.types.i == true) {
        printf("boolean, True");
      }
    } else if (memory->cells[i].value.value_type == RAM_TYPE_NONE) {
      printf("none, None");
    } else {
      printf("unknown type");
    }
    
   printf("\n");
  }

  printf("**END PRINT**\n");
}

