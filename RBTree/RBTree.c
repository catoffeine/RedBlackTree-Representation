#include <stdio.h>
#include <stdlib.h>
#include "RBTree.h"

#define RED 1
#define BLACK 0

// #define DBG

#ifdef DBG

#define PRINT_IF_DBG(cond, msg) {if (cond) fprintf(stderr, "%s: % 3d: %s\n", __FILE__, __LINE__, msg);}
#define PRINT_EC { fprintf(stderr, "%s: % 3d: *ERROR_CODE: %d\n", __FILE__, __LINE__, *ERROR_CODE); }

#else

#define PRINT_IF_DBG(cond, msg) {}
#define PRINT_EC

#endif

int nodeColor(const RBNode_t *Node) {
    if (!Node) return BLACK;
    if (Node->color) return RED;
    else return BLACK;
}

void deleteTree(RBNode_t *Node) {
    if (!Node) return;
    deleteTree(Node->left);
    deleteTree(Node->right);
    free(Node);
}

RBNode_t * searchTree(RBNode_t **root, RBNode_value_t value, int *ERROR_CODE) {
    RBNode_t *p = NULL;
    if (!root || !*root) {
        PRINT_IF_DBG(1, "EXIT OF FUNCTION searchTree in the begining, because root or *root was nullptr");
        return NULL;
    }
    #ifdef DBG
        printf("function searchTree BEGIN, root's value is %lld, searching value is %lld\n", (*root)->value, value);
    #endif
    p = *root;
    do {
        if (p->value < value) {
            if (!p->right) {
                *ERROR_CODE = 6;
                PRINT_EC
                return NULL;
            }
            p = p->right;
        } else if (value < p->value) {
            if (!p->left) {
                *ERROR_CODE = 6;
                PRINT_EC
                return NULL;
            }
            p = p->left;
        } else {
            #ifdef DBG
                printf("EXIT OF FUNCTION searchTree, value was founded, it's %lld\n", p->value);
            #endif
            return p;
        }
    } while (1);
}

RBNode_t * findRoot(RBNode_t *current) {
    while (current->parent) current = current->parent;
    return current;
}

RBNode_t * addValue(RBNode_t **root, RBNode_value_t value, int *ERROR_CODE) {
    PRINT_IF_DBG(1, "BEGIN function addValue");
    RBNode_t *p, *next;
    char isLeafLeft = 0;
    if (!root) {
        *ERROR_CODE = 3;
        return NULL;
    }
    next = *root;
    if (!*root) {
        RBNode_t *newNode = (RBNode_t*)malloc(sizeof(RBNode_t));
        if (!newNode) {
            *ERROR_CODE = 2;
            return NULL;
        }
        newNode->color = BLACK;
        newNode->value = value;
        newNode->right = NULL;
        newNode->left = NULL;
        newNode->parent = NULL;
        *root = newNode;
        PRINT_IF_DBG(1, "END of function addValue");
        return newNode;
    }
    do {
        p = next;
        if (value > next->value) {
            if (!next->right) {
                isLeafLeft = 0;
                break;
            }
            next = next->right;
        } else if (value < next->value) {
            if (!next->left) {
                isLeafLeft = 1;
                break;
            }
            next = next->left;
        } else {
            *ERROR_CODE = 1; //Добавляемый элемент уже существует в дереве
            return NULL;
        }
    } while(1);
    RBNode_t *newNode = (RBNode_t*)malloc(sizeof(RBNode_t));
    if (!newNode) {
        *ERROR_CODE = 2;
        return NULL;
    }

    newNode->parent = p;
    newNode->color = RED;
    newNode->right = NULL;
    newNode->left = NULL;
    newNode->value = value;

    if (isLeafLeft) {
        p->left = newNode;
        PRINT_IF_DBG(1, "addValue: isLeafLeft == true");
    } else {
        p->right = newNode;
        PRINT_IF_DBG(1, "addValue: isLeafLeft == false");
    }

    balanceTree(newNode, ERROR_CODE);
    if (*ERROR_CODE) {
        PRINT_EC
        return NULL;
    }
    *root = findRoot(newNode);
    PRINT_IF_DBG(1, "END of function addValue");
    return newNode;
}

void blackDeleteBalanceTree(RBNode_t *Node, int *ERROR_CODE) {
    PRINT_IF_DBG(1, "BEGIN function blackDeleteBalanceTree");
    RBNode_t *root;
    if (!Node) {
        *ERROR_CODE = 3;
        return;
    }

    root = findRoot(Node);

    if (Node == root) {
        PRINT_IF_DBG(1, "____Node is root____");
        root->color = BLACK;
        PRINT_IF_DBG(1, "END function blackDeleteBalanceTree");
        return;
    }

    if (nodeColor(Node)) return;
    
    if (Node->parent) { //Если существует родитель удаляемого элемента
        PRINT_IF_DBG(1, "PARENT EXIST");
        if (Node->parent->left == Node) { //Если удаляемый элемент находится слева
            PRINT_IF_DBG(1, "NODE TO DELETE TO THE LEFT OF THE PARENT");
            if (Node->parent->right) { //Проверка на существование брата
                PRINT_IF_DBG(1, "BROTHER EXIST");
                if (nodeColor(Node->parent)) { //Если родитель - красный
                    PRINT_IF_DBG(1, "PARENT IS RED");
                    if (!nodeColor(Node->parent->right)) { //Брат - черный
                        PRINT_IF_DBG(1, "BROTHER IS BLACK");
                        if (!nodeColor(Node->parent->right->left) && !nodeColor(Node->parent->right->right)) { //внуки - черные
                            PRINT_IF_DBG(1, "CASE BLACK GRANDCHILDREN");
                            Node->parent->color = BLACK;
                            Node->parent->right->color = RED;
                        } else if (nodeColor(Node->parent->right->right)) { // Правый внук - красный (левый неважно)
                            PRINT_IF_DBG(1, "CASE RED RIGHT GRANDCHILD");
                            Node->parent->right->right->color = BLACK;
                            leftBigRotate(Node->parent, ERROR_CODE);
                            if (ERROR_CODE) {PRINT_EC return;}
                        } else if(nodeColor(Node->parent->right->left)) { //Левый внук - красный
                            PRINT_IF_DBG(1, "CASE RED LEFT GRANDCHILD");
                            rightBigRotate(Node->parent->right, ERROR_CODE);
                            if (*ERROR_CODE) {PRINT_EC return;}
                            leftBigRotate(Node->parent, ERROR_CODE);
                            Node->parent->color = RED;
                            Node->parent->parent->color = BLACK;
                            if (*ERROR_CODE) {PRINT_EC return;}
//                            blackDeleteBalanceTree(Node->parent, ERROR_CODE);
                        }
                    }
                } else { //Если родитель - черный
                    PRINT_IF_DBG(1, "PARENT IS BLACK");
                    if (nodeColor(Node->parent->right)) { // Брат - красный
                        PRINT_IF_DBG(1, "BROTHER IS RED");
                        if (Node->parent->right->left) { //проверка на существование внука
                            PRINT_IF_DBG(1, "GRANDCHILD EXIST");
                            if (!nodeColor(Node->parent->right->left->left) && !nodeColor(Node->parent->right->left->right)) { //Правнуки левого внука - черные
                                PRINT_IF_DBG(1, "GREAT CRANDCHILDREN OF LEFT GRANDCHILD ARE BLACK");
                                Node->parent->right->left->color = RED;
                                leftBigRotate(Node->parent, ERROR_CODE);
                                if (*ERROR_CODE) {PRINT_EC return;}
                                Node->parent->color = BLACK;
                                Node->parent->parent->color = BLACK;
                            } else if (nodeColor(Node->parent->right->left->right)) { //У левого внука правый правнук - красный
                                PRINT_IF_DBG(1, "RIGHT GREAT CRANDCHILD OF LEFT GRANDCHILD IS RED");
                                if (Node->parent->right->left->right) Node->parent->right->left->right->color = BLACK;
                                rightBigRotate(Node->parent->right, ERROR_CODE);
                                if (*ERROR_CODE) {PRINT_EC return;}
                                Node->parent->right->color = BLACK;
                                Node->parent->right->right->color = RED;
                                leftBigRotate(Node->parent, ERROR_CODE);
                                if (*ERROR_CODE) {PRINT_EC return;}
                            } else if (nodeColor(Node->parent->right->left->left)) {//У левого внука левый правнук - красный
                                PRINT_IF_DBG(1, "LEFT GREAT CRANDCHILD OF LEFT GRANDCHILD IS RED");
                                leftBigRotate(Node->parent, ERROR_CODE);
                                if (*ERROR_CODE) {PRINT_EC return;}
                                rightBigRotate(Node->parent->right, ERROR_CODE);
                                if (*ERROR_CODE) {PRINT_EC return;}
                                leftBigRotate(Node->parent, ERROR_CODE);
                                if (*ERROR_CODE) {PRINT_EC return;}
                                Node->parent->color = RED;
                                Node->parent->parent->color = BLACK;
                            }
                        }
                    } else { //брат - черный
                        PRINT_IF_DBG(1, "BROTHER IS BLACK");
                        if (nodeColor(Node->parent->right->left)) { //Левый красный внук
                            PRINT_IF_DBG(1, "LEFT GRANDCHILD IS RED");
                            Node->parent->right->left->color = BLACK;
                            rightBigRotate(Node->parent->right, ERROR_CODE);
                            if (*ERROR_CODE) {PRINT_EC return;}
                            leftBigRotate(Node->parent, ERROR_CODE);
                            if (*ERROR_CODE) {PRINT_EC return;}
                        } else if (!nodeColor(Node->parent->right->right) && !nodeColor(Node->parent->right->left)) { //Черные внуки
                            PRINT_IF_DBG(1, "BLACK GRANDCHILDREN");
                            Node->parent->right->color = RED;
                            blackDeleteBalanceTree(Node->parent, ERROR_CODE);
                        } else if (nodeColor(Node->parent->right->right)) { //Правый красный внук
                            PRINT_IF_DBG(1, "RIGHT GRANDHILD IS RED");
                            Node->parent->right->right->color = BLACK;
                            leftBigRotate(Node->parent, ERROR_CODE);
                            if (*ERROR_CODE) {PRINT_EC return;}
                        }
                    }
                } //Если родитель - черный
            } //Проверка на существование брата
        } else { //Если удаляемый элемент находится справа
            PRINT_IF_DBG(1, "NODE TO DELETE TO THE RIGHT OF THE PARENT");
            if (Node->parent->left) { //Проверка на существование брата
                PRINT_IF_DBG(1, "BROTHER EXIST");
                if (nodeColor(Node->parent)) { //Если родитель - красный
                    PRINT_IF_DBG(1, "PARENT IS RED");
                    if (!nodeColor(Node->parent->left)) { //Брат - черный
                        PRINT_IF_DBG(1, "BROTHER IS BLACK");
                        if (!nodeColor(Node->parent->left->left) && !nodeColor(Node->parent->left->right)) { //внуки - черные
                            PRINT_IF_DBG(1, "GRANDCHILDREN ARE BLACK");
                            Node->parent->color = BLACK;
                            Node->parent->left->color = RED;
                        } else if (nodeColor(Node->parent->left->left)) { // Левый внук - красный (правый неважно)
                            PRINT_IF_DBG(1, "LEFT GRANDCHILD IS RED");
                            Node->parent->left->left->color = BLACK;
                            rightBigRotate(Node->parent, ERROR_CODE);
                            if (*ERROR_CODE) {PRINT_EC return;}
                        } else if(nodeColor(Node->parent->left->right)) {
                            PRINT_IF_DBG(1, "RIGHT GRANDCHILD IS RED");
                            leftBigRotate(Node->parent->left, ERROR_CODE);
                            if (*ERROR_CODE) {PRINT_EC return;}
                            rightBigRotate(Node->parent, ERROR_CODE);
                            Node->parent->color = RED;
                            Node->parent->parent->color = BLACK;
                            if (*ERROR_CODE) {PRINT_EC return;}
                        }
                    }
                } else { //Если родитель - черный
                    PRINT_IF_DBG(1, "PARENT IS BLACK");
                    if (nodeColor(Node->parent->left)) { // Брат - красный
                        PRINT_IF_DBG(1, "BROTHER IS RED");
                        if (Node->parent->left->right) { //проверка на существование внука
                            PRINT_IF_DBG(1, "GRANDCHILD EXIST");
                            if (!nodeColor(Node->parent->left->right->left) && !nodeColor(Node->parent->left->right->right)) { //Правнуки правого внука - черные
                                PRINT_IF_DBG(1, "GREAT GRANDCHILDREN OF THE RIGHT GRANDCHILD ARE BLACK");
                                Node->parent->left->right->color = RED;
                                rightBigRotate(Node->parent, ERROR_CODE);
                                if (*ERROR_CODE) {PRINT_EC return;}
                                Node->parent->color = BLACK;
                                Node->parent->parent->color = BLACK;
                            } else if (nodeColor(Node->parent->left->right->left)) { //У правого внука левый правнук - красный
                                PRINT_IF_DBG(1, "LEFT GREAT GRANDCHILD OF THE RIGHT GRANDCHILD IS RED");
                                if (Node->parent->left->right->left) Node->parent->left->right->left->color = BLACK;
                                leftBigRotate(Node->parent->left, ERROR_CODE);
                                if (*ERROR_CODE) {PRINT_EC return;}
                                Node->parent->left->color = BLACK;
                                Node->parent->left->left->color = RED;
                                rightBigRotate(Node->parent, ERROR_CODE);
                                if (*ERROR_CODE) {PRINT_EC return;}
                            } else if (nodeColor(Node->parent->left->right->right)) {//У правого внука правый правнук - красный
                                    PRINT_IF_DBG(1, "RIGHT GREAT GRANDCHILD OF THE RIGHT GRANDCHILD IS RED");
                                    rightBigRotate(Node->parent, ERROR_CODE);
                                    if (*ERROR_CODE) {PRINT_EC return;}
                                    leftBigRotate(Node->parent->left, ERROR_CODE);
                                    if (*ERROR_CODE) {PRINT_EC return;}
                                    rightBigRotate(Node->parent, ERROR_CODE);
                                    if (*ERROR_CODE) {PRINT_EC return;}
                                    Node->parent->color = RED;
                                    Node->parent->parent->color = BLACK;
                                }
                        }
                    } else { //брат - черный
                        PRINT_IF_DBG(1, "BROTHER IS BLACK");
                        if (nodeColor(Node->parent->left->right)) { //Правый красный внук
                            PRINT_IF_DBG(1, "RIGHT GRANDCHILD IS RED");
                            Node->parent->left->right->color = BLACK;
                            leftBigRotate(Node->parent->left, ERROR_CODE);
                            if (*ERROR_CODE) {PRINT_EC return;}
                            rightBigRotate(Node->parent, ERROR_CODE);
                            if (*ERROR_CODE) {PRINT_EC return;}
                        } else if (!nodeColor(Node->parent->left->right) && !nodeColor(Node->parent->left->left)) {
                            PRINT_IF_DBG(1, "BLACK GRANDCHILDREN ARE BLACK");
                            Node->parent->left->color = RED;
                            blackDeleteBalanceTree(Node->parent, ERROR_CODE);
                        } else if (nodeColor(Node->parent->left->left)) { //Правый левый внук - красный
                            PRINT_IF_DBG(1, "LEFT GRANDCHILD IS RED");
                            Node->parent->left->left->color = BLACK;
                            rightBigRotate(Node->parent, ERROR_CODE);
                            if (!*ERROR_CODE) {PRINT_EC return;}
                        }
                    }
                } //Если родитель - черный
            } //Проверка на существование брата
        } //Если удаляемый элемент находится справа
    }
    root = findRoot(Node);
    root->color = BLACK;
    PRINT_IF_DBG(1, "END function blackDeleteBalanceTree");
}


void deleteNode(RBNode_t **root, RBNode_value_t value, int *ERROR_CODE) {
    PRINT_IF_DBG(1, "BEGIN function deleteNode");
    RBNode_t *p = 0;
    RBNode_t *el;
    RBNode_value_t tmpN = 0;

    if (!root || !*root) {
        *ERROR_CODE = 3;
        return;
    }

    PRINT_IF_DBG(1, "searchTree next");
    el = searchTree(root, value, ERROR_CODE); //находим элемент, который собираемся удалять
    p = el;
    if (*ERROR_CODE) { //в случае возвращения ошибки делаем выход
        PRINT_EC
        return;
    }

    if (!el->right && !el->left) { //Если отсутствуют потомки
        if (el == *root) {
            if (nodeColor(el)) {
                *ERROR_CODE = 8;
                return;
            } else { //Если этот элемент - черный
                blackDeleteBalanceTree(el, ERROR_CODE);
            }
        }
        if (el != *root) {
            if (!nodeColor(el)) {
                blackDeleteBalanceTree(el, ERROR_CODE);
                if (*ERROR_CODE) {
                    PRINT_EC
                    return;
                }
            }
            if (el->parent->left == el) el->parent->left = NULL;
            else el->parent->right = NULL;
            *root = findRoot(el);
        } else {
            *root = NULL;
        }
        free(el);
        el = NULL;
        PRINT_IF_DBG(1, "END function deleteNode");
        return;
    }

    if ((!el->right && el->left) || (el->right && !el->left)) { //Если у вершины только один ребенок
        if (!el->color) { //Если элемент черный
            if (el->right) { //Если ребенок справа
                PRINT_IF_DBG(!nodeColor(el->right), "BLACK RIGHT SINGLE CHILD, IMPOSSIBLE")
                el->value = el->right->value; //Присваиваем значение ребенка и удаляем ребенка
                free(el->right);
                el->right = NULL;
            } else {  //Если ребенок слева
                PRINT_IF_DBG(!nodeColor(el->left), "BLACK LEFT SINGLE CHILD, IMPOSSIBLE")
                el->value = el->left->value; //Присваиваем значение ребенка и удаляем ребенка
                free(el->left);
                el->left = NULL;
            }
        } else {
            *ERROR_CODE = 7; //У красного не может быть никакого одного ребенка
            PRINT_EC
            return;
        }
        *root = findRoot(el);
        PRINT_IF_DBG(1, "END function deleteNode");
        return;
    }

    if (el->right && el->left) { //Если оба ребенка существуют
        p = el->right;
        while (p->left) { //Ищем минимальный справа
            p = p->left;
        }

        tmpN = p->value;
        p->value = el->value;
        el->value = tmpN;

        if (!nodeColor(p)) { //Если удаляемый элемент - черный
            if (p->right) { //Если есть правый ребенок
                PRINT_IF_DBG(nodeColor(el->right), "BLACK RIGHT SINGLE CHILD, IMPOSSIBLE")
                PRINT_IF_DBG(1, "BOTH CHILDREN EXIST, DELETING ELEMENT IS BLACK, RIGHT CHILD EXIST");
                p->value = p->right->value; //Присваиваем значение ребенка и удаляем ребенка
                p = p->right;
                p->parent->right = NULL;
            } else {
                PRINT_IF_DBG(1, "BOTH CHILDREN EXIST, DELETING ELEMENT IS BLACK, RIGHT CHILD DOESNT EXIST");
                blackDeleteBalanceTree(p, ERROR_CODE); //Вызываем балансировку от p
                if (p->parent->right == p) p->parent->right = NULL;
                else p->parent->left = NULL;
            }

            *root = findRoot(p);

            #ifdef DBG
                printf("p->parent->value is %lld\n", p->parent->value);
                printf("p->value is %lld\n", p->value);
            #endif
            free(p);
            p = NULL;
            PRINT_IF_DBG(1, "END function deleteNode");
            return;
        } else { // Если красный - удаляем и меняем указатель на NULL;
            PRINT_IF_DBG(1, "both children exist, case RED right child");
            if (!p->parent) {
                PRINT_IF_DBG(1, "ROOT IS RED, IMPOSSIBLE");
                *ERROR_CODE = 8;
                return;
            } else {
                PRINT_IF_DBG(1, "case p->parent->right = NULL");
                if (p->parent->left == p) p->parent->left = NULL;
                else p->parent->right = NULL;
            }
            PRINT_IF_DBG(1, "before free(p)");
            free(p);
            p = NULL;
        }
    }

    PRINT_IF_DBG(1, "END function deleteNode");
}


void balanceTree(RBNode_t *el, int *ERROR_CODE) { //Предпологаем, что el->color==1 - красный
    PRINT_IF_DBG(1, "BEGIN function balanceTree");
    char isLeftEl = 0;

    if (!el) {
        *ERROR_CODE = 3;
        return;
    }

    if (!el->parent)  {
        el->color = BLACK;
        return;
    }

    isLeftEl = el->parent->left == el ? 1 : 0;

    if (el->parent->color) { //Если красный
        if (isLeftEl) { //Если проверяемый элемент находится слева от родителя
            PRINT_IF_DBG(1, "ELEMENT TO THE LEFT FROM PARENT");
            PRINT_IF_DBG(!el->parent->parent, "BALANCE TREE: THERE IS NO GRANDPARENT");
            if (el->parent->parent->right == el->parent) { //Если дедушка проверяемого элемента находится по другую сторону
                rightBigRotate(el->parent, ERROR_CODE); //Вызываем правое малое вращение от родителя
                if (*ERROR_CODE) return;
                balanceTree(el->right, ERROR_CODE);
                if (*ERROR_CODE) return;

            } else { //Если дедушка проверяемого элемента находится на одной стороне
                PRINT_IF_DBG(1, "GRANDPARENT IS ON THE SAME DIRECTION");
                if (nodeColor(el->parent->parent->right)) { //Если дядя проверяемого элемента красный
                    el->parent->parent->right->color = BLACK; //Меняем цвета на черный для дяди и для родителя
                    el->parent->color = BLACK;
                    el->parent->parent->color = RED; //Меняем цвет деда на красный
                    balanceTree(el->parent->parent, ERROR_CODE);
                    if (*ERROR_CODE) return;
                } else { //Дядя черный (или NULL)
                    PRINT_IF_DBG(1, "UNCLE IS BLACK");
                    rightBigRotate(el->parent->parent, ERROR_CODE); //Вызываем большое вращение от дедушки проверяемого элемента
                }
            }
        } else { //Если проверяемый элемент находится справа от родителя
            PRINT_IF_DBG(!el->parent->parent, "BALANCE TREE: THERE IS NO GRANDPARENT");
            PRINT_IF_DBG(1, "ELEMENT TO THE RIGHT FROM PARENT");
            if (el->parent->parent->left == el->parent) { //Если дедушка проверяемого элемента находится по другую сторону
                leftBigRotate(el->parent, ERROR_CODE); //Вызываем левое малое вращение для родителя
                if (*ERROR_CODE) return;
                balanceTree(el->left, ERROR_CODE);
                if (*ERROR_CODE) return;
            } else {
                PRINT_IF_DBG(1, "GRANDPARENT IS ON THE SAME DIRECTION");
                if (nodeColor(el->parent->parent->left)) { //Если дядя проверяемого элемента тоже красный
                    PRINT_IF_DBG(1, "UNCLE IS RED TOO");
                    el->parent->parent->left->color = BLACK;
                    el->parent->color = BLACK;
                    el->parent->parent->color = RED;
                    balanceTree(el->parent->parent, ERROR_CODE);
                    if (*ERROR_CODE) return;
                } else { //Дядя черный
                    PRINT_IF_DBG(1, "UNCLE IS BLACK");
                    leftBigRotate(el->parent->parent, ERROR_CODE); //Вызываем большое вращение от дедушки проверяемого элемента
                }
            }
        }
    } else {
        PRINT_IF_DBG(1, "END of function balanceTree");
        return;
    }
}

void leftBigRotate(RBNode_t *Node, int *ERROR_CODE) {
    PRINT_IF_DBG(1, "BEGIN of function leftBigRotate");
    RBNode_t *p = Node, *tmp = 0;
    RBNode_value_t tmpColor;

    if (!Node) {
        *ERROR_CODE = 3;
        return;
    }

    if (!p->right) {
        *ERROR_CODE = 5; //Нет правого потомка
        PRINT_EC
        return;
    }

    if (!p->parent) {
        p->right->parent = NULL; //Так как родитель - корень, выставляем в качестве родителя - нулевой указатель
    } else {
        p->right->parent = tmp = p->parent; //в качестве родителя указываем родителя данного элемента
        if (tmp->right == p) { //Перемещаем указатель на с данной ноды на правого потомка
            tmp->right = p->right;
        } else {
            tmp->left = p->right;
        }
    }

    tmp = p->right->left; //Запоминаем указатель на левого потомка правой ноды

    p->parent = p->right; //Указываем в качестве родителя правую ноду
    p->right->left = p; //Для правого потомка в качестве левого указываем данную ноду
    p->right = tmp; //Для данной ноды в качестве правого потомка указываем ранее запомненного левого потомка правой ноды
    if (p->right) p->right->parent = p; //исправил p->right на p->right->parent
    p = p->parent; //Перемещаем указатель на родителя элемента

    tmpColor = p->color;
    p->color = p->left->color;
    p->left->color = tmpColor;

    PRINT_IF_DBG(1, "END of function leftBigRotate");
}

void rightBigRotate(RBNode_t *Node, int *ERROR_CODE) {
    PRINT_IF_DBG(1, "BEGIN of function rightBigRotate");
    RBNode_t *p = Node, *tmp = 0;
    RBNode_value_t tmpColor;

    if (!Node) {
        *ERROR_CODE = 3;
        return;
    }

    if (!p->left) {
        *ERROR_CODE = 5; //Нет левого потомка
        PRINT_EC
        return;
    }

    if (!p->parent) {
        p->left->parent = NULL; //Так как родитель - корень, выставляем в качестве родителя - нулевой указатель
    } else {
        p->left->parent = tmp = p->parent; //в качестве родителя указываем родителя данного элемента
        if (tmp->right == p) {//Перемещаем указатель на с данной ноды на левого потомка
            tmp->right = p->left;
        } else {
            tmp->left = p->left;
        }
    }

    tmp = p->left->right; //Запоминаем указатель на правого потомка левой ноды

    p->parent = p->left; //Указываем в качестве родителя левую ноду
    p->left->right = p; //Для левого потомка в качестве правого указываем данную ноду
    p->left = tmp; //Для данной ноды в качестве левого потомка указываем ранее запомненного правого потомка правой ноды
    if (p->left) p->left->parent = p;
    p = p->parent; //Перемещаем указатель на родителя элемента

    tmpColor = p->color;
    p->color = p->right->color;
    p->right->color = tmpColor;

    PRINT_IF_DBG(1, "END of function rightBigRotate");
}

RBNode_t* copyTreeFunc(const RBNode_t *node, int *ERROR_CODE) {
    if (!node) return NULL;
    RBNode_t *left = NULL, *right = NULL;

    left = copyTreeFunc(node->left, ERROR_CODE);
    if (*ERROR_CODE) {
        PRINT_EC
        return NULL;
    }
    right = copyTreeFunc(node->right, ERROR_CODE);
    if (*ERROR_CODE) {
        deleteTree(left);
        PRINT_EC
        return NULL;
    }

    RBNode_t *newNode = (RBNode_t*)malloc(sizeof(RBNode_t));
    if (!newNode) {
        deleteTree(left);
        deleteTree(right);
        *ERROR_CODE = 9;
        return NULL;
    }

    newNode->parent = NULL;
    newNode->left = left;
    if (left) left->parent = newNode;
    newNode->right = right;
    if (right) right->parent = newNode;

    newNode->value = node->value;
    newNode->color = node->color;

    return newNode;
}
