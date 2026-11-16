#include "index_lib.h"
#include "string_list.h"
#include "string_set.h"

#include <ctype.h>
#include <glob.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long djb_hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while (*str != '\0') {
    c = *str;
    hash = ((hash << 5) + hash) + (unsigned char)c; /* hash * 33 + c */
    str++;
  }
  return hash;
}

// Find the first newline in the input and replace it with \0
void chomp(char *s) {
  for (int i = 0; s[i]; i++) {
    if (s[i] == '\n' || s[i] == '\r') {
      // Check for \r in case we're on Windows or a very old Mac??
      s[i] = '\0';
      break;
    }
  }
}

// string utilities
void lowercase(char *s) {
  size_t len = strlen(s);
  for (size_t i = 0; i < len; i++) {
    s[i] = tolower((unsigned char)s[i]);
  }
}

bool startswith(char *longstring, char *prefix) {
  size_t len = strlen(prefix);
  size_t len_long = strlen(longstring);
  if (len > len_long) {
    return false;
  }
  for (size_t i = 0; i < len; i++) {
    if (prefix[i] != longstring[i]) {
      return false;
    }
  }
  return true;
}

PostMetadata *extract_metadata(char *filename) {
  FILE *infile;
  infile = fopen(filename, "r");
  if (!infile) {
    fprintf(stderr, "couldn't open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  PostMetadata *result = calloc(1, sizeof(PostMetadata));
  char buf[1024];

  while (fgets(buf, 1024, infile)) {
    chomp(buf);
    if (startswith(buf, "From: ") && !result->author) {
      result->author = strdup(buf + strlen("From: "));
    } else if (startswith(buf, "Subject: ") && !result->subject) {
      result->subject = strdup(buf + strlen("Subject: "));
    } else if (startswith(buf, "Message-ID: ") && !result->message_id) {
      result->message_id = strdup(buf + strlen("Message-ID: "));
    } else if (startswith(buf, "Newsgroups: ") && !result->newsgroups) {
      result->newsgroups = strdup(buf + strlen("Newsgroups: "));
    }
  }
  result->filename = strdup(filename);

  fclose(infile);
  return result;
}

static const char *const wordpat = "[0-9a-z']+";
#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

ll_string *tokenize_string(char *s) {
  regex_t regex;
  regmatch_t pmatch[1];
  regoff_t len;

  if (regcomp(&regex, wordpat, REG_EXTENDED | REG_NEWLINE | REG_ICASE)) {
    fprintf(stderr, "could not compile the regular expression??\n");
    exit(EXIT_FAILURE);
  }

  ll_string *out = NULL;
  while (true) {
    if (regexec(&regex, s, ARRAY_SIZE(pmatch), pmatch, 0)) {
      break;
    }
    len = pmatch[0].rm_eo - pmatch[0].rm_so;
    char *tokenbuf = strndup(s + pmatch[0].rm_so, (size_t)len);
    if (!tokenbuf) {
      perror("token");
      exit(EXIT_FAILURE);
    }
    out = append(out, tokenbuf);
    free(tokenbuf);
    s += pmatch[0].rm_eo;
  }

  regfree(&regex);
  return out;
}

ll_string *extract_terms(char *filename, string_set *stopwords) {
  regex_t regex;
  regmatch_t pmatch[1];
  regoff_t len;
  char linebuf[1024];

  if (regcomp(&regex, wordpat, REG_EXTENDED | REG_NEWLINE | REG_ICASE)) {
    fprintf(stderr, "could not compile the regular expression??\n");
    exit(EXIT_FAILURE);
  }

  FILE *infile = fopen(filename, "r");
  if (!infile) {
    perror(filename);
    regfree(&regex);
    return NULL;
  }
  ll_string *out = NULL;

  bool started = false;
  while (fgets(linebuf, 1024, infile)) {
    chomp(linebuf);
    lowercase(linebuf);
    if (!strcmp(linebuf, "")) {
      started = true;
    }
    if (!started) {
      continue;
    }

    char *s = linebuf;
    while (true) {
      if (regexec(&regex, s, ARRAY_SIZE(pmatch), pmatch, 0)) {
        break;
      }
      len = pmatch[0].rm_eo - pmatch[0].rm_so;
      char *tokenbuf = strndup(s + pmatch[0].rm_so, (size_t)len);
      if (!tokenbuf) {
        perror("token");
        exit(EXIT_FAILURE);
      }
      if (!contains(stopwords, tokenbuf)) {
        out = cons(tokenbuf, out);
      }
      free(tokenbuf);
      s += pmatch[0].rm_eo;
    }
  }

  regfree(&regex);
  fclose(infile);
  return out;
}

MetadataLookupTable *build_metadata_lookup_table(size_t num_buckets) {
  MetadataLookupTable *out = calloc(1, sizeof(MetadataLookupTable));
  out->num_buckets = num_buckets;

  out->buckets = calloc(num_buckets, sizeof(MetadataLookupList *));

  return out;
}

void free_post_metadata(PostMetadata *metadata) {
  if (!metadata)
    return;
  free(metadata->author);
  free(metadata->subject);
  free(metadata->message_id);
  free(metadata->newsgroups);
  free(metadata->filename);
  free(metadata);
}

void free_metadata_lookup_table(MetadataLookupTable *table) {
  if (!table) {
    return;
  }
  for (size_t i = 0; i < table->num_buckets; i++) {
    MetadataLookupList *ll = table->buckets[i];
    while (ll) {
      MetadataLookupList *next = ll->next;
      free_post_metadata(ll->metadata);
      free(ll);
      ll = next;
    }
  }
  free(table->buckets);
  free(table);
}

static char *dup_or_null(char *s) { return s ? strdup(s) : NULL; }

void store_metadata(MetadataLookupTable *table, PostMetadata *metadata) {
  unsigned long idx = djb_hash(metadata->message_id) % table->num_buckets;

  PostMetadata *copy = malloc(sizeof(PostMetadata));
  copy->author = dup_or_null(metadata->author);
  copy->subject = dup_or_null(metadata->subject);
  copy->message_id = strdup(metadata->message_id);
  copy->newsgroups = dup_or_null(metadata->newsgroups);
  copy->filename = strdup(metadata->filename);

  MetadataLookupList *node = malloc(sizeof(MetadataLookupList));
  node->message_id = copy->message_id;
  node->metadata = copy;
  node->next = table->buckets[idx];
  table->buckets[idx] = node;
}

PostMetadata *metadata_lookup(MetadataLookupTable *table, char *message_id) {
  unsigned long idx = djb_hash(message_id) % table->num_buckets;
  MetadataLookupList *node = table->buckets[idx];
  while (node) {
    if (!strcmp(node->message_id, message_id)) {
      return node->metadata;
    }
    node = node->next;
  }
  return NULL;
}

TermLookupTable *build_term_lookup_table(size_t num_buckets) {
  TermLookupTable *out = calloc(1, sizeof(TermLookupTable));
  out->num_buckets = num_buckets;
  out->buckets = calloc(num_buckets, sizeof(TermLookupList *));

  return out;
}

void free_term_lookup_table(TermLookupTable *table) {
  if (!table) {
    return;
  }
  for (size_t i = 0; i < table->num_buckets; i++) {
    TermLookupList *node = table->buckets[i];
    while (node) {
      TermLookupList *next = node->next;
      free(node->term);
      free_set(node->message_id_set);
      free(node);
      node = next;
    }
  }
  free(table->buckets);
  free(table);
}

string_set *term_lookup(TermLookupTable *table, char *term) {
  unsigned long idx = djb_hash(term) % table->num_buckets;
  TermLookupList *node = table->buckets[idx];
  while (node) {
    if (!strcmp(node->term, term)) {
      return node->message_id_set;
    }
    node = node->next;
  }
  return NULL;
}

string_set *union_lookup(TermLookupTable *table, ll_string *queries) {
  string_set *result = make_empty_set();
  while (queries) {
    string_set *term_set = term_lookup(table, queries->value);
    if (term_set) {
      string_set *new_result = set_union(result, term_set);
      free_set(result);
      result = new_result;
    }
    queries = queries->next;
  }
  return result;
}

string_set *intersection_lookup(TermLookupTable *table, ll_string *queries) {
  if (!queries) {
    return make_empty_set();
  }

  string_set *first_set = term_lookup(table, queries->value);
  if (!first_set) {
    return make_empty_set();
  }

  string_set *result = set_union(first_set, make_empty_set());
  queries = queries->next;

  while (queries) {
    string_set *term_set = term_lookup(table, queries->value);
    if (!term_set) {
      free_set(result);
      return make_empty_set();
    }
    string_set *new_result = set_intersection(result, term_set);
    free_set(result);
    result = new_result;
    queries = queries->next;
  }

  return result;
}

void add_message_id_to_term(TermLookupTable *table, char *term,
                            char *message_id) {
  unsigned long idx = djb_hash(term) % table->num_buckets;
  TermLookupList *node = table->buckets[idx];
  while (node) {
    if (!strcmp(node->term, term)) {
      node->message_id_set = add(node->message_id_set, message_id);
      return;
    }
    node = node->next;
  }
  TermLookupList *new_node = malloc(sizeof(TermLookupList));
  new_node->term = strdup(term);
  new_node->message_id_set = add(make_empty_set(), message_id);
  new_node->next = table->buckets[idx];
  table->buckets[idx] = new_node;
}
