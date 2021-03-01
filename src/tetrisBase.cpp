#include "tetrisBase.h"
QMap<bool, QVector<QVector<int>>> TetrisNode::kickDatas = std::invoke([]()
{
    QMap<bool, QVector<QVector<int>>> kickDatasTo;
    QVector<data> NomalkickData {//kickDatas for Piece Others
        data{-1, 0, -1, 1, 0, -2, -1, -2},//0->R
        data{  1, 0, 1, -1, 0, 2, 1, 2},  //R->0
        data{   1, 0, 1, -1, 0, 2, 1, 2}, //R->2
        data{  -1, 0, -1, 1, 0, -2, -1, -2},  //2->R
        data{  1, 0, 1, 1, 0, -2, 1, -2}, //2->L
        data{  -1, 0, -1, -1, 0, 2, -1, 2}, //L->2
        data{  -1, 0, -1, -1, 0, 2, -1, 2}, //L->0
        data{  1, 0, 1, 1, 0, -2, 1, -2}//0->L
    };
    QVector<data > IkickData {//kickDatas for Piece I
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
    using mino = std::pair<Piece, data>;
    mino minoTypes[] {
        mino{Piece::None, data{}},
        mino{Piece::O, data{0, 6, 6, 0}},
        mino{Piece::I, data{0, 15, 0, 0}},
        mino{Piece::T, data{2, 7, 0}},
        mino{Piece::L, data{4, 7, 0}},
        mino{Piece::J, data{1, 7, 0}},
        mino{Piece::S, data{6, 3, 0}},
        mino{Piece::Z, data{3, 6, 0}}
    };
    auto rot = [ ](data & block) {
        auto lh = block.size();
        data newLayout;
        newLayout.fill(0, lh);
        auto mdata = newLayout.data();
        for (auto x = 0; x < lh; x++)  //顺时针旋转
            for (auto y = 0; y < lh; y++) {
                auto ry = lh - 1 - y;
                mdata[x] = ((block[y] & (1 << x)) > 0) ? (mdata[x] | (1 << ry)) : (mdata[x] & (~(1 << ry)));
            }
        return newLayout;
    };
    for (auto &p : minoTypes) {
        QVector<data> sArr{std::get<1>(p)};
        while (sArr.size() < 4)
            sArr.append(rot(sArr.last()));
        rotateDatasTo.insert(std::get<0>(p), std::move(sArr));
    }
    return rotateDatasTo;
});



