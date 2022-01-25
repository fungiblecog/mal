#include <string.h>
#include "../../unity/src/unity.h"
#include "../hashmap.h"
#include "gc.h"

/* included for time and rand functions */
#include <time.h>
#include <stdlib.h>

/* magic number */
#define BUFFER_SIZE 32

/* number of items to add to test lists */
#define TEST_ITERATIONS 100

/* utility functions */
char *make_test_val (int i) {

  char *buf = GC_MALLOC(BUFFER_SIZE);
  snprintf(buf, BUFFER_SIZE - 1, "test_val_%d", i);
  return buf;
}

char *make_test_key(int i) {

  char *buf = GC_MALLOC(BUFFER_SIZE);
  snprintf(buf, BUFFER_SIZE - 1, "test_key_%d", i);
  return buf;
}

int char_fn(gptr key1, gptr key2) {

  char* keystring1 = (char*)key1;
  char* keystring2 = (char*)key2;

  return (strcmp(keystring1, keystring2) == 0);
}

char* char_val(gptr data) {
  return (char*)data;
}

void setUp(void) {
  /* set up global state here */
}

void tearDown(void) {
  /* clean up global state here */
}

/* tests */
void test_hashmap_make(void)
{
  /* create a hashmap */
  hashmap map = hashmap_make(char_fn);

  /* no elements */
  TEST_ASSERT_EQUAL_INT(0, map->count);
  /* no item in internal list */
  TEST_ASSERT_NULL(map->head);
}

void test_hashmap_put(void)
{
  hashmap map = hashmap_make(char_fn);

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);
    map = hashmap_put(map, (gptr)key, (gptr)val);
  }

  TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS, map->count);

  entry iter = map->head;

  for (int i = (TEST_ITERATIONS - 1); i >= 0; i--) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);

    /* check key */
    TEST_ASSERT_EQUAL_STRING(key, iter->key);
    /* check val */
    TEST_ASSERT_EQUAL_STRING(val, iter->val);

    iter = iter->next;
  }
}

void test_hashmap_get(void)
{
  srand((int)time(NULL));

  hashmap map = hashmap_make(char_fn);

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);
    map = hashmap_put(map, (gptr)key, (gptr)val);
  }

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    int r = (rand() % TEST_ITERATIONS);
    char* key = make_test_key(r);
    char* val = make_test_val(r);

    /* check val */
    TEST_ASSERT_EQUAL_STRING(val, hashmap_get(map, key));
  }
}

void test_hashmap_update(void)
{
  srand((int)time(NULL));

  hashmap map = hashmap_make(char_fn);

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);
    map = hashmap_put(map, (gptr)key, (gptr)val);
  }
  /* check all keys/vals added */
  TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS, hashmap_count(map));

  /* test that present key/vals can be updated */
  for (int i = 0; i < TEST_ITERATIONS; i++) {

    int r = (rand() % TEST_ITERATIONS);
    char* key = make_test_key(r);
    char* val = "updated";

    map = hashmap_update(map, (gptr)key, (gptr)val);

    /* test value is updated */
    TEST_ASSERT_EQUAL_STRING(val, hashmap_get(map, key));

    /* test all keys/values still there */
    TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS, hashmap_count(map));
  }

  /* test that not present key/vals are added */
  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i+2*TEST_ITERATIONS);
    char* val = make_test_val(i+2*TEST_ITERATIONS);

    map = hashmap_update(map, (gptr)key, (gptr)val);

    /* check the value is updated */
    TEST_ASSERT_EQUAL_STRING(val, hashmap_get(map, key));

    /* check the element count */
    TEST_ASSERT_EQUAL_INT((TEST_ITERATIONS+i+1), hashmap_count(map));
  }
}

void test_hashmap_copy(void) {

  hashmap map = hashmap_make(char_fn);

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);
    map = hashmap_put(map, (gptr)key, (gptr)val);
  }

  hashmap copy = hashmap_copy(map);
  TEST_ASSERT_EQUAL_INT(map->count, copy->count);

  entry iter1 = map->head;
  entry iter2 = copy->head;
  while(iter1) {

    /* check keys */
    TEST_ASSERT_EQUAL_STRING((char*)iter1->key, (char*)iter2->key);
    /* check vals */
    TEST_ASSERT_EQUAL_STRING((char*)iter1->val, (char*)iter2->val);

    iter1 = iter1->next;
    iter2 = iter2->next;
  }
}

void test_hashmap_delete(void) {

  hashmap map = hashmap_make(char_fn);

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);
    map = hashmap_put(map, (gptr)key, (gptr)val);
  }

  char* key_random = make_test_key(TEST_ITERATIONS/2);
  map = hashmap_delete(map, key_random);

  /* key no longer found */
  TEST_ASSERT_NULL(hashmap_get(map, key_random));
  TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS - 1, hashmap_count(map));
}


void test_hashmap_iterator(void) {

  hashmap map = hashmap_make(char_fn);

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);
    map = hashmap_put(map, (gptr)key, (gptr)val);
  }

  /* make an interator */
  iterator iter = hashmap_iterator_make(map);

  int i = TEST_ITERATIONS;
  while(iter) {
    i--;

    char* key = make_test_key(i);
    char* val = make_test_val(i);

    char* iter_key = (char*)iterator_value(iter);
    iter = iterator_next(iter);
    char* iter_val = (char*)iterator_value(iter);

    /* check iterator key/vals */
    TEST_ASSERT_EQUAL_STRING(key, iter_key);
    TEST_ASSERT_EQUAL_STRING(val, iter_val);

    iter = iterator_next(iter);
   }
    TEST_ASSERT_EQUAL_INT(0, i);
}


/* run tests */
int main(void)
{
  UNITY_BEGIN();

  RUN_TEST(test_hashmap_make);
  RUN_TEST(test_hashmap_put);
  RUN_TEST(test_hashmap_get);
  RUN_TEST(test_hashmap_delete);
  RUN_TEST(test_hashmap_update);
  RUN_TEST(test_hashmap_copy);
  RUN_TEST(test_hashmap_iterator);

  return UNITY_END();
}



/* Unity MACROS */

/*
TEST_ASSERT_TRUE(condition)
Evaluates whatever code is in condition and fails if it evaluates to false

TEST_ASSERT_FALSE(condition)
Evaluates whatever code is in condition and fails if it evaluates to true

TEST_ASSERT(condition)
Another way of calling TEST_ASSERT_TRUE

TEST_ASSERT_UNLESS(condition)
Another way of calling TEST_ASSERT_FALSE

TEST_FAIL()
TEST_FAIL_MESSAGE(message)
This test is automatically marked as a failure. The message is output stating why.

Numerical Assertions: Integers

TEST_ASSERT_EQUAL_INT(expected, actual)
TEST_ASSERT_EQUAL_INT8(expected, actual)
TEST_ASSERT_EQUAL_INT16(expected, actual)
TEST_ASSERT_EQUAL_INT32(expected, actual)
TEST_ASSERT_EQUAL_INT64(expected, actual)
Compare two integers for equality and display errors as signed integers. A cast will be performed to your natural integer size so often this can just be used. When you need to specify the exact size, like when comparing arrays, you can use a specific version:

TEST_ASSERT_EQUAL_UINT(expected, actual)
TEST_ASSERT_EQUAL_UINT8(expected, actual)
TEST_ASSERT_EQUAL_UINT16(expected, actual)
TEST_ASSERT_EQUAL_UINT32(expected, actual)
TEST_ASSERT_EQUAL_UINT64(expected, actual)
Compare two integers for equality and display errors as unsigned integers. Like INT, there are variants for different sizes also.

TEST_ASSERT_EQUAL_HEX(expected, actual)
TEST_ASSERT_EQUAL_HEX8(expected, actual)
TEST_ASSERT_EQUAL_HEX16(expected, actual)
TEST_ASSERT_EQUAL_HEX32(expected, actual)
TEST_ASSERT_EQUAL_HEX64(expected, actual)
Compares two integers for equality and display errors as hexadecimal. Like the other integer comparisons, you can specify the size... here the size will also effect how many nibbles are shown (for example, HEX16 will show 4 nibbles).

TEST_ASSERT_EQUAL(expected, actual)
Another way of calling TEST_ASSERT_EQUAL_INT

TEST_ASSERT_INT_WITHIN(delta, expected, actual)
Asserts that the actual value is within plus or minus delta of the expected value. This also comes in size specific variants.

TEST_ASSERT_GREATER_THAN(threshold, actual)
Asserts that the actual value is greater than the threshold. This also comes in size specific variants.

TEST_ASSERT_LESS_THAN(threshold, actual)
Asserts that the actual value is less than the threshold. This also comes in size specific variants.

Arrays
_ARRAY
You can append _ARRAY to any of these macros to make an array comparison of that type. Here you will need to care a bit more about the actual size of the value being checked. You will also specify an additional argument which is the number of elements to compare. For example:

TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, elements)

_EACH_EQUAL
Another array comparison option is to check that EVERY element of an array is equal to a single expected value. You do this by specifying the EACH_EQUAL macro. For example:

TEST_ASSERT_EACH_EQUAL_INT32(expected, actual, elements)
Numerical Assertions: Bitwise
TEST_ASSERT_BITS(mask, expected, actual)
Use an integer mask to specify which bits should be compared between two other integers. High bits in the mask are compared, low bits ignored.

TEST_ASSERT_BITS_HIGH(mask, actual)
Use an integer mask to specify which bits should be inspected to determine if they are all set high. High bits in the mask are compared, low bits ignored.

TEST_ASSERT_BITS_LOW(mask, actual)
Use an integer mask to specify which bits should be inspected to determine if they are all set low. High bits in the mask are compared, low bits ignored.

TEST_ASSERT_BIT_HIGH(bit, actual)
Test a single bit and verify that it is high. The bit is specified 0-31 for a 32-bit integer.

TEST_ASSERT_BIT_LOW(bit, actual)
Test a single bit and verify that it is low. The bit is specified 0-31 for a 32-bit integer.

Numerical Assertions: Floats
TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)
Asserts that the actual value is within plus or minus delta of the expected value.

TEST_ASSERT_EQUAL_FLOAT(expected, actual)
TEST_ASSERT_EQUAL_DOUBLE(expected, actual)
Asserts that two floating point values are "equal" within a small % delta of the expected value.

String Assertions
TEST_ASSERT_EQUAL_STRING(expected, actual)
Compare two null-terminate strings. Fail if any character is different or if the lengths are different.

TEST_ASSERT_EQUAL_STRING_LEN(expected, actual, len)
Compare two strings. Fail if any character is different, stop comparing after len characters.

TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, actual, message)
Compare two null-terminate strings. Fail if any character is different or if the lengths are different. Output a custom message on failure.

TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(expected, actual, len, message)
Compare two strings. Fail if any character is different, stop comparing after len characters. Output a custom message on failure.

Pointer Assertions
Most pointer operations can be performed by simply using the integer comparisons above. However, a couple of special cases are added for clarity.

TEST_ASSERT_NULL(pointer)
Fails if the pointer is not equal to NULL

TEST_ASSERT_NOT_NULL(pointer)
Fails if the pointer is equal to NULL

Memory Assertions
TEST_ASSERT_EQUAL_MEMORY(expected, actual, len)
Compare two blocks of memory. This is a good generic assertion for types that can't be coerced into acting like standard types... but since it's a memory compare, you have to be careful that your data types are packed.

_MESSAGE
You can append \_MESSAGE to any of the macros to make them take an additional argument. This argument is a string that will be printed at the end of the failure strings. This is useful for specifying more information about the problem.

*/
