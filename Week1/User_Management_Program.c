#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include "llist.h"

char fileName[] = "account.txt";

void showMenu(){
	printf("\nUSER MANAGEMENT PROGRAMM\n");
	printf("---------------------------------------------\n");
	printf("1. Register\n");
	printf("2. Sign in\n");
	printf("3. Search\n");
	printf("4. Sign out\n");
	printf("Your choice (1-4, other to quit): ");
}

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
		fprintf(fptr, "%s %s %d\n", i->user.name, i->user.password, i->user.status);
	}
	fclose(fptr);

	return 1;
}

void registerAccount(List *list, User u)
{
	insertAtfterCurrent(list, u);

	if (storeAccount(list, fileName))
	{
		printf("Successful registration\n");
	}
}

int validateUniqueName(List *list, char name[])
{
	if (searchByName(list, name) == NULL)
	{
		return 1;
	} else {
		return 0;
	}
	
}

void login(List *list, List *loginlist)
{
	char name[30];
	char password[30];
	int attempt=1;
	Node *tmp;

	printf("Username: "); scanf("%s", name);
	tmp = searchByName(list, name);
	if (tmp == NULL)
	{
		printf("Cannot find account\n");
	} else if (tmp->user.status == 0)
	{
		printf("Account is blocked\n");
	} else if (searchByName(loginlist, name) != NULL)
	{
		printf("Account is logged in\n");

	} else
	{
		do
		{
			printf("Passowrd: "); scanf("%s", password);
			if (strcmp(password, tmp->user.password)!=0)
			{
				printf("Password is incorrect\n");
				attempt++;
			} else
			{
				insertAtfterCurrent(loginlist, tmp->user);
				printf("Hello %s\n", tmp->user.name);
				attempt = 0;
			}
			

		} while (strcmp(password, tmp->user.password)!=0 && attempt<=3);

		if (attempt > 3)
		{
			updatedStatusAccount(list, tmp->user.name, 0);
			storeAccount(list, fileName);
			printf("Password is incorrect. Account is blocked\n");
		}
		
	}
	
}

void logout(List *list, List *listLogin)
{
	char name[30];
	Node *tmp, *tmpLogin;

	printf("Username: "); scanf("%s", name);
	tmp = searchByName(list, name);
	tmpLogin = searchByName(listLogin, name);

	if (tmp == NULL)
	{
		printf("Cannot find account\n");
	} else if (tmpLogin == NULL)
	{
		printf("Account is not sign in\n");
	} else
	{
		printf("Goodbye %s\n", tmpLogin->user.name);
		deleteNode(listLogin, tmp->user.name);

	}
}

void serachUser(List *list)
{
	char name[30];
	Node *tmp;

	printf("Username: "); scanf("%s", name);
	tmp = searchByName(list, name);
	if (tmp == NULL)
	{
		printf("Cannot find account\n");
	} else if (tmp->user.status == 0)
	{
		printf("Account is blocked\n");
	} else
	{
		printf("Account is active\n");
	}
}

int main()
{
    List *list = createList();
    List *loginList = createList();
	int feature = 0;

	if (getAllAccount(list, fileName))
	{
		do
		{
			showMenu();
			scanf("%d", &feature);

			User user;
			switch (feature)
			{
			case 1:
				printf("Username: "); scanf("%s", user.name);
				if (validateUniqueName(list, user.name))
				{
					printf("Passowrd: "); scanf("%s", user.password);
					user.status = 1;
					registerAccount(list, user);
				} else {
					printf("Account exitsted\n");
				}
				break;
			
			case 2:
				login(list, loginList);
				break;
			case 3:
				if (isEmptyList(loginList))
				{
					printf("No accounts logged yet\n");
				} else
				{
					serachUser(list);
				}
				break;
			case 4:
				if (isEmptyList(loginList))
				{
					printf("No accounts logged yet\n");
				} else 
				{
					logout(list, loginList);
				}
				break;
			default:
				break;
			}
		} while ((feature >= 1) && (feature <= 4));
		
	}
	
	return 0;
}
