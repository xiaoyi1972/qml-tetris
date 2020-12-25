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

template <class T, class Cmp>
struct stable_element
{
    stable_element(T &&o, std::size_t c)
    : object_(std::move(o))
    , insertion_order_(c)
    {
    }
    stable_element(const T &o, std::size_t c)
    : object_(o)
    , insertion_order_(c)
    {
    }
    operator T() { return object_; }

    T object_;
    Cmp cmp;
    std::size_t insertion_order_;
};

template <class T, class Cmp>
bool operator<(const stable_element<T, Cmp> &lhs, const stable_element<T, Cmp> &rhs)
{
    return lhs.cmp(lhs.object_, rhs.object_) || (!lhs.cmp(rhs.object_, lhs.object_) && (rhs.insertion_order_ < lhs.insertion_order_));
}

template <class T, class Cmp,
      class Container = std::vector<stable_element<T, Cmp>>,
      class Compare = std::less<typename Container::value_type>>
class stable_priority_queue : public std::priority_queue<stable_element<T, Cmp>, Container, Compare>
{
    using stableT = stable_element<T, Cmp>;
    using std::priority_queue<stableT, Container, Compare>::priority_queue;
public:
    const T &top() { return this->c.front().object_; }
    void push(const T &value)
    {
    this->c.push_back(stableT(value, counter_++));
    std::push_heap(this->c.begin(), this->c.end(), this->comp);
    }
    void push(T &&value)
    {
    this->c.push_back(stableT(std::move(value), counter_++));
    std::push_heap(this->c.begin(), this->c.end(), this->comp);
    }
    template<class ... Args>
    void emplace(Args &&... args)
    {
    this->c.emplace_back(T(std::forward<Args>(args)...), counter_++);
    std::push_heap(this->c.begin(), this->c.end(), this->comp);
    }
    void pop()
    {
    std::pop_heap(this->c.begin(), this->c.end(), this->comp);
    this->c.pop_back();
    if (this->empty())
        counter_ = 0;
    }

protected:
    std::size_t counter_ = 0;
};

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
QVector<TetrisNode> search(TetrisNode &, TetrisMap &);
auto make_path(TetrisNode &, TetrisNode &, TetrisMap &)->QVector<Oper>;
int evalute(TetrisNode &, TetrisMap &, int);
}

class TreeNode;
class CNP;
class TreeContext
{
public:
    ~TreeContext();
    void createRoot(TetrisNode &, TetrisMap &, QVector<Piece> &, Piece);
    void treeDelete(TreeNode *);
    void run();
    std::tuple<TetrisNode, bool, bool>getBest();
private:
    friend class TreeNode;
    QVector < stable_priority_queue<TreeNode *, CNP>>level; //保存每一个等级层
    QVector < stable_priority_queue<TreeNode *, CNP>>extendedLevel; //保存每一个等级层
    QVector<Piece>nexts; //预览队列
    int width = 0;
    bool isOpenHold = true;
    TreeNode *root = nullptr;
    bool test = false;
};

class CNP
{
public:
    bool operator()(TreeNode *const &a, TreeNode *const &b)const ;
};

class TreeNode
{
public:
    struct EvalParm
    {
    TetrisNode land_node;
    int clear = 0;
    int value = 0;
    };

    TreeNode() {}
    TreeNode(TreeContext *, TreeNode *, TetrisNode &, TetrisMap &, int, Piece, bool,  EvalParm &);
    void printInfoReverse(TetrisMap &);
    void search(bool hold_opposite = false);
    void search_hold(bool op = false);
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
