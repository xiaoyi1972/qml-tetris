#include "tetrisBase.h"
QMap<bool, QVector<QVector<int>>> TetrisNode::kickDatas = std::invoke([]()
{
    QMap<bool, QVector<QVector<int>>> kickDatasTo;
    QVector<data> NomalkickData = {//kickDatas for Piece Others
        data{-1, 0, -1, 1, 0, -2, -1, -2},//0->R
        data{  1, 0, 1, -1, 0, 2, 1, 2},  //R->0
        data{   1, 0, 1, -1, 0, 2, 1, 2}, //R->2
        data{  -1, 0, -1, 1, 0, -2, -1, -2},  //2->R
        data{  1, 0, 1, 1, 0, -2, 1, -2}, //2->L
        data{  -1, 0, -1, -1, 0, 2, -1, 2}, //L->2
        data{  -1, 0, -1, -1, 0, 2, -1, 2}, //L->0
        data{  1, 0, 1, 1, 0, -2, 1, -2}//0->L
    };
    QVector<data > IkickData =  {//kickDatas for Piece I
        data{-2, 0, 1, 0, -2, -1, 1, 2}, //0->R
        data{2, 0, -1, 0, 2, 1, -1, -2},  //R->0
        data{-1, 0, 2, 0, -1, 2, 2, -1}, //R->2
        data{1, 0, -2, 0, 1, -2, -2, 1},  //2->R
        data{2, 0, -1, 0, 2, 1, -1, -2}, //2->L
        data{ -2, 0, 1, 0, -2, -1, 1, 2}, //L->2
        data{ 1, 0, -2, 0, 1, -2, -2, 1}, //L->0
        data{ -1, 0, 2, 0, -1, 2, 2, -1}, //0->L
    };
    kickDatasTo.insert(true, std::move(NomalkickData));
    kickDatasTo.insert(false, std::move(IkickData));
    return kickDatasTo;
});

QMap<Piece, QVector<QVector<int>>> TetrisNode::rotateDatas = std::invoke([]()
{
    QMap<Piece, QVector<QVector<int>>> rotateDatasTo;
    QVector<data> minoTypes = {
        data{},//None
        data{0, 6, 6, 0},//O
        data{0, 15, 0, 0},//I
        data{2, 7, 0},//T
        data{4, 7, 0},//L
        data{1, 7, 0},//J
        data{6, 3, 0},//S
        data{3, 6, 0},//Z
    };
    auto rot = [ = ](data & block) {
        auto lh = block.size();
        data newLayout;
        newLayout.resize(lh);
        newLayout.fill(0);
        auto mdata = newLayout.data();
        for (auto x = 0; x < lh; x++) { //顺时针旋转
            for (auto y = 0; y < lh; y++) {
                auto ry = lh - 1 - y;
                if ((block[y] & (1 << x)) > 0)
                    mdata[x] |= (1 << ry);
                else
                    mdata[x] &= ~(1 << ry);
            }
        }
        return newLayout;
    };

    auto  initR = [&]() {
        for (auto p = 0; p < 8; p++) {
            auto i = 0;
            QVector<data> sArr;
            auto origin = minoTypes[p];
            do {
                sArr.append(origin);
                origin = rot(origin);
                i++;
            } while (i < 4);
            rotateDatasTo.insert(static_cast<Piece>(p - 1), std::move(sArr));
        }
    };
    initR();
    return rotateDatasTo;
});



