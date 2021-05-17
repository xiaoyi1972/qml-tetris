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
#include<vector>
enum class Piece {Trash = -2, None = -1, O, I, T, L, J, S, Z};
enum class Oper {None = -1, Left, Right, SoftDrop, HardDrop, Hold, Cw, Ccw, DropToBottom};
enum class TSpinType {None = -1, TSpinMini, TSpin};

class  Pos {
public:
    Pos() = default;
    Pos(int _x, int _y): x(_x), y(_y) {}
    bool operator==(const Pos &a) const {
        return x == a.x && y == a.y;
    }
    int x, y;
};

class TetrisMap {
public:
    TetrisMap(int _width = 0, int _height = 0): width(_width), height(_height), roof(_height), count(0) {
        data = new int[height];
        std::memset(data, 0, height * sizeof(int));
        std::memset(top, _height, 32 * sizeof(char));
    }

    TetrisMap(const TetrisMap &p) {
        width = p.width;
        height = p.height;
        roof = p.roof;
        count = p.count;
        data = new int[height];
        std::copy(p.data, p.data + height, data);
        std::copy(std::begin(p.top), std::end(p.top), std::begin(top));
    }

    TetrisMap(TetrisMap &&p) {
        width = p.width;
        height = p.height;
        roof = p.roof;
        count = p.count;
        data = std::exchange(p.data, nullptr);
        std::copy(std::begin(p.top), std::end(p.top), std::begin(top));
    }

    TetrisMap &operator=(const TetrisMap &p) {
        if (this != &p) {
            width = p.width;
            height = p.height;
            roof = p.roof;
            count = p.count;
            delete [] data;
            data = new int [height];
            std::copy(p.data, p.data + height, data);
            std::copy(std::begin(p.top), std::end(p.top), std::begin(top));
        }
        return *this;
    }

    TetrisMap &operator=(TetrisMap &&p) {
        if (this != &p) {
            width = p.width;
            height = p.height;
            roof = p.roof;
            count = p.count;
            delete [] data;
            data = std::exchange(p.data, nullptr);
            std::copy(std::begin(p.top), std::end(p.top), std::begin(top));
        }
        return *this;
    }

    ~TetrisMap() {
        delete [] data;
        data = nullptr;
    }

    auto clear() {
        std::vector<int> change;
        auto full = (1 << width) - 1;
        for (auto i = 0; i < height ; i++) {
            if (data[i] == full) {
                change.push_back(i);
                std::copy(data, data + i, data + 1);
                data[0] = 0;
            }
        }
        count -= change.size() * width;
        roof += change.size();
        for (auto i = 0; i < width; i++)
            top[i] += (top[i] < height ? change.size() : 0);
        return change;
    }

    int full(int x, int y) const {
        return  operator()(x, y);
    }

    int row(int x) const {
        if (x < 0 || x > height - 1)
            return 0;
        else
            return data[x];
    }

    int operator()(int x, int y) const {
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

class TetrisMapEx: public TetrisMap {
public:
    using colorDatasType = std::vector<std::vector<Piece>>;
    using pieceArrType = std::vector<Piece>;
    TetrisMapEx(int _width = 0, int _height = 0): TetrisMap(_width, _height) {
        colorDatas.clear();
        colorDatas.insert(colorDatas.begin(), height, pieceArrType(_width, Piece::None));
    }

    TetrisMapEx(const TetrisMapEx &p): TetrisMap(p) {
        //  qDebug() << QString::fromLocal8Bit("mapEx拷贝构造");
        colorDatas = p.colorDatas;
    }

    TetrisMapEx(TetrisMapEx &&p): TetrisMap(std::move(p)) {
//      qDebug() << QString::fromLocal8Bit("mapEx移动构造");
        colorDatas = std::move(p.colorDatas);
    }

    TetrisMapEx &operator=(const TetrisMapEx &p) {
        if (this != &p) {
            TetrisMap::operator=(p);
            colorDatas = p.colorDatas;
        }
        return *this;
    }

    TetrisMapEx &operator=(TetrisMapEx &&p) {
        //  qDebug() << QString::fromLocal8Bit("调用=移动");
        if (this != &p) {
            TetrisMap::operator=(p);
            colorDatas = std::move(p.colorDatas);
        }
        return *this;
    }

    auto clear() {
        std::vector<int> change = TetrisMap::clear();
        for (const auto &i : change) {
            colorDatas.erase(colorDatas.begin() + i);
            colorDatas.insert(colorDatas.begin(), pieceArrType(width, Piece::None));
        }
        return change;
    }

    void  update(bool isColor = true) {
        roof = height;
        count = 0;
        for (auto i = 0; i < height; i++)
            for (auto j = 0; j < width; j++) {
                if (full(i, j)) {
                    roof = std::min(roof, i);
                    top[j] = std::min<char>(top[j], i);
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

namespace customHash {
inline void hashCombine(std::size_t &) { }
template <typename T, typename... Rest>
inline void hashCombine(std::size_t &seed, const T &v, Rest... rest) {
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hashCombine(seed, rest...);
}
}


class TetrisNode {
    using data = std::vector<int>;
public:
    TetrisNode(Piece _type = Piece::None, Pos && _pos = Pos(3, 0), int _rotateState = 0) {
        pos = _pos;
        type = _type;
        rotateState = _rotateState;
    }

    inline int full(int x, int y) const {return operator()(x, y);}

    inline int operator()(int x, int y) const {
        const auto &data = rotateDatas[type][rotateState];
        if (x < 0 || x > data.size() - 1 || y < 0 || y > data.size() - 1)
            return 0;
        else
            return (data.at(x) >> y) & 1;
    }

    bool operator!=(const TetrisNode &p) const {return !operator==(p);}
    bool operator==(const TetrisNode &p) const {return pos == p.pos && rotateState == p.rotateState && type == p.type;}
    bool operator <(const TetrisNode &p) const {return rotateState == p.rotateState ? pos.x < p.pos.x : rotateState < p.rotateState;}
    bool operator >(const TetrisNode &p) const {return rotateState == p.rotateState ? pos.x > p.pos.x : rotateState > p.rotateState;}

    void shift(int _x, int _y) {
        pos.x += _x;
        pos.y += _y;
    }

    void rotate(int _reverse = 0, int _index = 1) {
        rotateState = !_reverse ? rotateState + _index : rotateState - _index;
        rotateState = (rotateState < 0 ? rotateState + 4 : rotateState) % 4;
    }

    bool check(TetrisMap &map, int _x, int  _y) const {
        auto real = true;
        const auto &data = rotateDatas[type][rotateState];
        auto size = data.size();
        for (int x = 0; x < size; x++)
            for (int y = 0; y < size; y++) {
                if (full(y, x) && map(_y + y, _x + x)) {
                    real = false;
                    return real;
                }
            }
        return real;
    }

    bool check(TetrisMap &map) const {return check(map, pos.x, pos.y);}

    std::tuple<std::vector<Pos>, std::vector<int>> attachs(TetrisMapEx &map, const Pos &_pos, char _rotateState) {
        std::vector<Pos> change;
        const auto &data = rotateDatas[type][_rotateState];
        auto size = data.size();
        for (auto i = 0; i < size; i++) {
            auto dataI = _pos.x > 0 ? data.at(i) << _pos.x : data.at(i) >> std::abs(_pos.x);
            map.data[i + _pos.y] |= dataI;
            if (dataI)
                map.roof = std::min(_pos.y + i, map.roof);
        }
        for (int x = 0; x < size; x++)
            for (int y = 0; y < size; y++) {
                if (full(y, x)) {
                    map.count++;
                    change.push_back(Pos{_pos.y + y, _pos.x + x});
                    map.colorDatas[_pos.y + y][_pos.x + x] = type;
                    map.top[_pos.x + x] = std::min<char>(_pos.y + y, map.top[_pos.x + x]);
                }
            }
        return {change, map.clear()};
    }

    std::size_t attach(TetrisMap &map, const Pos &_pos, char _rotateState) {
        const auto &data = rotateDatas[type][_rotateState];
        auto size = data.size();
        for (auto i = 0; i < size; i++) {
            auto dataI = _pos.x > 0 ? data.at(i) << _pos.x : data.at(i) >> std::abs(_pos.x);
            map.data[i + _pos.y] |= dataI;
            if (dataI)
                map.roof = std::min(_pos.y + i, map.roof);
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

    std::tuple<std::vector<Pos>, std::vector<int>> attachs(TetrisMapEx &map) { return attachs(map, pos, rotateState);}
    std::size_t attach(TetrisMap &map) {return attach(map, pos, rotateState);}

    std::size_t fillRows(TetrisMap &map, const Pos &_pos, char _rotateState) {
        const auto &data = rotateDatas[type][_rotateState];
        const auto full = (1 << map.width) - 1;
        auto size = data.size();
        size_t count = 0;
        for (auto i = 0; i < size; i++) {
            auto dataI = _pos.x > 0 ? data.at(i) << _pos.x : data.at(i) >> std::abs(_pos.x);
            if (map.data[i + _pos.y] & dataI)
                return 0;
            auto dataBits = (map.data[i + _pos.y] | dataI);
            if (dataI && dataBits == full)
                count++;
        }
        return count;
    }

    int getDrop(TetrisMap &map) {
        auto i = 0;
        if (!check(map, pos.x, pos.y))
            return i;
        else
            while (check(map, pos.x, pos.y + i))
                i++;
        return i - 1;
    }

    auto getkickDatas(bool dirReverse = false) {
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
        if (type != Piece::I)
            return kickDatas[true][i];
        else
            return kickDatas[false][i];
    }

    bool open(TetrisMap &_map) {
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

    TetrisNode drop(TetrisMap &_map) {
        auto next = *this;
        next.shift(0, next.getDrop(_map));
        return next;
    }

    static TetrisNode spawn(Piece _type) {return TetrisNode{_type, Pos{3, _type == Piece::O ? -1 : 0}};}
    static bool shift(TetrisNode &_tn, TetrisMap &_map, int _x, int _y) {
        _tn.shift(_x, _y);
        if (!_tn.check(_map)) {
            _tn.shift(-_x, -_y);
            return false;
        } else
            return true;
    }

    static bool rotate(TetrisNode &_tn, TetrisMap &_map, bool _reverse = true) {
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

    std::tuple<bool, bool> corner3(TetrisMap &map) {
        auto mini = 0, sum = 0;
        auto &_x = pos.x;
        auto &_y = pos.y;

        auto occupied = [&](int y, int x) {
            return int(!full(y, x) && map(_y + y, _x + x));
        };

        mini = occupied(0, 1) + occupied(1, 0) + occupied(2, 1) + occupied(1, 2);
        sum = map(_y, _x) + map(_y + 2, _x) + map(_y, _x + 2) + map(_y + 2, _x + 2);
        return{sum > 2, mini == 1};
    }

    auto cellsEqual() {
        size_t seed = 0;
        const auto &data = rotateDatas[type][rotateState];
        auto size = data.size();
        for (int x = 0; x < size; x++)
            for (int y = 0; y < size; y++) {
                if (full(y, x))
                    customHash::hashCombine(seed, pos.x + x, pos.y + y);
            }
        return seed;
    }

    Pos pos;
    Piece type;
    char rotateState;
    bool mini = false, spin = false, lastRotate = false;
    TSpinType typeTSpin = TSpinType::None;
    static std::map<Piece, std::vector<data>> rotateDatas;
    static std::map<bool, std::vector<data>> kickDatas;
};


struct searchNode {
    TetrisNode node;
    int count;
    searchNode(TetrisNode &_node, int _count): node{_node}, count(_count) {}
    searchNode(TetrisNode &&_node, int _count): node{_node}, count(_count) {}
    bool operator==(const searchNode &p) const {
        return const_cast<TetrisNode &>(node).cellsEqual() == const_cast<TetrisNode &>(p.node).cellsEqual();
    }
};

namespace std {
template<>
struct hash<TetrisNode> {
public:
    size_t operator()(const TetrisNode &key) const {
        size_t seed = 0;
        customHash::hashCombine(seed, key.pos.x, key.pos.y, key.rotateState, static_cast<int>(key.type));
        return seed;
    }
};

template<>
struct hash<searchNode> {
public:
    size_t operator()(const searchNode &key) const {
        return const_cast<TetrisNode &>(key.node).cellsEqual() ;
    }
};
}

inline uint qHash(const TetrisNode &key, uint seed) {
    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.pos.x);
    seed = hash(seed, key.pos.y);
    seed = hash(seed, key.rotateState);
    seed = hash(seed, static_cast<int>(key.type));
    return seed;
}

inline uint qHash(const searchNode &key, uint seed) {
    seed = const_cast<TetrisNode &>(key.node).cellsEqual();
    return seed;
}

class Random {
public:
    Random(int _seed = QTime(0, 0, 0).msecsTo(QTime::currentTime())) {
        seed = _seed;
        rand.seed(seed);
    }

    void getBag() {
        if (constexpr bool  test = false; test) {
            //  bag.fill(Piece::T, 7);
            bag = QVector<Piece> {Piece::O, Piece::I};
            return;
        }
        QVector<Piece> bagSets{Piece::O, Piece::I, Piece::T, Piece::L, Piece:: J, Piece:: S, Piece:: Z};
        while (bag.size() < 7) {
            auto piece = bagSets.takeAt(rand.generate() % bagSets.size());
            bag.append(piece);
        }
    }

    Piece getOne() {
        if (bag.size() == 0) { //[0,1,2,3,4,5,6]
            getBag();
            if (displayBag.size() == 0)
                displayBag = bag;
            bag.resize(0);
            getBag();
        }
        if (displayBag.size() < 7)
            displayBag.append(bag.takeFirst());
        auto num = displayBag.takeFirst();
        return num;
    }

    QVector<Piece> bag, displayBag;
    int seed;
    QRandomGenerator rand;
};

class Hold {
public:
    Hold(Random *_rand = nullptr) {
        able = true;
        type = Piece::None;
        rand = _rand;
    }

    ~Hold() {
        rand = nullptr;
    }

    void reset() {
        able = true;
    }

    int exchange(TetrisNode &_cur) {
        auto _curType = _cur.type;
        auto res = 0;
        if (able) {
            if (type == Piece::None) {
                type = _curType;
                _cur = TetrisNode::spawn(rand->getOne());
                res = 2;
            } else {
                std::swap(type, _curType);
                _cur = TetrisNode::spawn(_curType);
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

class Recorder {
public:
    int seed, firstTime = 0;
    int playIndex = 0, timeAll = 0, gameType = 1;
    QVector<int>time;
    QVector<Oper>oper;

    Recorder(int _seed = -1) {
        seed = _seed;
    }

    int timeRecord() {
        using std::chrono::high_resolution_clock;
        using std::chrono::milliseconds;
        static auto startTime = high_resolution_clock::now();
        auto now = high_resolution_clock::now();
        milliseconds timeDelta = std::chrono::duration_cast<milliseconds>(now - startTime);
        startTime = now;
        return timeDelta.count();
    }

    void clear() {
        time.clear();
        oper.clear();
        playIndex = 0;
    }

    friend QDataStream &operator<<(QDataStream &out, const Recorder &ss) {
        out << ss.seed << ss.playIndex << ss.timeAll << ss.gameType << ss. firstTime << ss.time << ss.oper ;
        return out;
    }

    friend  QDataStream &operator>>(QDataStream &in, Recorder &ss) {
        in >> ss.seed >> ss.playIndex >> ss.timeAll >> ss.gameType >> ss. firstTime >> ss.time >> ss.oper;
        return in;
    }

    static Recorder readStruct(const QByteArray &ba) {
        Recorder ss;
        QBuffer buf(&const_cast<QByteArray &>(ba));  //输出
        buf.open(QIODevice::ReadOnly);
        QDataStream in(&buf);
        in >> ss;
        buf.close();
        return ss;
    }

    static QByteArray writeStruct(Recorder &ss) {
        QByteArray ba;
        ba.resize(sizeof(Recorder));
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        QDataStream out(&buffer);      //输入
        out << ss;
        buffer.close();
        return ba;
    }

    void add(Oper _operation, int _time) {
        auto len = time.size();
        if (len == 0)
            firstTime = _time;
        oper.append(_operation);
        time.append(_time);
    }

    auto isEnd() {
        return playIndex < time.size() ? false : true;
    }

    std::tuple<int, Oper>play() {
        auto playIndex_ = playIndex++;
        return{playIndex >= time.size() ? 0 : time[playIndex], playIndex_ >= oper.size() ? Oper::None : oper[playIndex_]};
    }

    void reset() {
        playIndex = 0;
    }
};

#endif // TETRISBASE_H
