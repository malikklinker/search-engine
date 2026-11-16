#include "string_list.h"
#include <assert.h>
#include <string.h>

int main(void) {
  ll_string *left = cons("good", cons("afternoon", cons("everybody", NULL)));
  ll_string *right = cons("welcome", cons("to", cons("searchtown", NULL)));
  left = extend(left, right);
  char *expected[] = {"good",    "afternoon", "everybody",
                      "welcome", "to",        "searchtown"};
  for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
    ll_string *entry = get_element(left, i);
    assert(entry && !strcmp(entry->value, expected[i]));
  }
  assert(get_element(left, 6) == NULL);
  assert(get_element(left, 3) != right);
  assert(get_element(left, 3)->value != right->value);
  free_list(left);
  free_list(right);
  return 0;
}
