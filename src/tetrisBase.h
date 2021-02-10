#ifndef TETRISBASE_H
#define TETRISBASE_H
#include<QMap>
#include<QVector>
#include<QTime>
#include<QDebug>
#include<QDataStream>
#include<QBuffer>
#include<QRandomGenerator>
#include<functional>
#include<cmath>
enum class Piece {Trash = -2, None = -1, O, I, T, L, J, S, Z};
enum class Oper {None = -1, Left, Right, SoftDrop, HardDrop, Hold, Cw, Ccw, DropToBottom};
enum class  TSpinType {None = -1, TSpinMini, TSpin};

class  Pos
{
public:
    Pos() = default;
    Pos(int _x, int _y): x(_x), y(_y) {}
    bool operator==(const Pos &a) const
    {
        return x == a.x && y == a.y;
    }
    int x, y;
};

class TetrisMap
{
public:
    TetrisMap(int _width = 0, int _height = 0)
    {
        width = _width;
        height = _height;
        roof = _height;
        count = 0;
        data = new int[height];
        std::memset(data, 0, height * sizeof(int));
        std::memset(top, _height, 32 * sizeof(char));
    }

    TetrisMap(const TetrisMap &other)
    {
        width = other.width;
        height = other.height;
        roof = other.roof;
        count = other.count;
        data = new int[height];
        std::memcpy(data, other.data, height * sizeof(int));
        std::memcpy(top, other.top, 32 * sizeof(char));
    }

    void operator=(TetrisMap &other)
    {
        width = other.width;
        height = other.height;
        roof = other.roof;
        count = other.count;
        delete [] data;
        data = nullptr;
        data = new int[height];
        std::memcpy(data, other.data, height * sizeof(int));
        std::memcpy(top, other.top, 32 * sizeof(char));
    }

    void operator=(TetrisMap &&other)
    {
        width = other.width;
        height = other.height;
        roof = other.roof;
        count = other.count;
        delete [] data;
        data = nullptr;
        data = new int[height];
        std::memcpy(data, other.data, height * sizeof(int));
        std::memcpy(top, other.top, 32 * sizeof(char));
    }

    ~TetrisMap()
    {
        delete [] data;
        data = nullptr;
    }

    auto clear()
    {
        QVector<int> change;
        auto full = (1 << width) - 1;
        for (auto i = 0; i < height ; i++) {
            if (data[i] == full) {
                change.append(i);
                std::memcpy(data + 1, data, i * sizeof(int));
                data[0] = 0;
            }
        }
        count -= change.size() * width;
        roof += change.size();
        for (auto i = 0; i < width; i++) {
            top[i] += (top[i] < height ? change.size() : 0);
        }
        return change;
    }

    int full(int x, int y) const
    {
        return  operator()(x, y);
    }

    int operator()(int x, int y) const
    {
        if (x < 0 || x > height - 1 || y < 0 || y > width - 1)
            return 1;
        else
            return (data[x] >> y) & 1;
    }
    int width, height;//宽 高
    int roof, count; //最高位置 块数
    int *data = nullptr; //数据
    char top[32] = {}; //每列高度
};

class TetrisMapEx: public TetrisMap
{
public:
    using colorDatasType = QVector<QVector<Piece>>;
    using pieceArrType = QVector<Piece>;
    TetrisMapEx(int _width = 0, int _height = 0): TetrisMap(_width, _height)
    {
        colorDatas.resize(height);
        colorDatas.fill(pieceArrType(width).fill(Piece::None));
    }

    TetrisMapEx(const TetrisMapEx &other): TetrisMap(other)
    {
        colorDatas = other.colorDatas;
    }

    void operator=(TetrisMapEx &other)
    {
        TetrisMap::operator=(other);
        colorDatas = other.colorDatas;
    }

    void operator=(TetrisMapEx &&other)
    {
        TetrisMap::operator=(std::move(other));
        colorDatas = other.colorDatas;
    }

    ~TetrisMapEx()
    {
        delete [] data;
        data = nullptr;
    }

    auto clear()
    {
        QVector<int> change = TetrisMap::clear();
        for (const auto &i : change) {
            colorDatas.remove(i);
            colorDatas.push_front(pieceArrType(width).fill(Piece::None));
        }
        return change;
    }

    void  update(bool isColor = true)
    {
        roof = height;
        count = 0;
        for (auto i = 0; i < height; i++)
            for (auto j = 0; j < width; j++) {
                if (operator()(i, j)) {
                    roof = std::min(roof, i);
                    count++;
                    if (isColor)
                        if (colorDatas[i][j] == Piece::None)
                            colorDatas[i][j] = Piece::Trash;
                } else {
                    if (isColor)
                        colorDatas[i][j] = Piece::None;
                }
            }
    }
    colorDatasType colorDatas;
};

class TetrisNode
{
    using data = QVector<int>;
public:
    TetrisNode(Piece _type = Piece::None, Pos && _pos = Pos(3, 0), int _rotateState = 0)
    {
        if (_type == Piece::O)
            _pos.y--;
        pos = _pos;
        type = _type;
        rotateState = _rotateState;
        mdata = &rotateDatas[type][rotateState];
    }

    int full(int x, int y) const
    {
        return operator()(x, y);
    }

    int operator()(int x, int y) const
    {
        if (x < 0 || x > mdata->size() - 1 || y < 0 || y > mdata->size() - 1)
            return 0;
        else
            return (mdata->at(x) >> y) & 1;
    }

    bool operator==(const TetrisNode &other) const
    {
        return pos == other.pos && rotateState == other.rotateState && type == other.type;
    }

    bool operator <(const TetrisNode &other) const
    {
        return rotateState == other.rotateState ? pos.x < other.pos.x : rotateState < other.rotateState;
    }

    bool operator >(const TetrisNode &other) const
    {
        return rotateState == other.rotateState ? pos.x > other.pos.x : rotateState > other.rotateState;
    }

    void rotate(int _reverse = 0, int _index = 1)
    {
        rotateState = !_reverse ? rotateState + _index : rotateState - _index;
        rotateState = (rotateState < 0 ? rotateState + 4 : rotateState) % 4;
        mdata = &rotateDatas[type][rotateState];
    }

    bool check(TetrisMap &map, int _x, int  _y) const
    {
        auto real = true;
        auto size = mdata->size();
        for (int x = 0; x < size; x++)
            for (int y = 0; y < size; y++) {
                if (full(y, x) && map(_y + y, _x + x)) {
                    real = false;
                    return real;
                }
            }
        return real;
    }

    bool check(TetrisMap &map) const
    {
        return check(map, pos.x, pos.y);
    }

    void shift(int _x, int _y)
    {
        pos.x += _x;
        pos.y += _y;
    }

    std::tuple<QVector<Pos>, QVector<int>> attachs(TetrisMapEx &map)
    {
        QVector<Pos> change;
        auto size = mdata->size();
        for (auto i = 0; i < size; i++) {
            auto dataI = pos.x > 0 ? mdata->at(i) << pos.x : mdata->at(i) >> std::abs(pos.x);
            map.data[i + pos.y] |= dataI;
            if (dataI) {
                map.roof = std::min(pos.y + i, map.roof);
            }
        }
        for (int x = 0; x < size; x++)
            for (int y = 0; y < size; y++) {
                if (full(y, x)) {
                    map.count++;
                    change.append(Pos{pos.y + y, pos.x + x});
                    map.colorDatas[pos.y + y][pos.x + x] = type;
                    map.top[pos.x + x] = std::min<char>(pos.y + y, map.top[pos.x + x]);
                }
            }
        return {change, map.clear()};
    }

    int attach(TetrisMap &map)
    {
        return attach(map, pos);
    }

    int attach(TetrisMap &map, const Pos &_pos)
    {
        auto size = mdata->size();
        for (auto i = 0; i < size; i++) {
            auto dataI = _pos.x > 0 ? mdata->at(i) << _pos.x : mdata->at(i) >> std::abs(_pos.x);
            map.data[i + _pos.y] |= dataI;
            if (dataI) {
                map.roof = std::min(_pos.y + i, map.roof);
            }
        }
        for (int x = 0; x < size; x++)
            for (int y = 0; y < size; y++) {
                if (full(y, x)) {
                    map.top[_pos.x + x] = std::min<char>(_pos.y + y, map.top[_pos.x + x]);
                    map.count++;
                }
            }
        auto clears = map.clear();
        auto clearsNum = clears.size();
        return clearsNum;
    }

    int getDrop(TetrisMap &map)
    {
        auto i = 0;
        if (!check(map, pos.x, pos.y))
            return i;
        else
            while (check(map, pos.x, pos.y + i))
                i++;
        return i - 1;
    }

    auto getkickDatas(bool dirReverse = false)
    {
        auto r1 = rotateState;
        auto r2 = r1;
        r2 = !dirReverse ? ++r2 : --r2;
        r2 = (r2 < 0 ? r2 + 4 : r2) % 4;
        int i = -1;
        switch (r1) {
        case 0: i = r2 == 1 ? 0 : 7; break;
        case 1: i = r2 == 0 ? 1 : 2; break;
        case 2: i = r2 == 1 ? 3 : 4; break;
        case 3: i = r2 == 2 ? 5 : 6; break;
        default: i = -1; break;
        }
        if (type != Piece::I) {
            return kickDatas[true][i];
        } else {
            return kickDatas[false][i];
        }
    }

    bool open(TetrisMap &_map)
    {
        if (!check(_map, pos.x, pos.y))
            return false;
        else {
            auto i = 0;
            while (check(_map, pos.x, pos.y + i))
                i--;
            if (i + pos.y < 0)
                return true;
            else
                return false;
        }
    }

    static bool shift(TetrisNode &_tn, TetrisMap &_map, int _x, int _y)
    {
        _tn.shift(_x, _y);
        if (!_tn.check(_map)) {
            _tn.shift(-_x, -_y);
            return false;
        } else
            return true;
    }

    static bool rotate(TetrisNode &_tn, TetrisMap &_map, bool _reverse = true)
    {
        if (_tn.type == Piece::O)
            return false;
        auto kd = _tn.getkickDatas(_reverse);
        _tn.rotate(_reverse ? 1 : 0);
        auto prePos = _tn.pos;
        if (!_tn.check(_map)) {
            auto offsetRes = false;
            for (auto i = 0; i < kd.size() / 2; i++) {
                _tn.pos.x = prePos.x + kd.at(i * 2);
                _tn.pos.y = prePos.y - kd.at(i * 2 + 1);
                if (_tn.check(_map)) {
                    offsetRes = true;
                    break;
                }
            }
            if (!offsetRes) {
                _tn.pos = prePos;
                _tn.rotate(_reverse ? 0 : 1);
                return false;
            }
        }
        return true;
    }

    TetrisNode drop(TetrisMap &_map)
    {
        auto next = *this;
        next.shift(0, next.getDrop(_map));
        return next;
    }

    std::tuple<bool, bool> corner3(TetrisMap &map)
    {
        auto mini = 0, sum = 0;
        auto &_x = pos.x;
        auto &_y = pos.y;
#define occupied(y,x) int(!full(y, x) && map(_y + y, _x + x))
        mini = occupied(0, 1) + occupied(1, 0) + occupied(2, 1) + occupied(1, 2);
        sum = map(_y, _x) + map(_y + 2, _x) + map(_y, _x + 2) + map(_y + 2, _x + 2);
        return{sum > 2, mini == 1};
    }

    uint cellsEqual()
    {
        uint seed = 0;
        QtPrivate::QHashCombine hash;
        auto size = mdata->size();
        for (int x = 0; x < size; x++)
            for (int y = 0; y < size; y++) {
                if (full(y, x)) {
                    seed = hash(seed, pos.x + x);
                    seed = hash(seed, pos.y + y);
                }
            }
        return seed;
    }

    Pos pos;
    data *mdata;
    Piece type;
    char rotateState;
    bool mini = false, spin = false, lastRotate = false;
    TSpinType typeTSpin = TSpinType::None;
    static QMap<Piece, QVector<data >> rotateDatas;
    static QMap<bool, QVector<QVector<int> >> kickDatas;
};


struct info {
    TetrisNode node;
    int count;
    info(TetrisNode &_node, int _count)
    {
        node = _node;
        count = _count;
    }

    info(TetrisNode &&_node, int _count)
    {
        node = _node;
        count = _count;
    }

    bool operator==(const info &other) const
    {
        return const_cast<TetrisNode &>(node).cellsEqual() == const_cast<TetrisNode &>(other.node).cellsEqual();
    }
};

inline uint qHash(const TetrisNode &key, uint seed)
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.pos.x);
    seed = hash(seed, key.pos.y);
    seed = hash(seed, key.rotateState);
    seed = hash(seed, static_cast<int>(key.type));
    return seed;
}

inline uint qHash(const info &key, uint seed)
{
    seed = const_cast<TetrisNode &>(key.node).cellsEqual();
    return seed;
}


class Random
{
public:
    Random(int _seed = QTime(0, 0, 0).msecsTo(QTime::currentTime()))
    {
        seed = _seed;
        rand.seed(seed);
        // qsrand(seed);        //设置随机数种子
    }

    void getBag()
    {
        //bag.fill(Piece::T, 7);
        //bag=QVector<Piece>{Piece::J,Piece::Z,Piece::S,Piece::J,Piece::Z,Piece::T,Piece::I};
        //return;
        do {
            //  auto num = static_cast<Piece>(qrand() % 7);
            auto num = static_cast<Piece>(rand.generate() % 7);
            if (bag.contains(num)) {
                continue;
            } else {
                bag.append(num);
            }
        } while (bag.size() != 7);
    }

    Piece getOne()
    {
        if (bag.size() == 0) { //[0,1,2,3,4,5,6]
            getBag();
            if (displayBag.size() == 0)
                displayBag = bag;
            bag.resize(0);
            getBag();
        }
        if (displayBag.size() < 7) {
            displayBag.append(bag.takeFirst());
        }
        auto num = displayBag.takeFirst();
        return num;
    }

    QVector<Piece> bag, displayBag;
    int seed;
    QRandomGenerator rand;
};

class Hold
{
public:
    Hold(Random *_rand = nullptr)
    {
        able = true;
        type = Piece::None;
        rand = _rand;
    }

    ~Hold()
    {
        rand = nullptr;
    }

    void reset()
    {
        able = true;
    }

    int exchange(TetrisNode &_cur)
    {
        auto _curType = _cur.type;
        auto res = 0;
        if (able) {
            if (type == Piece::None) {
                type = _curType;
                _cur = TetrisNode{rand->getOne()};
                res = 2;
            } else {
                qSwap(type, _curType);
                _cur = TetrisNode{_curType};
                res = 1;
            }
            able = !able;
        }
        return res;
    }
    bool able = true;
    Piece type = Piece::None;
    Random *rand = nullptr;
};

class Recorder
{

public:
    int seed, firstTime = 0;
    int playIndex = 0, timeAll = 0, gameType = 1;
    QVector<int>time;
    QVector<Oper>oper;

    Recorder(int _seed = -1)
    {
        seed = _seed;
    }

    void clear()
    {
        time.clear();
        oper.clear();
        playIndex = 0;
    }

    friend QDataStream &operator<<(QDataStream &out, const Recorder &ss)
    {
        out << ss.seed << ss.playIndex << ss.timeAll << ss.gameType << ss. firstTime << ss.time << ss.oper ;
        return out;
    }

    friend  QDataStream &operator>>(QDataStream &in, Recorder &ss)
    {
        in >> ss.seed >> ss.playIndex >> ss.timeAll >> ss.gameType >> ss. firstTime >> ss.time >> ss.oper;
        return in;
    }

    static Recorder readStruct(const QByteArray &ba)
    {
        Recorder ss;
        QBuffer buf(&const_cast<QByteArray &>(ba));  //输出
        buf.open(QIODevice::ReadOnly);
        QDataStream in(&buf);
        in >> ss;
        buf.close();
        return ss;
    }

    static QByteArray writeStruct(Recorder &ss)
    {
        QByteArray ba;
        ba.resize(sizeof(Recorder));
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        QDataStream out(&buffer);      //输入
        out << ss;
        buffer.close();
        return ba;
    }

    void add(Oper _operation, int _time)
    {
        auto len = time.size();
        if (len == 0) {
            firstTime = _time;
        }
        oper.append(_operation);
        time.append(_time);
    }

    auto isEnd()
    {
        return playIndex < time.size() ? false : true;
    }

    std::tuple<int, Oper>play()
    {
        auto playIndex_ = playIndex++;
        return{playIndex >= time.size() ? 0 : time[playIndex], playIndex_ >= oper.size() ? Oper::None : oper[playIndex_]};
    }

    void reset()
    {
        playIndex = 0;
    }
};

#endif // TETRISBASE_H
