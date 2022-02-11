#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gc.h>

#include "../../unity/src/unity.h"
#include "../include/hashmap.h"

/* included for time and rand functions */
#include <time.h>
#include <stdlib.h>

/* magic number */
#define BUFFER_SIZE 32

/* number of items to add to test lists */
#define TEST_ITERATIONS 10000
#define TEST_ITERATIONS_COLLISIONS 1000

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

hash_t hash_str(void *obj) {

  hash_t hash = 5381;

  int c;
  while ((c = *(char*)obj++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

hash_t hash_collision(void *obj) {
  return 5381;
}

hash_t hash_int(void *obj) {
  return (uintptr_t)obj;
}

int equal_int(void *obj1, void *obj2) {
  return ((uintptr_t)obj1 == (uintptr_t)obj2);
}

int equal_str(void *obj1, void *obj2) {
  return (strcmp((char *)obj1, (char *)obj2) == 0);
}

/* test visit_fn */
void counter_fn (void *key, void *val, void **result) {
  uintptr_t *res_val = (uintptr_t *)(result);
  (*res_val)++;
}

/* internal struct used by make_hashmap_iterator */
struct list {
 void *data;
 struct list *next;
};

/* test visit_fn */
void list_fn(void *key, void *val, void **acc) {

  struct list *lst = *(struct list **)acc;

  struct list *new_val = GC_MALLOC(sizeof(*new_val));
  new_val->data = val;
  new_val->next = lst;

  struct list *new_key = GC_MALLOC(sizeof(*new_key));
  new_key->data = key;
  new_key->next = new_val;

  *acc = new_key;
}


void setUp(void) {
  /* set stuff up here */
  srand((int)time(NULL));
}

void tearDown(void) {
  /* clean stuff up here */
}

void test_hashmap_new(void) {

  Hashmap *h0 = hashmap_make(hash_str, equal_str, equal_str);

  /* no elements in fresh hashmap */
  TEST_ASSERT_EQUAL_INT(0, hashmap_count(h0));
  /* returns NULL when getting a non-existent element */
  TEST_ASSERT_NULL(hashmap_get(h0, "key_1"));
  /* dissoc on an empty hashmap returns the same hashmap */
  TEST_ASSERT_EQUAL_INT(h0, hashmap_dissoc(h0, "key_1"));
}

void test_hashmap_assoc(void)
{
  Hashmap *map = hashmap_make(NULL, NULL, NULL);

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char *key = make_test_key(i);
    char *val = make_test_val(i);
    map = hashmap_assoc(map, key, val);
  }

  /* check all keys/vals added */
  TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS, hashmap_count(map));

  /* check all the keys and vals are present */
  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);

    TEST_ASSERT_EQUAL_STRING(val, hashmap_get(map, key));
  }

  Hashmap *map0 = map;

  /* test that present key/vals can be updated */
  for (int i = 0; i < TEST_ITERATIONS; i++) {

    int r = rand() % TEST_ITERATIONS;
    char* key = make_test_key(r);
    char* val = "updated";

    if (hashmap_get(map, key)) {
      map = hashmap_assoc(map, key, val);
    }
    /* test value is updated */
    TEST_ASSERT_EQUAL_STRING(val, hashmap_get(map, key));

    /* test all keys/values still there */
    TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS, hashmap_count(map));
  }

  /* test that not present key/vals are added */
  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i+2*TEST_ITERATIONS);
    char* val = make_test_val(i+2*TEST_ITERATIONS);

    map = hashmap_assoc(map, key, val);

    /* check the value is updated */
    TEST_ASSERT_EQUAL_STRING(val, hashmap_get(map, key));

    /* check the element count */
    TEST_ASSERT_EQUAL_INT((TEST_ITERATIONS+i+1), hashmap_count(map));
  }

  /* check the original map is unaffected */
  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);

    TEST_ASSERT_EQUAL_STRING(val, hashmap_get(map0, key));
  }
  TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS, hashmap_count(map0));
}

void test_hashmap_dissoc(void) {

  Hashmap *map = hashmap_make(NULL, NULL, NULL);

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);
    map = hashmap_assoc(map, key, val);
  }

  Hashmap *map0 = map;

  /* remove keys at random */
  for (int i = 0; i < TEST_ITERATIONS; i++) {

    int r = rand() % TEST_ITERATIONS;
    char* key_random = make_test_key(r);
    map = hashmap_dissoc(map, key_random);

    /* key no longer found */
    TEST_ASSERT_NULL(hashmap_get(map, key_random));
  }

  /* original map unaffected */
  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);
    TEST_ASSERT_EQUAL_STRING(val, (char*)hashmap_get(map0, key));
  }
  TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS, hashmap_count(map0));
}

void test_hashmap_get(void) {

  Hashmap *map = hashmap_make(NULL, NULL, NULL);

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);
    map = hashmap_assoc(map, key, val);
  }

  /* check all keys/vals added */
  TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS, hashmap_count(map));

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    int r = (rand() % TEST_ITERATIONS);
    char* key = make_test_key(r);
    char* val = make_test_val(r);

    /* check val */
    TEST_ASSERT_EQUAL_STRING(val, hashmap_get(map, key));
  }
}

void test_hashmap_int_keys(void) {

  Hashmap *map = hashmap_make(hash_int, equal_int, equal_str);

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    uintptr_t key = i;
    char *val = make_test_val(i);
    map = hashmap_assoc(map, (void*)key, val);
  }
  /* check all keys/vals added */
  TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS, hashmap_count(map));

  /* check all the keys and vals are present */
  for (int i = 0; i < TEST_ITERATIONS; i++) {

    uintptr_t key = i;
    char* val = make_test_val(i);

    TEST_ASSERT_EQUAL_STRING(val, hashmap_get(map, (void *)key));
  }
}

void test_hashmap_int_vals(void) {

  Hashmap *map = hashmap_make(hash_str, equal_str, equal_int);

  TEST_ASSERT_EQUAL_INT(0, hashmap_count(map));

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char *key = make_test_key(i);
    uintptr_t val = i;
    map = hashmap_assoc(map, (void *)key, (void *)val);
  }
  /* check all keys/vals added */
  TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS, hashmap_count(map));

  /* check all the keys and vals are present */
  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char *key = make_test_key(i);
    uintptr_t val = i;

    TEST_ASSERT_EQUAL_INT(val, hashmap_get(map, (void *)key));
  }
}

void test_hashmap_count(void) {

  Hashmap *map = hashmap_make(hash_int, equal_int, equal_int);

  for (uintptr_t i = 1; i <= TEST_ITERATIONS; i++) {

    map = hashmap_assoc(map, (void*)i, (void*)i);
    TEST_ASSERT_EQUAL_INT(i, hashmap_count(map));
  }

  for (uintptr_t i = 1; i <= TEST_ITERATIONS; i++) {

    map = hashmap_dissoc(map, (void*)i);
    TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS - i, hashmap_count(map));
  }
}

void test_hashmap_collisions(void) {

  /* make a hashmap that is all collisions */
  Hashmap *map = hashmap_make(hash_collision, equal_str, equal_str);
  TEST_ASSERT_EQUAL_INT(0, hashmap_count(map));

  for (int i = 0; i < TEST_ITERATIONS_COLLISIONS; i++) {

    char *key = make_test_key(i);
    char *val = make_test_val(i);
    map = hashmap_assoc(map, (void *)key, (void *)val);
  }
  /* check all keys/vals added */
  TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS_COLLISIONS, hashmap_count(map));

  /* check all the keys and vals are present */
  for (int i = 0; i < TEST_ITERATIONS_COLLISIONS; i++) {

    char *key = make_test_key(i);
    char *val = make_test_val(i);

    TEST_ASSERT_EQUAL_STRING(val, hashmap_get(map, (void *)key));
  }

  /* save a copy */
  Hashmap *map0 = map;

  int added = 0;
  /* test that present key/vals can be updated */
  for (int i = 0; i < TEST_ITERATIONS_COLLISIONS; i++) {

    int r = rand() % TEST_ITERATIONS_COLLISIONS;
    char* key = make_test_key(r);
    char* val = make_test_val(r);
    char* new_val = "updated";

    /* check it hasn't already been updated */
    if (strcmp(val, (char*)hashmap_get(map, key)) == 0) {
      added++;

      /* update the value */
      map = hashmap_assoc(map, key, new_val);
      /* test value is updated */
      TEST_ASSERT_EQUAL_STRING(new_val, (char*)hashmap_get(map, key));

      /* test all keys/values still there */
      TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS_COLLISIONS, hashmap_count(map));
    }
  }
  /* check that some values have actually been added */
  TEST_ASSERT_GREATER_THAN(TEST_ITERATIONS_COLLISIONS / 2, added);

  /* check all the keys are present */
  for (int i = 0; i < TEST_ITERATIONS_COLLISIONS; i++) {

    char *key = make_test_key(i);
    TEST_ASSERT_NOT_NULL(hashmap_get(map, (void *)key));
  }

  /* test that not present key/vals are added */
  for (int i = 0; i < TEST_ITERATIONS_COLLISIONS; i++) {

    char* key = make_test_key(i+2*TEST_ITERATIONS_COLLISIONS);
    char* val = make_test_val(i+2*TEST_ITERATIONS_COLLISIONS);

    map = hashmap_assoc(map, key, val);

    /* check the value is added */
    TEST_ASSERT_EQUAL_STRING(val, hashmap_get(map, key));

    /* check the element count */
    TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS_COLLISIONS + (i+1), hashmap_count(map));
  }
  TEST_ASSERT_EQUAL_INT(2*TEST_ITERATIONS_COLLISIONS, hashmap_count(map));

  //   Hashmap *map1 = map0;

  /* remove keys at random */
  int removed = 0;
  for (int i = 0; i < TEST_ITERATIONS_COLLISIONS; i++) {

    int r = rand() % TEST_ITERATIONS_COLLISIONS;
    char* key_random = make_test_key(r);

    /* check it hasn't been removed already */
    if (hashmap_get(map, key_random)) {
      /* remove key */
      removed++;
      map = hashmap_dissoc(map, key_random);
      /* key no longer found */
      TEST_ASSERT_NULL(hashmap_get(map, key_random));
    }
  }
  /* check that some values have actually been removed */
  TEST_ASSERT_GREATER_THAN(TEST_ITERATIONS_COLLISIONS / 2, removed);

  /* original map unaffected */
  for (int i = 0; i < TEST_ITERATIONS_COLLISIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);
    TEST_ASSERT_EQUAL_STRING(val, (char*)hashmap_get(map0, key));
  }
  TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS_COLLISIONS, hashmap_count(map0));
}

void test_hashmap_visit_count(void) {

  Hashmap *map = hashmap_make(hash_int, equal_int, equal_int);
  uintptr_t result;

  for (uintptr_t i = 1; i <= TEST_ITERATIONS; i++) {
    result = 0;

    map = hashmap_assoc(map, (void*)i, (void*)i);
    hashmap_visit(map, counter_fn, (void*)&result);
    TEST_ASSERT_EQUAL_INT(i, result);
  }

  for (uintptr_t i = 1; i <= TEST_ITERATIONS; i++) {
    result = 0;

    map = hashmap_dissoc(map, (void*)i);
    hashmap_visit(map, counter_fn, (void*)&result);
    TEST_ASSERT_EQUAL_INT(TEST_ITERATIONS - i, result);
  }
}

void test_hashmap_visit_list(void) {

  Hashmap *map = hashmap_make(hash_str, equal_str, equal_str);

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);
    map = hashmap_assoc(map, key, val);
  }

  struct list *lst = NULL;

  hashmap_visit(map, list_fn, (void **)&lst);

  /* the order of keys and vals is not guaranteed */
  /* but keys come before vals */
  int count = 0;
  while (lst) {
    char *key = (char *)lst->data;
    lst = lst->next;
    char *val = (char *)lst->data;
    /* test the value matches that stored for the preceding key */
    TEST_ASSERT_EQUAL_STRING(val, hashmap_get(map, key));

    lst = lst->next;
    count++;
  }
  TEST_ASSERT_EQUAL_INT(hashmap_count(map), count);
}

void test_hashmap_readme(void) {

  /* create a hashmap by calling new with your functions for hash and equality */
  Hashmap *h1 = hashmap_make(hash_str, equal_str, equal_str);

  /* add a key/value pair */
  h1 = hashmap_assoc(h1, "key_1", "val_1");

  /* add another key/value pair */
  h1 = hashmap_assoc(h1, "key_2", "val_2");

  /* remove a key/value pair */
  Hashmap *h2 = hashmap_dissoc(h1, "key_1");

  /* get a value using the key */
  char *val_h1 = hashmap_get(h1, "key_1");
  TEST_ASSERT_EQUAL_STRING("val_1", val_h1);
  // ==> "val_ha is: val_1" (h1 not affected by dissoc)

  /* get a value using the key */
  char *val_h2 = hashmap_get(h2, "key_1");
  TEST_ASSERT_NULL(val_h2);
  // ==> "val_ha is: (null)" (not found in h2)

  int count_h1 = hashmap_count(h1);
  TEST_ASSERT_EQUAL_INT(2, count_h1);
  // ==> "count_h1 is: 2"

  int count_h2 = hashmap_count(h2);
  TEST_ASSERT_EQUAL_INT(1, count_h2);
  // ==> "count_h2 is: 1"
}

void test_hashmap_iterator(void) {

  Hashmap *map  = hashmap_make(hash_str, equal_str, equal_str);

  for (int i = 0; i < TEST_ITERATIONS; i++) {

    char* key = make_test_key(i);
    char* val = make_test_val(i);
    map = hashmap_assoc(map, key, val);
  }

  /* make an interator */
  Iterator *iter = hashmap_iterator_make(map);

  /* check a valid iterator is returned */
  TEST_ASSERT_NOT_NULL(iter);

  int count = 0;
  while (iter) {

    char* iter_key = (char*)iterator_value(iter);
    iter = iterator_next(iter);

    /* check key exists */
    TEST_ASSERT_NOT_NULL(hashmap_get(map, iter_key));

    char* iter_val = (char*)iterator_value(iter);
    iter = iterator_next(iter);

    /* check val exists for key */
    TEST_ASSERT_EQUAL_STRING(iter_val, hashmap_get(map, iter_key));
    count++;
  }
  /* check the number of elements matches */
  TEST_ASSERT_EQUAL_INT(hashmap_count(map), count);
}


int main(int argc, char **argv) {

  UNITY_BEGIN();

  RUN_TEST(test_hashmap_new);
  RUN_TEST(test_hashmap_assoc);
  RUN_TEST(test_hashmap_get);
  RUN_TEST(test_hashmap_dissoc);
  RUN_TEST(test_hashmap_count);
  RUN_TEST(test_hashmap_int_keys);
  RUN_TEST(test_hashmap_int_vals);
  RUN_TEST(test_hashmap_collisions);

  RUN_TEST(test_hashmap_visit_count);
  RUN_TEST(test_hashmap_visit_list);
  RUN_TEST(test_hashmap_iterator);
  RUN_TEST(test_hashmap_readme);

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
