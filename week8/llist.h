typedef struct 
{
    char name[30];
    char password[30];
    int status;
    int loginStatus;
} User;

typedef struct NodeLL
{
    User user;
    struct NodeLL *next;
} Node;

typedef struct 
{
    Node *root;
    Node *current;
} List;

// ----------------------
void insertAtHead(List *list, User u);
void insertAtfterCurrent(List *list, User u);
Node* createNode(User u);
void deleteHead(List *list);
void deletedAt(List *list, int position);
void deleteNode(List *list, char name[]);
Node* searchByName(List *list, char name[]);
void printList(List *list);
int isEmptyList(List *list);
int isSingletonList(List* list);
// -----------------------

List *createList()
{
    List *list = (List*) malloc(sizeof(List));
    list->root = NULL;
    list->current = list->root;
    
    return list;
}

void insertAtHead(List *list, User u)
{
    Node *node = createNode(u);
    node->next = list->root;
    list->root = node;
    list->current = list->root;
}

void insertAtfterCurrent(List *list, User u) 
{
    Node *node = createNode(u);

    if (isEmptyList(list))
    {
        list->root = node;
        list->current = list->root;
    } else
    {
        node->next = list->current->next;
        list->current->next = node;
        list->current = node;
    }
}

Node* createNode(User u)
{
    Node *node = (Node*) malloc(sizeof(Node));
    node->user = u;
    node->next = NULL;

    return node;
}

void deleteHead(List *list)
{
    if (!isEmptyList(list))
    {
        Node *tmp = list->root->next;
        free(list->root);
        if (list->current == list->root) 
        {
            list->current = tmp;
        }
        list->root = tmp;
    }
}

void deletedAt(List *list, int position)
{
    Node *tmp = list->root;
    Node *prev = NULL;

    if (position == 0)
    {
        deleteHead(list);
    } else if(position > 0)
    {
        int i;
        for (i = 0; i < position; i++)
        {
            if (tmp->next == NULL)
            {
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }

        if (i == position)
        {
            prev->next = tmp->next;
            free(tmp);
            list->current = list->root;
        }
    }
}

void deleteNode(List *list, char name[])
{
    int i = 0;
    Node *p;

    for (p = list->root; p != NULL; p = p->next)
    {
        if (strcmp(p->user.name, name) == 0)
        {
            deletedAt(list, i);
            break;
        }
        i++;
    }
    
}

Node* searchByName(List *list, char name[])
{
    for (Node *i = list->root; i != NULL; i = i->next)
    {
        if (strcmp(i->user.name, name) == 0)
        {
            return i;
        }
    }

    return NULL;
}

void updatedStatusAccount(List *list, char name[], int status)
{
    for (Node *i = list->root; i != NULL; i = i->next)
    {
        if (strcmp(i->user.name, name) == 0)
        {
            i->user.status = status;
        }
    }
}

void updatedLoginStatus(List *list, char name[], int status)
{
    for (Node *i = list->root; i != NULL; i = i->next)
    {
        if (strcmp(i->user.name, name) == 0)
        {
            i->user.loginStatus = status;
        }
    }
}

void updatedPasswordAccount(List *list, char name[], char password[])
{
    for (Node *i = list->root; i != NULL; i = i->next)
    {
        if (strcmp(i->user.name, name) == 0)
        {
            strcpy(i->user.password, password);
            break;
        }
    }
}

void printList(List *list)
{
    for (Node *i = list->root; i != NULL; i = i->next)
    {
        printf("Name: %-30s - Status: %d 0 LoginStatus: %d\n", i->user.name, i->user.status, i->user.loginStatus);
    }
    
}

int isEmptyList(List *list)
{
    if (list->root == NULL)
    {
        return 1;
    } else
    {
        return 0;
    }
}

int isSingletonList(List* list) {
    return (list->root != NULL && list->root->next == NULL);
}
