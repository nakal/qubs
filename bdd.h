
#ifndef LAMBDAX_BDD_INCLUDED
#define LAMBDAX_BDD_INCLUDED

#define VAR_HASH_SIZE 1024
#define BDD_VAR_LEN 32

/* types */
typedef struct _bdd_node_t {
	char var[BDD_VAR_LEN];
	struct _bdd_node_t *iszero;
	struct _bdd_node_t *isone;
	struct _bdd_node_t *parent;
} bdd_node_t;

#include "list.h"

typedef struct {
	bdd_node_t *start;
	list_t *nodelist;
} bdd_t;

typedef int (*bdd_opfunc_t)(bdd_node_t *);

/* constants */
extern bdd_node_t yes;
extern bdd_node_t no;

/* methods */
extern bdd_t *bdd_create(void);
extern bdd_node_t *bdd_init_node_ext
(bdd_t *bdd, bdd_node_t *parent, bdd_node_t *zerob, bdd_node_t *oneb,
	int append_to_list);
#define bdd_init_node(bdd,parent,zerob,oneb) \
	bdd_init_node_ext((bdd),(parent),(zerob),(oneb),1)
extern void dump_bdd(bdd_t *bdd);
extern int bdd_compact(bdd_t **bdd, int stats);
extern void bdd_merge(bdd_t *bdd, bdd_node_t *n, bdd_t *subbdd);
extern size_t bdd_destroy(bdd_t *bdd);
extern void bdd_output_candidates(bdd_t *bdd, list_t *vars);

/* we start on nodes, because it's more flexible */
extern int bdd_sat(bdd_node_t *bdd);
extern int bdd_true(bdd_node_t *bdd);
extern int bdd_false(bdd_node_t *bdd);

#endif

