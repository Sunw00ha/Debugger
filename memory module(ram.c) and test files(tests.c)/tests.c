/*tests.c*/

//
// these are tests used to check for memory leaks and the functionality of the code
//
// Irene Ha
//
// Initial template: Prof. Joe Hummel
// Northwestern University
// CS 211
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ram.h"
#include "gtest/gtest.h"

//
// private helper functions:
//


//
// some provided unit tests to get started:
//
//* satisfies test 0
TEST(memory_module, initialization) 
{
  //
  // create a new memory and make sure it's initialized properly:
  //
  struct RAM* memory = ram_init();

  ASSERT_TRUE(memory != NULL);        // use ASSERT_TRUE with pointers
  ASSERT_TRUE(memory->cells != NULL);

  ASSERT_EQ(memory->num_values, 0);  // use ASSERT_EQ for comparing values
  ASSERT_EQ(memory->capacity, 4);

  //
  // tests passed, free memory
  //
  ram_destroy(memory);
}

TEST(memory_module, read_by_name_does_not_exist) 
{
  //
  // create a new memory:
  //
  struct RAM* memory = ram_init();

  //
  // read a var that doesn't exist:
  //
  struct RAM_VALUE* value = ram_read_cell_by_name(memory, "x");
  ASSERT_TRUE(value == NULL);  // use ASSERT_TRUE with pointers

  //
  // tests passed, free memory
  //
  ram_destroy(memory);
}

TEST(memory_module, write_one_int) 
{
  //
  // create a new memory:
  //
  struct RAM* memory = ram_init();

  //
  // store the integer 123:
  //
  struct RAM_VALUE i;

  i.value_type = RAM_TYPE_INT;
  i.types.i = 123;

  bool success = ram_write_cell_by_name(memory, i, "x");
  ASSERT_TRUE(success);

  //
  // now check the memory, was x = 123 stored properly?
  //
  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->cells[0].value.value_type, RAM_TYPE_INT);
  ASSERT_EQ(memory->cells[0].value.types.i, 123);
  ASSERT_STREQ(memory->cells[0].identifier, "x");  // strings => ASSERT_STREQ

  //
  // tests passed, free memory
  //
  ram_destroy(memory);
}


// //
// // TODO: add lots more unit tests
// //

TEST(memory_module, test1_writeByName) 
{
  struct RAM* memory = ram_init();

  ASSERT_TRUE(memory != NULL);
  ASSERT_TRUE(memory->cells != NULL);
  ASSERT_TRUE(memory->num_values == 0);
  ASSERT_TRUE(memory->capacity == 4);

  struct RAM_VALUE i;

  i.value_type = RAM_TYPE_INT;
  i.types.i = 1;

  bool success = ram_write_cell_by_name(memory, i, "test1");
  ASSERT_TRUE(success);

  // check if memory has the correct info
  ASSERT_TRUE(memory->num_values == 1);
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_INT);
  ASSERT_TRUE(memory->cells[0].value.types.i == 1);
  ASSERT_TRUE(strcmp(memory->cells[0].identifier, "test1") == 0);

  ram_destroy(memory);

}


TEST(memory_module, test2_writeByName_TypeDouble) 
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE i;

  i.value_type = RAM_TYPE_REAL;
  i.types.d = 1.111;

  bool success = ram_write_cell_by_name(memory, i, "test2");
  ASSERT_TRUE(success);

  ASSERT_TRUE(memory->num_values == 1);
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_REAL);
  ASSERT_TRUE(memory->cells[0].value.types.d == 1.111);
  ASSERT_TRUE(strcmp(memory->cells[0].identifier, "test2") == 0);

  ram_destroy(memory);
}


TEST(memory_module, test3_getByAddr_NULL) 
{
  struct RAM* memory = ram_init();

  ASSERT_TRUE(memory != NULL);
  ASSERT_TRUE(memory->cells != NULL);
  ASSERT_TRUE(memory->num_values == 0);
  ASSERT_TRUE(memory->capacity == 4);

  struct RAM_VALUE i;

  i.value_type = RAM_TYPE_INT;
  i.types.i = 1351;

  bool success = ram_write_cell_by_name(memory, i, "test3_beep");
  ASSERT_TRUE(success);

  ASSERT_TRUE(ram_get_addr(memory, "test3_boop") == -1); // should be null

  ram_destroy(memory);
}

TEST(memory_module, write_and_overwrite_by_addr) //* attempt at test 2
{
  struct RAM* memory = ram_init();

  // Write an initial value by name
  struct RAM_VALUE initial_value;
  initial_value.value_type = RAM_TYPE_REAL;
  initial_value.types.d = 3.14;

  ram_write_cell_by_name(memory, initial_value, "pi");

  // Overwrite the value w/ a diff type
  struct RAM_VALUE new_value;
  new_value.value_type = RAM_TYPE_BOOLEAN;
  new_value.types.i = 1;

  bool success = ram_write_cell_by_addr(memory, new_value, 0);
  ASSERT_TRUE(success);

  // Verify memory
  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->cells[0].value.value_type, RAM_TYPE_BOOLEAN);
  ASSERT_EQ(memory->cells[0].value.types.i, 1);
  ASSERT_STREQ(memory->cells[0].identifier, "pi");

  ram_destroy(memory);
}

TEST(memory_module, overwrite_integer_with_string_potato_tomato) 
{
  struct RAM* memory = ram_init();

  // Write an int value by name
  struct RAM_VALUE int_value;
  int_value.value_type = RAM_TYPE_INT;
  int_value.types.i = 999;

  ram_write_cell_by_name(memory, int_value, "potato");

  // Overwrite the int w/ a string value by addr
  struct RAM_VALUE string_value;
  string_value.value_type = RAM_TYPE_STR;
  string_value.types.s = "tomato";

  bool success = ram_write_cell_by_addr(memory, string_value, 0);
  ASSERT_TRUE(success);

  // check that the value has been updated
  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->cells[0].value.value_type, RAM_TYPE_STR);
  ASSERT_STREQ(memory->cells[0].value.types.s, "tomato");
  ASSERT_STREQ(memory->cells[0].identifier, "potato");

  ram_destroy(memory);
}


TEST(memory_module, get_address_check) 
{
  struct RAM* memory = ram_init();

  // Write value by name
  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_INT;
  value.types.i = 456;

  ram_write_cell_by_name(memory, value, "apple");

  // check the correct address is returned
  ASSERT_TRUE(ram_get_addr(memory, "apple") == 0);

  ram_destroy(memory);
}


TEST(memory_module, read_value_by_address) //* attempt at test 2
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;

  value.value_type = RAM_TYPE_STR;
  value.types.s = "potato";

  ram_write_cell_by_name(memory, value, "tomato");

  struct RAM_VALUE* ram_val = ram_read_cell_by_addr(memory, 0);
  ASSERT_TRUE(ram_val->value_type == RAM_TYPE_STR);
  ASSERT_TRUE(strcmp(ram_val->types.s, "potato") == 0);

  ram_free_value(ram_val);
  ram_destroy(memory);
}



TEST(memory_module, read_invalid_address_returns_NULL) //* attempt at test 2
{
    struct RAM* memory = ram_init();

    struct RAM_VALUE value;

    char test_string[3] = "ok";

    value.value_type = RAM_TYPE_STR;
    value.types.s = &test_string[0];

    ram_write_cell_by_name(memory, value, "key");

    struct RAM_VALUE* ram_val2 = ram_read_cell_by_name(memory, "key");
    test_string[1] = 'x'; // Modify the original string

    ASSERT_TRUE(strcmp("ok", memory->cells[0].value.types.s) == 0);

    ram_write_cell_by_addr(memory, value, 0);
    ASSERT_TRUE(strcmp("ok", ram_val2->types.s) == 0);
    ASSERT_TRUE(&(value.types.s) != &(memory->cells[0].value.types.s));
    ASSERT_TRUE(&(value.types.s) != &(ram_val2->types.s));
    ASSERT_TRUE(&(ram_val2->types.s) != &(memory->cells[0].value.types.s));

    struct RAM_VALUE* ram_val = ram_read_cell_by_addr(memory, 1);
    ASSERT_TRUE(ram_read_cell_by_addr(memory, 1) == NULL);

    // clean
    ram_free_value(ram_val);
    ram_free_value(ram_val2);
    ram_destroy(memory);
}




TEST(memory_module, read_by_name_modified) 
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE int_value;
  int_value.value_type = RAM_TYPE_INT;
  int_value.types.i = 456;

  ram_write_cell_by_name(memory, int_value, "key123");

  // Overwrite the value with a string
  struct RAM_VALUE str_value;
  str_value.value_type = RAM_TYPE_STR;
  str_value.types.s = "hello_world";

  ram_write_cell_by_addr(memory, str_value, 0);

  struct RAM_VALUE* ram_val = ram_read_cell_by_name(memory, "key123");
  ASSERT_TRUE(ram_val->value_type == RAM_TYPE_STR);
  ASSERT_TRUE(strcmp(ram_val->types.s, "hello_world") == 0);

  // Clean up
  ram_free_value(ram_val);
  ram_destroy(memory);
}


TEST(memory_module, read_by_name_invalid_key) 
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE first_value;
  first_value.value_type = RAM_TYPE_INT;
  first_value.types.i = 100;

  ram_write_cell_by_name(memory, first_value, "key1");

  // Overwrite the value at the same address
  struct RAM_VALUE second_value;
  second_value.value_type = RAM_TYPE_INT;
  second_value.types.i = 200;

  ram_write_cell_by_addr(memory, second_value, 0);

  // Attempt to read a non-existent key
  struct RAM_VALUE* ram_val = ram_read_cell_by_name(memory, "nonexistent_key");
  ASSERT_TRUE(ram_val == NULL);

  // Clean
  ram_destroy(memory);
}


TEST(memory_module, capacity_expansion_check) //* used to pass test 5
{
  struct RAM* memory = ram_init();

  // Ensure memory is initialized
  ASSERT_TRUE(memory != NULL);
  ASSERT_TRUE(memory->cells != NULL);
  ASSERT_EQ(memory->num_values, 0);
  ASSERT_EQ(memory->capacity, 4);

  // Add various types of values to the memory
  struct RAM_VALUE str_value1;
  str_value1.value_type = RAM_TYPE_STR;
  str_value1.types.s = "hello";
  ASSERT_TRUE(ram_write_cell_by_name(memory, str_value1, "key1"));

  struct RAM_VALUE int_value1;
  int_value1.value_type = RAM_TYPE_INT;
  int_value1.types.i = 123;
  ASSERT_TRUE(ram_write_cell_by_name(memory, int_value1, "key2"));

  struct RAM_VALUE str_value2;
  str_value2.value_type = RAM_TYPE_STR;
  str_value2.types.s = "world";
  ASSERT_TRUE(ram_write_cell_by_name(memory, str_value2, "key3"));

  struct RAM_VALUE int_value2;
  int_value2.value_type = RAM_TYPE_INT;
  int_value2.types.i = 456;
  ASSERT_TRUE(ram_write_cell_by_name(memory, int_value2, "key4"));

  struct RAM_VALUE int_value3;
  int_value3.value_type = RAM_TYPE_INT;
  int_value3.types.i = 789;
  ASSERT_TRUE(ram_write_cell_by_name(memory, int_value3, "key5"));

  struct RAM_VALUE str_value3;
  str_value3.value_type = RAM_TYPE_STR;
  str_value3.types.s = "expand";
  ASSERT_TRUE(ram_write_cell_by_name(memory, str_value3, "key6"));

  // Assertions to verify memory functionality
  ASSERT_TRUE(memory->num_values == 6);               // number of values is correct
  ASSERT_TRUE(memory->capacity >= 6);                 // capacity expanded as needed
  ASSERT_TRUE(memory->cells[5].identifier != NULL);   // Ensure identifier is not null
  ASSERT_TRUE(strcmp(memory->cells[5].identifier, "key6") == 0);

  // Read and validate value at address 2
  struct RAM_VALUE* read_value = ram_read_cell_by_addr(memory, 2);
  ASSERT_TRUE(read_value != NULL);                     // Ensure read_value is not null
  ASSERT_TRUE(read_value->value_type == RAM_TYPE_STR); // type is correct
  ASSERT_TRUE(strcmp(read_value->types.s, "world") == 0);

  // Clean
  ram_free_value(read_value);
  ram_destroy(memory);
}



TEST(memory_module, capacity_growth_test) //* used to pass test 5
{
  struct RAM* memory = ram_init();

  // Add integer values to the memory
  struct RAM_VALUE val1;
  val1.value_type = RAM_TYPE_INT;
  val1.types.i = 100;
  ram_write_cell_by_name(memory, val1, "key1");

  struct RAM_VALUE val2;
  val2.value_type = RAM_TYPE_INT;
  val2.types.i = 200;
  ram_write_cell_by_name(memory, val2, "key2");

  struct RAM_VALUE val3;
  val3.value_type = RAM_TYPE_INT;
  val3.types.i = 300;
  ram_write_cell_by_name(memory, val3, "key3");

  struct RAM_VALUE val4;
  val4.value_type = RAM_TYPE_INT;
  val4.types.i = 400;
  ram_write_cell_by_name(memory, val4, "key4");

  struct RAM_VALUE val5;
  val5.value_type = RAM_TYPE_INT;
  val5.types.i = 500;
  ram_write_cell_by_name(memory, val5, "key5");

  // check values in memory right now
  ASSERT_TRUE(memory->cells[0].value.types.i == 100);
  ASSERT_TRUE(memory->cells[1].value.types.i == 200);
  ASSERT_TRUE(memory->cells[2].value.types.i == 300);
  ASSERT_TRUE(memory->cells[3].value.types.i == 400);
  ASSERT_TRUE(memory->cells[4].value.types.i == 500);

  // call read by name to check
  struct RAM_VALUE* retrieved_value = ram_read_cell_by_name(memory, "key5");
  ASSERT_TRUE(retrieved_value->value_type == RAM_TYPE_INT);
  ASSERT_TRUE(retrieved_value->types.i == 500);

  ASSERT_TRUE(memory->num_values == 5);

  ram_free_value(retrieved_value);
  ram_destroy(memory);
}


TEST(memory_module, aLotOf_capacity_expansion_tests) //* this satisfies test 4
{
  struct RAM* memory = ram_init();

  // Ensure memory is initialized correctly
  ASSERT_TRUE(memory != NULL);
  ASSERT_TRUE(memory->cells != NULL);
  ASSERT_EQ(memory->num_values, 0);
  ASSERT_EQ(memory->capacity, 4);

  struct RAM_VALUE val1;
  val1.value_type = RAM_TYPE_STR;
  val1.types.s = "hello";

  struct RAM_VALUE val2;
  val2.value_type = RAM_TYPE_STR;
  val2.types.s = "world";

  // Add multiple entries to trigger capacity doubling
  ASSERT_TRUE(ram_write_cell_by_name(memory, val1, "key1"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val1, "key2"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val1, "key3"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val1, "key4"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val2, "key5"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val2, "key6"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val2, "key7"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val2, "key8"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val2, "key9"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val2, "key10"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val2, "key11"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val2, "key12"));

  // Verify memory capacity has expanded
  ASSERT_EQ(memory->capacity, 16);
  ASSERT_EQ(memory->num_values, 12);

  // Validate the memory entries
  struct RAM_VALUE* retrieved_val1 = ram_read_cell_by_name(memory, "key1");
  ASSERT_TRUE(retrieved_val1 != NULL);
  ASSERT_TRUE(strcmp(retrieved_val1->types.s, "hello") == 0);

  struct RAM_VALUE* retrieved_val2 = ram_read_cell_by_name(memory, "key11");
  ASSERT_TRUE(retrieved_val2 != NULL);
  ASSERT_TRUE(strcmp(retrieved_val2->types.s, "world") == 0);

  // Verify specific cell values
  ASSERT_TRUE(memory->cells[2].identifier != NULL);
  ASSERT_TRUE(strcmp(memory->cells[2].value.types.s, "hello") == 0);
  ASSERT_TRUE(memory->cells[9].identifier != NULL);
  ASSERT_TRUE(strcmp(memory->cells[9].identifier, "key10") == 0);
  ASSERT_TRUE(strcmp(memory->cells[9].value.types.s, "world") == 0);

  // Free and clean
  ram_free_value(retrieved_val1);
  ram_free_value(retrieved_val2);
  ram_destroy(memory);
}



TEST(memory_module, overwrite_multiple_identifiers) //*crashing
{
  struct RAM* memory = ram_init();

  // Ensure memory is initialized
  ASSERT_TRUE(memory != NULL);
  ASSERT_TRUE(memory->cells != NULL);
  ASSERT_EQ(memory->num_values, 0);
  ASSERT_EQ(memory->capacity, 4);

  struct RAM_VALUE val1;
  val1.value_type = RAM_TYPE_STR;
  val1.types.s = "alpha";

  struct RAM_VALUE val2;
  val2.value_type = RAM_TYPE_STR;
  val2.types.s = "beta";

  struct RAM_VALUE val3;
  val3.value_type = RAM_TYPE_BOOLEAN;
  val3.types.i = 1;

  // Write initial values
  ASSERT_TRUE(ram_write_cell_by_name(memory, val1, "id1"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val1, "id2"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val1, "id3"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val1, "id4"));

  // Ensure the number of values and identifiers are correct after initial writes
  ASSERT_EQ(memory->num_values, 4);
  for (int i = 0; i < 4; i++) {
    ASSERT_TRUE(memory->cells[i].identifier != NULL);
    ASSERT_TRUE(memory->cells[i].value.types.s != NULL);
  }

  // Overwrite specific identifiers
  ASSERT_TRUE(ram_write_cell_by_name(memory, val2, "id2"));
  ASSERT_TRUE(ram_write_cell_by_name(memory, val3, "id4"));

  // Assertions to validate overwrites and memory integrity
  ASSERT_TRUE(memory->cells[3].value.value_type == RAM_TYPE_BOOLEAN);
  ASSERT_TRUE(memory->cells[3].identifier != NULL);
  ASSERT_TRUE(strcmp(memory->cells[1].identifier, "id2") == 0);
  ASSERT_TRUE(memory->cells[1].value.types.s != NULL);
  ASSERT_TRUE(strcmp(memory->cells[1].value.types.s, "beta") == 0);
  ASSERT_TRUE(memory->capacity == 4);
  ASSERT_TRUE(memory->num_values == 4);

  ram_destroy(memory);
}



TEST(memory_module, overwrite_by_address) 
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE val1;
  val1.value_type = RAM_TYPE_STR;
  val1.types.s = "first";

  struct RAM_VALUE val2;
  val2.value_type = RAM_TYPE_STR;
  val2.types.s = "second";

  struct RAM_VALUE val3;
  val3.value_type = RAM_TYPE_BOOLEAN;
  val3.types.i = 0;

  struct RAM_VALUE val4;
  val4.value_type = RAM_TYPE_BOOLEAN;
  val4.types.i = 1;

  // Initial writes by name
  ram_write_cell_by_name(memory, val3, "key1");
  ram_write_cell_by_name(memory, val2, "key2");
  ram_write_cell_by_name(memory, val4, "key3");
  ram_write_cell_by_name(memory, val1, "key4");

  // check initial writes
  ASSERT_TRUE(strcmp(memory->cells[1].value.types.s, "second") == 0);
  ASSERT_TRUE(memory->cells[2].value.types.i == 1);
  ASSERT_TRUE(strcmp(memory->cells[3].value.types.s, "first") == 0);
  ASSERT_TRUE(memory->cells[0].value.types.i == 0);

  // Overwrite values by address
  ram_write_cell_by_addr(memory, val2, 0);
  ram_write_cell_by_addr(memory, val4, 1);
  ram_write_cell_by_addr(memory, val3, 3);
  ram_write_cell_by_addr(memory, val1, 2);

  // Read back value from address 2
  struct RAM_VALUE* ram_val = ram_read_cell_by_addr(memory, 2);
  ASSERT_TRUE(ram_val->value_type == RAM_TYPE_STR);
  ASSERT_TRUE(strcmp(ram_val->types.s, "first") == 0);

  // Check final overwrites
  ASSERT_TRUE(strcmp(memory->cells[0].value.types.s, "second") == 0);
  ASSERT_TRUE(memory->cells[1].value.types.i == 1);
  ASSERT_TRUE(strcmp(memory->cells[2].value.types.s, "first") == 0);
  ASSERT_TRUE(memory->cells[3].value.types.i == 0);

  ASSERT_TRUE(memory->capacity == 4);

  ram_free_value(ram_val);
  ram_destroy(memory);
}


TEST(memory_module, get_address_of_unset_value) 
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE str_value;
  str_value.value_type = RAM_TYPE_STR;
  str_value.types.s = "example";

  struct RAM_VALUE none_value;
  none_value.value_type = RAM_TYPE_NONE;

  // Write string values by name
  ram_write_cell_by_name(memory, str_value, "key1");
  ram_write_cell_by_name(memory, str_value, "key2");
  ram_write_cell_by_name(memory, str_value, "key3");

  ram_write_cell_by_addr(memory, none_value, 1);

  // Check the overwritten value and memory
  ASSERT_TRUE(memory->cells[1].value.value_type == RAM_TYPE_NONE);
  ASSERT_TRUE(memory->capacity == 4);
  ASSERT_TRUE(memory->num_values == 3);

  ram_destroy(memory);
}


TEST(memory_module, large_capacity_expansion) //* this makes test 1 pass
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_INT;

  // Add a large number of entries to force multiple capacity expansions
  for (int i = 0; i < 100; i++) {
    value.types.i = i;
    char key[16];
    snprintf(key, sizeof(key), "key%d", i);
    ASSERT_TRUE(ram_write_cell_by_name(memory, value, key));
  }

  // Verify the number of values and capacity
  ASSERT_EQ(memory->num_values, 100);
  ASSERT_TRUE(memory->capacity >= 100);

  // Check values
  for (int i = 95; i < 100; i++) {
    char key[16];
    snprintf(key, sizeof(key), "key%d", i);
    struct RAM_VALUE* read_value = ram_read_cell_by_name(memory, key);
    ASSERT_TRUE(read_value != NULL);
    ASSERT_EQ(read_value->value_type, RAM_TYPE_INT);
    ASSERT_EQ(read_value->types.i, i);
    ram_free_value(read_value);
  }

  ram_destroy(memory);
}



//
//attempt to find test 3
//

TEST(memory_module, read_write_one_past_last_index) //* fufills test 3
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_INT;
  
  for (int i = 0; i < memory->capacity; i++) {
    value.types.i = i;
    char key[10];
    snprintf(key, sizeof(key), "key%d", i);
    ASSERT_TRUE(ram_write_cell_by_name(memory, value, key));
  }

  // Try to read from one past the last valid index
  struct RAM_VALUE* read_value = ram_read_cell_by_addr(memory, memory->capacity);
  ASSERT_TRUE(read_value == NULL);

  // Try to write to one past the last valid index
  value.types.i = 999;
  bool success = ram_write_cell_by_addr(memory, value, memory->capacity);
  ASSERT_FALSE(success);

  ram_destroy(memory);
}

TEST(memory_module, read_by_non_existent_name) 
{
  struct RAM* memory = ram_init();

  // Write some values into the memory
  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_INT;

  value.types.i = 123;
  ASSERT_TRUE(ram_write_cell_by_name(memory, value, "key1"));

  value.types.i = 456;
  ASSERT_TRUE(ram_write_cell_by_name(memory, value, "key2"));

  value.types.i = 789;
  ASSERT_TRUE(ram_write_cell_by_name(memory, value, "key3"));

  // Try to read a non-existent key
  struct RAM_VALUE* read_value = ram_read_cell_by_name(memory, "non_existent_key");
  ASSERT_TRUE(read_value == NULL);  // Should return NULL

  ram_destroy(memory);
}




TEST(memory_module, get_addr_skipped_a_house) //* attempt at test 2
{
 struct RAM* memory = ram_init();

  // Write some values into the memory
  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_INT;

  value.types.i = 123;
  ASSERT_TRUE(ram_write_cell_by_name(memory, value, "key1"));

  value.types.i = 456;
  ASSERT_TRUE(ram_write_cell_by_name(memory, value, "key2"));

  value.types.i = 789;
  ASSERT_TRUE(ram_write_cell_by_name(memory, value, "key3"));

  ASSERT_TRUE(ram_get_addr(memory, "key2") == 1);

  ram_destroy(memory);
}