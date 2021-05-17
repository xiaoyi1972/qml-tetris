#ifndef TETRISGAME_H
#define TETRISGAME_H
#include<vector>
#include<deque>
#include<tetrisBase.h>
#include<chrono>
#include<variant>
#include<type_traits>
#include<utility>
// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace Modes {
struct dig {
    int digRows = 10;
    int digRowsEnd = 100;
    template<class T>
    void digModAdd(T *tg, int needDigRows, bool isInit) {
        auto &map = tg->map;
        auto &randSys = tg->rS;
        for (auto i = 0; i < needDigRows; i++) {
            auto row = 0, num = 0;
            do {
                row = 0b1111111111;
                num = int(randSys.rand.generate() % 10);
                row &= ~(1 << num);
            } while (row == map.data[map.height - 1]);
            std::memcpy(map.data, map.data + 1, (map.height - 1) * sizeof(int));
            map.data[map.height - 1 ] = row;
            if (!isInit) {
                map.colorDatas.erase(map.colorDatas.begin());
                map.colorDatas.push_back(std::vector<Piece>(map.width, Piece::Trash));
                map.colorDatas[map.height - 1][num] = Piece::None;
            }
        }
        digRows = 10;
        map.update();
    }

    template<class T>
    void handle(T *tg, std::vector<int> &clear, std::vector<Pos> &pieceChanges) {
        auto &map = tg->map;
        if (digRowsEnd > 0) {
            for (auto &row_i : clear) {
                if (((map.height - 11) + (10 - digRows)) < row_i && row_i < map.height)
                    digRows--;
            }
            if (clear.size() == 0) {
                auto needDigRows = 10 - digRows;
                digRowsEnd -= needDigRows;
                digModAdd(tg, needDigRows, false);
                digRows = 10;
                for (auto &piecePos : pieceChanges)
                    piecePos.x -= needDigRows;
            }
        }
    }
};

struct versus {
    int trashLinesCount = 0;
    std::list<int> trashLines;
    template<class T>
    void handle(T *tg, bool noClear, int &attack, std::vector<Pos> &pieceChanges) {
        auto &map = tg->map;
        auto &rS = tg->rS;
        auto addLines = [&](int rows) {
            auto row = 0b1111111111;
            auto num = 0;
            num = int(rS.rand.generate() % 10);
            row &= ~(1 << num);
            for (auto i = 0; i < rows; i++) {
                std::copy(map.data + 1, map.data + map.height - 1, map.data);
                map.data[map.height - 1 ] = row;
                map.colorDatas.erase(map.colorDatas.begin());
                map.colorDatas.push_back(std::vector<Piece>(map.width, Piece::Trash));
                map.colorDatas[map.height - 1][num] = Piece::None;
            }
        };
        auto addCount = 0;
        while (trashLinesCount != 0) {
            auto &first = trashLines.front();
            if (!noClear) {
                if (attack <= 0)
                    break;
                auto offset = std::min(attack, first);
                attack -= (first -= offset, offset);
                trashLinesCount -= offset;
                if (first <= 0)
                    trashLines.pop_front();
            } else {
                trashLines.pop_front();
                addCount += first;
                addLines(first);
            }
        }
        if (noClear &&  addCount > 0) {
            trashLinesCount = 0;
            map.update(false);
            for (auto &piecePos : pieceChanges)
                piecePos.x -= addCount;
        }
        attack = std::max(0, attack);
    }
};

struct sprint {

} ;
}

class TetrisGame {
public:
    enum class Mode {dig = 0, versus, sprint};
    struct gameData {
        std::size_t clear = 0;
        std::size_t b2b = 0;
        std::size_t combo = 0;
        std::size_t pieces = 0;
        bool comboState = false;
        std::deque<int> trashLines;
        int trashLinesCount = 0;
        bool isReplay = false;
    };//数据

    TetrisGame();
    void opers(Oper a);//操作
    int sendTrash(const std::tuple<TSpinType, int, bool, int> &);//计算攻击
    Random rS;
    Hold hS{&rS};
    gameData gd;
    std::variant<Modes::versus, Modes::dig, Modes::sprint> gm{std::in_place_index_t<0>()};
    TetrisNode tn{rS.getOne()};
    TetrisMapEx map{10, 20};
    Recorder record{rS.seed}, recordPath{rS.seed};
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

    std::vector<int> comboTable{ 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5, -1};
    int tableMax = 14, underAttack = 0;
    int digRows = 10, digRowsEnd = 100;
    bool deaded = false, digMod = false;

private:
    void hold();//暂存
    void left();//左移
    void right();//右移
    void softDrop();//软降
    void hardDrop();//硬降
    void ccw();//逆时针旋转
    void cw();//顺时针旋转
};

#endif // TETRISGAME_H
