#ifndef TETRISBASE_H
#define TETRISBASE_H
#include<QMap>
#include<QVector>
#include <QTime>
#include<QDebug>
#include<QString>
#include <QtMath>
#include <QSet>

enum class Piece {Trash = -2, None = -1, O, I, T, L, J, S, Z};
enum class Oper {None = -1, Left, Right, SoftDrop, HardDrop, Hold, Cw, Ccw, DropToBottom};
enum class  TspinType {None = -1, TspinMini, Tspin};
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
    using colorDatasType = QVector<QVector<Piece>>;
    using pieceArrType = QVector<Piece>;
    TetrisMap(int _width = 0, int _height = 0): width(_width), height(_height)
    {
    roof = _height;
    count = 0;
    data = new int[height];
    memset(data, 0, height * sizeof(int));
    colorDatas.resize(height);
    colorDatas.fill(pieceArrType(width).fill(Piece::None));
    }

    TetrisMap(const TetrisMap &A)
    {
    width = A.width;
    height = A.height;
    roof = A.roof;
    count = A.count;
    data = new int[height];
    memcpy(data, A.data, height * sizeof(int));
    colorDatas = A.colorDatas;
    }

    void operator=(TetrisMap &A)
    {
    width = A.width;
    height = A.height;
    roof = A.roof;
    count = A.count;
    delete [] data;
    data = nullptr;
    data = new int[height];
    memcpy(data, A.data, height * sizeof(int));
    colorDatas = A.colorDatas;
    }

    void operator=(TetrisMap &&A)
    {
    width = A.width;
    height = A.height;
    roof = A.roof;
    count = A.count;
    delete [] data;
    data = nullptr;
    data = new int[height];
    memcpy(data, A.data, height * sizeof(int));
    colorDatas = A.colorDatas;
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
    for (auto i = 0; i < height ; i++)
    {
        if (data[i] == full)
        {
        change.append(i);
        memcpy(data + 1, data, i * sizeof(int));
        data[0] = 0;
        colorDatas.remove(i);
        colorDatas.push_front(pieceArrType(width).fill(Piece::None));
        }
    }
    count -= change.size() * width;
    roof += change.size();
    return change;
    }

    int operator()(int x, int y)
    {
    if (x < 0 || x > height - 1 || y < 0 || y > width - 1)
        return 1;
    else
        return (data[x] >> y) & 1;
    }
    int width, height;//宽 高
    int roof, count; //最高位置 块数
    int *data = nullptr; //数据
    colorDatasType colorDatas;
};


class TetrisNode
{
    using datas = QVector<QVector<int>*> ;
    using data = QVector<int>;
public:
    TetrisNode(Piece _type = Piece::None, Pos && _pos = Pos(3, 0), int _rotateState = 0)
    {
    if (_type == Piece::O)
        _pos.y--;
    pos = _pos;
    type = _type;
    rotateState = _rotateState;
    mdata = rotateDatas[type]->at(rotateState);
    }

    int full(int x, int y)
    {
    return operator()(x, y);
    }

    int operator()(int x, int y)
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
    mdata = rotateDatas[type]->at(rotateState);
    }

    int check(TetrisMap &map, int _x, int  _y)
    {
    int real = 1;
    auto size = mdata->size();
    for (int x = 0; x < size; x++)
        for (int y = 0; y < size; y++)
        {
        if (operator()(y, x) && map(_y + y, _x + x))
        {
            real = 0;
            return real;
        }
        }
    return real;
    }


    int check(TetrisMap &map)
    {
    return check(map, pos.x, pos.y);
    }

    void shift(int _x, int _y)
    {
    pos.x += _x;
    pos.y += _y;
    }

    std::tuple<QVector<Pos>, QVector<int>> attachs(TetrisMap &map)
    {
    QVector<Pos> change;
    auto size = mdata->size();
    for (auto i = 0; i < size; i++)
    {
        auto dataI = pos.x > 0 ? mdata->at(i) << pos.x : mdata->at(i) >> qAbs(pos.x);
        map.data[i + pos.y] |= dataI;
        if (dataI)
        {
        map.roof = std::min(pos.y + i, map.roof);
        }
    }
    for (int x = 0; x < size; x++)
        for (int y = 0; y < size; y++)
        {
        if (operator()(y, x))
        {
            map.count++;
            change.append(Pos{pos.y + y, pos.x + x});
            map.colorDatas[pos.y + y][pos.x + x] = type;
        }
        }
    return {change, map.clear()};
    }

    int attach(TetrisMap &map)
    {
    auto size = mdata->size();
    for (auto i = 0; i < size; i++)
    {
        auto dataI = pos.x > 0 ? mdata->at(i) << pos.x : mdata->at(i) >> qAbs(pos.x);
        map.data[i + pos.y] |= dataI;
        if (dataI)
        {
        map.roof = std::min(pos.y + i, map.roof);
        }
    }
    for (int x = 0; x < size; x++)
        for (int y = 0; y < size; y++)
        {
        if (operator()(y, x))
        {
            map.count++;
            map.colorDatas[pos.y + y][pos.x + x] = type;
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

    if (r1 == 0)
    {
        switch (r2)
        {
        case 1: i = 0; break; //0->R
        case 3: i = 7; break; //0->L
        }
    }
    else if (r1 == 1)
    {
        switch (r2)
        {
        case 0: i = 1; break; //R->0
        case 2: i = 2; break; //R->2
        }
    }
    else if (r1 == 2)
    {
        switch (r2)
        {
        case 1: i = 3; break;//2->R
        case 3: i = 4; break;//2->L
        }
    }
    else if (r1 == 3)
    {
        switch (r2)
        {
        case 2: i = 5; break;//L->2
        case 0: i = 6; break;//L->0
        }
    }
    if (type != Piece::I)
    {
        return kickDatas[true]->at(i);
    }
    else
    {
        return kickDatas[false]->at(i);
    }
    }

    bool open(TetrisMap &_map)
    {

    if (!check(_map, pos.x, pos.y))
        return false;
    else
    {
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
    if (!_tn.check(_map))
    {
        _tn.shift(-_x, -_y);
        return false;
    }
    else
        return true;
    }

    static bool rotate(TetrisNode &_tn, TetrisMap &_map, bool _reverse = true)
    {
    if (_tn.type == Piece::O)
        return false;
    auto kd = _tn.getkickDatas(_reverse);
    _tn.rotate(_reverse ? 1 : 0);
    auto prePos = _tn.pos;
    if (!_tn.check(_map))
    {
        auto offsetRes = false;
        for (auto i = 0; i < kd->size() / 2; i++)
        {
        _tn.pos.x = prePos.x + kd->at(i * 2);
        _tn.pos.y = prePos.y - kd->at(i * 2 + 1);

        if (_tn.check(_map))
        {
            offsetRes = true;
            break;
        }
        }
        if (!offsetRes)
        {
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
    auto checkCrossBorder = [&](int x, int y)
    {
        auto res = 1;
        if (x < 0 || x > (map.width - 1) || y < 0 || y > (map.height - 1))
        res = 0;
        return res;
    };

    auto ifspin = 0, ifmini = 0, mini = 0, sum = 0;
    auto &_x = pos.x;
    auto &_y = pos.y;

//	qDebug() << checkCrossBorder(_x + 1, _y + 2) << full(1, 2) << map(_x + 1, _y + 2);
    if (!checkCrossBorder(_x + 1, _y + 0) || (!full(0, 1) && map(_y + 0, _x + 1)))
        mini++;
    if (!checkCrossBorder(_x + 0, _y + 1) || (!full(1, 0) && map(_y + 1, _x+ 0)))
        mini++;
    if (!checkCrossBorder(_x + 1, _y + 2) || (!full(2, 1) && map(_y + 2, _x + 1)))
        mini++;
    if (!checkCrossBorder(_x + 2, _y + 1) || (!full(1, 2) && map(_y + 1, _x + 2)))
        mini++;

  //  qDebug() << "mini " << mini;
    if (!checkCrossBorder(_x, _y) || map(_y, _x))
        sum++;
    if (!checkCrossBorder(_x, _y + 2) || map(_y + 2, _x))
        sum++;
    if (!checkCrossBorder(_x + 2, _y) || map(_y, _x + 2))
        sum++;
    if (!checkCrossBorder(_x + 2, _y + 2) || map(_y + 2, _x + 2))
        sum++;

    if (sum > 2)
        ifspin = 1;
    if (mini == 1)
        ifmini = 1;
    return {  ifspin, ifmini };
    }

    Pos pos;
    data *mdata;
    Piece type;
    int rotateState;
    bool mini = false, spin = false, lastRotate = false;
    TspinType typeTspin = TspinType::None;
    static QMap<Piece, QVector<data *>*> rotateDatas;
    static QMap<bool, QVector<QVector<int> *>*> kickDatas;
    void static init();
};

inline uint qHash(const TetrisNode &key, uint seed)
{

    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.pos.x);
    seed = hash(seed, key.pos.y);
    seed = hash(seed, key.rotateState);
    seed = hash(seed, static_cast<int>(key.type));
    return seed;
    //return qHash(key.pos.x, seed) ^ qHash(key.pos.y, seed) ^ qHash(key.rotateState, seed)^qHash(static_cast<int>(key.type), seed);
}

class Random
{
public:
    Random(int _seed = QTime(0, 0, 0).msecsTo(QTime::currentTime()))
    {
    seed = _seed;
    qsrand(seed);        //设置随机数种子
    }

    void getBag()
    {
    //bag.fill(Piece::Z,7);
    //bag=QVector<Piece>{Piece::Z,Piece::L,Piece::L,Piece::L,Piece::L,Piece::L,Piece::L};
    // return;
    do
    {
        auto num = static_cast<Piece>(qrand() % 7);
        if (bag.contains(num))
        {
        continue;
        }
        else
        {
        bag.append(num);
        }
    }
    while (bag.size() != 7);
    }

    Piece getOne()
    {
    if (bag.size() == 0) //[0,1,2,3,4,5,6]
    {
        getBag();
        if (displayBag.size() == 0)
        displayBag = bag;
        bag.resize(0);
        getBag();
    }
    if (displayBag.size() < 7)
    {
        displayBag.append(bag.takeFirst());
    }
    auto num = displayBag.takeFirst();
    return num;
    }

    QVector<Piece> bag, displayBag;
    int seed;
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
    if (able)
    {
        if (type == Piece::None)
        {
        type = _curType;
        _cur = TetrisNode{rand->getOne(), Pos(3, 0), 0};
        res = 2;
        }
        else
        {
        qSwap(type, _curType);
        _cur = TetrisNode{_curType, Pos(3, 0), 0};
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
    Recorder(int _seed = -1, const std::function<void(Oper)> &_operFunc = nullptr)
    {
    seed = _seed;
    operFunc = _operFunc;
    }

    int seed, playIndex = 0, timeAll = 0, gameType = 1, firstTime = 0;
    QVector<int>time;
    QVector<Oper>oper;
    std::function<void(Oper)> operFunc = nullptr;

    void add(Oper _operation, int _time)
    {
    auto len = time.size();
    if (len == 0)
    {
        firstTime = _time;
    }
    oper.append(_operation);
    time.append(_time);
    }

    static Recorder readStruct(QByteArray ba)
    {
    Recorder *test = (Recorder *)ba.data();
    return *test;
    }

    static QByteArray writeStruct(Recorder &ss)
    {
    QByteArray ba;
    ba.resize(sizeof(ss));
    memset(ba.data(), 0, sizeof(Recorder));
    memcpy(ba.data(), &ss, sizeof(Recorder));
    return ba;
    }

    auto isEnd()
    {
    return playIndex < time.size() ? false : true;
    }

    void play(int interval)   //1.5 3 5.5 4 ->0 1
    {
    for (auto i = playIndex; i < time.size(); i++)
    {
        if (time[i] <= interval)
        {
        operFunc(oper[i]);
        playIndex = i + 1;
        //break;
        }
        else
        {
        break;
        }
    }
    }

    void play1()   //1.5 3 5.5 4 ->0 1
    {
    operFunc(oper[playIndex]);
    playIndex ++;
    }
};
#endif // TETRISBASE_H
