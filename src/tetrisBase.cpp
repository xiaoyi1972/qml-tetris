#include "tetrisBase.h"

QMap<Piece, QVector<QVector<int> *>*> TetrisNode::rotateDatas;
QMap<bool, QVector<QVector<int> *>*> TetrisNode::kickDatas;
void TetrisNode::init()
{
    datas minoTypes =
    {
    new data{},//None
    new data{0, 6, 6, 0},//O
    new data{0, 15, 0, 0},//I
    new data{2, 7, 0},//T
    new data{4, 7, 0},//L
    new data{1, 7, 0},//J
    new data{6, 3, 0},//S
    new data{3, 6, 0},//Z
    };

    auto *NomalkickData = new QVector<data *>
    {
    new data{-1, 0, -1, 1, 0, -2, -1, -2},//0->R
    new data{  1, 0, 1, -1, 0, 2, 1, 2},  //R->0
    new data{   1, 0, 1, -1, 0, 2, 1, 2}, //R->2
    new data{  -1, 0, -1, 1, 0, -2, -1, -2},  //2->R
    new data{  1, 0, 1, 1, 0, -2, 1, -2}, //2->L
    new data{  -1, 0, -1, -1, 0, 2, -1, 2}, //L->2
    new data{  -1, 0, -1, -1, 0, 2, -1, 2}, //L->0
    new data{  1, 0, 1, 1, 0, -2, 1, -2}//0->L
    };

    //----I
    auto *IkickData = new QVector<data *>
    {
    new data{-2, 0, 1, 0, -2, -1, 1, 2}, //0->R
    new data{2, 0, -1, 0, 2, 1, -1, -2},  //R->0
    new data{-1, 0, 2, 0, -1, 2, 2, -1}, //R->2
    new data{1, 0, -2, 0, 1, -2, -2, 1},  //2->R
    new data{2, 0, -1, 0, 2, 1, -1, -2}, //2->L
    new data{ -2, 0, 1, 0, -2, -1, 1, 2}, //L->2
    new data{ 1, 0, -2, 0, 1, -2, -2, 1}, //L->0
    new data{ -1, 0, 2, 0, -1, 2, 2, -1}, //0->L
    };

    kickDatas.insert(true, NomalkickData);
    kickDatas.insert(false, IkickData);

    auto rot = [ = ](data * block)
    {
    auto lh = block->size();
    data *newLayout = new data;
    newLayout->resize(lh);
    newLayout->fill(0);
    auto mdata = newLayout->data();
    for (auto x = 0; x < lh; x++)  //顺时针旋转
    {
        for (auto y = 0; y < lh; y++)
        {
        //auto rx = lh - 1 - x;
        auto ry = lh - 1 - y;
        if ((block->at(y) & (1 << x)) > 0)
            mdata[x] |= (1 << ry);
        else
            mdata[x] &= ~(1 << ry);
        }
    }
    return newLayout;
    };

    auto  initR = [&]()
    {
    for (auto p = 0; p < 8; p++)
    {
        auto i = 0;
        QVector<data *> *sArr = new QVector<data *>;
        auto *origin = minoTypes[p];
        do
        {
        sArr->push_back(origin);
        origin = rot(origin);
        i++;
        }
        while (i < 4);
        rotateDatas.insert(static_cast<Piece>(p - 1), sArr);
    }
    };
    initR();
}



