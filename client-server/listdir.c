#include "global.h"

void listdir(const char *name, int level){
	DIR *dir;
  struct dirent *entry;

  if (!(dir = opendir(name)))
      return;
  if (!(entry = readdir(dir)))
      return;

  do {
      if (entry->d_type == DT_DIR) {
				char path[MAXLINE];
        int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
				path[len] = 0;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        // printf("%*s[%s]\n", level*2, "", entry->d_name);
				// recursive
      	listdir(path, level + 1);
      }
      else{
				char path[MAXLINE];
				int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
        path[len] = 0;
				printf("file path: %s\n", path);
			}

		} while (entry = readdir(dir));

    closedir(dir);
}

void uploadDir(const char *name){
	DIR *dir;
  struct dirent *entry;

  if (!(dir = opendir(name)))
      return;
  if (!(entry = readdir(dir)))
      return;

  do {
      if (entry->d_type == DT_DIR) {
				char path[MAXLINE];
        int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
				path[len] = 0;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
				// recursive
      	uploadDir(path);
      }
      else{
				char path[MAXLINE];
				int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
				path[len] = 0;
				printf("%s\n", path);
				// upload(path);
			}

		} while (entry = readdir(dir));

    closedir(dir);
}

int main(int argc, char **argv){
  char *dir = argv[1];
  uploadDir(dir);
}
