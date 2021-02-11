#include <assert.h>
#include <string.h>

#include "../../macro.h"

void test_macro(void) {
  {
    MacroDef *macro_def = find_macro_def("FOO", 3, 0);
    assert(macro_def == NULL);
  }
  {
    Token token = {kTokenNum, NULL, 42, "42", 2};
    new_macro_def("FOO", 3, &token);
    MacroDef *macro_def = find_macro_def("FOO", 3, 0);
    assert(macro_def != NULL);
    assert(!memcmp(macro_def->name, "FOO", 3));
    assert(macro_def->name_len == 3);
    assert(!memcmp(macro_def->tok->str, "42", 2));
    assert(macro_def->end_tok == NULL);
  }
  {
    Token token = {kTokenNum, NULL, 42, "42", 2};
    new_macro_def("BAR", 3, &token);
    MacroDef *macro_def = find_macro_def("BAR", 3, 0);
    assert(macro_def != NULL);
    assert(!memcmp(macro_def->name, "BAR", 3));
    assert(macro_def->name_len == 3);
    assert(!memcmp(macro_def->tok->str, "42", 2));
    assert(macro_def->end_tok == NULL);
  }
  {
    MacroDef *macro_def;
    delete_macro_def("FOO", 3);
    macro_def = find_macro_def("FOO", 3, 0);
    assert(macro_def == NULL);
    macro_def = find_macro_def("BAR", 3, 0);
    assert(macro_def != NULL);
  }
}

int main(void) { test_macro(); }
