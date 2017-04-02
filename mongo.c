#include <mongoc.h>
#include <stdio.h>

int main() {}

static void print_all_documents (mongoc_collection_t *collection)
{
   mongoc_cursor_t *cursor;
   bson_error_t error;
   const bson_t *doc;
   char *str;
   bson_t *query;

   query = BCON_NEW ("$query", "{", "foo", BCON_INT32 (1), "}",
                     "$orderby", "{", "bar", BCON_INT32 (-1), "}");
   cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

   while (mongoc_cursor_next (cursor, &doc)) {
      str = bson_as_json (doc, NULL);
      printf ("%s\n", str);
      bson_free (str);
   }

   if (mongoc_cursor_error (cursor, &error)) {
      fprintf (stderr, "An error occurred: %s\n", error.message);
   }

   mongoc_cursor_destroy (cursor);
   bson_destroy (query);
}
