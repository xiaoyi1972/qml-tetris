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
    switch (x) {
    case Piece::None:
        a = "None";
        break;
    case Piece::O:
        a = "O";
        break;
    case Piece::J:
        a = "J";
        break;
    case Piece::I:
        a = "I";
        break;
    case Piece::T:
        a = "T";
        break;
    case Piece::S:
        a = "S";
        break;
    case Piece::Z:
        a = "Z";
        break;
    case Piece::L:
        a = "L";
        break;
    case Piece::Trash:
        a = "transh";
        break;
    }
    return a;
}

QString Tool:: printPath(QVector<Oper> &path)
{
    QString a;
    for (auto x : path) {
        switch (x) {
        case Oper::Left:
            a += "l";
            break;
        case Oper::Right:
            a += "r";
            break;
        case Oper::SoftDrop:
            a += "d";
            break;
        case Oper::HardDrop:
            a += "V";
            break;
        case Oper::Hold:
            a += "v";
            break;
        case Oper::Cw:
            a += "z";
            break;
        case Oper::Ccw:
            a += "x";
            break;
        case Oper::DropToBottom:
            a += "D";
            break;
        default:
            a += "";
        }
    }
    return a;
}

QString Tool:: printOper(Oper & x){
    QString a = "a";
    switch (x) {
    case Oper::None:
        a = "None";
        break;
    case Oper::Left:
        a = "Left";
        break;
    case Oper::Right:
        a = "Right";
        break;
    case Oper::SoftDrop:
        a = "SoftDrop";
        break;
    case Oper::HardDrop:
        a = "HardDrop";
        break;
    case Oper::Hold:
        a = "Hold";
        break;
    case Oper::Cw:
        a = "Cw";
        break;
    case Oper::Ccw:
        a = "Ccw";
        break;
    case Oper::DropToBottom:
        a = "DropToBottom";
        break;
    }
    return a;
}

void Tool::printMap(TetrisMap &map)
{

    for (auto i = 0; i < map.height; i++) {
        QString str;
        for (auto j = 0; j < map.width; j++) {
            if (map(i, j)) {
                str += "[  ]";
            } else
                str += "     ";
        }
        if (map.data[i] > 0)
            qDebug() << str;
    }
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
    QVector<info> queue;
    QHash<info, int> results;
    queue.append(info{_first, 0});
    checked.insert(_first);
    while (queue.size() != 0) {
        auto [node, count] = queue.takeFirst();
        auto next = node;
        if (!TetrisNode::shift(next, _map, 0, 1)) {
            auto infoKey = info{next, count};
            if (results.contains(infoKey)) {
                auto keyCount = results.value(infoKey);
                if (keyCount > count) {
                    results.insert(infoKey, count);
                } else {
                    if (next.rotateState == _first.rotateState) {
                        results.erase(results.find(infoKey));
                        results.insert(infoKey, count);
                    }
                }
            } else {
                results.insert(infoKey, count);
            }
        }
        next = node;
        if (TetrisNode::shift(next, _map, -1, 0)) { //left
            if (!checked.contains(next)) {
                next.lastRotate = false;
                checked.insert(next);
                queue.append(info{std::move(next), count + 1});
            }
        }
        next = node;
        if (TetrisNode::shift(next, _map, 1, 0)) { //right
            if (!checked.contains(next)) {
                next.lastRotate = false;
                checked.insert(next);
                queue.append(info{std::move(next), count + 1});
            }
        }
        next = node;
        if (TetrisNode::rotate(next, _map, true)) { //cw
            if (!checked.contains(next)) {
                next.lastRotate = true;
                checked.insert(next);
                queue.append(info{std::move(next), count + 1});
            }
        }
        next = node;
        if (TetrisNode::rotate(next, _map, false)) { //ccw
            if (!checked.contains(next)) {
                next.lastRotate = true;
                checked.insert(next);
                queue.append(info{std::move(next), count + 1});
            }
        }
        next = node;
        next.shift(0, next.getDrop(_map)); //软降
        if (!checked.contains(next)) {
            next.lastRotate = false;
            checked.insert(next);
            queue.append(info{std::move(next), count + 1});
        }
    }
    QVector<TetrisNode> result;
    for (auto &x : results.keys()) {
        auto [node, count] = x;
        if (_first.type == Piece::T) {
            auto [spin, mini] = node.corner3(_map);
            node.mini = mini;
            node.spin = spin;
        }
        result.append(node);
    }
    return result;
}

auto TetrisBot::make_path(TetrisNode &start_node, TetrisNode &land_point, TetrisMap &map, bool NoSoftToBottom = false)->QVector<Oper>
{
    QVector<TetrisNode> node_search;
    QHash<TetrisNode, std::tuple<TetrisNode, Oper>>node_mark;
    auto &land_node = land_point;

    auto mark = [&](TetrisNode & key, std::tuple<TetrisNode, Oper> value) {
        if (!node_mark.contains(key)) {
            node_mark.insert(key, value);
            return true;
        } else
            return false;
    };

    auto build_path = [&](TetrisNode & lp) {
        QVector<Oper> path;
        auto node = lp;
        while (true) {
            if (!node_mark.contains(node))
                break;
            auto result = node_mark.value(node);
            auto next = std::get<0>(result);
            auto dropToSd = (path.size() > 0 && std::get<1>(result) == Oper::DropToBottom);
            auto softDropDis = node.pos.y - next.pos.y;
            node = next;
            if (node.type == Piece::None) {
                break;
            }
            path.push_front(std::get<1>(result));
            if (dropToSd && NoSoftToBottom) {
                //    qDebug()<<"youle";
                path.remove(0);
                path.insert(0, softDropDis, Oper::SoftDrop);
            }
        }
        while (path.size() != 0 && (path[path.size() - 1] == Oper::SoftDrop || path[path.size() - 1] == Oper::DropToBottom)) {
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
    while (node_search.size() != 0) {
        auto next = node_search.takeFirst();
        auto node = next;
        if (disable_d) {
            auto node_D = node.drop(map);
            if (mark(node_D, std::pair<TetrisNode, Oper> { next, Oper::DropToBottom })) {
                if (node_D == land_node) {
                    return build_path(land_node);
                }
            }
        }

        //逆时针旋转
        node = next;
        if (TetrisNode::rotate(node, map, true) && mark(node, std::pair<TetrisNode, Oper> { next, Oper::Cw })) {
            if (node == land_node)
                return build_path(land_node);
            else
                node_search.append(node);
        }

        //顺时针旋转
        node = next;
        if (TetrisNode::rotate(node, map, false) && mark(node, std::pair<TetrisNode, Oper> { next, Oper::Ccw })) {
            if (node == land_node)
                return build_path(land_node);
            else
                node_search.append(node);
        }

        //左
        node = next;
        if (TetrisNode::shift(node, map, -1, 0) && mark(node, std::pair<TetrisNode, Oper> { next, Oper::Left })) {
            if (node == land_node)
                return build_path(land_node);
            else
                node_search.append(node);
        }

        //右
        node = next;
        if (TetrisNode::shift(node, map, 1, 0) && mark(node, std::pair<TetrisNode, Oper> {next, Oper::Right })) {
            if (node == land_node)
                return build_path(land_node);
            else
                node_search.append(node);
        }

        //软降到底
        if (!disable_d) {
            node = next;
            auto node_D = node.drop(map);
            if (mark(node_D, std::pair<TetrisNode, Oper> { next, Oper::DropToBottom })) {
                if (node_D == land_node)
                    return build_path(land_node);
                else
                    node_search.append(node_D);
            }
        }
    }
    return QVector<Oper> {};
}

TetrisBot::EvalResult TetrisBot::evalute(TetrisNode &lp, TetrisMap &map, int clear)
{
    TetrisBot::EvalResult evalResult;
    if (lp.lastRotate && lp.type == Piece::T) {
        if (clear > 0 && lp.lastRotate) {
            if (clear == 1 && lp.mini && lp.spin) {
                lp.typeTspin = TspinType::TspinMini;
            } else if (lp.spin) {
                lp.typeTspin = TspinType::Tspin;
            } else {
                lp.typeTspin = TspinType::None;
            }
        }
    }

    auto eval_landpoint = [&]() {
        auto LandHeight = map.height - map.roof;
        auto Middle = qAbs((lp.pos.x + 1) * 2 - map.width);
        int value = 0
                    - LandHeight / map.height * 40
                    + Middle * 0.2
                    + (clear * 6);
        return value;
    };

    auto  eval_map = [&]() {
        struct {
            int colTrans;
            int rowTrans;
            int holes;
            int holeLines;
            int wellDepth;
            int holeDepth;
            int wellNum[32];
            int holeNum[32];
            int roof;
            int holePosyIndex;
            int clearWidth[40];
        } m;
        memset(&m, 0, sizeof(m));
        m.roof = map.height - map.roof;
        //行列变换
        for (auto j = (map.roof == 0 ?  0 : map.roof - 1); j < map.height; j++) {
            auto ct = (map.data[j] ^
                       (map.data[j] & (1 << (map.width - 1)) ? (map.data[j] >> 1) | (1 << (map.width - 1)) : map.data[j] >> 1));
            m.colTrans += BitCount(ct);
            if (j < map.height - 1) {
                auto rt = (map.data[j] ^ map.data[j + 1]);
                m.rowTrans += BitCount(rt);
            }
        }

        //洞井
        auto LineCoverBits = 0, width_l1 = map.width - 1;
        for (auto y = map.roof; y < map.height; y++) {
            LineCoverBits |= map.data[y];
            auto LineHole = LineCoverBits ^ map.data[y];
            if (LineHole != 0) {
                m.holes += BitCount(LineHole);
                m.holeLines++;
                m.clearWidth[m.holePosyIndex] = 0;
                for (auto hy = y - 1; hy >= map.roof; hy--) {
                    auto CheckLine = LineHole & map.data[hy];
                    if (CheckLine == 0) {
                        break;
                    }
                    m.clearWidth[m.holePosyIndex] += (map.height - (hy - 1)) * BitCount(CheckLine);
                }
                m.holePosyIndex++;
            }

            for (auto x = 0; x < map.width; x++) {
                if ((LineHole >> x) & 1) {
                    m.holeDepth += ++m.holeNum[x];
                } else {
                    m.holeNum[x] = 0;
                }
                auto ifWell = x == 0 ? (LineCoverBits & 3) == 2 :
                              (x == width_l1 ? ((LineCoverBits >> (width_l1 - 1)) & 3) == 1 : ((LineCoverBits >> (x - 1)) & 7) == 5);
                if (ifWell)
                    m.wellDepth += ++m.wellNum[x];
            }
        }

        auto value = (0.
                      - m.roof * 128
                      - m.colTrans  * 120
                      - m.rowTrans * 120
                      // - m.holes * 60
                      - m.holeLines * 380
                      - m.wellDepth * 100
                      - m.holeDepth * 40
                     );

        double rate = 32 / 5., mul = 1.0 / 4;
        for (auto i = 0; i < m.holePosyIndex; ++i, rate *= mul) {
            value -= m.clearWidth[i] * rate;
        }

        return value;
    };


    auto tspinDetect = [&]() {
        bool finding2 = true;
        bool finding3 = true;
        for (int y = map.roof; (finding2 || finding3) && y < map.height; ++y) {
            auto y0 = map.data[y];
            auto y1 = y - 1 > -1 ? map.data[y - 1] : 0;
            auto y2 = y - 2 > -1 ? map.data[y - 2] : 0;
            auto y3 = y - 3 > -1 ? map.data[y - 3] : 0;
            auto y4 = y - 4 > -1 ? map.data[y - 4] : 0;
            for (int x = 0; finding2 && x < map.width ; ++x) {
                if (((y0 >> x & 7) == 5) && ((y1 >> x & 7) == 0)) {
                    if (BitCount(y0) == map.width - 1) {
                        evalResult.t2Value += 1;
                        if (BitCount(y1) == map.width - 3) {
                            evalResult.t2Value += 2;
                            if (((y2 >> x & 7) == 4 || (y2 >> x & 7) == 1)) {
                                evalResult.t2Value += 2;
                                finding2 = false;
                            }
                        }
                    }
                }
            }
            for (int x = 0; finding3 && x < map.width - 2; ++x) {
                if ((y0 >> x & 7) == 3 && (y1 >> x & 7) == 1 && x < map.width - 3) {
                    auto value = 0;
                    if (BitCount(y0) == map.width - 1) {
                        //  qDebug()<<"l";
                        value += 1;
                        if (BitCount(y1) == map.width - 2) {
                            value += 1;
                            if ((y2 >> x & 7) == 3 && (y3 >> x & 7) == 0) {
                                value += 2;
                                if (BitCount(y2) == map.width - 1) {
                                    value += 2;
                                    if ((y3 >> x & 7) == 0) {
                                        value += 1;
                                        if ((y4 >> x & 7) == 4) {
                                            value += 1;
                                        } else {
                                            value -= 2;
                                        }
                                    } else
                                        value = 0;
                                }
                            }
                        }
                    }
                    if (value > 3)
                        finding3 = false;
                    evalResult.t3Value += value;
                }

                else if ((y0 >> x & 7) == 6 && (y1 >> x & 7) == 4 && x > 0) {
                    auto value = 0;
                    if (BitCount(y0) == map.width - 1) {
                        //     qDebug()<<"r";
                        value += 1;
                        if (BitCount(y1) == map.width - 2) {
                            value += 1;
                            if ((y2 >> x & 7) == 6 && (y3 >> x & 7) == 0) {
                                value += 2;
                                if (BitCount(y2) == map.width - 1) {
                                    value += 2;
                                    if ((y3 >> x & 7) == 0) {
                                        value += 1;
                                        if ((y4 >> x & 7) == 1) {
                                            value += 1;
                                        } else {
                                            value -= 2;
                                        }
                                    } else
                                        value = 0;
                                }
                            }
                        }
                    }
                    if (value > 3)
                        finding3 = false;
                    evalResult.t3Value += value;
                }
            }
        }
        return 0;
    };

    evalResult.value = eval_map() +  tspinDetect();
    //qDebug() << evalResult.value;
    evalResult.clear = clear;
    evalResult.typeTspin = lp.typeTspin;
    evalResult.safe = 999;
    evalResult.count = map.count;
    return evalResult;
}


TetrisBot::Status TetrisBot::get(const TetrisBot::EvalResult &evalResult, TetrisBot::Status &status, Piece hold, QVector<Piece> *next, int depth)
{
    QVector<int> comboTable{0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5, -1};
    int tableMax = 14;
    auto full_count_ = 24 * 10;
    auto result = status;
    result.value += evalResult.value;
    result.mapRise = 0;
    if (evalResult.safe <= 0) {
        result.value -= 99999;
    }

    switch (evalResult.clear) {
    case 0:
        if (status.combo > 0 && status.combo < 3) {
            result.like -= 2;
        }
        result.combo = 0;
        if (status.underAttack > 0) {
            result.mapRise += std::max(0, qAbs(status.underAttack - status.attack));
            if (result.mapRise >= evalResult.safe) {
                result.value -= 99999;
            }
            result.underAttack = 0;
        }
        result.underAttack = 0;
        break;
    case 1:
        if (evalResult.typeTspin == TspinType::TspinMini)
            result.attack += status.b2b ? 2 : 1;
        else if (evalResult.typeTspin == TspinType::Tspin)
            result.attack += status.b2b ? 3 : 2;
        result.attack += comboTable[std::min(tableMax - 1, ++result.combo)];
        result.b2b = evalResult.typeTspin != TspinType::None;
        break;
    case 2:
        result.attack += comboTable[std::min(tableMax - 1, ++result.combo)];
        if (evalResult.typeTspin != TspinType::None) {
            result.like += 8;
            result.attack += status.b2b ? 5 : 4;
        } else
            result.attack += 1;
        result.b2b = evalResult.typeTspin != TspinType::None;
        break;
    case 3:
        result.attack += comboTable[std::min(tableMax - 1, ++result.combo)] ;
        if (evalResult.typeTspin != TspinType::None) {
            result.like += 12;
            result.attack += status.b2b ? 8 : 6;
        } else
            result.attack +=  2;
        result.b2b = evalResult.typeTspin != TspinType::None;
        break;
    case 4:
        result.like += 8;
        result.attack += comboTable[std::min(tableMax - 1, ++result.combo)] + (status.b2b ? 5 : 4);
        result.b2b = true;
        break;
    }
    if (result.combo < 5) {
        result.like -= 1.5 * result.combo;
    }
    if (evalResult.count == 0 && result.mapRise == 0) {
        result.like += 20;
        result.attack += 6;
    }
    if (status.b2b && !result.b2b) {
        result.like -= 2;
    }

    auto t_expect = [&]() {
        if (hold == Piece::T)
            return 0;
        for (auto i = depth - 1; i < next->size(); ++i) {
            if (next->at(i) == Piece::T)
                return (i - (depth - 1));
        }
        return 14;
    };

    switch (hold) {
    case Piece::T:
        if (evalResult.typeTspin == TspinType::None)
            result.like += 4;
        break;
    case Piece::I:
        if (evalResult.clear != 4)
            result.like += 2;
        break;
    default: break;
    }

    double rate = 1. / (depth) + 3;
    result.maxCombo = std::max(result.combo, result.maxCombo);
    result.maxAttack = std::max(result.attack, result.maxAttack);
    result.value += ((
                             0.
                             + result.maxAttack * 40
                             + result.attack * 256  * rate
                             + (evalResult.t2Value) * (t_expect() < 8 ? 512 : 320) * 1.5
                             + (evalResult.safe >= 12 ? evalResult.t3Value * (t_expect() < 4 ? 10 : 8) * (result.b2b ? 512 : 256) / (6 + result.underAttack) : 0)
                             + (result.b2b ? 512 : 0)
                             + result.like * 64
                     ) * std::max<double>(0.05, (full_count_ - evalResult.count - result.mapRise * 10) / double(full_count_))
                     + result.maxCombo * (result.maxCombo - 1) * 40);
    return result;
}

TreeContext::~TreeContext()
{
    treeDelete(root);
    return;
}

bool CNP::operator()(TreeNode *const &a, TreeNode *const &b) const
{
    return a->evalParm.value < b->evalParm.value;
    /*if (a->evalParm.value != b->evalParm.value)
     return a->evalParm.value < b->evalParm.value;
     else
     {
     return a->evalParm.land_node > b->evalParm.land_node;
     }*/
}

void TreeContext::createRoot(TetrisNode &_node, TetrisMap &_map, QVector<Piece> &dp, Piece _hold, int _b2b, int _combo)
{
    nexts = dp;
    noneHoldFirstNexts = dp.mid(1, -1);
    level.resize(dp.size() + 1);
    extendedLevel.resize(dp.size() + 1);
    TreeNode *tree = new TreeNode;
    tree->context = this;
    tree->nexts = &nexts;
    tree->node = _node;
    tree->map = _map;
    tree->hold = _hold;
    tree->evalParm.status.b2b = _b2b;
    tree->evalParm.status.combo = _combo;
    root = tree;
}

void TreeContext::treeDelete(TreeNode *t)
{
    if (t == nullptr) {
        return;
    }
    for (auto i = 0; i < t->children.size(); i++) {
        if (t->children[i] != nullptr) {
            treeDelete(t->children[i]);
        }
    }
    delete t;
}

void TreeContext::run()
{
    if (root != nullptr) {
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
    nexts = _parent->nexts;
    nextIndex = _nextIndex;
    hold = _hold;
    isHold = _isHold;
    evalParm = _evalParm;
}

void TreeNode::printInfoReverse(TetrisMap &map_,    TreeNode *_test = nullptr)
{
    TreeNode *test = _test == nullptr ? this : _test;
    context->test = true;
    while (test != nullptr) {
        qDebug() << Tool::printType(test->node.type);
        Tool::printMap(test->map);
        test = test->parent;
    }
}

void  TreeNode::search(bool hold_opposite)
{
    if (context->isOpenHold && !isHoldLock) {
        search_hold();
        return;
    }

    if (land_point.size() == 0) {
        land_point.append(node);
        auto land_points = TetrisBot::search(node, map);
        for (auto &i_node : land_points) {
            auto next_node = TetrisNode{(nextIndex == nexts->size() ? Piece::None : nexts->at(nextIndex))};
            auto map_ = map;
            auto clear = i_node.attach(map_);
            auto  status_ = TetrisBot::get(TetrisBot::evalute(i_node, map_, clear), this->evalParm.status, hold, &context->nexts, nextIndex + 1);
            EvalParm evalParm_ = { i_node, clear, status_.value, status_};
            auto *new_tree = new TreeNode{context, this, next_node, map_, nextIndex + 1, hold, bool(false ^ hold_opposite), evalParm_};
            new_tree->isHoldLock = isHoldLock;
            children.append(new_tree);
        }
    }
}

void TreeNode::search_hold(bool op, bool noneFirstHold)
{
    if (hold == Piece::None || nexts->size() == 0) {
        auto  hold_save = hold;
        auto nexts_save = nexts->takeFirst();
        if (hold == Piece::None) {
            hold = node.type;
            node = TetrisNode{nexts_save};
            search_hold(true, true);
        }
        node = TetrisNode{hold};
        hold = hold_save;
        nexts->push_front(nexts_save);
        return;
    }
    if (node.type == hold) {
        if (land_point.size() == 0) {
            land_point.append(node);
            auto land_points = TetrisBot::search(node, map);
            for (auto &i_node : land_points) {
                auto next_node = TetrisNode{(nextIndex == nexts->size() ? Piece::None : nexts->at(nextIndex))};
                auto map_ = map;
                auto clear = i_node.attach(map_);
                auto  status_ = TetrisBot::get(TetrisBot::evalute(i_node, map_, clear), this->evalParm.status, hold, &context->nexts, nextIndex + 1);
                EvalParm evalParm_ = { i_node, clear, status_.value, status_};
                auto *new_tree = new TreeNode{context, this, next_node, map_, nextIndex + 1, hold, bool(false), evalParm_};
                new_tree->isHoldLock = true;
                children.append(new_tree);
            }
        }
    } else if (node.type != hold) {
        auto hold_node = TetrisNode{hold};
        if (land_point.size() == 0) {
            land_point.append(node);
            land_point.append(hold_node);
            auto land_points = TetrisBot::search(node, map);
            for (auto &i_node : land_points) {
                auto next_node = TetrisNode{(nextIndex == nexts->size() ? Piece::None : nexts->at(nextIndex))};
                auto map_ = map;
                auto clear = i_node.attach(map_);
                auto  status_ = TetrisBot::get(TetrisBot::evalute(i_node, map_, clear), this->evalParm.status, hold, &context->nexts, nextIndex + 1);
                EvalParm evalParm_ = { i_node, clear, status_.value, status_};
                auto *new_tree = new TreeNode{context, this, next_node, map_, nextIndex + 1, hold, bool(op ^ false), evalParm_};
                if (noneFirstHold)
                    new_tree->nexts = & context->noneHoldFirstNexts;
                new_tree->isHoldLock = true;
                children.append(new_tree);
            }

            auto hold_land_points = TetrisBot::search(hold_node, map);
            for (auto &i_node :  hold_land_points) {
                auto next_node = TetrisNode{(nextIndex == nexts->size() ? Piece::None : nexts->at(nextIndex))};
                auto map_ = map;
                auto clear = i_node.attach(map_);
                auto  status_ = TetrisBot::get(TetrisBot::evalute(i_node, map_, clear), this->evalParm.status, node.type, &context->nexts, nextIndex + 1);
                EvalParm evalParm_ = { i_node, clear, status_.value, status_};
                auto *new_tree = new TreeNode{context, this, next_node, map_, nextIndex + 1, node.type, bool(op ^ true), evalParm_};
                if (noneFirstHold)
                    new_tree->nexts = & context->noneHoldFirstNexts;
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
    if (children.size() == 0) {
        search();
        extended = true;
    }
    return true;
}

void  TreeNode::run()
{
    auto next_length_max = nexts->size();
    if (context->isOpenHold && hold == Piece::None)
        --next_length_max;
    auto  i = next_length_max;
    if (context->width == 0) {
        auto &firstLevel = context->level[next_length_max - 1];
        if (this->eval())
            for (auto child : this->children)
                firstLevel.push(child);
        context->width = 2;
    } else
        context->width += 1;

    auto prune_hold = ++context->width;
    auto prune_hold_max = prune_hold * 3;
    while (--i > 0) {
        auto level_prune_hold = qFloor((prune_hold_max) * (next_length_max - i) / next_length_max) + prune_hold;
        auto _deepIndex = i;
        auto &levelSets = context->level[_deepIndex];
        auto &nextLevelSets = context->level[_deepIndex - 1];
        auto &extendedLevelSets = context->extendedLevel[_deepIndex];
        QVector<TreeNode *> work, haole;
        for (auto pi = 0; !levelSets.empty() && pi < (level_prune_hold - extendedLevelSets.size()); pi++) {
            auto x = levelSets.top();
            work.append(x);
            levelSets.pop();
            extendedLevelSets.push(x);
        }

        auto mapFunc = [&](TreeNode  * tree) {
            tree->eval();
        };
        QtConcurrent::blockingMap(work, mapFunc);
        for (auto &i : work)
            for (auto &x : i->children) {
                nextLevelSets.push(x);
            }
    }
}

std::tuple<TetrisNode, bool, bool> TreeNode::getBest()
{
    TreeNode *best = nullptr;
    best = context->level.first().top();
    QVector<TreeNode *> record{best};
    //printInfoReverse(best->map, best);
    //  Tool::printMap(best->map);
    while (best->parent != nullptr) {
        record.push_front(best->parent);
        best = best->parent;
    }

    //qDebug("\n");
    // qDebug() << Tool::printNode(record[1]->evalParm.land_node);
    return {record[1]->evalParm.land_node, record[1]->isHold, context->test};
}



