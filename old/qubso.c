
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#define ID_VAL	0
#define ID_AND	1
#define ID_OR	2
#define ID_NOT	3

typedef struct _string_t {

	int identifier;

	size_t len;
	char *str;

	struct string_t *left;
	struct string_t *right;
} string_t;

typedef struct _bdd_node_t {
	char var;
} bdd_node_t;

static void input(string_t *t, size_t depth, const char *nodename, size_t arg);
static void dump(const string_t *t);

int main(int argc, char *argv[]) {

	string_t root;

	input(&root, 0, "ROOT", 0);

	printf("\n");
	dump(&root);
	printf("\n");

	exit(0);
}

void input(string_t *t, size_t depth, const char *nodename, size_t arg) {

	size_t slen;
	char buf[256];

	do {
		printf("(depth: %02d; %5s arg: %d) ", depth, nodename, arg);
		fgets(buf, sizeof(buf), stdin);
	} while (buf[0]=='#');

	slen=strlen(buf);
	while (slen>0 && buf[slen-1]<=' ') buf[(slen--)-1]=0;

	memset(t, 0, sizeof(string_t));
	if (strcmp(buf, "AND")==0) {
		t->identifier=ID_AND;
		t->left=malloc(sizeof(string_t));
		t->right=malloc(sizeof(string_t));
		input((string_t *)t->left, depth+1, buf, 1);
		input((string_t *)t->right, depth+1, buf, 2);
	} else if (strcmp(buf, "OR")==0) {
		t->identifier=ID_OR;
		t->left=malloc(sizeof(string_t));
		t->right=malloc(sizeof(string_t));
		input((string_t *)t->left, depth+1, buf, 1);
		input((string_t *)t->right, depth+1, buf, 2);
	} else if (strcmp(buf, "NOT")==0) {
		t->identifier=ID_NOT;
		t->left=t->right=malloc(sizeof(string_t));
		input((string_t *)t->left, depth+1, buf, 1);
	} else {
		t->identifier=ID_VAL;
		t->str=malloc(slen+1);
		memcpy(t->str, buf, slen+1);
		t->len=slen;
	}
}

static void dump(const string_t *t) {

	switch (t->identifier) {
		case ID_AND:
		printf("(");
		dump((string_t *)t->left);
		dump((string_t *)t->right);
		printf("F)");
		break;
		case ID_OR:
		printf("(");
		dump((string_t *)t->left);
		printf("T");
		dump((string_t *)t->right);
		printf(")");
		break;
		case ID_NOT:
		printf("(");
		dump((string_t *)t->left);
		printf("FT)");
		break;
		default:
		printf(t->str);
		break;
	}
}
