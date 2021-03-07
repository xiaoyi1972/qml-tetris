#include "tetrisBot.h"
#include "tetris.h"

int BitCount(int n)
{
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

QString Tool:: printOper(Oper &x)
{
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
    auto softDropDisable = true;
    QSet<TetrisNode> checked;
    QVector<searchNode> queue;
    QHash<searchNode, int> results;
    queue.append(searchNode{_first, 0});
    checked.insert(_first);
    while (queue.size() != 0) {
        auto [node, count] = queue.takeFirst();

        if (auto next = node; !TetrisNode::shift(next, _map, 0, 1)) {
            auto infoKey = searchNode{std::move(next), count};
            if (results.contains(infoKey)) {
                if (auto keyCount = results.value(infoKey); keyCount > count) {
                    results.insert(infoKey, count);
                } else if (next.rotateState == _first.rotateState) {
                    results.erase(results.find(infoKey));
                    results.insert(infoKey, count);
                }
            } else {
                results.insert(infoKey, count);
            }
        }

        if (auto next = node; TetrisNode::shift(next, _map, -1, 0)) { //左
            if (!checked.contains(next)) {
                next.lastRotate = false;
                checked.insert(next);
                queue.append(searchNode{std::move(next), count + 1});
            }
        }

        if (auto next = node; TetrisNode::shift(next, _map, 1, 0)) { //右
            if (!checked.contains(next)) {
                next.lastRotate = false;
                checked.insert(next);
                queue.append(searchNode{std::move(next), count + 1});
            }
        }

        if (auto next = node; TetrisNode::rotate(next, _map, true)) { //逆时针旋转
            if (!checked.contains(next)) {
                next.lastRotate = true;
                checked.insert(next);
                queue.append(searchNode{std::move(next), count + 1});
            }
        }

        if (auto next = node; TetrisNode::rotate(next, _map, false)) { //顺时针旋转
            if (!checked.contains(next)) {
                next.lastRotate = true;
                checked.insert(next);
                queue.append(searchNode{std::move(next), count + 1});
            }
        }

        if (auto next = node.drop(_map); !checked.contains(next)) { //软降到底
            if (!checked.contains(next)) {
                next.lastRotate = false;
                checked.insert(next);
                queue.append(searchNode{std::move(next), count + 1});
            }
        }

        if (softDropDisable) {
            if (auto next = node; TetrisNode::shift(next, _map, 0, 1)) { //单次软降
                if (!checked.contains(next)) {
                    next.lastRotate = false;
                    checked.insert(next);
                    queue.append(searchNode{std::move(next), count + 1});
                }
            }
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

auto TetrisBot::make_path(TetrisNode &startNode, TetrisNode &landPoint, TetrisMap &map, bool NoSoftToBottom = false)->QVector<Oper>
{
    using OperMark = std::pair<TetrisNode, Oper>;
    auto softDropDisable = true;
    QVector<TetrisNode> nodeSearch;
    QHash<TetrisNode, OperMark>nodeMark;
    auto &landNode = landPoint;

    auto mark = [&nodeMark](TetrisNode & key, OperMark value) {
        if (!nodeMark.contains(key)) {
            nodeMark.insert(key, value);
            return true;
        } else
            return false;
    };

    auto buildPath = [&nodeMark, &NoSoftToBottom](TetrisNode & lp) {
        QVector<Oper> path;
        auto node = lp;
        while (true) {
            if (!nodeMark.contains(node))
                break;
            auto result = nodeMark.value(node);
            auto next = std::get<0>(result);
            auto dropToSd = (path.size() > 0 && std::get<1>(result) == Oper::DropToBottom);
            auto softDropDis = node.pos.y - next.pos.y;
            node = next;
            if (node.type == Piece::None) {
                break;
            }
            path.push_front(std::get<1>(result));
            if (dropToSd && NoSoftToBottom) {
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

    nodeSearch.append(startNode);
    nodeMark.insert(startNode, OperMark {TetrisNode{Piece::None}, Oper::None });
    auto disable_d = landNode.open(map);
    if (landNode.type == Piece::T && landNode.typeTSpin == TSpinType::TSpinMini)
        disable_d = false;
    while (nodeSearch.size() != 0) {
        auto next = nodeSearch.takeFirst();
        //逆时针旋转
        if (auto node = next; TetrisNode::rotate(node, map, true) && mark(node, OperMark { next, Oper::Cw })) {
            if (node == landNode)
                return buildPath(landNode);
            else
                nodeSearch.append(node);
        }

        //顺时针旋转
        if (auto node = next; TetrisNode::rotate(node, map, false) && mark(node, OperMark { next, Oper::Ccw })) {
            if (node == landNode)
                return buildPath(landNode);
            else
                nodeSearch.append(node);
        }

        //左
        if (auto node = next; TetrisNode::shift(node, map, -1, 0) && mark(node, OperMark { next, Oper::Left })) {
            if (node == landNode)
                return buildPath(landNode);
            else
                nodeSearch.append(node);
        }

        //右
        if (auto node = next; TetrisNode::shift(node, map, 1, 0) && mark(node, OperMark {next, Oper::Right })) {
            if (node == landNode)
                return buildPath(landNode);
            else
                nodeSearch.append(node);
        }

        //下
        if (softDropDisable) {
            if (auto node = next; TetrisNode::shift(node, map, 0, 1) && mark(node, OperMark{next, Oper::SoftDrop })) {
                if (node == landNode)
                    return buildPath(landNode);
                else
                    nodeSearch.append(node);
            }
        }

        //软降到底
        if (auto node = next.drop(map); mark(node, OperMark { next, Oper::DropToBottom })) {
            if (node == landNode)
                return buildPath(landNode);
            else if (!disable_d)
                nodeSearch.append(node);
        }
    }
    return QVector<Oper> {};
}

TetrisBot::EvalResult TetrisBot::evalute(TetrisNode &lp, TetrisMap &map, int clear, int tCount = 0)
{
    TetrisBot::EvalResult evalResult;
    if (lp.lastRotate && lp.type == Piece::T) {
        if (clear > 0 && lp.lastRotate) {
            if (clear == 1 && lp.mini && lp.spin) {
                lp.typeTSpin = TSpinType::TSpinMini;
            } else if (lp.spin) {
                lp.typeTSpin = TSpinType::TSpin;
            } else {
                lp.typeTSpin = TSpinType::None;
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

    auto  eval_map = [](TetrisMap & map) {

        struct {
            int colTrans;
            int rowTrans;
            int holes;
            int holeLines;
            int roof;
            int lowestRoof;
            double clearWidth;
            int wide[31];
        } m;

        memset(&m, 0, sizeof(m));
        m.roof = map.height - map.roof;
        m.lowestRoof = map.height;

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

        //洞井改
        auto LineCoverBits = 0, widthL1 = map.width - 1;
        for (auto y = map.roof; y < map.height; y++) {
            LineCoverBits |= map.data[y];
            auto LineHole = LineCoverBits ^ map.data[y];
            if (BitCount(LineCoverBits) == map.width) {
                m.lowestRoof = std::min(m.lowestRoof, y);
            }
            if (LineHole != 0) {
                m.holes += BitCount(LineHole);
                m.holeLines++;
                //  if (m.holeLines == 1)
                for (auto hy = y - 1; hy >= map.roof; hy--) {
                    auto CheckLine = LineHole & map.data[hy];
                    if (CheckLine == 0) {
                        break;
                    }
                    m.clearWidth += ((map.width - BitCount(map.data[hy])));
                }
            }
            widthL1 = std::min<int>(widthL1, map.width - BitCount(LineCoverBits));
            if (m.holeLines == 0) {
                ++m.wide[widthL1];
            }
        }

        m.lowestRoof = map.height - m.lowestRoof;
        auto dangerH = map.roof < 7;
        auto value = (0.
                      - (dangerH ? m.roof * 100 : 0)
                      - m.colTrans  * 9 * (dangerH  ? 1 : 4)
                      + (- m.lowestRoof * 10
                         -  m.holes * 5
                         - m.holeLines * 38
                         - m.clearWidth * 10) * (dangerH  ? 0.5 : 1)
                     );
        return value;
    };

    auto tspinDetect = [&]() {
        bool finding2, finding3;
        auto *loopMap = &map;
        auto tc = 0;
        auto rowBelow = [&](int x, int y, int yCheck) {
            auto ifCover = (loopMap->top[x] <= y) |
                           ((loopMap->top[x + 1] <= y) << 1) |
                           ((loopMap->top[x + 2] <= y) << 2);
            return !(ifCover ^ yCheck);
        };

loop:
        finding2 =  finding3 = true;
        for (int y = loopMap->roof; (finding2 || finding3) && y < loopMap->height; ++y) {
            auto y0 = loopMap->data[y];
            auto y1 = y - 1 > -1 ? loopMap->data[y - 1] : 0;
            auto y2 = y - 2 > -1 ? loopMap->data[y - 2] : 0;
            auto y3 = y - 3 > -1 ? loopMap->data[y - 3] : 0;
            auto y4 = y - 4 > -1 ? loopMap->data[y - 4] : 0;

            for (int x = 0; finding2 && x < loopMap->width - 2 ; ++x) {
                if (auto y2Check = (y2 >> x & 7);
                    (((y0 >> x & 7) == 5) && ((y1 >> x & 7) == 0) && (evalResult.t2Value += 1)) &&
                    ((BitCount(y0) == loopMap->width - 1) && (evalResult.t2Value += 1)) &&
                    ((BitCount(y1) == loopMap->width - 3) && (evalResult.t2Value += 2)) &&
                    ((y2Check == 4 || y2Check == 1) && rowBelow(x, y - 2, y2Check) && (evalResult.t2Value += 2))
                   ) {
                    finding2 = false;
                    auto *virtualNode = &TreeContext::nodes[static_cast<int>(Piece::T)];
                    if (!tc) {
                        loopMap = nullptr;
                        loopMap = new TetrisMap{map};
                    }
                    virtualNode->attach(*loopMap, Pos{x, y - 2}, 2);
                    if (tc++ < tCount)
                        goto loop;
                }
            }

            for (int x = 0; finding3 && x < loopMap->width - 2; ++x) {
                if (auto value = 0; ((y0 >> x & 7) == 3 && (y1 >> x & 7) == 1 && x < loopMap->width - 3)) {
                    if (auto y4Check = (y4 >> x & 7);
                        ((BitCount(y0) == loopMap->width - 1) && (value += 1)) &&
                        ((BitCount(y1) == loopMap->width - 2) && (value += 1)) &&
                        (((y2 >> x & 7) == 3 && (y3 >> x & 7) == 0) && (value += 2)) &&
                        ((BitCount(y2) == loopMap->width - 1) && (value += 2)) &&
                        (((y3 >> x & 7) == 0) ? (value += 1) : (value = 0)) &&
                        ((y4Check == 4 && rowBelow(x, y - 4, y4Check)) ? (value += 1) : ((value -= 2) && false))
                       ) {
                        evalResult.t3Value += value;
                        finding3 = false;
                        auto *virtualNode = &TreeContext::nodes[static_cast<int>(Piece::T)];
                        if (!tc) {
                            loopMap = nullptr;
                            loopMap = new TetrisMap{map};
                        }
                        virtualNode->attach(*loopMap, Pos{x + 1, y - 2}, 3);
                        if (tc++ < tCount)
                            goto loop;
                    }
                    if (finding3)
                        evalResult.t3Value += value;
                    if (value > 3)
                        finding3 = false;
                }

                else  if (auto value = 0; ((y0 >> x & 7) == 6 && (y1 >> x & 7) == 4 && x > 0)) {
                    if (auto y4Check = (y4 >> x & 7);
                        ((BitCount(y0) == loopMap->width - 1) && (value += 1)) &&
                        ((BitCount(y1) == loopMap->width - 2) && (value += 1)) &&
                        (((y2 >> x & 7) == 6 && (y3 >> x & 7) == 0) && (value += 2)) &&
                        ((BitCount(y2) == loopMap->width - 1) && (value += 2)) &&
                        (((y3 >> x & 7) == 0) ? (value += 1) : (value = 0)) &&
                        ((y4Check == 1 && rowBelow(x, y - 4, y4Check)) ? (value += 1) : ((value -= 2) && false))
                       ) {
                        evalResult.t3Value += value;
                        finding3 = false;
                        auto *virtualNode = &TreeContext::nodes[static_cast<int>(Piece::T)];
                        if (!tc) {
                            loopMap = nullptr;
                            loopMap = new TetrisMap{map};
                        }
                        virtualNode->attach(*loopMap, Pos{x - 1, y - 2}, 1);
                        if (tc++ < tCount)
                            goto loop;
                    }
                    if (finding3)
                        evalResult.t3Value += value;
                    if (value > 3)
                        finding3 = false;
                }
            }
        }
        auto valueX = eval_map(*loopMap);
        if (loopMap != &map) {
            delete loopMap;
        }
        loopMap = nullptr;
        return valueX;
    };

    auto getSafe = [&]() {
        int safe = INT_MAX;
        for (auto type = 0; type < 7; type++) {
            auto *virtualNode = &TreeContext::nodes[type];
            if (!virtualNode->check(map)) {
                safe = -1;
                break;
            } else {
                auto dropDis = virtualNode->getDrop(map);
                safe = std::min(dropDis, safe);
            }
        }
        return safe;
    };

    evalResult.value = /*eval_map(map) +*/  tspinDetect();
    evalResult.clear = clear;
    evalResult.typeTSpin = lp.typeTSpin;
    evalResult.safe = getSafe();
    evalResult.count = map.count;
    evalResult.type = lp.type;
    return evalResult;
}


TetrisBot::Status TetrisBot::get(const TetrisBot::EvalResult &evalResult, TetrisBot::Status &status, Piece hold)
{
    auto &comboTable = TreeContext::comboTable;
    auto tableMax = int(comboTable.size());
    auto result = status;
    result.value = evalResult.value;
    result.mapRise = 0;

    if (evalResult.safe <= 0) {
        result.deaded = true;
        return result;
    }

    switch (evalResult.clear) {
    case 0:
        if (status.combo > 0 && status.combo < 3) {
            result.like -= 2;
        }
        result.combo = 0;
        if (status.underAttack > 0) {
            result.mapRise += std::max(0, std::abs(status.underAttack - status.attack));
            if (result.mapRise >= evalResult.safe) {
                result.deaded = true;
                return result;
            }
            result.underAttack = 0;
        }
        break;
    case 1:
        if (evalResult.typeTSpin == TSpinType::TSpinMini)
            result.attack += status.b2b ? 2 : 1;
        else if (evalResult.typeTSpin == TSpinType::TSpin)
            result.attack += status.b2b ? 3 : 2;
        result.attack += comboTable[std::min(tableMax - 1, ++result.combo)];
        result.b2b = evalResult.typeTSpin != TSpinType::None;
        break;
    case 2:
        result.attack += comboTable[std::min(tableMax - 1, ++result.combo)];
        if (evalResult.typeTSpin != TSpinType::None) {
            result.like += 8;
            result.attack += status.b2b ? 5 : 4;
        } else
            result.attack += 1;
        result.b2b = evalResult.typeTSpin != TSpinType::None;
        break;
    case 3:
        result.attack += comboTable[std::min(tableMax - 1, ++result.combo)] ;
        if (evalResult.typeTSpin != TSpinType::None) {
            result.like += 12;
            result.attack += status.b2b ? 8 : 6;
        } else
            result.attack +=  2;
        result.b2b = evalResult.typeTSpin != TSpinType::None;
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
        result.cutB2b = true;
    }

    switch (hold) {
    case Piece::T:
        if (evalResult.typeTSpin == TSpinType::None)
            result.like += 4;
        break;
    case Piece::I:
        if (evalResult.clear != 4)
            result.like += 2;
        break;
    default: break;
    }

    result.maxAttack = std::max(result.attack, result.maxAttack);
    //  return result;
    result.value += ((0.
                      + result.attack * 40
                      + result.like * 6
                      + (result.b2b ? (result.cutB2b ? 101 * 0.5 : 101) : 0)
                      + (result.cutB2b ? 0 : std::max(evalResult.t2Value, 0) * 30)
                      + (evalResult.safe >= 10  ? 30  * (evalResult.t3Value - result.underAttack) : 0)
                     ));
    return result;
}

QVector<TetrisNode>TreeContext::nodes = std::invoke([]()
{
    QVector<TetrisNode>_nodes;
    for (auto i = 0; i < 7; i++) {
        _nodes.append(TetrisNode{static_cast<Piece>(i)});
    }
    return _nodes;
});

std::array<int, 13>TreeContext::comboTable = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5, -1};

TreeContext::~TreeContext()
{
    treeDelete(root);
    return;
}

bool TreeNodeCompare::operator()(TreeNode *const &a, TreeNode *const &b) const
{
    return  a->evalParm.status < b->evalParm.status;
}

void TreeContext::createRoot(TetrisNode &_node, TetrisMap &_map, QVector<Piece> &dp, Piece _hold, int _b2b, int _combo, int _underAttack)
{
    nexts = dp;
    noneHoldFirstNexts = dp.mid(1, -1);
    auto depth = dp.size() + 1 + ((_hold == Piece::None && isOpenHold) ?  -1 : 0); // + 1;
    level.resize(depth);
    extendedLevel.resize(depth);
    tCount = int (_hold == Piece::T && isOpenHold) + int(_node.type == Piece::T);
    for (const auto &i : dp) {
        if (i == Piece::T)
            tCount++;
    }
    if (depth > 0) {
        double ratio = 1.5;
        while (width_cache.size() < depth - 1) {
            width_cache.push_back(std::pow(width_cache.size() + 2, ratio));
        }
        div_ratio = 2 / *std::max_element(width_cache.begin(), width_cache.end());
    }
    tCount = std::max(0, --tCount);
    TreeNode *tree = new TreeNode;
    tree->context = this;
    tree->nexts = &nexts;
    tree->node = &nodes[static_cast<int>(_node.type)];
    tree->map = _map;
    tree->hold = _hold;
    tree->evalParm.status.underAttack = _underAttack;
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

TreeContext::Result TreeContext::empty()
{
    return {TetrisNode{root->node->type}, false, test};
}

TreeContext::Result TreeContext::getBest()
{
    return root->getBest();
}

TreeNode::TreeNode(TreeContext *_ctx, TreeNode *_parent, TetrisNode &_node, TetrisMap &_map,
                   int _nextIndex, Piece _hold, bool _isHold,  EvalParm &_evalParm)
{
    context = _ctx;
    parent = _parent;
    node = _node.type == Piece::None ? nullptr : &_ctx->nodes[static_cast<int>(_node.type)];
    map = _map;
    nexts = _parent->nexts;
    nextIndex = _nextIndex;
    hold = _hold;
    isHold = _isHold;
    evalParm = _evalParm;
}

void TreeNode::printInfoReverse(TetrisMap &map_,    TreeNode *_test = nullptr)
{
    if (false)
        Tool::printMap(map_);
    TreeNode *test = _test == nullptr ? this : _test;
    context->test = true;
    Piece first = test->  evalParm.land_node.type;
    auto isChange = false;
    while (test != nullptr) {
        if (test->isHold) {
            isChange = true;
            first = test->  evalParm.land_node.type;
        }
        if (test->parent == nullptr && isChange)
            qDebug() <<  Tool::printType(first);
        else
            qDebug() << (test->node == nullptr ? "None" : Tool::printType(test->node->type));
        Tool::printMap(test->map);
        test = test->parent;
    }
}

TreeNode *TreeNode::generateChildNode(TetrisNode &i_node, bool _isHoldLock, Piece _hold, bool _isHold)
{
    auto next_node = TetrisNode{(nextIndex == nexts->size() ? Piece::None : nexts->at(nextIndex))};
    auto map_ = map;
    auto clear = i_node.attach(map_);
    auto status_ = TetrisBot::get(TetrisBot::evalute(i_node, map_, clear, context->tCount),
                                  evalParm.status, hold);
    EvalParm evalParm_ = { i_node, clear, status_};
    TreeNode *new_tree = nullptr;
    if (!status_.deaded) {
        new_tree = new TreeNode{context, this, next_node, map_, nextIndex + 1, _hold, _isHold, evalParm_};
        new_tree->isHoldLock = _isHoldLock;
        children.append(new_tree);
    }
    return new_tree;
}

void TreeNode::search()
{
    if (context->isOpenHold && !isHoldLock) {
        search_hold();
        return;
    }
    for (auto &i_node : TetrisBot::search(*node, map)) {
        generateChildNode(i_node,  isHoldLock, hold, bool(isHold));
    }
}

void TreeNode::search_hold(bool op, bool noneFirstHold)
{
    if (hold == Piece::None || nexts->size() == 0) {
        auto hold_save = hold;
        auto nexts_save = nexts->takeFirst();
        if (hold == Piece::None) {
            hold = node->type;
            node = & context->nodes[static_cast<int>(nexts_save)];
            search_hold(true, true);
        }
        node = & context->nodes[static_cast<int>(hold)];
        hold = hold_save;
        nexts->push_front(nexts_save);
        return;
    }
    if (node->type == hold) {
        for (auto &i_node :  TetrisBot::search(*node, map)) {
            generateChildNode(i_node, true, hold, false);
        }
    } else if (node->type != hold) {
        auto hold_node = TetrisNode{hold};
        for (auto &i_node : TetrisBot::search(*node, map)) {
            auto childTree = generateChildNode(i_node, true, hold, bool(op ^ false));
            if (noneFirstHold && childTree != nullptr)
                childTree->nexts = & context->noneHoldFirstNexts;
        }
        for (auto &i_node :  TetrisBot::search(hold_node, map)) {
            auto childTree = generateChildNode(i_node, true, node->type, bool(op ^ true));
            if (noneFirstHold && childTree != nullptr)
                childTree->nexts = & context->noneHoldFirstNexts;
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
    auto previewLength = context->level.size() - 1;
    auto  i = previewLength + 1;
    if (context->width == 0) {
        auto &firstLevel = context->level[previewLength];
        if (this->eval())
            for (auto child : this->children)
                firstLevel.push(child);
        context->width = 2;
    } else
        context->width += 2;
    while (--i > 0) {
        auto levelPruneHold = std::max<size_t>(1, size_t(context->width_cache[i - 1] * context->width *  context->div_ratio));
        auto &deepIndex = i;
        auto &levelSets = context->level[deepIndex];
        auto &nextLevelSets = context->level[deepIndex - 1];
        auto &extendedLevelSets = context->extendedLevel[deepIndex];
        if (levelSets.empty())
            continue;
        QVector<TreeNode *> work;
        for (auto pi = 0; !levelSets.empty() && extendedLevelSets.size() < levelPruneHold; pi++) {
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

TreeContext::Result TreeNode::getBest()
{
    auto &lastLevel = context->level;
    TreeNode *best = lastLevel.first().empty() ? nullptr : lastLevel.first().top();
    if (best == nullptr)
        for (const auto &level : context->extendedLevel) {
            if (!level.empty()) {
                best = level.top();
                break;
            }
        }
    if (best == nullptr)
        return context->empty();
    QVector<TreeNode *> record{best};
    while (best->parent != nullptr) {
        record.push_front(best->parent);
        best = best->parent;
    }
    return {record[1]->evalParm.land_node, record[1]->isHold, context->test};
}



