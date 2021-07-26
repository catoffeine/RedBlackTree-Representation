#ifndef RBTREE_H
#define RBTREE_H

#ifdef __cplusplus
extern "C" {

#endif

struct RBNode_t;

typedef struct RBNode_t RBNode_t;
typedef long long RBNode_value_t;

struct RBNode_t {
    RBNode_t *right, *left, *parent;
    RBNode_value_t value;
    char color; //1 - Red, 0 - black
};

RBNode_t * addValue(RBNode_t **root, RBNode_value_t value, int *ERROR_CODE);
RBNode_t * searchTree(RBNode_t **root, RBNode_value_t value, int *ERROR_CODE);
RBNode_t * findRoot(RBNode_t *current);
void deleteNode(RBNode_t **root, RBNode_value_t value, int *ERROR_CODE);
void blackDeleteBalanceTree(RBNode_t *Node, int *ERROR_CODE);
int nodeColor(const RBNode_t *Node);

void deleteTree(RBNode_t *Node);

void balanceTree(RBNode_t *el, int *ERROR_CODE);
//void leftSmallRotate(RBNode_t *Node, int *ERROR_CODE);
//void rightSmallRotate(RBNode_t *Node, int *ERROR_CODE);
void leftBigRotate(RBNode_t *Node, int *ERROR_CODE);
void rightBigRotate(RBNode_t *Node, int *ERROR_CODE);

RBNode_t* copyTreeFunc(const RBNode_t *node, int *ERROR_CODE);

#ifdef __cplusplus
}

#endif

#endif
