#include "index_lib.h"
#include "string_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

void str_should_be_exactly_equal(const char *message, char *expected,
                                 char *actual) {
  failures += strcmp(expected, actual) != 0;
  printf("%s\n", message);
  printf("%s: wanted \"%s\", got \"%s\"\n",
         !strcmp(expected, actual) ? "SUCCESS" : "FAILURE", expected, actual);
}

void should_be_exactly_equal(const char *message, int expected, int actual) {
  failures += expected != actual;
  printf("%s\n", message);
  printf("%s: wanted %d, got %d\n",
         (expected == actual) ? "SUCCESS" : "FAILURE", expected, actual);
}

bool list_includes_string(ll_string *list, char *s) {
  for (; list; list = list->next) {
    if (!strcmp(list->value, s)) {
      return true;
    }
  }
  return false;
}

int main(void) {
  printf("heck yes!\n");

  should_be_exactly_equal("'this' is a prefix", true,
                          startswith("this is a test", "this"));
  should_be_exactly_equal("'this is a' is a prefix", true,
                          startswith("this is a test", "this is a"));
  should_be_exactly_equal("'wrong' is not a prefix", false,
                          startswith("this is a test", "wrong"));

  char *test_string = strdup("CONTAINS different cASes");
  lowercase(test_string);
  str_should_be_exactly_equal("now it's lowercase", "contains different cases",
                              test_string);

  should_be_exactly_equal("'wrong' is not a prefix", false,
                          startswith("this is a test", "wrong"));

  string_set *stopwords = make_empty_set();
  ll_string *terms = extract_terms("testdata/testpost1", stopwords);

  for (ll_string *here = terms; here; here = here->next) {
    printf("%s ", here->value);
  }
  printf("\n");
  free_list(terms);

  PostMetadata *metadata = extract_metadata("examples/orbits.txt");
  printf("newsgroups: %s\n", metadata->newsgroups);

  free(test_string);
  free_post_metadata(metadata);
  free_set(stopwords);
  return failures ? 1 : 0;
}
