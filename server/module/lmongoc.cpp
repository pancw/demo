#include "lmongoc.h"

namespace lmongodb {
	bool mongo_init(int port)
	{
		printf("--------mongo_init\n");

		mongoc_client_t      *client;
		mongoc_database_t    *database;
		mongoc_collection_t  *collection;
		bson_t               *command,
							 reply,
							*insert;
		bson_error_t          error;
		char                 *str;
		bool                  retval;

		mongoc_init();

		client = mongoc_client_new ("mongodb://localhost:1238");	

		database = mongoc_client_get_database (client, "pancw");	
		collection = mongoc_client_get_collection (client, "pancw", "user");

		command = BCON_NEW ("ping", BCON_INT32 (1));

		retval = mongoc_client_command_simple (client, "admin", command, NULL, &reply, &error);
		if (!retval) {
			fprintf (stderr, "%s\n", error.message);
			return EXIT_FAILURE;
		}

		str = bson_as_json (&reply, NULL);
		printf ("%s\n", str);

		insert = BCON_NEW ("hello", BCON_UTF8 ("world"));

		if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, insert, NULL, &error)) {
			fprintf (stderr, "%s\n", error.message);
		}

		bson_destroy (insert);
		bson_destroy (&reply);
		bson_destroy (command);
		bson_free (str);

		return true;
	}
}
