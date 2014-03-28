
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "bdd.h"
#include "hash.h"

bdd_node_t yes = { "1", NULL, NULL, NULL };
bdd_node_t no = { "0", NULL, NULL, NULL };

static int bdd_compact_r(bdd_node_t *cur, hash_t *h,
	bdd_t *outbdd, bdd_node_t **out);
static void dump_bdd_r(bdd_node_t *n, unsigned int level);
static void bdd_output_candidate(bdd_node_t *n, list_t *vars);

#undef VERBOSE

bdd_t *bdd_create(void) {

	bdd_t *b;

	b=mem_calloc(1, sizeof(bdd_t));
	if (b==NULL) return NULL;

	b->nodelist=list_create();
	if (b->nodelist==NULL) {
		mem_free(b);
		return NULL;
	}

	return b;
}

/*
	bdd_init_node_ext won't be usually used, because it handles
	a special case where bdd_compact_r does not add nodes to the
	list (it does, but lazy!).

	Please use bdd_init_node macro in bdd.h that does not have
	the last parameter (it's always set to 1).
*/
bdd_node_t *bdd_init_node_ext(bdd_t *bdd, bdd_node_t *parent,
	bdd_node_t *zerob, bdd_node_t *oneb, int append_to_list) {

	bdd_node_t *n;

	n=mem_calloc(1, sizeof(bdd_node_t));
	if (n==NULL) return NULL;

	n->parent=parent;
	n->iszero=zerob;
	n->isone=oneb;

	if (append_to_list) {
		if (list_append(bdd->nodelist, n)<0) {
			mem_free(n);
			return NULL;
		}
	}

	return n;
}

void dump_bdd(bdd_t *bdd) {
	dump_bdd_r(bdd->start, 0);
}

static void dump_bdd_r(bdd_node_t *n, unsigned int level) {

	if (n->iszero!=NULL) {
		unsigned int i;

#ifdef VERBOSE
		printf("(%s[%p] ? ", n->var, n);
		dump_bdd_r(n->isone, level+1); printf("[%p] : ", n->isone);
		dump_bdd_r(n->iszero, level+1); printf("[%p])", n->iszero);
#else
		printf("(%s ? ", n->var);
		dump_bdd_r(n->isone, level+1); printf(" : ");
		dump_bdd_r(n->iszero, level+1); printf(")");
#endif
	} else printf("%s", n->var);

	if (level==0) printf("\n");
}

int bdd_sat(bdd_node_t *bdd) {

	if (bdd==NULL) return 0;

	if (strcmp(bdd->var, yes.var)==0) return 1;
	if (bdd_sat(bdd->iszero)!=0) return 1;
	if (bdd_sat(bdd->isone)!=0) return 1;

	return 0;
}

int bdd_true(bdd_node_t *bdd) {

	if (bdd==NULL) return 1;

	if (strcmp(bdd->var, no.var)==0) return 0;
	if (bdd_true(bdd->iszero)==0) return 0;
	if (bdd_true(bdd->isone)==0) return 0;

	return 1;
}

int bdd_false(bdd_node_t *bdd) {

	if (bdd==NULL) return 0;

	return bdd_sat(bdd) ? 0 : 1;
}

int bdd_compact(bdd_t **bdd, int stats) {

	int result;
	bdd_t *out;
	hash_t *h;
	size_t size;

	h=hash_create(VAR_HASH_SIZE);
	if (h==NULL) return -1;

	out=bdd_create();
	if (out==NULL) {
		hash_destroy(h);
		return -1;
	}

	out->start=bdd_init_node(out, NULL, NULL, NULL);
	if (out->start==NULL) {
		bdd_destroy(out);
		hash_destroy(h);
		return -1;
	}

	if (hash_put(h, "0", "0", 2)<0 || hash_put(h, "1", "1", 2)<0) {
		bdd_destroy(out);
		hash_destroy(h);
		return -1;
	}

	result=bdd_compact_r((*bdd)->start, h, out, &out->start);
	hash_destroy(h);

	if (stats) {
		printf("Freeing unoptimized BDD... "); fflush(stdout);
	}
	size=bdd_destroy(*bdd);
	*bdd=out;

	if (stats) {
		printf("done.\nSize of unoptimized BDD: %lu\n", size);
	}

	return result;
}

void bdd_merge(bdd_t *bdd, bdd_node_t *n, bdd_t *subbdd) {

	bdd_node_t *n2;

	n2=list_removefirst(subbdd->nodelist);
	list_list_append(bdd->nodelist, subbdd->nodelist);
	n->iszero=n2->iszero;
	n->isone=n2->isone;
	memcpy(n->var, n2->var, sizeof(n->var));

	mem_free(n2);
}

static int bdd_compact_r(bdd_node_t *cur, hash_t *h,
	bdd_t *outbdd, bdd_node_t **out) {
	
	if (cur->iszero!=NULL && cur->isone!=NULL) {
		char c0='0', c1='1', ce='r', *check;

		check=hash_get(h, cur->var);
		if (check==NULL || *check==ce) {
			/*printf("%s not yet set.\n", cur->var);
			hash_dump(h);*/
			hash_put(h, cur->var, &c1, sizeof(c1));
			strlcpy((*out)->var, cur->var, sizeof((*out)->var));
			/* do not append to nodelist yet! */
			(*out)->iszero=
				bdd_init_node_ext(outbdd, *out, NULL, NULL, 0);
			(*out)->isone=
				bdd_init_node_ext(outbdd, *out, NULL, NULL, 0);
			bdd_compact_r(cur->isone, h, outbdd, &(*out)->isone);
			hash_put(h, cur->var, &c0, sizeof(c0));
			bdd_compact_r(cur->iszero, h, outbdd, &(*out)->iszero);
			hash_put(h, cur->var, &ce, sizeof(ce));

			if (
				(strcmp((*out)->iszero->var, no.var)==0 ||
				strcmp((*out)->iszero->var, yes.var)==0) &&
				strcmp((*out)->iszero->var, (*out)->isone->var)==0) {
				/*
					We check node for equality of both
					descendants. We can remove them in this
					case and push the value 1 position up. 
				*/

				(*out)->var[0] = (*out)->iszero->var[0];
				(*out)->var[1] = 0;

				mem_free((*out)->isone);
				mem_free((*out)->iszero);

				(*out)->iszero = NULL;
				(*out)->isone = NULL;
				/*printf("Contracted 2x %s.\n", (*out)->var);*/
			} else {
				/* now fixup nodelist */
				if (list_append(outbdd->nodelist, (*out)->iszero)<0) return -1;
				if (list_append(outbdd->nodelist, (*out)->isone)<0) return -1;
			}
		} else if (*check==c0) {
			/*printf("Ignoring 1, because %s set.\n", cur->var);
			hash_dump(h);*/
			bdd_compact_r(cur->iszero, h, outbdd, out);
		} else {
			/*printf("Ignoring 0, because %s set.\n", cur->var);
			hash_dump(h);*/
			bdd_compact_r(cur->isone, h, outbdd, out);
		}
	} else {
		strlcpy((*out)->var, cur->var, sizeof((*out)->var));
#ifdef VERBOSE
		printf("bdd_compact_r: %p <- %s[%p] \n", (*out)->parent, (*out)->var, *out);
#endif
	}

	return 0;
}

size_t bdd_destroy(bdd_t *bdd) {

	listentry_t *e;
	size_t size=0;

	e=bdd->nodelist->first;
	while (e!=NULL) { mem_free(e->element); e=e->next; size++; }

	list_destroy(bdd->nodelist);
	mem_free(bdd);

	return size;
}


void bdd_output_candidates(bdd_t *bdd, list_t *vars) {

	listentry_t *e, *node;

	printf("Variable order:\n");
	e=vars->first;

	while (e!=NULL) {
		printf("%s", (char *)e->element);
		e=e->next;
		if (e!=NULL) printf(",");
		else printf("\n");
	}

	node=bdd->nodelist->first;
	while (node!=NULL) {

		bdd_node_t *ne = (bdd_node_t *)node->element;

		if (strcmp(ne->var, yes.var)==0)
			bdd_output_candidate(ne, vars);

		node=node->next;
	}
}

void bdd_output_candidate(bdd_node_t *n, list_t *vars) {

	hash_t *used;
	bdd_node_t *cur;
	listentry_t *ve;

	used = hash_create(VAR_HASH_SIZE);

#ifdef VERBOSE
	printf("Tracing solution:\n");
#endif
	while ((cur = n->parent)!=NULL) {
#ifdef VERBOSE
		printf("n: %p(\"%s\") -> (%p,%p); parent: %p(\"%s\") -> (%p,%p)\n",
			n, n->var, n->iszero, n->isone,
			cur, cur->var, cur->iszero, cur->isone
		);
#endif
		if (cur->iszero==n) hash_put(used, cur->var, no.var, 2);
		else if (cur->isone==n) hash_put(used, cur->var, yes.var, 2);
		else abort();
		n = cur;
	}

	ve = vars->first;
	while (ve!=NULL) {
		char *res;

		res=hash_get(used, (char *)ve->element);
		if (res==NULL) printf("*");
		else printf("%c", res[0]);
		ve=ve->next;
		if (ve!=NULL) printf(" ");
		else printf("\n");
	}

	hash_destroy(used);
}

