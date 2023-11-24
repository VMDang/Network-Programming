#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include "llist.h"

int getAllAccount(List *list, char *file){
	FILE *fptr;

	if((fptr=fopen(file,"r")) == NULL){
		printf("File %s is not found!\n", file);
		return 0;
	}
	else{
		User user;
		while(1){
			fscanf(fptr,"%s", user.name);
			fscanf(fptr,"%s", user.password);
			fscanf(fptr,"%d", &user.status);
			fscanf(fptr,"%d", &user.loginStatus);
			if(feof(fptr)) break;
			insertAtfterCurrent(list, user);
		}
	}
	fclose(fptr);
	return 1;
}

int storeAccount(List *list, char *file)
{
	FILE *fptr = fopen(file, "wb");
	if (isEmptyList(list))
	{
		return 0;
	}

	for (Node *i = list->root; i != NULL; i = i->next)
	{
		fprintf(fptr, "%s %s %d 0\n", i->user.name, i->user.password, i->user.status);
	}
	fclose(fptr);

	return 1;
}
