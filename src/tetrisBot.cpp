#include "tetrisBot.h"

int BitCount(int n) {
    n = n - ((n >> 1) & 0x55555555);
    n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
    n = (n + (n >> 4)) & 0x0f0f0f0f;
    n = n + (n >> 8);
    n = n + (n >> 16);
    return n & 0x3f;
}

QString Tool:: printType(Piece x) {
    QString a = "a";
    switch (x) {
        case Piece::None: a = "None"; break;
        case Piece::O: a = "O"; break;
        case Piece::J: a = "J"; break;
        case Piece::I: a = "I"; break;
        case Piece::T: a = "T"; break;
        case Piece::S: a = "S"; break;
        case Piece::Z: a = "Z"; break;
        case Piece::L: a = "L"; break;
        case Piece::Trash: a = "trash"; break;
    }
    return a;
}

QString Tool:: printPath(QVector<Oper> &path) {
    QString a;
    for (auto x : path) {
        switch (x) {
            case Oper::Left: a += "l"; break;
            case Oper::Right: a += "r"; break;
            case Oper::SoftDrop: a += "d"; break;
            case Oper::HardDrop: a += "V"; break;
            case Oper::Hold: a += "v"; break;
            case Oper::Cw: a += "z"; break;
            case Oper::Ccw: a += "x"; break;
            case Oper::DropToBottom: a += "D"; break;
            default: break;
        }
    }
    return a;
}

QString Tool:: printOper(Oper &x) {
    QString a = "a";
    switch (x) {
        case Oper::None: a = "None"; break;
        case Oper::Left: a = "Left"; break;
        case Oper::Right: a = "Right"; break;
        case Oper::SoftDrop: a = "SoftDrop"; break;
        case Oper::HardDrop: a = "HardDrop"; break;
        case Oper::Hold: a = "Hold"; break;
        case Oper::Cw: a = "Cw"; break;
        case Oper::Ccw: a = "Ccw"; break;
        case Oper::DropToBottom: a = "DropToBottom"; break;
    }
    return a;
}

void Tool::printMap(TetrisMap &map) {
    for (auto i = 0; i < map.height; i++) {
        QString str;
        for (auto j = 0; j < map.width; j++) {
            if (map(i, j))
                str += "[  ]";
            else
                str += "     ";
        }
        if (map.data[i] > 0)
            qDebug() << str;
    }
}

QString Tool::printNode(TetrisNode &node) {
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


QVector<TetrisNode> TetrisBot::search(TetrisNode &_first, TetrisMap &_map) {
    auto softDropDisable = true;
    std::unordered_set<TetrisNode>checked;
    std::deque<searchNode> queue;
    std::unordered_map<searchNode, int> results;
    queue.push_back(searchNode{_first, 0});
    checked.insert(_first);

    while (queue.size() != 0) {
        auto [node, count] = queue.back();
        queue.pop_back();
        if (auto next = node; !TetrisNode::shift(next, _map, 0, 1)) {
            auto infoKey = searchNode{std::move(next), count};
            if (results.find(infoKey) != results.end()) {
                if (auto keyCount = results.at(infoKey); keyCount > count) {
                    results.insert({infoKey, count});
                } else if (next.rotateState == _first.rotateState) {
                    results.erase(results.find(infoKey));
                    results.insert({infoKey, count});
                }
            } else
                results.insert({infoKey, count});
        }
        if (auto next = node; TetrisNode::shift(next, _map, -1, 0)) {     //左
            if (checked.find(next) == checked.end()) {
                next.lastRotate = false;
                checked.insert(next);
                queue.push_back(searchNode{std::move(next), count + 1});
            }
        }

        if (auto next = node; TetrisNode::shift(next, _map, 1, 0)) {     //右
            if (checked.find(next) == checked.end()) {
                next.lastRotate = false;
                checked.insert(next);
                queue.push_back(searchNode{std::move(next), count + 1});
            }
        }

        if (auto next = node; TetrisNode::rotate(next, _map, true)) {     //逆时针旋转
            if (checked.find(next) == checked.end()) {
                next.lastRotate = true;
                checked.insert(next);
                queue.push_back(searchNode{std::move(next), count + 1});
            }
        }

        if (auto next = node; TetrisNode::rotate(next, _map, false)) {     //顺时针旋转
            if (checked.find(next) == checked.end()) {
                next.lastRotate = true;
                checked.insert(next);
                queue.push_back(searchNode{std::move(next), count + 1});
            }
        }

        if (auto next = node.drop(_map); checked.find(next) == checked.end()) {       //软降到底
            if (checked.find(next) == checked.end()) {
                next.lastRotate = false;
                checked.insert(next);
                queue.push_back(searchNode{std::move(next), count + 1});
            }
        }

        if (!softDropDisable) {
            if (auto next = node; TetrisNode::shift(next, _map, 0, 1)) {     //单次软降
                if (checked.find(next) == checked.end()) {
                    next.lastRotate = false;
                    checked.insert(next);
                    queue.push_back(searchNode{std::move(next), count + 1});
                }
            }
        }
    }

    QVector<TetrisNode> result;
    for (auto &x : results) {
        auto [node, count] = std::get<0>(x);
        if (_first.type == Piece::T) {
            auto [spin, mini] = node.corner3(_map);
            node.mini = mini;
            node.spin = spin;
        }
        result.append(node);
    }
    return result;
}

auto TetrisBot::make_path(TetrisNode &startNode, TetrisNode &landPoint, TetrisMap &map, bool NoSoftToBottom = false)->QVector<Oper> {
    using OperMark = std::pair<TetrisNode, Oper>;
    auto softDropDisable = true;
    std::deque<TetrisNode> nodeSearch;
    std::unordered_map<TetrisNode, OperMark> nodeMark;
    auto &landNode = landPoint;

    auto mark = [&nodeMark](TetrisNode & key, OperMark value) {
        if (nodeMark.find(key) == nodeMark.end()) {
            nodeMark.insert({key, value});
            return true;
        } else
            return false;
    };

    auto buildPath = [&nodeMark, &NoSoftToBottom](TetrisNode & lp) {
        QVector<Oper> path;
        TetrisNode *node = &lp;
        while (nodeMark.find(*node) != nodeMark.end()) {
            auto &result = nodeMark.at(*node);
            auto &next = std::get<0>(result);
            auto dropToSd = (path.size() > 0 && std::get<1>(result) == Oper::DropToBottom);
            auto softDropDis = node->pos.y - next.pos.y;
            node = &next;
            if (node->type == Piece::None)
                break;
            path.push_front(std::get<1>(result));
            if (dropToSd && NoSoftToBottom) {
                path.remove(0);
                path.insert(0, softDropDis, Oper::SoftDrop);
            }
        }
        while (path.size() != 0 && (path[path.size() - 1] == Oper::SoftDrop || path[path.size() - 1] == Oper::DropToBottom))
            path.remove(path.size() - 1, 1);
        path.push_back(Oper::HardDrop);
        return path;
    };

    nodeSearch.push_back(startNode);
    nodeMark.insert({startNode, OperMark {TetrisNode::spawn(Piece::None), Oper::None }});
    auto disable_d = landNode.open(map);
    static constexpr Oper opers[] = {Oper::Cw, Oper::Ccw, Oper::Left, Oper::Right, Oper::SoftDrop, Oper::DropToBottom};
    if (landNode.type == Piece::T && landNode.typeTSpin == TSpinType::TSpinMini)
        disable_d = false;
    while (!nodeSearch.empty()) {
        auto next = std::move(nodeSearch.front());
        nodeSearch.pop_front();
        for (auto oper : opers) {
            auto node = (oper == Oper::DropToBottom ? next.drop(map) : next);
            if (oper == Oper::SoftDrop && softDropDisable)
                continue;
            bool res = false;
            switch (oper) {
                case Oper::Cw: res = TetrisNode::rotate(node, map, true) && mark(node, OperMark { next, oper}); break;
                case Oper::Ccw: res = TetrisNode::rotate(node, map, false) && mark(node, OperMark { next, oper}); break;
                case Oper::Left: res = TetrisNode::shift(node, map, -1, 0) && mark(node, OperMark { next, oper }); break;
                case Oper::Right: res = TetrisNode::shift(node, map, 1, 0) && mark(node, OperMark {next, oper }); break;
                case Oper::SoftDrop: res = TetrisNode::shift(node, map, 0, 1) && mark(node, OperMark{next, oper}); break;
                case Oper::DropToBottom: res = mark(node, OperMark { next, oper}); break;
                default: break;
            }
            if (res) {
                if (node == landNode)
                    return buildPath(landNode);
                else if (oper == Oper::DropToBottom ? !disable_d : true)
                    nodeSearch.push_back(node);
            }
        }
    }
    return QVector<Oper> {};
}

TetrisBot::EvalResult TetrisBot::evalute(TetrisNode &lp, TetrisMap &map, int clear, int tCount = 0) {
    TetrisBot::EvalResult evalResult;
    if (lp.lastRotate && lp.type == Piece::T) {
        if (clear > 0 && lp.lastRotate) {
            if (clear == 1 && lp.mini && lp.spin)
                lp.typeTSpin = TSpinType::TSpinMini;
            else if (lp.spin)
                lp.typeTSpin = TSpinType::TSpin;
            else
                lp.typeTSpin = TSpinType::None;
        }
    }

    auto eval_landpoint = [&]() {
        auto LandHeight = map.height - map.roof;
        auto Middle = std::abs((lp.pos.x + 1) * 2 - map.width);
        int
        value = 0
                - LandHeight / map.height * 40
                + Middle * 0.2
                + (clear * 6);
        return value;
    };

    auto  eval_map = [](TetrisMap & map) {

        auto isCompact = [&](int num) {
            int filter = (1 << map.width) - 1 ;
            unsigned x = filter & (~num);
            return (x & x + (x & -x)) == 0;
        };

        struct {
            int colTrans;
            int rowTrans;
            int holes;
            int holeLines;
            int roof;
            int lowestRoof;
            double clearWidth;
            int wide[31];
            int messy;
            int wellDepth;
            int holeDepth;
            int wellNum[32];
            int holeNum[32];
        } m;

        memset(&m, 0, sizeof(m));
        m.roof = map.height - map.roof;
        m.lowestRoof = map.height;

        //行列变换
        /*for (auto j = (map.roof == 0 ?  0 : map.roof - 1); j < map.height; j++) {
            auto ct = (map.data[j] ^
                   (map.data[j] & (1 << (map.width - 1)) ? (map.data[j] >> 1) | (1 << (map.width - 1)) : map.data[j] >> 1));
            m.colTrans += BitCount(ct);
            if (j < map.height - 1) {
            auto rt = (map.data[j] ^ map.data[j + 1]);
            m.rowTrans += BitCount(rt);
            }
        }*/

        for (auto j = (map.roof == 0 ?  0 : map.roof - 1); j < map.height; j++) {
            auto cty = ((~map.data[j]) & ((1 << map.width) - 1));
            auto ctyF = ((~map.data[j + 1]) & ((1 << map.width) - 1));
            auto ct = (cty ^
                       (cty & (1 << (map.width - 1)) ? (cty >> 1) | (1 << (map.width - 1)) : cty >> 1));
            m.colTrans += BitCount(ct);
            if (j < map.height - 1) {
                auto rt = (cty ^ ctyF);
                m.rowTrans += BitCount(rt);
            }
        }

        //洞井改
        unsigned LineCoverBits = 0, widthL1 = map.width - 1;
        for (auto y = map.roof; y < map.height; y++) {
            LineCoverBits |= map.data[y];
            auto LineHole = LineCoverBits ^ map.data[y];
            if (BitCount(LineCoverBits) == map.width) {
                m.lowestRoof = std::min(m.lowestRoof, y);   //最低的屋檐
            }
            if (LineHole != 0) {
                m.holes += BitCount(LineHole);   //洞个数
                m.holeLines++;//洞行数
                for (auto hy = y - 1; hy >= map.roof; hy--) {   //覆盖的洞层
                    auto CheckLine = LineHole & map.data[hy];
                    if (CheckLine == 0) break;
                    m.clearWidth += ((map.width - BitCount(map.data[hy]))) * (map.height - hy);
                }
            }
            auto spacing = std::min<int>(widthL1, map.width - BitCount(LineCoverBits));
            if (isCompact(LineCoverBits))
                ++m.wide[spacing]; //几宽
            else
                m.messy++; //离散的分布
            for (auto x = 0; x < map.width; x++) {
                if ((LineHole >> x) & 1) m.holeDepth += ++m.holeNum[x];     //洞深
                else m.holeNum[x] = 0;
                auto isWell = x == 0 ? (LineCoverBits & 3) == 2 :
                              (x == widthL1 ? ((LineCoverBits >> (widthL1 - 1)) & 3) == 1 :
                               ((LineCoverBits >> (x - 1)) & 7) == 5);
                if (isWell) m.wellDepth += ++m.wellNum[x];   //井深
            }
        }

        auto n = std::min({map.top[3], map.top[4], map.top[5], map.top[6]});
        m.roof = map.height - n;
        m.lowestRoof = map.height - m.lowestRoof;
        auto  value = (0.
                        - m.colTrans * 160 / 10
                       // - m.rowTrans * 160 / 10
                       - m.roof * 120
                       - m.holeLines * 380 / 2
                       - m.clearWidth * 50 / 3
                       - m.wellDepth * 100
                       - m.holeDepth * 100
                   //    - m.messy * 100
                      );
        return value * 0.142;
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
            auto y0 = loopMap->row(y);
            auto y1 = loopMap->row(y - 1) ;
            auto y2 = loopMap->row(y - 2);
            auto y3 = loopMap->row(y - 3);
            auto y4 = loopMap->row(y - 4);
            for (int x = 0; finding2 && x < loopMap->width - 2 ; ++x) {
                if (auto value = 0, y2Check = (y2 >> x & 7);
                    (((y0 >> x & 7) == 5) && ((y1 >> x & 7) == 0) && (value += 1)) &&
                    ((BitCount(y0) == loopMap->width - 1) && (value += 1)) &&
                    ((BitCount(y1) == loopMap->width - 3) && (value += 2)) &&
                    ((y2Check == 4 || y2Check == 1) && rowBelow(x, y - 2, y2Check) && (value += 2))
                   ) {
                    evalResult.t2Value += value;
                    finding2 = false;
                    if (tc++ < tCount) {
                        auto *virtualNode = &TreeContext::nodes[static_cast<int>(Piece::T)];
                        if (!tc) {
                            loopMap = nullptr;
                            loopMap = new TetrisMap{map};
                        }
                        virtualNode->attach(*loopMap, Pos{x, y - 2}, 2);
                        goto loop;
                    }
                }
            }
            for (int x = 0; finding3 && x < loopMap->width - 2; ++x) {
                if (auto value = 0, mirror = 0;
                    (((y0 >> x & 7) == 3 || ((y0 >> x & 7) == 6 && (++mirror))) &&
                     (y1 >> x & 7) == (!mirror ? 1 : 4) && x < loopMap->width - 3)) {
                    if (auto y4Check = (y4 >> x & 7);
                        ((BitCount(y0) == loopMap->width - 1) && (value += 1)) &&
                        ((BitCount(y1) == loopMap->width - 2) && (value += 1)) &&
                        (((y2 >> x & 7) == (!mirror ? 3 : 6) && (y3 >> x & 7) == 0) && (value += 2)) &&
                        ((BitCount(y2) == loopMap->width - 1) && (value += 2)) &&
                        (((y3 >> x & 7) == 0) ? (value += 1) : (value = 0)) &&
                        ((y4Check == (!mirror ? 4 : 1) && rowBelow(x, y - 4, y4Check)) ? (value += 1) : ((value -= 2) && false))
                       ) {
                        evalResult.t3Value += value;
                        finding3 = false;
                        if (tc++ < tCount) {
                            auto *virtualNode = &TreeContext::nodes[static_cast<int>(Piece::T)];
                            if (!tc) {
                                loopMap = nullptr;
                                loopMap = new TetrisMap{map};
                            }
                            virtualNode->attach(*loopMap, Pos{x + (!mirror ? 1 : -1), y - 2}, !mirror ? 3 : 1);
                            goto loop;
                        }
                    }
                    if (finding3) evalResult.t3Value += value;
                    if (value > 3) finding3 = false;
                }
            }
        }
        //auto valueX = eval_map(*loopMap);
        auto valueX = eval_map(map);
        if (loopMap != &map)
            delete loopMap;
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

    tCount = 0;
    evalResult.value = /*eval_map(map) +*/  tspinDetect();
    evalResult.clear = clear;
    evalResult.typeTSpin = lp.typeTSpin;
    evalResult.safe = getSafe();
    evalResult.count = map.count;
    evalResult.type = lp.type;
    return evalResult;
}


TetrisBot::Status TetrisBot::get(const TetrisBot::EvalResult &evalResult, TetrisBot::Status &status, Piece hold) {
    auto &comboTable = TreeContext::comboTable;
    auto &nZCI = TreeContext::noneZeroComboIndex;
    auto tableMax = int(comboTable.size());
    auto result = status;
    result.value =  evalResult.value;
    result.attack = 0;
    result.t2Value = evalResult.t2Value;
    result.t3Value = evalResult.t3Value;
    result.like = 0;
    if (evalResult.safe <= 0) {
        result.deaded = true;
        return result;
    }

    switch (evalResult.clear) {
        case 0:
            result.combo = 0;
            if (status.underAttack > 0) {
                result.mapRise = std::max(0, std::abs(status.underAttack - status.attack));
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
            result.b2b = evalResult.typeTSpin != TSpinType::None;
            break;
        case 2:
            if (evalResult.typeTSpin != TSpinType::None) {
                result.like += 8;
                result.attack += status.b2b ? 5 : 4;
            } else
                result.attack += 1;
            result.b2b = evalResult.typeTSpin != TSpinType::None;
            break;
        case 3:
            if (evalResult.typeTSpin != TSpinType::None) {
                result.like += 12;
                result.attack += status.b2b ? 8 : 6;
            } else
                result.attack +=  2;
            result.b2b = evalResult.typeTSpin != TSpinType::None;
            break;
        case 4:
            result.like += 8;
            result.attack += (status.b2b ? 5 : 4);
            result.b2b = true;
            break;
    }

    if (evalResult.clear > 0)
        result.attack += comboTable[std::max(std::min(tableMax - 1, (++result.combo) - 1), 0)];
    if (evalResult.count == 0 && result.mapRise == 0) {
        result.like += 20;
        result.attack += 6;
    }
    if (status.b2b && !result.b2b) {
        result.like -= 2;
        result.cutB2b = true;
    }
    if (result.attack > 0)
        result.underAttack = std::max(0, result.underAttack - result.attack);

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

//   bool wasteT = evalResult.type == Piece::T && evalResult.typeTSpin == TSpinType::None && evalResult.clear == 0;
    result.maxAttack = std::max(result.attack, result.maxAttack);
    result.maxCombo = std::max(result.combo, result.maxCombo);
    double base = (evalResult.count) / (24. * 10 - evalResult.count) ;
    //return result;
    auto accValue = ((0.
                      + result.maxAttack * 60
                      + result.attack * 60
                      + result.like * 15
                      + (result.maxCombo - nZCI) * (result.maxCombo - nZCI) * 50
                      + (result.b2b ? ((!evalResult.clear ? 210 : 0) * (result.cutB2b ? 0.5 : 1)) : 0)
                      //+ (result.b2b && (!!evalResult.clear) ? 210 : 0)
                      //  + (wasteT ? -400 : 0)
                      + evalResult.t2Value * 30
                      + (evalResult.safe >= 10  ? 30  * (evalResult.t3Value - result.underAttack) : 0)
                     ));
    result.value = evalResult.value * (1 + std::max(0., base)) + accValue;
    return result;
}

QVector<TetrisNode>TreeContext::nodes = std::invoke([]() {
    QVector<TetrisNode>_nodes;
    for (auto i = 0; i < 7; i++)
        _nodes.append(TetrisNode::spawn(static_cast<Piece>(i)));
    return _nodes;
});

std::array<int, 13>TreeContext::comboTable = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5, -1};
int TreeContext::noneZeroComboIndex = std::invoke([]() {
    auto &temp = TreeContext::comboTable;
    return std::distance(std::begin(temp), std:: find_if(std::begin(temp), std::end(temp), [](auto x) { return x != 0; }));
});

TreeContext::~TreeContext() {treeDelete(root); return;}
bool TreeNodeCompare::operator()(TreeNode *const &a, TreeNode *const &b) const {return  a->evalParm.status < b->evalParm.status;}

void TreeContext::createRoot(TetrisNode &_node, TetrisMap &_map, QVector<Piece> &dp, Piece _hold, int _b2b, int _combo, int _underAttack) {
    nexts = dp;
    noneHoldFirstNexts = dp.mid(1, -1);
    auto depth = dp.size() + 1 + ((_hold == Piece::None && isOpenHold) ?  -1 : 0);     // + 1;
    level.resize(depth);
    extendedLevel.resize(depth);
    tCount = int (_hold == Piece::T && isOpenHold) + int(_node.type == Piece::T);
    for (const auto &i : dp) {
        if (i == Piece::T)
            tCount++;
    }
    if (depth > 0) {
        double ratio = 1.5;
        while (width_cache.size() < depth - 1)
            width_cache.push_back(std::pow(width_cache.size() + 2, ratio));
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
    tree->evalParm.status.maxCombo = _combo;
    root = tree;
}

void TreeContext::treeDelete(TreeNode *t) {
    if (t == nullptr)
        return;
    for (auto i = 0; i < t->children.size(); i++) {
        if (t->children[i] != nullptr)
            treeDelete(t->children[i]);
    }
    delete t;
}

void TreeContext::run() {
    if (root != nullptr)
        root->run();
}

TreeContext::Result TreeContext::empty() {return {TetrisNode::spawn(root->node->type), false, test};}
TreeContext::Result TreeContext::getBest() {return root->getBest();}

TreeNode::TreeNode(TreeContext *_ctx, TreeNode *_parent, TetrisNode &_node, TetrisMap _map,
                   int _nextIndex, Piece _hold, bool _isHold,  EvalParm &_evalParm): map(std::move(_map)) {
    context = _ctx;
    parent = _parent;
    node = _node.type == Piece::None ? nullptr : &_ctx->nodes[static_cast<int>(_node.type)];
    // map = _map;
    nexts = _parent->nexts;
    nextIndex = _nextIndex;
    hold = _hold;
    isHold = _isHold;
    evalParm = _evalParm;
}

void TreeNode::printInfoReverse(TetrisMap &map_,    TreeNode *_test = nullptr) {
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

TreeNode *TreeNode::generateChildNode(TetrisNode &i_node, bool _isHoldLock, Piece _hold, bool _isHold) {
    auto next_node = TetrisNode::spawn((nextIndex == nexts->size() ? Piece::None : nexts->at(nextIndex)));
    auto map_ = map;
    auto clear = i_node.attach(map_);
    auto status_ = TetrisBot::get(TetrisBot::evalute(i_node, map_, clear, context->tCount),
                                  evalParm.status, hold);
    auto ifOpen = i_node.open(map);
    auto needSd = this->evalParm.needSd + int (i_node.type != Piece::T ? !ifOpen : !i_node.lastRotate || (i_node.lastRotate && !clear));
    status_.value -= needSd * 150;
    EvalParm evalParm_ = { i_node, static_cast<int>(clear), std::move(status_), needSd};
    TreeNode *new_tree = nullptr;
    if (!status_.deaded) {
        new_tree = new TreeNode{context, this, next_node, map_, nextIndex + 1, _hold, _isHold, evalParm_};
        new_tree->isHoldLock = _isHoldLock;
        children.append(new_tree);
    }
    return new_tree;
}

void TreeNode::search() {
    if (context->isOpenHold && !isHoldLock) {
        search_hold();
        return;
    }
    for (auto &i_node : TetrisBot::search(*node, map))
        generateChildNode(i_node,  isHoldLock, hold, bool(isHold));
}

void TreeNode::search_hold(bool op, bool noneFirstHold) {
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
        for (auto &i_node :  TetrisBot::search(*node, map))
            generateChildNode(i_node, true, hold, false);
    } else if (node->type != hold) {
        auto hold_node = TetrisNode::spawn(hold);
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

bool TreeNode::eval() {
    if (extended == true)
        return false;
    if (children.size() == 0) {
        search();
        extended = true;
    }
    return true;
}

void  TreeNode::run() {
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
        std::mutex m;
        std::for_each(std::execution::par, work.begin(), work.end(), [&](TreeNode  * tree) {
            tree->eval();
            std::lock_guard<std::mutex> guard(m);
            for (auto &x : tree->children)
                nextLevelSets.push(x);
        });
    }
}

TreeContext::Result TreeNode::getBest() {
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
    //Tool::printMap(best->map);
    // qDebug() << "\n";
    return {record[1]->evalParm.land_node, record[1]->isHold, context->test};
}



