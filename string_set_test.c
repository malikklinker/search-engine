#include "string_list.h"
#include "string_set.h"
#include <assert.h>
#include <string.h>

static void expect_entries(string_set *set, const char **expected,
                           size_t count) {
  ll_string *entries = list_entries(set), *entry = entries;
  for (size_t i = 0; i < count; ++i) {
    assert(entry && !strcmp(entry->value, expected[i]));
    entry = entry->next;
  }
  assert(!entry);
  free_list(entries);
}

int main(void) {
  string_set *set = add(add(NULL, "hello"), "avocado");
  assert(contains(set, "hello") && contains(set, "avocado"));
  assert(!contains(set, "dimetrodon"));
  set = add(set, "dimetrodon");
  assert(contains(set, "dimetrodon"));
  const char *initial[] = {"avocado", "dimetrodon", "hello"};
  expect_entries(set, initial, 3);
  free_set(set);

  string_set *animals = add(add(NULL, "kitty"), "lizard");
  string_set *fluffy = add(add(add(NULL, "cloud"), "pillow"), "kitty");
  string_set *intersection = set_intersection(animals, fluffy);
  const char *common[] = {"kitty"};
  expect_entries(intersection, common, 1);
  string_set *combined = set_union(animals, fluffy);
  const char *all[] = {"cloud", "kitty", "lizard", "pillow"};
  expect_entries(combined, all, 4);
  free_set(intersection);
  free_set(combined);
  free_set(animals);
  free_set(fluffy);
  return 0;
}
