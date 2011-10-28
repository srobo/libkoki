/* Copyright 2011 Chris Kirkham, Robert Spanton

   This file is part of libkoki

   libkoki is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   libkoki is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with libkoki.  If not, see <http://www.gnu.org/licenses/>. */
#include <stdio.h>
#include <yaml.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "camera.h"

#include "yaml_config.h"



/**
 *
 */
static void add_param(char *key, char *value, koki_camera_params_t *params)
{

	char *endp;
	float f;


	if (strcmp(key, "frameWidth") == 0){              /* width */

		params->size.x = atoi(value);

	} else if (strcmp(key, "frameHeight") == 0){      /* height */

		params->size.y = atoi(value);

	} else if (strcmp(key, "focalLengthX") == 0){     /* focal length x */

		f = strtod(value, &endp);
		if (value != endp && *endp == '\0')
			params->focal_length.x = f;

	}  else if (strcmp(key, "focalLengthY") == 0){    /* focal length y */

		f = strtod(value, &endp);
		if (value != endp && *endp == '\0')
			params->focal_length.y = f;

	} else if (strcmp(key, "principalPointX") == 0){  /* principal point x */

		f = strtod(value, &endp);
		if (value != endp && *endp == '\0')
			params->principal_point.x = f;

	} else if (strcmp(key, "principalPointY") == 0){  /* principal point y */

		f = strtod(value, &endp);
		if (value != endp && *endp == '\0')
			params->principal_point.y = f;

	}

	/* if we get this far, just ignore it */

}



/**
 *
 */
bool koki_cam_read_params(const char *filename, koki_camera_params_t *params)
{

	yaml_parser_t        parser;
	yaml_token_t         token;
	bool                 ret = 1;
	unsigned char        *key, *value;

	assert(filename != NULL);
	assert(params != NULL);

	/* prepare */

	FILE *fh = fopen(filename, "r");

	if (fh == NULL){
		fprintf(stderr, "Failed to open file '%s'\n", filename);
		return 0;
	}

	if (!yaml_parser_initialize(&parser)){
		fprintf(stderr, "Failed to init YAML parser\n");
		return 0;
	}

	yaml_parser_set_input_file(&parser, fh);


	do { /* start reading tokens */

		yaml_parser_scan(&parser, &token);

		if (token.type == YAML_NO_TOKEN)
			goto error;

		/* recover key/values, ignore almost everything else */
		if (token.type == YAML_KEY_TOKEN){

			yaml_parser_scan(&parser, &token);
			if (token.type != YAML_SCALAR_TOKEN)
				goto error;

			key = token.data.scalar.value;

			yaml_parser_scan(&parser, &token);
			if(token.type != YAML_VALUE_TOKEN)
				goto error;

			yaml_parser_scan(&parser, &token);
			if (token.type != YAML_SCALAR_TOKEN)
				goto error;

			value = token.data.scalar.value;

			add_param((char*)key, (char*)value, params);

		}

		if (token.type != YAML_STREAM_END_TOKEN)
			yaml_token_delete(&token);

	} while (token.type != YAML_STREAM_END_TOKEN);
	yaml_token_delete(&token);

	goto clean;

error:
	ret = 0;
	fprintf(stderr, "Failed to parse config file. Choked at:\n");
	fprintf(stderr, "  %s : %d:%d\n",
		filename,
		(int)token.start_mark.line,
		(int)token.start_mark.column);

clean:
	yaml_parser_delete(&parser);
	fclose(fh);
	return ret;

}
