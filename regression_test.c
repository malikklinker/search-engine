#include "index_lib.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  TermLookupTable *table = build_term_lookup_table(1);
  add_message_id_to_term(table, "gravity", "first");
  add_message_id_to_term(table, "orbit", "first");
  add_message_id_to_term(table, "orbit", "second");
  ll_string *queries = cons("missing", cons("orbit", NULL));
  string_set *result = union_lookup(table, queries);
  assert(contains(result, "first"));
  assert(contains(result, "second"));
  free_set(result);
  result = intersection_lookup(table, queries);
  assert(result == NULL);
  free_list(queries);
  queries = cons("gravity", cons("orbit", NULL));
  result = intersection_lookup(table, queries);
  assert(contains(result, "first") && !contains(result, "second"));
  free_set(result);
  free_list(queries);
  free_term_lookup_table(table);
  char longword[5001];
  memset(longword, 'a', 5000);
  longword[5000] = '\0';
  ll_string *tokens = tokenize_string(longword);
  assert(tokens && strlen(tokens->value) == 5000 && !tokens->next);
  free_list(tokens);
  return 0;
}
