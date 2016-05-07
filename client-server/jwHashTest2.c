/*
 * test it in multi processes ....
 * vonzhou
*/

#include "global.h"

static jwHashTable * table = NULL;

int basic_test();

int main(int argc, char *argv[]){
	pid_t	childpid;
	// create
	table = create_hash(10);
	if(!table) {
		// fail
		return 1;
	}
	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */
	if ( (childpid = Fork()) == 0) {	/* child process */
		basic_test1();
		exit(0);
	}

	sleep(1000);

	if ( (childpid = Fork()) == 0) {	/* child process */
		basic_test2();
		exit(0);
	}
	sleep(1000);
	return 0;
}

int basic_test1()
{

	char * str1 = "string 1";
	char * str2 = "string 2";
	char * str3 = "string 3";
	char * str4 = "string 4";
	char * str5 = "string 5";
	add_str_by_int(table,0,str1);
	add_str_by_int(table,1,str2);
	add_str_by_int(table,2,str3);
	add_str_by_int(table,3,str4);
	add_str_by_int(table,4,str5);
	
	// char * sstr1; get_str_by_int(table,0,&sstr1);
	// char * sstr2; get_str_by_int(table,1,&sstr2);
	// char * sstr3; get_str_by_int(table,2,&sstr3);
	// char * sstr4; get_str_by_int(table,3,&sstr4);
	// char * sstr5; get_str_by_int(table,4,&sstr5);
	// printf("got strings: \n1->%s\n2->%s\n3->%s\n4->%s\n5->%s\n",sstr1,sstr2,sstr3,sstr4,sstr5);
	
	// del_by_int(table, 0);
	// del_by_int(table, 1);
	// del_by_int(table, 2);
	// del_by_int(table, 3);
	// del_by_int(table, 4);
	
	// // Test hashing by string
	// char * strv1 = "Jonathan";
	// char * strv2 = "Zevi";
	// char * strv3 = "Jude";
	// char * strv4 = "Voldemort";
	
	// add_str_by_str(table,"oldest",strv1);
	// add_str_by_str(table,"2ndoldest",strv2);
	// add_str_by_str(table,"3rdoldest",strv3);
	// add_str_by_str(table,"4tholdest",strv4);
	
	// char * sstrv1; get_str_by_str(table,"oldest",&sstrv1);
	// char * sstrv2; get_str_by_str(table,"2ndoldest",&sstrv2);
	// char * sstrv3; get_str_by_str(table,"3rdoldest",&sstrv3);
	// char * sstrv4; get_str_by_str(table,"4tholdest",&sstrv4);
	// printf("got strings:\noldest->%s \n2ndoldest->%s \n3rdoldest->%s \n4tholdest->%s\n",
	// 	sstrv1,sstrv2,sstrv3,sstrv4);
	return 0;
}	


int basic_test2()
{
	char * sstr1; get_str_by_int(table,0,&sstr1);
	char * sstr2; get_str_by_int(table,1,&sstr2);
	char * sstr3; get_str_by_int(table,2,&sstr3);
	char * sstr4; get_str_by_int(table,3,&sstr4);
	char * sstr5; get_str_by_int(table,4,&sstr5);
	printf("got strings: \n1->%s\n2->%s\n3->%s\n4->%s\n5->%s\n",sstr1,sstr2,sstr3,sstr4,sstr5);
	
	return 0;
}