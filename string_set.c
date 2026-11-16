#include "string_set.h"
#include "string_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

string_set *make_empty_set(void) { return NULL; }

bool contains(string_set *set, char *s) {
  if (!set) {
    return false;
  }
  int cmp = strcmp(s, set->value);
  if (cmp == 0) {
    return true;
  } else if (cmp < 0) {
    return contains(set->left, s);
  } else {
    return contains(set->right, s);
  }
}

string_set *string_create(char *s) {
  string_set *tree = malloc(sizeof(string_set));
  tree->value = strdup(s);
  tree->left = tree->right = NULL;
  return tree;
}

string_set *add(string_set *set, char *s) {
  if (!set) {
    return string_create(s);
  }

  int cmp = strcmp(s, set->value);

  if (cmp == 0) {
    return set;
  } else if (cmp < 0) {
    set->left = add(set->left, s);
  } else {
    set->right = add(set->right, s);
  }
  return set;
}

ll_string *list_entries(string_set *set) {
  if (!set) {
    return NULL;
  }
  ll_string *left = list_entries(set->left);
  ll_string *right = list_entries(set->right);
  ll_string *ll_entry = cons(set->value, right);

  if (!left) {
    return ll_entry;
  }

  ll_string *tail = left;
  while (tail->next) {
    tail = tail->next;
  }
  tail->next = ll_entry;

  return left;
}

string_set *add_all(string_set *destination, string_set *source) {
  if (!source) {
    return destination;
  }
  destination = add_all(destination, source->left);
  destination = add(destination, source->value);
  destination = add_all(destination, source->right);
  return destination;
}

string_set *set_union(string_set *set_a, string_set *set_b) {
  string_set *result = make_empty_set();
  result = add_all(result, set_a);
  result = add_all(result, set_b);
  return result;
}
// im making a helper because I want to recurse with a destination as well not
// just set a and b
string_set *intersect_helper(string_set *destination, string_set *set_a,
                             string_set *set_b) {
  if (!set_a) {
    return destination;
  }
  destination = intersect_helper(destination, set_a->left, set_b);
  if (contains(set_b, set_a->value)) {
    destination = add(destination, set_a->value);
  }
  destination = intersect_helper(destination, set_a->right, set_b);
  return destination;
}

string_set *set_intersection(string_set *set_a, string_set *set_b) {
  return intersect_helper(make_empty_set(), set_a, set_b);
}

void free_set(string_set *set) {
  if (!set) {
    return;
  }
  free_set(set->left);
  free_set(set->right);
  free(set->value);
  free(set);
}
