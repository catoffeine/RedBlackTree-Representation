#pragma once

#include <cstddef>
#include <vector>

#include <QObject>
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QString>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include "../RBTree/RBTree.h"

struct TreeWidth;
void buildTree(const RBNode_t *Node, long long index, void *state);
size_t calcHeightY(const RBNode_t *nodeToAdd);
struct StackNode;

struct RunOverTheTreeData {
    const size_t *y_lvl_cur = nullptr;
};

struct StateForWidthCalc {
    RunOverTheTreeData rdata;
    StackNode *top = nullptr;
    size_t size = 0;
    size_t minWidth = 0;
    size_t r = 0;
};

class Backend : public QWidget {
    Q_OBJECT

    QApplication &app;
    QPushButton *exitButton {nullptr};
    double factor {1};
    double factorX {1};

    std::vector<size_t> currentTree;
    RBNode_t *rbroot {nullptr};
    int ERROR_CODE {0};
    long long tmpValue {0};
    TreeWidth *trW {NULL};
    StateForWidthCalc stateFWCalc;
    std::vector<RBNode_t*> treeBackup {};
    bool treeChanged {false};
    int currentState {-1};

    QTextEdit *qte {nullptr};
    QTextEdit *numberOfTests {nullptr};

    QTextEdit *logger {nullptr};

    //info about actions
    RBNode_t *currentNode {nullptr};
    bool isAdded = false;
public:
    Backend(QApplication &_app, QWidget *parent = nullptr);
    ~Backend();

    void errorHandle(int ERROR_CODE);
    virtual void paintEvent(QPaintEvent *ev) override;
    virtual void resizeEvent(QResizeEvent *ev) override;
    void fillVectorOfTree(const RBNode_t *node, void *smth);

public slots:
    void doubleFactor();
    void halfFactorX();
    void doubleFactorX();
    void slotAddValue();
    void slotDeleteValue();
    void slotStartTest();
    void slotSaveTree();
    void slotRollBackTree();
    void slotDeleteTree();
    void slotRollForwardTree();
};

void SN_push(StackNode **Node, const RBNode_t *value);
const StackNode* SN_find(const StackNode *top, const RBNode_t *node);
StackNode* SN_find_rw(StackNode *top, const RBNode_t *node);

void freeStackNodes(StateForWidthCalc *state);

struct TreeWidth {
    StackNode *stack = nullptr;
    long long w;
    long long value;
    long long *tree = nullptr;
    long long size;
};


struct StackNode {
    const RBNode_t *nodePtr = nullptr;
    StackNode *previous = nullptr;
    size_t width = 0;
    size_t x = 0, y = 0;
};

struct ListNode {
    ListNode *right = nullptr;
    ListNode *left = nullptr;
    const RBNode_t *value = nullptr;
    size_t index = 0;
};

void printList(const ListNode *Node);
void pushFront(ListNode **Node, RBNode_t *value);
const RBNode_t *popBack(ListNode **Node);

bool checkConsistencyOfTheTree(const RBNode_t*);
