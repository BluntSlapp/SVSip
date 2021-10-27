/*
 * SvSIP, SIP/VoIP client for Nintendo DS
 * Copyright (C) 2007-2009  Samuel Vinson <samuelv0304@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define THIS_FILE	"utils.c"
#define PROG_NAME "svsip"

/*
 * Read command arguments from config file. Build argc and argv like arguments in command line.
 * Before to call this function, don't forget to define the first argument, i.e program name.
 * @param filename
 * @param app_argc
 * @param app_argv
 *
 * @return PJ_SUCCESS on success.
 */
int read_config_file(const char *filename, 
	int *app_argc, char ***app_argv)
{
    int i;
    FILE *fhnd;
    char line[200];
    int argc = 0;
    char **argv;
    enum { MAX_ARGS = 64 };

	/* Réinitialisation du système d'interprétation de la ligne de commande. */
	pj_optind = 0; // FIXME: à faire disparaitre quand configuration wifi sera fait dans le programme.
	
    /* Allocate MAX_ARGS+1 (argv needs to be terminated with NULL argument) */
	argv = calloc(MAX_ARGS+1, sizeof(char*));

	/*if (*app_argv != NULL)
	{
		argv[argc++] = *app_argv[0];
	}
	else*/
	{
		argv[0] = malloc((strlen(PROG_NAME) + 1) * sizeof(char));
		strcpy(argv[0], PROG_NAME);
		argc++;
	}

    /* Open config file. */
    fhnd = fopen(filename, "rt");
    if (!fhnd) 
	{
		PJ_LOG(1,(THIS_FILE, "Unable to open config file %s", filename));
		fflush(stdout);
		return -1;
    }

    /* Scan tokens in the file. */
    while (argc < MAX_ARGS && !feof(fhnd)) 
	{
		char  *token;
		char  *p;
		const char *whitespace = " \t\r\n";
		char  cDelimiter;
		int   len, token_len;
		
		if (fgets(line, sizeof(line), fhnd) == NULL) 
		{
			break;
		}
		
		// Trim ending newlines
		len = strlen(line);
		if (line[len-1]=='\n')
		    line[--len] = '\0';
		if (line[len-1]=='\r')
		    line[--len] = '\0';

		if (len==0) continue;

		for (p = line; *p != '\0' && argc < MAX_ARGS; p++) 
		{
		    // first, scan whitespaces
		    while (*p != '\0' && strchr(whitespace, *p) != NULL) 
			{
				p++;
			}

		    if (*p == '\0')		    // are we done yet?
			{
				break;
			}
		    
		    if (*p == '"' || *p == '\'') 
			{    // is token a quoted string
				cDelimiter = *p++;	    // save quote delimiter
				token = p;
				
				while (*p != '\0' && *p != cDelimiter) 
				{
					p++;
				}
				
				if (*p == '\0')		// found end of the line, but,
				{
				    cDelimiter = '\0';	// didn't find a matching quote
				}
		    }
			else 
			{			// token's not a quoted string
				token = p;
				
				while (*p != '\0' && strchr(whitespace, *p) == NULL) 
				{
					p++;
				}
				
				cDelimiter = *p;
		    }
		    
		    *p = '\0';
		    token_len = p-token;
		    
		    if (token_len > 0) 
			{
				if (*token == '#')
				{
				    break;  // ignore remainder of line
				}
				
				argv[argc] = malloc(token_len + 1);

				//argv[argc] = malloc(token_len + 1);
				pj_memcpy(argv[argc], token, token_len + 1);
				++argc;
		    }
		    
		    *p = cDelimiter;
		}
    }

    /* Copy arguments from command line */
    for (i=1; i<*app_argc && argc < MAX_ARGS; ++i)
	{
		argv[argc++] = (*app_argv)[i];
	}

    if (argc == MAX_ARGS && (i!=*app_argc || !feof(fhnd))) 
	{
		PJ_LOG(1,(THIS_FILE, 
			  "Too many arguments specified in cmd line/config file"));
		fflush(stdout);
		fclose(fhnd);
		return -2;
    }

    fclose(fhnd);

    /* Assign the new command line back to the original command line. */
    *app_argc = argc;
    *app_argv = argv;

    return PJ_SUCCESS;
}
