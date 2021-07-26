//qt headers
#include <QDebug>
#include <QPushButton>
#include <QBoxLayout>
#include <QTextEdit>
#include <QLabel>

//cpp headers
#include <iostream>
#include <vector>
#include <stack>
#include <map>
#include <random>
#include <string>
#include <new>
#include <cctype>
#include <sstream>

//include headers
#include "Backend.h"
#include "../RBTree/RBTree.h"

//c headers
#include <unistd.h>


void Backend::errorHandle(int ERROR_CODE) {
    switch (ERROR_CODE) {
        case 9: {
            logger->append(std::string("ERROR_CODE is " + std::to_string(ERROR_CODE) + " -> bad allocation memory\n").data());
            break;
        }
    }
}

static inline size_t unwrap_or(const void *const ptr, size_t ofst, size_t default_value) {
    return (ptr ? (*(size_t*)(((char*)ptr) + ofst)) : default_value);
}

void printStateNodes(StateForWidthCalc *stateFWCalc) {
    StackNode *p{0};
    if (!stateFWCalc) {
        return;
    }
    p = stateFWCalc->top;
    qDebug() << "________________________________________";
    while (p) {
        qDebug() << "RBNode_t's value: " << p->nodePtr->value;
        qDebug() << "StackNode's width position: " << p->width;
        qDebug() << "StackNode's Y position: " << p->y;
        p = p->previous;
    }
    qDebug() << "________________________________________";
}

void freeStackNodes(StateForWidthCalc *state) {
    StackNode *p = NULL;
    StackNode *tmp = NULL;
    if (!state || !state->top) return;
    p = state->top;
    while (p) {
       tmp = p->previous;
       free(p);
       p = tmp;
    }
    state->top = NULL;
}

void calcWidthX(const RBNode_t *node, void* state) {
    StateForWidthCalc *st = (StateForWidthCalc*)state;
    if (!st) return;
    SN_push(&(st->top), node);
    size_t ofst = offsetof(StackNode, width);
    st->top->width = 2*st->r+2*std::max(unwrap_or(SN_find(st->top, node->left), ofst, st->minWidth), unwrap_or(SN_find(st->top, node->right), ofst, st->minWidth));

    st->top->y = *(st->rdata.y_lvl_cur) + 1;
}

size_t calcHeightY(const RBNode_t *nodeToAdd) {
    size_t yHeight{0};
    const RBNode_t *tmp = nodeToAdd;
    if (!tmp) {
        return -1;
    }
    while (tmp->parent) {
        tmp = tmp->parent;
        yHeight++;
    }
    return yHeight;
}

bool checkConsistencyOfTheTree(const RBNode_t *root) {
    if (!root) return true;

    if (root->parent) {
        qDebug() << "ERROR: root has parent: root->value: " << root->value << ", root->parent->value: " << root->parent->value;
        return false;
    }

    std::stack<const RBNode_t *> v;
    std::map<const RBNode_t *, int> m;

    v.push(root);
    m[root] = 0;

    while (!v.empty()) {
        const RBNode_t *cur = v.top();
        if (cur->left) {
            if (!m[cur]) {
                if (cur->left->parent != cur) {
                    qDebug() << "ERROR: " << __PRETTY_FUNCTION__ << ": v.size() = " << v.size() << ": left has wrong parent";
                    qDebug() << "ERROR: " << "current value is " << cur->value << ", left value is " << cur->left->value;
                    qDebug() << "ERROR: " << "current == current->left: " << (cur == cur->left);
                    if (cur->left->parent) {
                        qDebug() << "cur->left->parent there is";
                    } else {
                        qDebug() << "ERROR: " << "left has no parent";
                    }
                    return false;
                }
                v.push(cur->left);
                m[cur] = 1;
                continue;
            }
        } else if (!m[cur]){
            m[cur] = 1;
        }
        if (cur->right) {
            if (m[cur] == 1) {
                if (cur->right->parent != cur) {
                    qDebug() << "ERROR: " << __PRETTY_FUNCTION__ << ": v.size() = " << v.size() << ": right has wrong parent";
                    qDebug() << "ERROR: " << "current value is " << cur->value << ", right value is " << cur->right->value;
                    qDebug() << "ERROR: " << "{current, current->right, current->right->par}\n" << "{" << cur << ", " << cur->right << ", " << cur->right->parent << "}";
                    if (cur->right->parent) {
                        qDebug() << "ERROR: " << "right has parent, its value: " << cur->right->parent->value;
                    } else {
                        qDebug() << "ERROR: " << "right has no parent";
                    }
                    return false;
                }
                v.push(cur->right);
                m[cur] = 2;
                continue;
            }
        } else if (m[cur] == 1) {
            m[cur] = 2;
        }
        if ((!cur->left && !cur->right) || m[cur] == 2) {
            v.pop();
        }
    }

    return true;
}

void runOverTheTree(RBNode_t *root, void(*func)(const RBNode_t *Node, void* state), void* state) {
    static size_t y_lvl = 0;
    if (!root) {
        return;
    }
    RunOverTheTreeData *st = (RunOverTheTreeData*)(state);
    if (!y_lvl && st) {st->y_lvl_cur = &y_lvl;}

    y_lvl++;
    runOverTheTree(root->left, func, state);
    runOverTheTree(root->right, func, state);
    y_lvl--;
    func(root, state);
}

const StackNode* SN_find(const StackNode *top, const RBNode_t *node) {
    const StackNode *p = top;
    while (p && p->nodePtr != node) {
        p = p->previous;
    }
    return p;
}

StackNode* SN_find_rw(StackNode *top, const RBNode_t *node) {
    StackNode *p = top;
    while (p && p->nodePtr != node) p = p->previous;
    return p;
}

void SN_push(StackNode **Node, const RBNode_t *value) {
    if (!Node) {
        throw std::runtime_error(std::string(__FUNCTION__) + ": Invalid StackNode");
    }
    if (!*Node) {
        *Node = new StackNode;
        (*Node)->nodePtr = value;
        (*Node)->previous = nullptr;
        (*Node)->width = 0; //минимальная ширина
    } else {
        StackNode *newNode = new StackNode;
        newNode->nodePtr = value;
        newNode->previous = *Node;
        newNode->width = 0;
        *Node = newNode;
    }
}

void SN_pop(StackNode **Node) {
    if (!Node) throw std::runtime_error(std::string(__FUNCTION__) + ": Invalid StackNode");
    if (!*Node) return;
    StackNode *nodeToDelete = *Node;
    *Node = (*Node)->previous;
    delete nodeToDelete;
}

void pushFront(ListNode **Node, RBNode_t *value) {
    if (!Node) {
        throw std::runtime_error("Node is invalid\n");
    }
    if (!*Node) {
        *Node = new ListNode;
        (*Node)->right = nullptr;
        (*Node)->left = nullptr;
        (*Node)->value = value;
        (*Node)->index = 0;
    } else {
        ListNode *newNode = new ListNode;
        newNode->right = *Node;
        newNode->left = nullptr;
        newNode->value = value;
        newNode->index = (*Node)->index + 1;
        (*Node)->left = newNode;
        *Node = newNode;
    }
}

const RBNode_t *popBack(ListNode **Node) {
    if (!Node || !*Node) return nullptr;
    ListNode *nodeToDelete = *Node;
    while (nodeToDelete->right) nodeToDelete = nodeToDelete->right;
    const RBNode_t *delValue = nodeToDelete->value;
    if (nodeToDelete->left) nodeToDelete->left->right = nullptr;
    else *Node = nullptr;
    delete nodeToDelete;
    return delValue;
}

void printList(const ListNode *Node) {
    const ListNode *p = Node;
    if (!Node) return;
    while (p) {
        qDebug() << "RBNode value is " << p->value->value;
        p = p->right;
    }
}

void runWidthTheTree(RBNode_t *root, void(*func)(const RBNode_t *Node, void *state), void *state) {
    ListNode *Node{nullptr};
    try {
        pushFront(&Node, root);
        while (Node) {
            const RBNode_t *rbNode = popBack(&Node);
            func(rbNode, state);
            if (rbNode->left) pushFront(&Node, rbNode->left);
            if (rbNode->right) pushFront(&Node, rbNode->right);
        }
    } catch(const std::exception &ex) {
       qDebug() << ex.what();
    }
}

void calcShiftX(const RBNode_t *root, void *state) {
    StateForWidthCalc *st = static_cast<StateForWidthCalc*>(state);
    StackNode *p = st->top;
    StackNode *snParent{0};
    const RBNode_t *rbNode = root;
    if (!root || !st) {
        throw;
    }
    p = SN_find_rw(st->top, rbNode);
    if (!p->nodePtr->parent) { st->top->x = 0; }
    else {
        snParent = SN_find_rw(st->top, rbNode->parent);
        if (snParent->nodePtr->right == rbNode) {
            p->x = snParent->x + snParent->width;
        } else {
            p->x = snParent->x - snParent->width;
        }
    }
}

Backend::Backend(QApplication &_app, QWidget *parent): QWidget(parent), app{_app} {
    trW = new TreeWidth;
    stateFWCalc.minWidth = 10;

    QWidget *wgt = new QWidget(this);
    wgt->setFixedWidth(200);

    QGridLayout *gd = new QGridLayout(wgt);

    //Hide/Show GUI_______________________________
    QPushButton *buttonHide = new QPushButton("HideGUI");
    QPushButton *buttonShow = new QPushButton("ShowGUI", this);
    buttonShow->hide();
    buttonShow->setFixedWidth(190);
    QObject::connect(buttonHide, &QPushButton::clicked, this, [w = wgt, buttonShow]{w->hide(); buttonShow->show();});
    QObject::connect(buttonShow, &QPushButton::clicked, this, [wgt, buttonShow]{buttonShow->hide(); wgt->show();});

    gd->addWidget(buttonHide, 0, 0, 1, 2);
    //Hide/Show GUI_______________________________

    //Double/Half radius___________________________
    QHBoxLayout *hlayoutModR = new QHBoxLayout();

    QPushButton *buttonDoubleR = new QPushButton("Double R");
    QObject::connect(buttonDoubleR, SIGNAL(clicked()), this, SLOT(doubleFactor()));

    QPushButton *buttonHalfR = new QPushButton("Half R");
    QObject::connect(buttonHalfR, &QPushButton::clicked, this, [&]() {factor /= 1.5; treeChanged = true; repaint();});

    hlayoutModR->addWidget(buttonDoubleR);
    hlayoutModR->addWidget(buttonHalfR);
    gd->addLayout(hlayoutModR, 1, 0, 1, 2);
    //Double/Half radius____________________________

    //HalfX/DoubleX_______________________
    QHBoxLayout *hlayoutModX = new QHBoxLayout();

    QPushButton *buttonHalfX = new QPushButton("Half X");
    QObject::connect(buttonHalfX, SIGNAL(clicked()), this, SLOT(halfFactorX()));

    QPushButton *buttonDoubleX = new QPushButton("Double X");
    QObject::connect(buttonDoubleX, SIGNAL(clicked()), this, SLOT(doubleFactorX()));

    hlayoutModX->addWidget(buttonDoubleX);
    hlayoutModX->addWidget(buttonHalfX);
    gd->addLayout(hlayoutModX, 2, 0, 1, 2);
    //HalfX/DoubleX_______________________

    //Input range/number to add/delete
    qte = new QTextEdit();
    qte->setPlaceholderText(QStringLiteral("Input number or range via ','"));
    qte->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qte->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qte->setFixedHeight(30);

    gd->addWidget(qte, 3, 0, 1, 2);
    //Input range/number to add/delete

    //Additing/Deleting Value buttons
    QPushButton *buttonAddValue = new QPushButton("Add Value");
    QObject::connect(buttonAddValue, SIGNAL(clicked()), this, SLOT(slotAddValue()));

    QPushButton *buttonDeleteValue = new QPushButton("Delete Value");
    QObject::connect(buttonDeleteValue, SIGNAL(clicked()), this, SLOT(slotDeleteValue()));

    gd->addWidget(buttonAddValue, 4, 0, 1, 1);
    gd->addWidget(buttonDeleteValue, 4, 1, 1, 1);
    //Additing/Deleting Value buttons

    //Start tests
    QPushButton *buttonStartTest = new QPushButton("Start Test");
    QObject::connect(buttonStartTest, SIGNAL(clicked()), this, SLOT(slotStartTest()));

    numberOfTests = new QTextEdit();
    numberOfTests->setPlaceholderText(QStringLiteral("Tests number"));
    numberOfTests->setFixedHeight((QFontMetrics(numberOfTests->currentFont()).lineSpacing())*2);
    numberOfTests->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    numberOfTests->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    gd->addWidget(buttonStartTest, 5, 0, 1, 1);
    gd->addWidget(numberOfTests, 5, 1, 1, 1);
    //Start tests

    //Delete/Save Tree buttons
    QPushButton *buttonDeleteTree = new QPushButton("Clear All");
    QObject::connect(buttonDeleteTree, SIGNAL(clicked()), this, SLOT(slotDeleteTree()));

    QPushButton *buttonSaveTree = new QPushButton("Save Tree");
    QObject::connect(buttonSaveTree, SIGNAL(clicked()), this, SLOT(slotSaveTree()));

    QPushButton *buttonRollBackTree = new QPushButton("Back State");
    QObject::connect(buttonRollBackTree, SIGNAL(clicked()), this, SLOT(slotRollBackTree()));

    QPushButton *buttonRollForwardTree = new QPushButton("Forward State");
    QObject::connect(buttonRollForwardTree, SIGNAL(clicked()), this, SLOT(slotRollForwardTree()));

    gd->addWidget(buttonDeleteTree, 6, 0, 1, 1);
    gd->addWidget(buttonSaveTree, 6, 1, 1, 1);

    gd->addWidget(buttonRollBackTree, 7, 0, 1, 1);
    gd->addWidget(buttonRollForwardTree, 7, 1, 1, 1);
    //Delete/Save Tree buttons

    //LogLabel
    logger = new QTextEdit();
    logger->setPlaceholderText("Logger");
    logger->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    logger->setMaximumHeight(200);
    logger->setAlignment(Qt::AlignTop);
    logger->setReadOnly(true);
    logger->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //logger->setWordWrap(true);
    gd->addWidget(logger, 8, 0, 2, 2);
    //LogLabel

    //Exit button
    exitButton = new QPushButton("Exit", this);
    exitButton->move(this->width() - exitButton->width(), 0);
    QObject::connect(exitButton, &QPushButton::clicked, &app, &QCoreApplication::quit, Qt::QueuedConnection);
    //Exit button
}

Backend::~Backend() {
    free(rbroot);
}

void Backend::halfFactorX() {
    factorX /= 1.5;
    treeChanged = true;
    repaint();
}

void Backend::doubleFactorX() {
    factorX *= 1.5;
    treeChanged = true;
    repaint();
}


void Backend::slotSaveTree() {
    if (!rbroot) return;
    RBNode_t *cpRoot = copyTreeFunc(rbroot, &ERROR_CODE);
    if (ERROR_CODE) {
        errorHandle(ERROR_CODE);
    }
    try {
        if (currentState < treeBackup.size() - 1) {
            logger->append(std::string("current state is " + std::to_string(currentState + 1) + ", but last state is " +
                        std::to_string(treeBackup.size()) + ", all the next states will be deleted...").data());
            size_t tmpsize = treeBackup.size();
            for (size_t i = currentState + 1; i < treeBackup.size(); i++) {
                deleteTree(treeBackup[i]);
            }
            treeBackup.erase(treeBackup.begin() + currentState + 1, treeBackup.end());
            logger->append(std::string("All states from current + 1 - " + std::to_string(currentState + 2)  + " to " +
                        std::to_string(tmpsize) + " deleted succesefully").data());
        }
        treeBackup.emplace_back(cpRoot);
        currentState = treeBackup.size() - 1;
    } catch (const std::exception &ex) {
        logger->append(std::string("Vector error: " + std::string(ex.what())).data());
    }
    logger->append("__________");
    logger->append(std::string("Saving tree succesefull").data());
    logger->append(std::string("current state is " + std::to_string(currentState + 1)).data());
}

void Backend::slotRollBackTree() {
    if (!rbroot || !treeBackup.size() || currentState - 1 < 0) {
        return;
    }
    rbroot = treeBackup[--currentState];
    logger->append("__________");
    logger->append(std::string("current state is " + std::to_string(currentState + 1)).data());
    treeChanged = true;
    repaint();
}

void Backend::slotRollForwardTree() {
    if (!rbroot || !treeBackup.size() || currentState + 1 >= treeBackup.size()) {
        return;
    }
    rbroot = treeBackup[++currentState];
    logger->append("__________");
    logger->append(std::string("current state is " + std::to_string(currentState + 1)).data());
    treeChanged = true;
    repaint();
}

void Backend::paintEvent(QPaintEvent *ev) {
    Q_UNUSED(ev);

    QPainter *QPainterPtr = new QPainter(this);
    QPainterPtr->setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::FlatCap));

    if (!rbroot) {
        QPainterPtr->eraseRect(0, 0, width(), height());
    } else {
        QPainterPtr->eraseRect(0, 0, width(), height());
        stateFWCalc.r = 50*factor;
        if (treeChanged) {
            freeStackNodes(&stateFWCalc);
            runOverTheTree(rbroot, calcWidthX, &stateFWCalc);
            runWidthTheTree(rbroot, calcShiftX, &stateFWCalc);
        }

        StackNode *tmp = stateFWCalc.top;
        StackNode *tmpTop = tmp;
        StackNode *tmpParent;

        size_t addWidth = width()/2;
        size_t addHeight = 20;
        ssize_t heightFactor = 50;

        while (tmp) {
            QPainterPtr->setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::FlatCap));
            if (nodeColor(tmp->nodePtr)) QPainterPtr->setBrush(QBrush(Qt::red, Qt::SolidPattern));
            else QPainterPtr->setBrush(QBrush(Qt::black, Qt::SolidPattern));
            tmpParent = SN_find_rw(tmpTop, tmp->nodePtr->parent);
            if (tmpParent) {
                ssize_t x1 = (ssize_t)tmp->x * factorX + addWidth + stateFWCalc.r/2;
                ssize_t y1 = (ssize_t)tmp->y * heightFactor + addHeight;
                ssize_t x2 = (ssize_t)tmpParent->x * factorX + addWidth + stateFWCalc.r/2;
                ssize_t y2 = (ssize_t)tmpParent->y * heightFactor + addHeight + stateFWCalc.r;
                QPainterPtr->drawLine(x1, y1, x2, y2);
            }

            QPainterPtr->drawEllipse((ssize_t)tmp->x * factorX + addWidth, tmp->y * heightFactor + addHeight, stateFWCalc.r, stateFWCalc.r);
            QPainterPtr->drawText((ssize_t)tmp->x * factorX + addWidth + stateFWCalc.r/2.5, tmp->y * heightFactor + stateFWCalc.r/2 + addHeight, QString::number(tmp->nodePtr->value));
            tmp = tmp->previous;
        }
    }
    QPainterPtr->end();
    treeChanged = false;
}

void Backend::doubleFactor() {
    factor *= 1.5;
    repaint();
}

void Backend::slotAddValue() {
    if (qte->toPlainText().trimmed().isEmpty()) {
        logger->append("__________");
        logger->append("Input a number you want to add");
        return;
    }
    std::string inputStr = qte->toPlainText().trimmed().toUtf8().constData();
    for (size_t i = 0; i < inputStr.length(); i++) {
        if (!std::isdigit(inputStr[i])) {
            logger->append("__________");
            logger->append("Input string have incompatible symbols");
            return;
        }
    }
    tmpValue = qte->toPlainText().trimmed().toLongLong();
    RBNode_t *tmp = addValue(&rbroot, tmpValue, &ERROR_CODE);

    if (ERROR_CODE) {
        qDebug() << "function addValue returned ERROR_CODE " << ERROR_CODE;
        qDebug() << "the value was " << tmpValue;
        qDebug() << "ERROR_CODE will be changed to 0";
        ERROR_CODE = 0;
    }

    if (tmp){
        logger->append("__________");
        logger->append(std::string("value " + std::to_string(tmp->value) + " added succesefully").data());
        treeChanged = true;
        qDebug() << "treeChanged is set to true";
        if (checkConsistencyOfTheTree(rbroot)) {
            logger->append(QStringLiteral("Consistency of the tree is good"));
            qDebug() << "checkConsistencyOfTheTree(rbroot): " << checkConsistencyOfTheTree(rbroot);
        }
    } else {
        logger->append("__________");
        logger->append(QStringLiteral("addValue func returned NULL"));
        logger->append(QStringLiteral("Note: you can't add value, that is already in the Tree"));
        qDebug() << "addValue returned null";
    }
    repaint();
}


void Backend::slotDeleteValue() {
    if (qte->toPlainText().trimmed().isEmpty()) {
        logger->append("__________");
        logger->append("Input a number you want to delete");
        return;
    }
    std::string inputStr = qte->toPlainText().trimmed().toUtf8().constData();
    for (size_t i = 0; i < inputStr.length(); i++) {
        if (!std::isdigit(inputStr[i])) {
            logger->append("__________");
            logger->append("Input string have incompatible symbols");
            return;
        }
    }
    tmpValue = qte->toPlainText().trimmed().toLongLong();
    deleteNode(&rbroot, tmpValue, &ERROR_CODE);
    if (ERROR_CODE) {
        logger->append("__________");
        logger->append(std::string("ERROR: " + std::to_string(ERROR_CODE) + " returned").data());
        if (ERROR_CODE == 3 || ERROR_CODE == 6) {
            logger->append("you'are trying to delete a nonexistent node");
        }
        qDebug() << "function deleteNode returned ERROR_CODE " << ERROR_CODE;
        qDebug() << "the value was " << tmpValue;
        qDebug() << "ERROR_CODE will be changed to 0";
        ERROR_CODE = 0;
    } else {
        logger->append("__________");
        logger->append(std::string("Value " + std::to_string(tmpValue) + " deleted succesefully").data());
    }
    treeChanged = true;
    repaint();
}

void Backend::slotStartTest() {
    int maxVertexes = 40;
    int countOfTests = numberOfTests->toPlainText().trimmed().toLongLong();
    if (numberOfTests->toPlainText().trimmed().isEmpty()) {
        logger->append("__________");
        logger->append("Input count of Tests");
        qDebug() << "input count of Tests";
        return;
    }
    int minD = 0, maxD = 0, isDip = 0, size = 0;
    std::string inputStr = qte->toPlainText().trimmed().toUtf8().constData();
    if (qte->toPlainText().trimmed().isEmpty()) {
        logger->append("__________");
        logger->append("Input range of generating numbers to add or delete\n(or simply a number, so the range will be 0 to the {Number}");
        return;
    }
    inputStr.erase(std::remove(inputStr.begin(), inputStr.end(), ' '), inputStr.end());
    for (size_t i = 0; i < inputStr.length(); i++) {
        if (!std::isdigit(inputStr[i]) && inputStr[i] != ',' && inputStr[i] != ';') {
            logger->append("__________");
            logger->append("Input string have incompatible symbols\n(you can enter a number, or range like '5,100')");
            return;
        }
    }
    for (size_t i = 0; i < inputStr.length(); i++) {
        if (inputStr[i] == ',' || inputStr[i] == ';') {
          minD = std::stoi(inputStr.data());
          maxD = std::stoi(inputStr.data() + i + 1);
          isDip = 1;
          break;
        }
    }

    if (isDip) {
        size = maxD - minD + 1;
    } else {
        size = std::stoi(inputStr.data());
        minD = 0;
        maxD = size;
    }

    if (minD > maxD) {
        logger->append("__________");
        logger->append("Min number in range bigger than Max number, swapping...");
        int tmp = maxD;
        maxD = minD;
        minD = tmp; 
    }
    qDebug() << "{minD, maxD} -> {" << minD << ", " << maxD << "}";

    std::random_device rd;
    std::mt19937 mersen(rd());
    int randNum; RBNode_t *returnedValue;

    while (currentTree.size()) currentTree.pop_back();
    StackNode *p = stateFWCalc.top;
    while (p) {
        currentTree.push_back(p->nodePtr->value);
        p = p->previous;
    }

    double currentChance = currentTree.size() / (double)maxVertexes * 100; char isIncreased = 2;
    std::string choice;

    while (countOfTests) {
        if (isIncreased == 1) {
            currentChance = currentTree.size() / (double)maxVertexes * 100;
        } else if (isIncreased == 0) {
            currentChance = currentTree.size() / (double)maxVertexes * 100;
        }
        logger->append("__________");
        logger->append(std::string("current chance to delete value is " + std::to_string(currentChance) + "%").data());
        double chance = mersen() % 100 + (double)((mersen() % 100 + 1) / 100); //генерирует шанс с дробной частью до 2 знаков после запятой
        logger->append(std::string("generated chance is " + std::to_string(round(chance*100)/100) + "%").data());
        if (chance > currentChance) {
           randNum = mersen() % (maxD - minD) + minD;
            logger->append(std::string("Rand number to add was generated, it is " + std::to_string(round(randNum*100)/100)).data());
           qDebug() << "RandNum to add was generated, it's " << randNum;
           if (std::find(currentTree.begin(), currentTree.end(), randNum) == currentTree.end()) {
                qDebug() << "no such value in the tree, additing value...";
                logger->append("No such value in the tree, additing value...");
                 if (treeBackup.size() > 1000) {
                     deleteTree(treeBackup[0]);
                      treeBackup.erase(treeBackup.begin());
                      currentState--;
                 }
                slotSaveTree();
                returnedValue = addValue(&rbroot, randNum, &ERROR_CODE);
                if (ERROR_CODE) {
                    qDebug() << "addValue function has returned the ERROR_CODE " << ERROR_CODE;
                    if (ERROR_CODE == 1) {
                        qDebug() << "additing value is already is the tree";
                    }
                } else {
                    if (!checkConsistencyOfTheTree(rbroot)) {
                        logger->append("__________");
                        logger->append("Tree consistency violated, exit from a function");
                        return;
                    } else {
                        logger->append("__________");
                        logger->append("Tree consistency is good");
                        logger->append(std::string("Additing \"" + std::to_string(returnedValue->value) + "\" value completed succesefully").data());
                        qDebug() << "---------------\nadditing value completed succesefully\nnew node's value is " << returnedValue->value;
//                        std::cout << "new node parent's value is ";
//                        if (returnedValue->parent) std::cout << returnedValue->parent->value << std::endl;
//                        else std::cout << "<no parent>\n";
//                        std::cout << "new node left child's value is ";
//                        if (returnedValue->left) std::cout << returnedValue->left->value << std::endl;
//                        else std::cout << "<no left child>\n";
//                        std::cout << "new node right child's value is ";
//                        if (returnedValue->right) std::cout << returnedValue->right->value << std::endl;
//                        else std::cout << "<no right child>\n";
//                        qDebug() << "---------------";
                        currentTree.push_back(returnedValue->value);
                        isIncreased = 1;
                        treeChanged = true;
                        repaint();
                    }
                }
           } else {
                logger->append("__________");
                logger->append("There is such value in the tree, chance stuck");
                isIncreased = 2;
           }
        } else {
            randNum = mersen() % currentTree.size();
            qDebug() << "RandNum to delete was generated, it's " << currentTree[randNum];
            logger->append("__________");
            logger->append(std::string("Rand number to delete was generated, it's " + std::to_string(currentTree[randNum])).data());
             if (treeBackup.size() > 1000) {
                  deleteTree(treeBackup[0]);
                  treeBackup.erase(treeBackup.begin());
                  currentState--;
             }
            slotSaveTree();

            deleteNode(&rbroot, currentTree[randNum], &ERROR_CODE);
            if (ERROR_CODE) {
                qDebug() << "deleting function has returned the ERROR_CODE " << ERROR_CODE;
                logger->append("__________");
                logger->append(std::string("ERROR: " + std::to_string(ERROR_CODE) + " returned from the deleteNode func").data());
                ERROR_CODE = 0;
            } else {
                if (!checkConsistencyOfTheTree(rbroot)) {
                    logger->append("__________");
                    logger->append("Tree consistency violated, exit from a function");
                    return;
                } else {
                    logger->append("__________");
                    logger->append("Tree consistency is good");
                    logger->append(std::string("Deleting \"" + std::to_string(currentTree[randNum]) + "\" value completed succesefully").data());
                    isIncreased = 0;
                    currentTree.erase(currentTree.begin() + randNum);
                    treeChanged = true;
                    repaint();
                }
            }
        }
        countOfTests--;
    }

}

void Backend::slotDeleteTree() {
    logger->append("__________");
    deleteTree(rbroot);
    rbroot = NULL;
    for (size_t i = 0; i < treeBackup.size(); i++) {
        deleteTree(treeBackup[i]);
    }
    treeBackup.erase(treeBackup.begin(), treeBackup.end());
    currentState = 0;
    while (currentTree.size()) currentTree.pop_back();
    freeStackNodes(&stateFWCalc);
    qDebug() << "currentTree.size()" << currentTree.size();
    logger->clear();
    logger->append("Deleting Tree completed succesefully");
    logger->append("current state is 1");
    treeChanged = true;
    repaint();
}

void Backend::resizeEvent(QResizeEvent *ev) {
    Q_UNUSED(ev)
    exitButton->move(this->width() - exitButton->width(), 0);
}
