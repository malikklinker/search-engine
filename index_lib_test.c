#include "index_lib.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static bool includes(ll_string *list, char *word) {
  for (; list; list = list->next)
    if (!strcmp(list->value, word))
      return true;
  return false;
}

int main(void) {
  assert(startswith("this is a test", "this"));
  assert(startswith("this is a test", "this is a"));
  assert(!startswith("this is a test", "wrong"));
  char mixed[] = "CONTAINS different cASes";
  lowercase(mixed);
  assert(!strcmp(mixed, "contains different cases"));

  string_set *stopwords = make_empty_set();
  ll_string *terms = extract_terms("testdata/testpost1", stopwords);
  char *expected[] = {"amazing", "exciting", "secret", "discussions"};
  for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i)
    assert(includes(terms, expected[i]));
  free_list(terms);

  PostMetadata *metadata = extract_metadata("examples/orbits.txt");
  assert(!strcmp(metadata->newsgroups, "science.physics"));
  assert(!strcmp(metadata->subject, "Orbit integration"));
  assert(!strcmp(metadata->message_id, "<orbits@example.invalid>"));
  free_post_metadata(metadata);
  free_set(stopwords);
  return 0;
}
