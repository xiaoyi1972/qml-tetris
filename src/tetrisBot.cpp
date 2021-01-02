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
    QVector<info> quene;
    QHash<info, int> results;
    quene.append(info{_first, 0});
    checked.insert(_first);
    while (quene.size() != 0) {
        auto [node, count] = quene.takeFirst();
        auto next = node;
        if (!TetrisNode::shift(next, _map, 0, 1)) {
            auto infoKey = info{next, count};
            if (results.contains(infoKey)) {
                auto keyCount = results.value(infoKey);
                if (keyCount > count) {
                    results.insert(infoKey, count);
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
                quene.append(info{std::move(next), count + 1});
            }
        }
        next = node;
        if (TetrisNode::shift(next, _map, 1, 0)) { //right
            if (!checked.contains(next)) {
                next.lastRotate = false;
                checked.insert(next);
                quene.append(info{std::move(next), count + 1});
            }
        }
        next = node;
        if (TetrisNode::rotate(next, _map, true)) { //cw
            if (!checked.contains(next)) {
                next.lastRotate = true;
                checked.insert(next);
                quene.append(info{std::move(next), count + 1});
            }
        }
        next = node;
        if (TetrisNode::rotate(next, _map, false)) { //ccw
            if (!checked.contains(next)) {
                next.lastRotate = true;
                checked.insert(next);
                quene.append(info{std::move(next), count + 1});
            }
        }
        next = node;
        next.shift(0, next.getDrop(_map)); //软降
        if (!checked.contains(next)) {
            next.lastRotate = false;
            checked.insert(next);
            quene.append(info{std::move(next), count + 1});
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

auto TetrisBot::make_path(TetrisNode &start_node, TetrisNode &land_point, TetrisMap &map)->QVector<Oper>
{
    //qDebug() << "path:" << Tool::printNode(start_node)<<Tool::printNode(land_point);
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
            node = std::get<0>(result);
            if (node.type == Piece::None) {
                break;
            }
            path.push_front(std::get<1>(result));
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
                      - m.roof * 100
                      - m.colTrans  * 160
                      - m.rowTrans * 130
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
        auto sky_tslot = [](TetrisMap & map)->std::tuple<bool, int, int, int> {
            auto score = 0;
            for (auto x = 0; x < map.width; x++)
                for (auto y = map.roof; y < map.height; y++)
                {
                    auto y0 = map.data[y];
                    auto y1 = y - 1 > -1 ? map.data[y - 1] : 0;
                    auto y2 = y - 2 > -1 ? map.data[y - 2] : 0;
                    if (((y0 >> x & 7) == 5) && ((y1 >> x & 7) == 0)) {
                        if (BitCount(y0) == map.width - 1) {
                            score += 1;
                            if (BitCount(y1) == map.width - 3) {
                                score += 2;
                                if (((y2 >> x & 7) == 4 || (y2 >> x & 7) == 1)) {
                                    score += 2;
                                    for (auto y1 = y - 3; y1 > 0; y1--) {
                                        if (map(y1, x + 1)) {
                                            score = 0;
                                            break;
                                        }
                                    }
                                    return {true, y - 2, x, score};
                                }
                            }
                        }
                    }
                }
            return {false, -1, -1, score};
        };

        struct TstTwist {
            bool point_left;
            bool  is_tslot;
            int cover;
            int  x;
            int  y;
            bool row1;
            bool row2;
            bool row3;
            int filledRows;
            bool rPoint;
            TstTwist(bool _point_left, int _cover, int _x, int _y, bool _rPoint = false)
            {
                point_left = _point_left;
                cover = _cover;
                is_tslot = (_cover >= 3);
                x = _x;
                y = _y;
                rPoint = _rPoint;
                filledRows = int(row1) + int(row2) + int(row3);
            }
        };

        auto cave_tslot = [](TetrisMap & map, bool West)->std::tuple<bool, int, int> {
            for (auto x = 0; x < map.width; x++)
                for (auto y = map.roof; y < map.height; y++)
                {
                    auto node = TetrisNode{Piece::T, Pos{y, x}, (!West) ? 1 : 3};
                    auto dropDis = node.getDrop(map);
                    if (dropDis == 0 || dropDis % 2 == 1)
                        return {false, -1, -1};
                    auto i = (x + dropDis);
                    auto &j = y;
                    if (West) {
                        if (map(i, j) && map(i + 2, j) && map(i + 2, j + 2) && !map(i + 1, j))
                            return {true, i, j};
                        else if (map(i + 1, j) && map(i + 1, j + 3) && map(i + 3, j + 1) && map(i + 3, j + 3) && !map(i + 2, j + 2) && !map(i + 2, j + 3) && !map(i + 3, j + 2))
                            return {true, i + 1, j + 1};
                    } else {
                        if (map(i, j + 2) && map(i + 2, j + 2) && map(i + 2, j) && !map(i + 1, j + 2))
                            return {true, j, i};
                        else if (map(i + 1, j - 1) && map(i + 3, j - 1) && map(i + 3, j + 1) && map(i + 1, j + 2) && !map(i + 2, j - 1) && !map(i + 2, j) && !map(i + 3, j))
                            return {true, i + 1, j - 1};
                    }
                    return {false, -1, -1};
                }
        };

        auto tst_twist = [](TetrisMap & map)->std::tuple<bool, TstTwist> {
            for (auto x = -1; x < map.width - 2; x++)
                for (auto y = map.roof; y < map.height; y++)
                {
                    auto &i = y;
                    auto &j = x;
                    auto isTstSlot_Left =
                    map(i, j) && !map(i + 1, j) && !map(i + 2, j) && !map(i + 3, j) && !map(i + 3, j + 1) && !map(i + 4, j) && map(i, j + 3) == map(i + 1, j + 3) &&
                    (!bool(map(i + 4, j + 1)) ? map(i + 5, j) == 1 : true) &&
                    !map(i, j + 1) && !map(i, j + 2) && !map(i + 1, j + 1) && !map(i + 1, j + 2);// &&

                    auto isTSlotLeft = (map(i + 2, j - 1) + map(i + 2, j + 1) + map(i + 4, j - 1));
                    if (isTstSlot_Left) {
                        //qDebug() << "youle left";
                        QVector<int> grid{1, 2, 1 + int(!map(i + 4, j + 1))};
                        return{bool(map(i + 2, j + 2)), TstTwist{false, isTSlotLeft, i + 2, j - 1, bool(map(i + 2, j + 2))}
                              };
                    }

                    auto isTstSlot_Right =
                            map(i, j + 3) && !map(i + 1, j + 3) && !map(i + 2, j + 3) && !map(i + 3, j + 3) && !map(i + 3, j + 2) && !map(i + 4, j + 3) && map(i, j) == map(i + 1, j) &&
                            (!bool(map(i + 4, j + 2)) ? map(i + 5, j + 3) == 1 : true) &&
                            !map(i, j + 1) && !map(i, j + 2) && !map(i + 1, j + 1) && !map(i + 1, j + 2);// &&


                    auto isTSlotRight = (map(i + 2, j + 2) + map(i + 2, j + 4) + map(i + 4, j + 4));

                    if (isTstSlot_Right) {
                        //qDebug() << "youle rightt";
                        QVector<int> grid{1, 2, 1 + int(!map(i + 4, j + 2))};
                        return { bool(map(i + 2, j + 2)), TstTwist{true, isTSlotRight, i + 2, j + 2, bool(map(i + 2, j + 2))}
                               };
                    }
                }
            return {false, TstTwist{false, -1, -1, -1}};
        };

        auto tst_twist1 = [](TetrisMap & map)->std::tuple<bool, TstTwist, int> {
            auto value1 = 0;
            for (auto x = 0; x < map.width - 2; x++)
                for (auto y = map.roof; y < map.height; y++)
                {
                    auto y0 = map.data[y];
                    auto y1 = y - 1 > -1 ? map.data[y - 1] : 0;
                    auto y2 = y - 2 > -1 ? map.data[y - 2] : 0;
                    auto y3 = y - 3 > -1 ? map.data[y - 3] : 0;
                    auto y4 = y - 4 > -1 ? map.data[y - 4] : 0;
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
                                                return {true, TstTwist{false, y - 2, x + 1, -1}, value};
                                            } else {
                                                value -= 2;
                                            }
                                        } else
                                            value = 0;
                                    }
                                }
                            }
                            if (value > 3)
                                return {false, TstTwist{false, y - 2, x + 1, -1}, value};
                        }
                        return {false, TstTwist{false, y - 2, x + 1, -1}, value};
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
                                                return {true, TstTwist{false, y - 2, x - 1, -1}, value};
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
                            return {false, TstTwist{false, y - 2, x + 1, -1}, value};
                    }
                }
            return {false, TstTwist{false, -1, -1, -1}, value1};
        };

        auto cutout_tslot = [](TetrisMap & map, TetrisNode & piece)->std::tuple<int, TetrisMap> {
            auto clear = 0;
            if (piece.rotateState == 2)
            {
                clear = piece.attach(map);
            } else
            {
                clear = piece.attach(map);
            }
            /*else
            {
            auto imperial = TetrisNode{piece.type, Pos{piece.pos.x, piece.pos.y}, (piece.rotateState == 1 ? 3 : 1)};
            }*/
            return {clear, map};
        };

        auto value = 0;
        auto ts = 1;
        auto map1 = map;
        int tsShapes[] = {5, 100, 100, 500};

        for (auto i = 0; i <  ts; i++) {
            auto [is, x, y, t2score] = sky_tslot(map1);
            auto [isTst, twist, t3score] = tst_twist1(map1);
            evalResult.t2Value += t2score;
            evalResult.t3Value += t3score;
            /*if (evalResult.t3Value > 4) {
                qDebug() << evalResult.t2Value << evalResult.t3Value;
                Tool::printMap(map1);
            }*/
            if (is) {
                //qDebug()<<"youle t2";
                auto node = TetrisNode{Piece::T, Pos{y, x}, 2};
                auto [clear, mapAfter] = cutout_tslot(map1, node);
                map1 = mapAfter;
                value += tsShapes[clear];
            } else if (isTst) {
                auto node = TetrisNode{Piece::T, Pos{twist.y, twist.x}, (twist.point_left ? 3 : 1)};
                auto [clear, mapAfter] = cutout_tslot(map1, node);
                map1 = mapAfter;
                value += tsShapes[clear];
                //   qDebug() << "youle" << clear;
            } else
                break;
        }

        return value;
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
                             + result.maxAttack * 40
                             + result.attack * 256  * rate
                             + (evalResult.t2Value) * (t_expect() < 8 ? 512 : 320) * 1.5
                             + (evalResult.safe >= 12 ? evalResult.t3Value * (t_expect() < 4 ? 10 : 8) * (result.b2b ? 512 : 256) / (6 + result.underAttack) : 0)
                             + (result.b2b ? 512 : 0)
                             + result.like * 64
                             + 0.
                     ) * std::max<double>(0.05, (full_count_ - evalResult.count - result.mapRise * 10) / double(full_count_))
                     + result.maxCombo * (result.maxCombo) * 40);
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



