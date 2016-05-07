#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main (int argc, char **argv)
{
  int aflag = 0;
  int bflag = 0;
  char *cvalue = NULL;
  int index;
  int c;
  char *serverip;
  char *directory;

  printf("argc = %d\n", argc);

  if (argc != 7)
    printf("Usage: client -i server_ip_addr -d backup_dir -m [base/bloom/sdna]");

  //handle the options 
  int opterr = 0;
  while ((c = getopt (argc, argv, "i:d:m:")) != -1){
    switch (c)
      {
      case 'i':
        serverip = optarg;
        break;
      case 'd':
        directory = optarg;
        break;
      case 'm':
        cvalue = optarg;
        break;
      case '?':
          fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
        return 1;
      default:
        abort ();
      } 
  }

  printf("%s,%s,%s\n", serverip, directory, cvalue);
    

  return 0;
}