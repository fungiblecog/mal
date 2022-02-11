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
#include "core.h"

#define SYMBOL_DEFBANG "def!"
#define SYMBOL_LETSTAR "let*"
#define SYMBOL_IF "if"
#define SYMBOL_FNSTAR "fn*"
#define SYMBOL_DO "do"

#define PROMPT_STRING "user> "

MalType *READ(char *str) {

  return read_str(str);
}

MalType *EVAL(MalType *ast, Env *env) {

  /* forward references */
  MalType *eval_ast(MalType *ast, Env *env);
  MalType *eval_defbang(MalType *ast, Env **env);
  void eval_letstar(MalType **ast, Env **env);
  void eval_if(MalType **ast, Env **env);
  MalType *eval_fnstar(MalType *ast, Env *env);
  MalType *eval_do(MalType *ast, Env *env);

  /* Use goto to jump here rather than calling eval for tail-call elimination */
 TCE_entry_point:

  /* NULL */
  if (!ast) { return make_nil(); }

  /* not a list */
  if (!is_list(ast)) { return eval_ast(ast, env); }

  /* empty list */
  if (ast->value.mal_list == NULL) { return ast; }

  /* list */
  MalType *first = (ast->value.mal_list)->data;
  char *symbol = first->value.mal_symbol;

  if (is_symbol(first)) {

    /* handle special symbols first */
    if (strcmp(symbol, SYMBOL_DEFBANG) == 0) {
      return eval_defbang(ast, &env);
    }
    else if (strcmp(symbol, SYMBOL_LETSTAR) == 0) {

      /* TCE - modify ast and env directly and jump back to eval */
      eval_letstar(&ast, &env);

      if (is_error(ast)) { return ast; }
      goto TCE_entry_point;
    }
    else if (strcmp(symbol, SYMBOL_IF) == 0) {

      /* TCE - modify ast directly and jump back to eval */
      eval_if(&ast, &env);

      if (is_error(ast)) { return ast; }
      goto TCE_entry_point;
    }
    else if (strcmp(symbol, SYMBOL_FNSTAR) == 0) {
      return eval_fnstar(ast, env);
    }
    else if (strcmp(symbol, SYMBOL_DO) == 0) {

      /* TCE - modify ast and env directly and jump back to eval */
      ast = eval_do(ast, env);

      if (is_error(ast)) { return ast; }
      goto TCE_entry_point;
    }
  }
  /* first element is not a special symbol */
  MalType *evaluated_list = eval_ast(ast, env);

  /* apply the first element of the list to the arguments */
  List *evlst = evaluated_list->value.mal_list;
  MalType *func = evlst->data;

  if (is_function(func)) {
    return (*func->value.mal_function)(evlst->next);
  }
  else if (is_closure(func)) {

    MalClosure *closure = func->value.mal_closure;
    List *params = (closure->parameters)->value.mal_list;

    long param_count = list_count(params);
    long arg_count = list_count(evlst->next);

    if (param_count > arg_count) {
      return make_error("too few arguments supplied to function");
    }
    else if ((param_count < arg_count) && !closure->more_symbol) {
      return make_error("too many arguments supplied to function");
    }
    else {

      /* TCE - modify ast and env directly and jump back to eval */
      env = env_make(closure->env, params, evlst->next, closure->more_symbol);
      ast = func->value.mal_closure->definition;

      if (is_error(ast)) { return ast; }
      goto TCE_entry_point;
    }
  }
  else {
    return make_error_fmt("first item in list is not callable: '%s'",   \
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

  /* Greeting message */
  puts("Make-a-lisp version 0.0.5\n");
  puts("Press Ctrl+d to exit\n");

  Env *repl_env = env_make(NULL, NULL, NULL, NULL);

  ns *core = ns_make_core();
  Iterator *iter = hashmap_iterator_make(core->mappings);

  while (iter) {
    char *symbol = iter->value;

    iter = iterator_next(iter);
    MalType *(*function)(List *) = (MalType *(*)(List *))iter->value;

    env_set_C_fn(repl_env, symbol, function);

    iter = iterator_next(iter);
  }

  /* add not function */
  /* not using rep as it prints the result */
  EVAL(READ("(def! not (fn* (a) (if a false true)))"), repl_env);

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
  MalType *evaluate_list(List *lst, Env *env);
  MalType *evaluate_vector(Vector *vec, Env *env);
  MalType *evaluate_hashmap(Hashmap *map, Env *env);

  if (is_symbol(ast)) {

    MalType *symbol_value = env_get(env, ast);

    if (symbol_value) {
      return symbol_value;
    } else {
      return make_error_fmt("var '%s' not found", pr_str(ast, UNREADABLY));
    }
  }
  else if (is_list(ast)) {

    return evaluate_list(ast->value.mal_list, env);
  }
  else if (is_vector(ast)) {

    return evaluate_vector(ast->value.mal_vector, env);
  }
  else if (is_hashmap(ast)) {

    return evaluate_hashmap(ast->value.mal_hashmap, env);
  }
  else {
    return ast;
  }
}

MalType *eval_defbang(MalType *ast, Env **env) {

  List *lst = (ast->value.mal_list)->next;

  if (!lst || !lst->next || lst->next->next) {
    return make_error_fmt("'def!': expected exactly two arguments");
  }

  MalType *defbang_symbol = lst->data;

  if (!is_symbol(defbang_symbol)) {
    return make_error_fmt("'def!': expected symbol for first argument");
  }

  MalType *defbang_value = lst->next->data;
  MalType *result = EVAL(defbang_value, *env);

  if (!is_error(result)){
    *env = env_set(*env, defbang_symbol, result);
  }
  return result;
}

void eval_letstar(MalType **ast, Env **env) {

  List *lst = (*ast)->value.mal_list;

  if (!lst->next) {
    *ast = make_error("'let*': missing bindings list");
    return;
  }

  MalType *bindings = lst->next->data;
  MalType *forms = lst->next->next ? lst->next->next->data : make_nil();

  if (!is_sequential(bindings)) {
    *ast = make_error("'let*': first argument is not list or vector");
    return;
  }

  Iterator *bindings_iter = NULL;

  /* bindings can be a list or vector */
  if (is_vector(bindings)) {

    if (vector_count(bindings->value.mal_vector) % 2 == 1) {
      *ast = make_error("'let*': expected an even number of binding pairs");
      return;
    }
    bindings_iter = vector_iterator_make(bindings->value.mal_vector);
  }
  else {
    if (list_count(bindings->value.mal_list) % 2 == 1) {
      *ast = make_error("'let*': expected an even number of binding pairs");
      return;
    }
    bindings_iter = list_iterator_make(bindings->value.mal_list);
  }

  Env *letstar_env = env_make(*env, NULL, NULL, NULL);

  /* evaluate the bindings */
  while (bindings_iter) {

    MalType *symbol = bindings_iter->value;
    bindings_iter = iterator_next(bindings_iter);
    MalType *value = EVAL(bindings_iter->value, letstar_env);

    /* early return from error */
    if (is_error(value)) {
      *ast = value;
      return;
    }

    env_set(letstar_env, symbol, value);
    bindings_iter = iterator_next(bindings_iter);
  }

  *env = letstar_env;
  *ast = forms;
  return;
}

void eval_if(MalType **ast, Env **env) {

  List *lst = (*ast)->value.mal_list;

  if (!lst->next || !lst->next->next) {
    *ast = make_error("'if': too few arguments");
    return;
  }

  if (lst->next->next->next && lst->next->next->next->next) {
    *ast = make_error("'if': too many arguments");
    return;
  }

  MalType *condition = EVAL(lst->next->data, *env);

  if (is_error(condition)) {
    *ast = condition;
    return;
  }

  if (is_false(condition) || is_nil(condition)) {

    /* check whether false branch is present */
    if (lst->next->next->next) {
      *ast = lst->next->next->next->data;
      return;
    }
    else {
      *ast = make_nil();
      return;
    }

  } else {
    *ast = lst->next->next->data;
    return;
  }
}

MalType *eval_fnstar(MalType *ast, Env *env) {

  /* forward reference */
  MalType *regularise_parameters(List **params, MalType **more);

  List *lst = ast->value.mal_list;

  if (!lst->next) {
    return make_error("'fn*': missing argument list");
  }
  else if (!lst->next->next) {
    return make_error("'fn*': missing function body");
  }

  MalType *params = lst->next->data;

  List *args = NULL;
  if (is_vector(params)) {
    args = vector_to_list(params->value.mal_vector);
  } else {
    args = params->value.mal_list;
  }

  MalType *more_symbol = NULL;

  MalType *result = regularise_parameters(&args, &more_symbol);
  if (is_error(result)) { return result; }

  return make_closure(env, make_list(args), lst->next->next->data, more_symbol);
}

MalType *eval_do(MalType *ast, Env *env) {

  List *lst = ast->value.mal_list;

  /* handle empty 'do' */
  if (!lst->next) {
    return make_nil();
  }

  /* evaluate all but the last form */
  lst = lst->next;
  while (lst->next) {

    MalType *val = EVAL(lst->data, env);

    /* return error early */
    if (is_error(val)) {
      return val;
    }
    lst = lst->next;
  }
  /* return the last form for TCE evaluation */
  return lst->data;
}

MalType *evaluate_list(List *lst, Env *env) {

  List *evlst = NULL;
  while (lst) {

    MalType *val = EVAL(lst->data, env);

    if (is_error(val)) {
      return val;
    }

    evlst = list_cons(evlst, val);
    lst = lst->next;
  }
  return make_list(list_reverse(evlst));
}

MalType *evaluate_vector(Vector *vec, Env *env) {

  Iterator *iter = vector_iterator_make(vec);
  Vector *evec = vector_make();

  while (iter) {

    MalType *val = EVAL(iter->value, env);

    if (is_error(val)) { return val; }

    evec = vector_push(evec, val);
    iter = iterator_next(iter);
  }
  return make_vector(evec);
}

MalType *evaluate_hashmap(Hashmap *map, Env *env) {

  Iterator *iter = hashmap_iterator_make(map);
  Hashmap *emap = hashmap_make(NULL, cmp_maltypes, cmp_maltypes);;

  while (iter) {

    /* keys are unevaluated */
    MalType *key = iter->value;
    /* values are evaluated */
    iter = iterator_next(iter);
    MalType *val = EVAL(iter->value, env);

    if (is_error(val)) { return val; }

    emap = hashmap_assoc(emap, key, val);
    iter = iterator_next(iter);
  }
  return make_hashmap(emap);
}

MalType *regularise_parameters(List **args, MalType **more_symbol) {

  List *regular_args = NULL;
  while (*args) {

    MalType *val = (*args)->data;

    if (!is_symbol(val)) {
      return make_error_fmt("non-symbol found in fn argument list '%s'", \
                            pr_str(val, UNREADABLY));
    }

    if (val->value.mal_symbol[0] == '&') {

      /* & is found but there is no symbol */
      if (val->value.mal_symbol[1] == '\0' && !(*args)->next) {
        return make_error("missing symbol after '&' in argument list");
      }
      /* & is found and there is a single symbol after */
      else if ((val->value.mal_symbol[1] == '\0' && (*args)->next &&
                is_symbol((*args)->next->data) && !(*args)->next->next)) {

        /* TODO: check symbol is no a duplicate of one already on the list */
        *more_symbol = (*args)->next->data;
        break;
      }
      /* & is found and there extra symbols after */
      else if ((val->value.mal_symbol[1] == '\0' && (*args)->next && (*args)->next->next)) {
        return make_error_fmt("unexpected symbol after '& %s' in argument list: '%s'", \
                              pr_str((*args)->next->data, UNREADABLY),  \
                              pr_str((*args)->next->next->data, UNREADABLY));
      }
      /* & is found as part of the symbol and no other symbols */
      else if (val->value.mal_symbol[1] != '\0' && !(*args)->next) {
        *more_symbol = make_symbol((val->value.mal_symbol + 1));
        break;
      }
      /* & is found as part of the symbol but there are other symbols after */
      else if (val->value.mal_symbol[1] != '\0' && (*args)->next) {
        return make_error_fmt("unexpected symbol after '%s' in argument list: '%s'", \
                              pr_str(val, UNREADABLY),  \
                              pr_str((*args)->next->data, UNREADABLY));
       }
    }

    /* & is not found - add the symbol to the regular argument list */
    else {

      if (list_find(regular_args, val->value.mal_symbol, cmp_chars) > 0) {
        return make_error_fmt("duplicate symbol in argument list: '%s'", \
                              pr_str(val, UNREADABLY));
      }
      else {
        regular_args = list_cons(regular_args, val);
      }
    }
    *args = (*args)->next;
  }

  *args = list_reverse(regular_args);
  return make_nil();
}

/* silence the compiler after swap!, apply, and map
   are added to the core */
MalType *apply(MalType *ast, Env *env) {
  return make_nil();
}
