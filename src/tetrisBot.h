#ifndef TETRISBOT_H
#define TETRISBOT_H
#include<QVector>
#include<QSet>
#include<tetrisBase.h>
#include <bitset>
#include<queue>
#include<limits>
#include <QtConcurrent>

int BitCount(int);
namespace Tool
{
QString  printType(Piece);
void printMap(TetrisMap &);
QString printNode(TetrisNode &);
QString printPath(QVector<Oper> &);
void sleepTo(int msec);
}

namespace TetrisBot
{
struct Status {
    int combo = 0;
    bool b2b = false;
    int attack = 0;
    int like = 0;
    int maxCombo = 0;
    int maxAttack = 0;
    double value = 0.;
    int mapRise = 0;
    int underAttack = 0;
};

struct EvalResult {
    int safe = 0;
    int t2Value = 0;
    int t3Value = 0;
    double value = 0.;
    int clear = 0;
    int count = 0;
    TspinType typeTspin = TspinType::None;
};

QVector<TetrisNode> search(TetrisNode &, TetrisMap &);
auto make_path(TetrisNode &, TetrisNode &, TetrisMap &,bool)->QVector<Oper>;
TetrisBot::EvalResult evalute(TetrisNode &, TetrisMap &, int);
TetrisBot::Status get(const TetrisBot::EvalResult &, TetrisBot::Status &, Piece, QVector<Piece> *, int);
}

class TreeNode;
class CNP;
class TreeContext
{
public:
    ~TreeContext();
    void createRoot(TetrisNode &, TetrisMap &, QVector<Piece> &, Piece, int, int);
    void treeDelete(TreeNode *);
    void run();
    std::tuple<TetrisNode, bool, bool>getBest();
private:
    friend class TreeNode;
    using treeQueue = std::priority_queue<TreeNode *, std::vector<TreeNode *>, CNP>;
    QVector < treeQueue>level; //保存每一个等级层
    QVector < treeQueue>extendedLevel; //保存每一个等级层
    QVector<Piece>nexts, noneHoldFirstNexts; //预览队列
    int width = 0;
    bool isOpenHold = true;
    TreeNode *root = nullptr;
    bool test = false;
    QVector<double> width_cache;
};

class CNP
{
public:
    bool operator()(TreeNode *const &a, TreeNode *const &b)const ;
};

class TreeNode
{
public:
    struct EvalParm {
        TetrisNode land_node;
        int clear = 0;
        double value = 0.;
        TetrisBot::Status status;
    };

    TreeNode() {}
    TreeNode(TreeContext *, TreeNode *, TetrisNode &, TetrisMap &, int, Piece, bool,  EvalParm &);
    void printInfoReverse(TetrisMap &, TreeNode *);
    void search(bool hold_opposite = false);
    void search_hold(bool op = false, bool noneFirstHold = false);
    bool eval();
    void  run();
    std::tuple<TetrisNode, bool, bool> getBest();

    TreeNode *parent = nullptr;  //父节点
    TreeContext *context = nullptr; //公用的层次对象
    QVector<TetrisNode>land_point; //落点
    QVector<Piece> *nexts = nullptr; //预览队列
    int nextIndex = 0; //第几个Next
    QVector<TreeNode *>children; //子节点
    TetrisNode node; //当前块
    TetrisMap map; //当前场地
    EvalParm evalParm; //落点信息
    bool extended = false, isHold = false, isHoldLock = false;
    Piece hold = Piece::None;
};

#endif // TETRISBOT_H
