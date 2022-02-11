#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gc.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "types.h"
#include "reader.h"
#include "printer.h"
#include "env.h"

#define PROMPT_STRING "user> "

MalType *READ(char *str) {

  return read_str(str);
}

MalType *EVAL(MalType *ast, Env *env) {

  /* forward references */
  MalType *eval_ast(MalType *ast, Env *env);

  /* NULL */
  if (!ast) { return make_nil(); }

  /* not a list */
  if (!is_list(ast)) { return eval_ast(ast, env); }

  /* empty list */
  if (ast->value.mal_list == NULL) { return ast; }

  /* list */

  /* evaluate the list */
  MalType *evaluated_list = eval_ast(ast, env);

  if (is_error(evaluated_list)) { return evaluated_list; }

  /* apply the first element of the list to the arguments */
  List *evlst = evaluated_list->value.mal_list;
  MalType *func = evlst->data;

  if (is_function(func)) {
    return (*func->value.mal_function)(evlst->next);
  }
  else {
    return make_error_fmt("Error: first item in list is not callable: %s.", \
                          pr_str(func, UNREADABLY));
  }
}

void PRINT(MalType *val) {

  char *output = pr_str(val, READABLY);
  printf("%s\n", output);
}

void rep(char *str, Env *env) {

  PRINT(EVAL(READ(str), env));
}

int main(int argc, char **argv) {

  MalType *mal_add(List *args);
  MalType *mal_sub(List *args);
  MalType *mal_mul(List *args);
  MalType *mal_div(List *args);

  /* Greeting message */
  puts("Make-a-lisp version 0.0.2\n");
  puts("Press Ctrl+d to exit\n");

  MalType *func_add = make_function(&mal_add);
  MalType *func_sub = make_function(&mal_sub);
  MalType *func_mul = make_function(&mal_mul);
  MalType *func_div = make_function(&mal_div);

  Hashmap *g = hashmap_make(NULL, cmp_chars, cmp_chars);
  g = hashmap_assoc(g, "+", func_add);
  g = hashmap_assoc(g, "-", func_sub);
  g = hashmap_assoc(g, "*", func_mul);
  g = hashmap_assoc(g, "/", func_div);

  Env *repl_env = GC_MALLOC(sizeof(*repl_env));
  repl_env->data = g;

  while (1) {

    /* print prompt and get input*/
    /* readline allocates memory for input */
    char *input = readline(PROMPT_STRING);

    /* Check for EOF (Ctrl-D) */
    if (!input) {
      printf("\n");
      return 0;
    }

    /* add input to history */
    add_history(input);

    /* call Read-Eval-Print */
    rep(input, repl_env);

    /* have to release the memory used by readline */
    free(input);
  }

  return 0;
}

MalType *eval_ast(MalType *ast, Env *env) {

  /* forward references */
  List *evaluate_list(List *lst, Env *env);
  Vector *evaluate_vector(Vector *vec, Env *env);
  Hashmap *evaluate_hashmap(Hashmap *map, Env *env);

  if (is_symbol(ast)) {

    MalType *symbol_value = hashmap_get(env->data, ast->value.mal_symbol);

    if (symbol_value) {
      return symbol_value;
    } else {
      return make_error_fmt("var '%s' not found", pr_str(ast, UNREADABLY));
    }
  }
  else if (is_list(ast)) {

    List *lst = ast->value.mal_list;
    List *evlst = NULL;

    while (lst) {

      MalType *result = EVAL(lst->data, env);

      if (is_error(result)) {
        return result;
      }

      evlst = list_cons(evlst, result);
      lst = lst->next;
    }
    return make_list(list_reverse(evlst));
  }
  else if (is_vector(ast)) {

    Iterator *iter = vector_iterator_make(ast->value.mal_vector);
    Vector *evec = vector_make();

    while (iter) {

      MalType *result = EVAL(iter->value, env);

      if (is_error(result)) {
        return result;
      }

      evec = vector_push(evec, result);
      iter = iterator_next(iter);
    }
    return make_vector(evec);
  }
  else if (is_hashmap(ast)) {

    Iterator *iter = hashmap_iterator_make(ast->value.mal_hashmap);
    Hashmap *emap = hashmap_make(NULL, cmp_maltypes, cmp_maltypes);

    while (iter) {

      /* keys are unevaluated */
      MalType *key = iter->value;

      iter = iterator_next(iter);

      /* values are evaluated */
      MalType *val = EVAL(iter->value, env);

      if (is_error(val)) {
        return val;
      }

      emap = hashmap_assoc(emap, key, val);
      iter = iterator_next(iter);
    }
    return make_hashmap(emap);
  }
  else {
    return ast;
  }
}

MalType *mal_add(List *args) {

  MalType *result = GC_MALLOC(sizeof(*result));
  result->type = MALTYPE_INTEGER;

  List *arg_list = args;

  long sum = 0;
  while(arg_list) {

    MalType *val = arg_list->data;
    /* TODO: check argument type */

    sum = sum + val->value.mal_integer;

    arg_list = arg_list->next;
  }

  result->value.mal_integer = sum;
  return result;
}

MalType *mal_sub(List *args) {

  long sum;
  MalType *result = GC_MALLOC(sizeof(*result));
  result->type = MALTYPE_INTEGER;

  List *arg_list = args;
  if (arg_list) {

    MalType *first_val = arg_list->data;
    arg_list = arg_list->next;
    /* TODO: check argument type */

    sum = first_val->value.mal_integer;
    while(arg_list) {

      MalType *val = arg_list->data;
      /* TODO: check argument type */

      sum = sum - val->value.mal_integer;

      arg_list = arg_list->next;
    }
  }
  else {
    sum = 0;
  }

  result->value.mal_integer = sum;
  return result;
}

MalType *mal_mul(List *args) {

  MalType *result = GC_MALLOC(sizeof(*result));
  result->type = MALTYPE_INTEGER;

  List *arg_list = args;

  long product = 1;
  while(arg_list) {

    MalType *val = arg_list->data;
    /* TODO: check argument type */

    product *= val->value.mal_integer;

    arg_list = arg_list->next;
  }

  result->value.mal_integer = product;
  return result;
}

MalType *mal_div(List *args) {

  long product;
  MalType *result = GC_MALLOC(sizeof(*result));
  result->type = MALTYPE_INTEGER;

  List *arg_list = args;

  if (arg_list) {
    MalType *first_val = arg_list->data;
    /* TODO: check argument type */
    product = first_val->value.mal_integer;
    arg_list = arg_list->next;

    while(arg_list) {

      MalType *val = arg_list->data;
      /* TODO: check argument type */

    product /= (val->value.mal_integer);
    arg_list = arg_list->next;
    }
  } else {
    product = 1;
  }
  result->value.mal_integer = product;
  return result;
}
