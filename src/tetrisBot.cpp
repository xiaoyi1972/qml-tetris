#include "tetrisBot.h"
#include "tetris.h"
int BitCount(int n)
{
    // HD, Figure 5-2
    n = n - ((n >> 1) & 0x55555555);
    n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
    n = (n + (n >> 4)) & 0x0f0f0f0f;
    n = n + (n >> 8);
    n = n + (n >> 16);
    return n & 0x3f;
}

QString Tool:: printType(Piece x)
{
    QString a = "a";
    switch (x)
    {
    case Piece::None: a = "None"; break;
    case Piece::O: a = "O"; break;
    case Piece::J: a = "J"; break;
    case Piece::I: a = "I"; break;
    case Piece::T: a = "T"; break;
    case Piece::S: a = "S"; break;
    case Piece::Z: a = "Z"; break;
    case Piece::L: a = "L"; break;
    }
    return a;
}

QString Tool:: printPath(QVector<Oper> &path)
{
    QString a;
    for (auto x : path)
    {
    switch (x)
    {
    case Oper::Left: a += "l"; break;
    case Oper::Right: a += "r"; break;
    case Oper::SoftDrop: a += "d"; break;
    case Oper::HardDrop: a += "V"; break;
    case Oper::Hold: a += "v"; break;
    case Oper::Cw: a += "z"; break;
    case Oper::Ccw: a += "x"; break;
    case Oper::DropToBottom: a += "D"; break;
    }
    }
    return a;
}

void Tool::printMap(TetrisMap &map)
{

    for (auto i = 0; i < map.height; i++)
    {
    QString str;
    for (auto j = 0; j < map.width; j++)
    {
        if (map(i, j))
        {
        str += "[  ]";
        }
        else
        str += "<>";
    }
    if (map.data[i] > 0)
        qDebug() << str;
    }
//qDebug() << str;
}

QString Tool::printNode(TetrisNode &node)
{
    QString str;
    str += "pos [";
    str += QString::number(node.pos.x);
    str += " ";
    str += QString::number(node.pos.y);
    str += "] rs ";
    str += QString::number(node.rotateState);
    str += " type ";
    str += Tool::printType(node.type);
    return str;
    //  qDebug("pos [%d ,%d] rs %d", node.pos.x, node.pos.y, node.rotateState);
}

void Tool::sleepTo(int msec)
{
    QEventLoop eventloop;
    QTimer::singleShot(msec, &eventloop, SLOT(quit()));
    eventloop.exec();
}

QVector<TetrisNode> TetrisBot::search(TetrisNode &_first, TetrisMap &_map)
{
    QSet<TetrisNode> checked;
    QVector<TetrisNode> quene, result;
    quene.append(_first);
    checked.insert(_first);
    while (quene.size() != 0)
    {
    auto node = quene.takeFirst();
    auto next = node;
    if (!TetrisNode::shift(next, _map, 0, 1))
    {
        result.append(next);
    }
    next = node;
    if (TetrisNode::shift(next, _map, -1, 0)) //left
    {
        if (!checked.contains(next))
        {
        next.lastRotate = false;
        checked.insert(next);
        quene.append(next);
        }
    }
    next = node;
    if (TetrisNode::shift(next, _map, 1, 0)) //right
    {
        if (!checked.contains(next))
        {
        next.lastRotate = false;
        checked.insert(next);
        quene.append(next);
        }
    }
    next = node;
    if (TetrisNode::rotate(next, _map, true)) //cw
    {
        if (!checked.contains(next))
        {
        next.lastRotate = true;
        checked.insert(next);
        quene.append(next);
        }
    }
    next = node;
    if (TetrisNode::rotate(next, _map, false)) //ccw
    {
        if (!checked.contains(next))
        {
        next.lastRotate = true;
        checked.insert(next);
        quene.append(next);
        }
    }
    next = node;
    next.shift(0, next.getDrop(_map)); //软降
    if (!checked.contains(next))
    {
        next.lastRotate = false;
        checked.insert(next);
        quene.append(next);
    }
    }
    if (_first.type == Piece::T)
    for (auto &x : result)
    {
        auto [spin, mini] = x.corner3(_map);
        x.mini = mini;
        x.spin = spin;
    }
    return result;
}

auto TetrisBot::make_path(TetrisNode &start_node, TetrisNode &land_point, TetrisMap &map)->QVector<Oper>
{
    QVector<TetrisNode> node_search;
    QHash<TetrisNode, std::tuple<TetrisNode, Oper>>node_mark;
    auto &land_node = land_point;

    auto mark = [&](TetrisNode & key, std::tuple<TetrisNode, Oper> value)
    {
    if (!node_mark.contains(key))
    {
        node_mark.insert(key, value);
        return true;
    }
    else
        return false;
    };

    auto build_path = [&](TetrisNode & lp)
    {
    QVector<Oper> path;
    auto node = lp;
    while (true)
    {
        if (!node_mark.contains(node))
        break;
        auto result = node_mark.value(node);
        node = std::get<0>(result);
        if (node.type == Piece::None)
        {
        break;
        }
        path.push_front(std::get<1>(result));
    }
    while (path.size() != 0 && (path[path.size() - 1] == Oper::SoftDrop || path[path.size() - 1] == Oper::DropToBottom))
    {
        path.remove(path.size() - 1, 1);
    }
    path.push_back(Oper::HardDrop);
    return path;
    };

    node_search.append(start_node);
    node_mark.insert(start_node, std::pair<TetrisNode, Oper> {TetrisNode{Piece::None}, Oper::None });
    auto disable_d = land_node.open(map);
    if (land_node.type == Piece::T && land_node.typeTspin == TspinType::TspinMini)
    disable_d = false;
    while (node_search.size() != 0)
    {
    auto next = node_search.takeFirst();
    auto node = next;
    if (disable_d)
    {
        auto node_D = node.drop(map);
        if (mark(node_D, std::pair<TetrisNode, Oper> { next, Oper::DropToBottom }))
        {
        if (node_D == land_node)
        {
            return build_path(land_node);
        }
        }
    }

    //逆时针旋转
    node = next;
    if (TetrisNode::rotate(node, map, true) && mark(node, std::pair<TetrisNode, Oper> { next, Oper::Cw }))
    {
        if (node == land_node)
        return build_path(land_node);
        else
        node_search.append(node);
    }

    //顺时针旋转
    node = next;
    if (TetrisNode::rotate(node, map, false) && mark(node, std::pair<TetrisNode, Oper> { next, Oper::Ccw }))
    {
        if (node == land_node)
        return build_path(land_node);
        else
        node_search.append(node);
    }

    //左
    node = next;
    if (TetrisNode::shift(node, map, -1, 0) && mark(node, std::pair<TetrisNode, Oper> { next, Oper::Left }))
    {
        if (node == land_node)
        return build_path(land_node);
        else
        node_search.append(node);
    }

    //右
    node = next;
    if (TetrisNode::shift(node, map, 1, 0) && mark(node, std::pair<TetrisNode, Oper> {next, Oper::Right }))
    {
        if (node == land_node)
        return build_path(land_node);
        else
        node_search.append(node);
    }

    //D
    if (!disable_d)
    {
        node = next;
        auto node_D = node.drop(map);
        if (mark(node_D, std::pair<TetrisNode, Oper> { next, Oper::DropToBottom }))
        {
        if (node_D == land_node)
            return build_path(land_node);
        else
            node_search.append(node_D);
        }
    }
    }
    return QVector<Oper> {};
}

int TetrisBot::evalute(TetrisNode &lp, TetrisMap &map, int clear)
{
    if (lp.lastRotate && lp.type == Piece::T)
    {
    if (clear > 0 && lp.lastRotate)
    {
        if (clear == 1 && lp.mini)
        {
        lp.typeTspin = TspinType::TspinMini;
        }
        else if (lp.spin)
        {
        lp.typeTspin = TspinType::Tspin;
        }
        else
        {
        lp.typeTspin = TspinType::None;
        }
    }
    }


    auto eval_landpoint = [&]()
    {
    auto LandHeight = map.height - map.roof;
    auto Middle = qAbs((lp.pos.x + 1) * 2 - map.width);

    int value = 0
            - LandHeight / map.height * 40
            + Middle * 0.2
            + (clear * 6);

    if (map.count == 0)
    {
        value += 99999999;
    }
    return value;
    };

    auto  eval_map = [&]()
    {
    struct
    {
        int colTrans;
        int rowTrans;
        int holes;
        int holeLines;
        int clearWidth;
        int wellDepth;
        int holeDepth;
        int wellNum[32];
        int holeNum[32];
        int roof;
    } m;
    memset(&m, 0, sizeof(m));
    m.roof = map.height - map.roof;
    //行列变换
    for (auto j = (map.roof == 0 ?  0 : map.roof - 1); j < map.height; j++)
    {
        auto ct = (map.data[j] ^
               (map.data[j] & (1 << (map.width - 1)) ? (map.data[j] >> 1) | (1 << (map.width - 1)) : map.data[j] >> 1));
        m.colTrans += BitCount(ct);
        if (j < map.height - 1)
        {
        auto rt = (map.data[j] ^ map.data[j + 1]);
        m.rowTrans += BitCount(rt);
        }
    }

    //洞井
    auto LineCoverBits = 0, width_l1 = map.width - 1;
    for (auto y = map.roof; y < map.height; y++)
    {
        LineCoverBits |= map.data[y];
        auto LineHole = LineCoverBits ^ map.data[y];
        if (LineHole != 0)
        {
        m.holes += BitCount(LineHole);
        m.holeLines++;
        for (auto hy = y - 1; hy >= map.roof; hy--)
        {
            auto CheckLine = LineHole & map.data[hy];
            if (CheckLine == 0)
            {
            break;
            }
            m.clearWidth += BitCount(CheckLine);
        }
        }

        for (auto x = 0; x < map.width; x++)
        {
        if ((LineHole >> x) & 1)
        {
            m.holeDepth += ++m.holeNum[x];
        }
        else
        {
            m.holeNum[x] = 0;
        }
        auto ifWell = x == 0 ? (LineCoverBits & 3) == 2 :
                  (x == width_l1 ? ((LineCoverBits >> (width_l1 - 1)) & 3) == 1 : ((LineCoverBits >> (x - 1)) & 7) == 5);
        if (ifWell)
            m.wellDepth += ++m.wellNum[x];
        }
    }

    auto value = (0.
              - m.roof * 10
              - m.colTrans * 16
              - m.rowTrans * 13
              - m.holes * 6
              - m.holeLines * 38
              - m.wellDepth * 10
              - m.holeDepth * 4
              - m.clearWidth * 3
             );

    if(lp.typeTspin == TspinType::TspinMini)
        value+=9999999;

    return value;
    };


    auto value = eval_landpoint() + eval_map();
    return value;
}

TreeContext::~TreeContext()
{
    treeDelete(root);
    return;
}

bool CNP::operator()(TreeNode *const &a, TreeNode *const &b) const
{
    if (a->evalParm.value != b->evalParm.value)
    return a->evalParm.value < b->evalParm.value;
    else
    {
    return a->evalParm.land_node > b->evalParm.land_node;
    }
}

void TreeContext::createRoot(TetrisNode &_node, TetrisMap &_map, QVector<Piece> &dp, Piece _hold)
{
    nexts = dp;
    level.resize(dp.size() + 1);
    extendedLevel.resize(dp.size() + 1);
    TreeNode *tree = new TreeNode;
    tree->context = this;
    tree->nexts = &nexts;
    tree->node = _node;
    tree->map = _map;
    tree->hold = _hold;
    root = tree;
}

void TreeContext::treeDelete(TreeNode *t)
{
    if (t == nullptr)
    {
    return;
    }
    for (auto i = 0; i < t->children.size(); i++)
    {
    if (t->children[i])
    {
        treeDelete(t->children[i]);
    }
    }
    delete t;
}

void TreeContext::run()
{
    if (root != nullptr)
    {
    root->run();
    }
}

std::tuple<TetrisNode, bool, bool> TreeContext::getBest()
{
    return root->getBest();
}

TreeNode::TreeNode(TreeContext *_ctx, TreeNode *_parent, TetrisNode &_node, TetrisMap &_map,
           int _nextIndex, Piece _hold, bool _isHold,  EvalParm &_evalParm)
{
    context = _ctx;
    parent = _parent;
    node = _node;
    map = _map;
    nexts = &context->nexts;
    nextIndex = _nextIndex;
    hold = _hold;
    isHold = _isHold;
    evalParm = _evalParm;
}

void TreeNode::printInfoReverse(TetrisMap &map_)
{
    return;
    if (map_.count == 0)
    {
    TreeNode *test = this;
    context->test = true;
    while (test != nullptr)
    {
        qDebug() << Tool::printType(test->node.type);
        Tool::printMap(test->map);
        test = test->parent;
    }
    Tool::sleepTo(10);
    }
}

void  TreeNode::search(bool hold_opposite)
{
    if (context->isOpenHold && !isHoldLock)
    {
    search_hold();
    return;
    }
    if (land_point.size() == 0)
    {
    land_point.append(node);
    auto land_points = TetrisBot::search(node, map);
    for (auto &i_node : land_points)
    {
        auto next_node = TetrisNode{(nextIndex == nexts->size() ? Piece::None : nexts->at(nextIndex))};
        auto map_ = map;
        auto clear = i_node.attach(map_);
        printInfoReverse(map_);
        auto value = TetrisBot::evalute(i_node, map_, clear) + evalParm.value;
        EvalParm evalParm_ = { i_node, clear, value};
        auto *new_tree = new TreeNode{context, this, next_node, map_, nextIndex + 1, hold, bool(false ^ hold_opposite), evalParm_};
        new_tree->isHoldLock = isHoldLock;
        children.append(new_tree);
    }
    }
}

void TreeNode::search_hold(bool op)
{
    if (hold == Piece::None || nexts->size() == 0)
    {
    auto  hold_save = hold;
    auto nexts_save = nexts;
    if (hold == Piece::None)
    {
        hold = node.type;
        node = TetrisNode{nexts->takeFirst()};
        search_hold(true);
    }
    hold = hold_save;
    nexts = nexts_save;
    return;
    }
    if (node.type == hold)
    {
    if (land_point.size() == 0)
    {
        land_point.append(node);
        auto land_points = TetrisBot::search(node, map);
        for (auto &i_node : land_points)
        {
        auto next_node = TetrisNode{(nextIndex == nexts->size() ? Piece::None : nexts->at(nextIndex))};
        auto map_ = map;
        auto clear = i_node.attach(map_);
        printInfoReverse(map_);
        auto value = TetrisBot::evalute(i_node, map_, clear) + evalParm.value;
        EvalParm evalParm_ = { i_node, clear, value};
        auto *new_tree = new TreeNode{context, this, next_node, map_, nextIndex + 1, hold, bool(false), evalParm_};
        new_tree->isHoldLock = true;
        children.append(new_tree);
        }
    }
    }
    else if (node.type != hold)
    {
    auto hold_node = TetrisNode{hold};
    if (land_point.size() == 0)
    {
        land_point.append(node);
        land_point.append(hold_node);
        auto land_points = TetrisBot::search(node, map);
        for (auto &i_node : land_points)
        {
        auto next_node = TetrisNode{(nextIndex == nexts->size() ? Piece::None : nexts->at(nextIndex))};
        auto map_ = map;
        auto clear = i_node.attach(map_);
        printInfoReverse(map_);
        auto value = TetrisBot::evalute(i_node, map_, clear) + evalParm.value;
        EvalParm evalParm_ = { i_node, clear, value};
        auto *new_tree = new TreeNode{context, this, next_node, map_, nextIndex + 1, hold, bool(op ^ false), evalParm_};
        new_tree->isHoldLock = true;
        children.append(new_tree);
        }

        auto hold_land_points = TetrisBot::search(hold_node, map);
        for (auto &i_node :  hold_land_points)
        {
        auto next_node = TetrisNode{(nextIndex == nexts->size() ? Piece::None : nexts->at(nextIndex))};
        auto map_ = map;
        auto clear = i_node.attach(map_);
        printInfoReverse(map_);
        auto value = TetrisBot::evalute(i_node, map_, clear) + evalParm.value;
        EvalParm evalParm_ = { i_node, clear, value};
        auto *new_tree = new TreeNode{context, this, next_node, map_, nextIndex + 1, hold, bool(op ^ true), evalParm_};
        new_tree->isHoldLock = true;
        children.append(new_tree);
        }
    }
    }
}

bool TreeNode::eval()
{
    if (extended == true)
    return false;
    if (children.size() == 0)
    {
    search();
    extended = true;
    }
    return true;
}

void  TreeNode::run()
{
    auto next_length_max = nexts->size();
    if (context->isOpenHold && hold == Piece::None)
    {
    --next_length_max;
    }
    auto  i = next_length_max;
    if (context->width == 0)
    {
    auto &firstLevel = context->level[next_length_max - 1];
    if (this->eval())
        for (auto child : this->children)
        firstLevel.push(child);
    }
    auto prune_hold = ++context->width;
    auto prune_hold_max = prune_hold * 3;
    while (--i > 0)
    {
    auto level_prune_hold = qFloor((prune_hold_max) * (i) / next_length_max) + prune_hold;
    auto _deepIndex = i;
    auto &levelSets = context->level[_deepIndex];
    auto &nextLevelSets = context->level[_deepIndex - 1];
    auto &extendedLevelSets = context->extendedLevel[_deepIndex];
    QVector<TreeNode *> work, haole;
    for (auto pi = 0; !levelSets.empty() && pi < level_prune_hold; pi++)
    {
        auto x = levelSets.top();
        work.append(x);
        levelSets.pop();
        extendedLevelSets.push(x);
    }
    auto mapFunc = [&](TreeNode  * tree)
    {
        tree->eval();
    };
    QtConcurrent::blockingMap(work, mapFunc);
    for (auto &i : work)
        for (auto &x : i->children)
        {
        nextLevelSets.push(x);
        }
    }
}

std::tuple<TetrisNode, bool, bool> TreeNode::getBest()
{
    TreeNode *best = nullptr;
    auto &wait = context->level;
    auto &sort = context->extendedLevel;
    for (auto i = 0; i < wait.size() && i < sort.size(); ++i)
    {
    auto wait_best = (wait[i].empty()) ? nullptr : wait[i].top();
    auto sort_best = (sort[i].empty()) ? nullptr : sort[i].top();
    if (wait_best == nullptr)
    {
        if (sort_best == nullptr)
        continue;
        else
        best = sort_best;
    }
    else
    {
        if (sort_best == nullptr)
        best = wait_best;
        else
        best = sort_best->evalParm.value < wait_best->evalParm.value ? wait_best : sort_best;
    }
    break;
    }
    QVector<TreeNode *> record{best};
    while (best->parent != nullptr)
    {
    record.push_front(best->parent);
    best = best->parent;
    }
    return {record[1]->evalParm.land_node, record[1]->isHold, context->test};
}



