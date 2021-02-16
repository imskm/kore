#include "kore.h"
#include "dotenv.h"

static struct {
	FILE *fp;
	char *keys[DOTENV_MAX_SIZE];
	char *values[DOTENV_MAX_SIZE];
	int   size;
} dotenv;


static int  dotenv_load(void);
static void set_env_vars(void);

void dotenv_init(void)
{
	FILE *fp;

	dotenv.fp 			= NULL;
	dotenv.keys[0] 		= NULL;
	dotenv.values[0] 	= NULL;
	dotenv.size 		= 0;

	if ((fp = fopen(DOTENV_FILENAME, "r")) == NULL) {
		kore_log(LOG_NOTICE,
				"%s_init: envioronment file '%s' not found\n",
				DOTENV_LIBNAME,
				DOTENV_FILENAME);
		return;
	}
	dotenv.fp = fp;
	dotenv_load();
}

void dotenv_teardown(void)
{
	if (dotenv.fp)
		fclose(dotenv.fp);

	for (int i = 0; i < dotenv.size; ++i) {
		if (dotenv.keys[i]) {
			if (unsetenv(dotenv.keys[i]) == -1)
				kore_log(LOG_ERR,
						"%s_teardown: %s\n",
						DOTENV_LIBNAME,
						strerror(errno));
			/* No need to free dotenv.keys[i] and dotenv.values[i] because
			 * unsetenv() frees the memory itself. Freeing it again will give
			 * double free or corruption error. */
		}
		dotenv.keys[i] = dotenv.values[i] = NULL;
	}
	dotenv.size = 0;
	dotenv.fp   = NULL;
}

static int dotenv_load(void)
{
	int ret, iline, len;
	char line[DOTENV_MAXLINE];
	char *out[3];

	iline = 0;
	while (kore_read_line(dotenv.fp, line, sizeof(line))) {
		len = strlen(line);
		kore_text_trim(line, len);
		/* If line is empty then skip this line */
		if (line[0] == '\0')
			continue;
		ret = kore_split_string(line, "=", out, 3);
		/* Skip it if empty variable given */
		if (ret == 1)
			continue;
		if (ret != 2) {
			kore_log(LOG_ERR,
					"%s: parser error at line %d: expected format <key>=<value>",
					DOTENV_LIBNAME, (iline + 1));
			/* Skip the curren line and continue with the next line */
			continue;
		}
		kore_text_trim(out[0], strlen(out[0]));
		kore_text_trim(out[1], strlen(out[1]));
		dotenv.keys[iline] = kore_strdup(out[0]);
		dotenv.values[iline] = kore_strdup(out[1]);
		++iline;
		++dotenv.size;
	}
	/* No need to keep the file open any more */
	fclose(dotenv.fp);
	dotenv.fp = NULL;

	/* Set the environment variables */
	set_env_vars();

	return 0;
}

void set_env_vars(void)
{
	for (int i = 0; i < dotenv.size; ++i) {
		/* Set the loaded value in environment (1 = overrite) */
		if (setenv(dotenv.keys[i], dotenv.values[i], 1) == -1)
			kore_log(LOG_NOTICE, "%s: %s\n", DOTENV_LIBNAME,
					strerror(errno));
	}
}
