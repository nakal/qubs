
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "hash.h"
#include "memory.h"
#include "bdd.h"

#define ID_VAL		0
#define ID_AND		1
#define ID_OR		2
#define ID_NOT		3
#define ID_EXISTS	4
#define ID_FORALL	5
#define ID_IMPLIES	6
#define ID_EQUIVALENT	7

typedef struct _string_t {

	int identifier;

	size_t len;
	char *str;

	struct string_t *left;
	struct string_t *right;
} string_t;

static char const_zero[]="0";
static char const_one[]="1";
static char const_empty[]="e";

#define ALLOW_ALL 0
#define ALLOW_VAL 1
static int input(string_t *t, size_t depth, const char *nodename, size_t arg,
	int restriction);
static int build_bdd(const string_t *t, bdd_t *bdd, bdd_node_t *cur,
	bdd_node_t *zerob, bdd_node_t *oneb, hash_t *fixedval);
static void destroy_input(string_t *s);
static void var_element_free(void *elem);

#define EVALMODE_SAT		1
#define EVALMODE_TRUTH		2
#define EVALMODE_CONTRADICTION	3
int evalmode=0;
bdd_opfunc_t opfunc=NULL;
bdd_opfunc_t opfuncs[4]= { NULL, NULL, NULL, NULL };

int interactive=0;
int verbose=0;
int output=0;

hash_t *var_exists;
list_t *var_list;
size_t line_number;

int main(int argc, char *argv[]) {

	string_t root;
	bdd_t *bdd;
	hash_t *v;
	int help=0;
	int c=0, result;
	size_t size;

	opfuncs[1]=bdd_sat;
	opfuncs[2]=bdd_true;
	opfuncs[3]=bdd_false;

	do {

		if ((c=getopt(argc, argv, "stcvio"))>0) {
			switch (c) {
			case 's':
			if (evalmode==0) evalmode=EVALMODE_SAT;
			else help=1;
			break;
			case 't':
			if (evalmode==0) evalmode=EVALMODE_TRUTH;
			else help=1;
			break;
			case 'c':
			if (evalmode==0) evalmode=EVALMODE_CONTRADICTION;
			else help=1;
			break;
			case 'i':
			interactive=1;
			break;
			case 'v':
			verbose++;
			break;
			case 'o':
			output=1;
			break;
			default:
			help=1;
			break;
			}
		}
	} while (c>0);

	if (help) {
		printf("%s [ -s | -t | -c ]\n", argv[0]);
		printf("\tEvaluates first order term in postfix notation.\n");
		printf("\n\tParameters:\n");
		printf("\t\t-s satisfiability (default)\n");
		printf("\t\t-t tautology\n");
		printf("\t\t-c contradiction\n");
		printf("\t\t-i interactive\n");
		printf("\t\t-v verbose (can be specified multiple times)\n");
		printf("\t\t\t1 = query written to terminal\n");
		printf("\t\t\t2 = plus progress output\n");
		printf("\t\t\t3 = plus optimized BDD output\n");
		printf("\t\t\t4 = plus unoptimized BDD output\n");
		printf("\n");
		exit(-1);
	}

	if (evalmode==0) evalmode=EVALMODE_SAT;
	opfunc=opfuncs[evalmode];

	mem_init;

	if (verbose>1) {
		printf("Reading input... "); fflush(stdout);
	}

	var_exists = hash_create(VAR_HASH_SIZE);
	var_list = list_create();

	line_number=0;
	if (input(&root, 0, "START", 0, ALLOW_ALL)<0) {
		fprintf(stderr, "Syntax error in line %lu.\n", line_number);
		exit(-1);
	}

	hash_destroy(var_exists);

	if (verbose>1) {
		printf("done.\nBuilding BDD... "); fflush(stdout);
	}

	bdd=bdd_create();
	bdd->start=bdd_init_node(bdd, NULL, &no, &yes);

	v=hash_create(1024);
	build_bdd(&root, bdd, bdd->start, &no, &yes, v);
	hash_destroy(v);
	destroy_input(&root);

	if (verbose>1) {
		printf("done.\n");
		if (verbose>3) dump_bdd(bdd);
		printf("Compacting BDD... "); fflush(stdout);
	}

	bdd_compact(&bdd, verbose>1);

	if (verbose>1) {
		printf("done.\nCalculating result... "); fflush(stdout);
	}
	result=opfunc(bdd->start);

	if (verbose>0) {
		if (verbose>1) printf("done.\n");
		printf("%s: %s\n", (evalmode==1) ? "satisfiable" :
		(evalmode==2) ? "tautology" : "contradiction",
		result>0 ? "yes" : "no");
		if (verbose>2) dump_bdd(bdd);
	}

	if (output) {
		if (((evalmode==2) && result>0) || bdd_true(bdd->start)) {
			printf("Everything matches.\n");
		} else if (((evalmode==3) && result>0) ||
			bdd_false(bdd->start)) {
			printf("No matches.\n");
		} else {
			bdd_output_candidates(bdd, var_list);
		}
	}

	if (verbose>1) {
		printf("Freeing memory... "); fflush(stdout);
	}
	list_destroywithelements(var_list, var_element_free);
	size=bdd_destroy(bdd);
	if (verbose>1) {
		printf("done.\n");
		printf("Size of optimized BDD: %lu\n", size);
	}

	mem_done;
	exit(result);
}

int input(string_t *t, size_t depth, const char *nodename, size_t arg,
	int restriction) {

	size_t slen;
	char buf[256];
#define check_input(l,r,b,p,a) if (input(l,r,b,p,a)<0) return -1

	do {
		if (interactive)
			printf("(depth: %02zu; %5s arg: %zu) ",
			depth, nodename, arg);
		if (fgets(buf, sizeof(buf), stdin)==NULL) {
			fprintf(stderr, "Error in depth: %02zu in %s\n", depth, nodename);
			return -1;
		}
		line_number++;

		slen=strlen(buf);
		while (slen>0 && buf[slen-1]<=' ') buf[(slen--)-1]=0;
		if (buf[0]=='#') slen=0;
		if (slen<=0) buf[0]=0;
	} while (slen<=0);

	memset(t, 0, sizeof(string_t));
	if (strcmp(buf, "AND")==0) {
		t->identifier=ID_AND;
		t->left=mem_malloc(sizeof(string_t));
		t->right=mem_malloc(sizeof(string_t));
		check_input((string_t *)t->left, depth+1, buf, 1, ALLOW_ALL);
		check_input((string_t *)t->right, depth+1, buf, 2, ALLOW_ALL);
	} else if (strcmp(buf, "OR")==0) {
		t->identifier=ID_OR;
		t->left=mem_malloc(sizeof(string_t));
		t->right=mem_malloc(sizeof(string_t));
		check_input((string_t *)t->left, depth+1, buf, 1, ALLOW_ALL);
		check_input((string_t *)t->right, depth+1, buf, 2, ALLOW_ALL);
	} else if (strcmp(buf, "NOT")==0) {
		t->identifier=ID_NOT;
		t->left=t->right=mem_malloc(sizeof(string_t));
		check_input((string_t *)t->left, depth+1, buf, 1, ALLOW_ALL);
	} else if (strcmp(buf, "EXISTS")==0) {
		t->identifier=ID_EXISTS;
		t->left=mem_malloc(sizeof(string_t));
		t->right=mem_malloc(sizeof(string_t));
		check_input((string_t *)t->left, depth+1, buf, 1, ALLOW_VAL);
		check_input((string_t *)t->right, depth+1, buf, 2, ALLOW_ALL);
	} else if (strcmp(buf, "FORALL")==0) {
		t->identifier=ID_FORALL;
		t->left=mem_malloc(sizeof(string_t));
		t->right=mem_malloc(sizeof(string_t));
		check_input((string_t *)t->left, depth+1, buf, 1, ALLOW_VAL);
		check_input((string_t *)t->right, depth+1, buf, 2, ALLOW_ALL);
	} else if (strcmp(buf, "IMPLIES")==0 || strcmp(buf, "IF")==0) {
		t->identifier=ID_IMPLIES;
		t->left=mem_malloc(sizeof(string_t));
		t->right=mem_malloc(sizeof(string_t));
		check_input((string_t *)t->left, depth+1, buf, 1, ALLOW_ALL);
		check_input((string_t *)t->right, depth+1, buf, 2, ALLOW_ALL);
	} else if (strcmp(buf, "EQUIVALENT")==0 || strcmp(buf, "IFF")==0) {
		t->identifier=ID_EQUIVALENT;
		t->left=mem_malloc(sizeof(string_t));
		t->right=mem_malloc(sizeof(string_t));
		check_input((string_t *)t->left, depth+1, buf, 1, ALLOW_ALL);
		check_input((string_t *)t->right, depth+1, buf, 2, ALLOW_ALL);
	} else {
		t->identifier=ID_VAL;
		t->str=mem_malloc(slen+1);
		memcpy(t->str, buf, slen+1);
		t->len=slen;

		if (!hash_get(var_exists, t->str)) {
			char *var;

			hash_put(var_exists, t->str, "", 1);
			var=mem_malloc(slen+1);
			memcpy(var, buf, slen+1);
			list_append(var_list, var);
		}
	}

	if (restriction==ALLOW_VAL && t->identifier!=ID_VAL) {
		fprintf(stderr, "Syntax error: value expected.\n");
		abort();
	}

	return 0;
}

static void destroy_input(string_t *s) {

	if (s->left!=NULL) {
		switch (s->identifier) {
		case ID_NOT:
			destroy_input((string_t *)s->left);
			mem_free(s->left);
			break;
		default:
			destroy_input((string_t *)s->left);
			destroy_input((string_t *)s->right);
			mem_free(s->left);
			mem_free(s->right);
			break;
		}
	} else mem_free(s->str);
}

static int build_bdd(const string_t *t, bdd_t *bdd, bdd_node_t *cur,
	bdd_node_t *zerob, bdd_node_t *oneb, hash_t *fixedval) {

	bdd_node_t *n1, *n2;
	char *hv;
	string_t *s1;

	if (t!=NULL) {

		switch (t->identifier) {
		case ID_VAL:
			/* printf("cur: %s\n", t->str); */
			hv=hash_get(fixedval, t->str);
			/*printf("hv: %s\n", hv);*/
			strlcpy(cur->var, t->str, sizeof(cur->var));
			if (hv==NULL || strcmp(hv, const_empty)==0) {
				cur->iszero=zerob;
				cur->isone=oneb;
			} else {
				if (strcmp(hv, const_one)==0) {
					cur->iszero=oneb;
					cur->isone=oneb;
				} else {
					cur->iszero=zerob;
					cur->isone=zerob;
				}
			}
			break;
		case ID_NOT:
			if (build_bdd((string_t *)t->left, bdd, cur,
				oneb, zerob, fixedval)<0) return -1;
			break;
		case ID_OR:
			n1=bdd_init_node(bdd, cur, zerob, oneb);
			if (n1==NULL) return -1;
			cur->iszero=n1;
			if (build_bdd((string_t *)t->left, bdd, n1,
				zerob, oneb, fixedval)<0) return -1;
			if (build_bdd((string_t *)t->right, bdd, cur,
				n1, oneb, fixedval)<0) return -1;
			break;
		case ID_AND:
			n1=bdd_init_node(bdd, cur, zerob, oneb);
			if (n1==NULL) return -1;
			cur->isone=n1;
			if (build_bdd((string_t *)t->right, bdd, n1,
				zerob, oneb, fixedval)<0) return -1;
			if (build_bdd((string_t *)t->left, bdd, cur,
				zerob, n1, fixedval)<0) return -1;
			break;
		case ID_EXISTS:

			s1=(string_t *)t->left;
			if (s1->identifier!=ID_VAL) {
				fprintf(stderr, "Variable expected for EXISTS.\n");
				abort();
			}

			hash_put(fixedval, s1->str, const_zero, 2);

			n1=bdd_init_node(bdd, cur, zerob, oneb);
			if (n1==NULL) return -1;
			cur->iszero=n1;
			/* XXX use bdd_merge */
			if (build_bdd((string_t *)t->right, bdd, n1,
				zerob, oneb, fixedval)<0) return -1;
			hash_put(fixedval, s1->str, const_one, 2);
			/* XXX use bdd_merge */
			if (build_bdd((string_t *)t->right, bdd, cur,
				n1, oneb, fixedval)<0) return -1;

			hash_put(fixedval, s1->str, const_empty, 2);
			break;
		case ID_FORALL:

			s1=(string_t *)t->left;
			if (s1->identifier!=ID_VAL) {
				fprintf(stderr, "Variable expected for FORALL.\n");
				abort();
			}

			hash_put(fixedval, s1->str, const_one, 2);
			n1=bdd_init_node(bdd, cur, zerob, oneb);
			if (n1==NULL) return -1;
			cur->isone=n1;
			/* XXX use bdd_merge */
			if (build_bdd((string_t *)t->right, bdd, n1,
				zerob, oneb, fixedval)<0) return -1;
			hash_put(fixedval, s1->str, const_zero, 2);
			/* XXX use bdd_merge */
			if (build_bdd((string_t *)t->right, bdd, cur,
				zerob, n1, fixedval)<0) return -1;

			hash_put(fixedval, s1->str, const_empty, 2);
			break;
		case ID_IMPLIES:
			n1=bdd_init_node(bdd, cur, zerob, oneb);
			if (n1==NULL) return -1;
			cur->iszero=n1;
			if (build_bdd((string_t *)t->left, bdd, n1,
				oneb, zerob, fixedval)<0) return -1;
			if (build_bdd((string_t *)t->right, bdd, cur,
				n1, oneb, fixedval)<0) return -1;
			break;
		case ID_EQUIVALENT:
			n1=bdd_init_node(bdd, cur, zerob, oneb);
			if (n1==NULL) return -1;
			n2=bdd_init_node(bdd, cur, zerob, oneb);
			if (n2==NULL) return -1;
			cur->iszero=n1;
			cur->isone=n2;
			if (build_bdd((string_t *)t->right, bdd, n1,
				oneb, zerob, fixedval)<0) return -1;
			if (build_bdd((string_t *)t->right, bdd, n2,
				zerob, oneb, fixedval)<0) return -1;
			if (build_bdd((string_t *)t->left, bdd, cur,
				n1, n2, fixedval)<0) return -1;
			break;
		default:
			return -1;
		}
	}

	return 0;
}

static void var_element_free(void *elem) {
	mem_free(elem);
}

