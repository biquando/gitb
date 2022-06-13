#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Checks directory entries for filename query. */
int dir_contains(const char *dirname, const char *query)
{
	DIR *dirp = opendir(dirname);
	if (!dirp)
		exit(errno);

	int found = 0;
	while (1) {
		struct dirent *ent = readdir(dirp);
		if (!ent)
			break;
		if (strcmp(ent->d_name, query) == 0) {
			found = 1;
			break;
		}
	}

	if (closedir(dirp) < 0)
		exit(errno);
	return found;
}

/* Goes to the parent directory of the git repository. If no .git directory is
 * found, the program exits. */
void goto_repo(void)
{
	char cwd[4096];
	while (1) {
		if (getcwd(cwd, sizeof cwd) < 0)
			exit(errno);

		if (access(cwd, R_OK) < 0)
			goto next_dir;

		if (dir_contains(cwd, ".git"))
			return;

		/* Are we at the root directory? */
		if (strlen(cwd) == 1 && cwd[0] == '/')
			exit(0);

	next_dir:
		if (chdir("..") < 0)
			exit(errno);
	}
}

/* Reads the current branch from .git/HEAD into buf and returns a string. */
char *read_head(char *buf, size_t len)
{
	int headfd = open(".git/HEAD", O_RDONLY);
	size_t nbytes;
	int i;

	if (headfd < 0)
		exit(errno);
	if (len == 0) {
		buf = NULL;
		goto cleanup;
	}

	nbytes = read(headfd, buf, len - 1);
	buf[nbytes] = '\0';

	if (nbytes > 16 && strncmp(buf, "ref: refs/heads/", 16) == 0) {
		/* Replace the \n with \0 */
		for (i = 16; buf[i] != '\n' && i < nbytes; i++)
			;
		buf[i] = '\0';
		buf += 16;
	} else if (nbytes >= 40) {
		buf[7] = '\0';
	} else {
		exit(1);
	}

cleanup:
	close(headfd);
	return buf;
}

void print_fmt(char *fmt, char *head)
{
	for (; *fmt != '\0'; fmt++) {
		if (*fmt != '%') {
			putchar(*fmt);
			continue;
		}

		switch (fmt[1]) {
		case '\0':
			return;
		case 'b':
			printf("%s", head);
			break;
		default:
			putchar(fmt[1]);
		}
		fmt++;
	}
}

int main(int argc, char *argv[])
{
	char buf[256];
	char *head;
	char *fmt;

	if (argc > 1)
		fmt = argv[1];
	else
		fmt = "%b";

	goto_repo();
	head = read_head(buf, sizeof buf);
	print_fmt(fmt, head);
}
