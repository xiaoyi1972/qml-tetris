#ifndef TETRIS_H
#define TETRIS_H
#include<keyState.hpp>
#include<tetrisBase.h>
#include<tetrisBot.h>
#include<functional>
#include<QtQuick>
#include<QDebug>
#include<QMap>
#include<QVector>
#include<QThread>
#include<QMutex>
#include<algorithm>
#include<chrono>
#include<QtConcurrent>
#include <QSGRectangleNode>
#include <QQuickWindow>

class MyThread: public QThread {
    using QThread::run;
public: MyThread(QObject *parent = 0): QThread(parent) {}
    ~MyThread() {quit(); wait();};
};

class Task: public QObject {
    Q_OBJECT
public slots:
    int setTimeOut(const std::function<void()> &, int delay);
    int setInterval(const std::function<int()> &, int delay);
    void test(QThread *);
    void clearTimeout(int);
    void clearInterval(int);
    void killTimer_(int);
    int startTimer_(int);
    void timerEvent(QTimerEvent *);
    void pressHandle(int);
    void releaseHandle(int);
    void back(QThread *);
public:
    Task() = default;
    QHash<int, std::function<int()>> m_intervalHash;
    QHash<int, std::function<void()>> m_timeoutHash;
    KeyState<class Tetris, Task> leftKey{this, nullptr};
    KeyState<class Tetris, Task> rightKey{this, nullptr};
    KeyState<class Tetris, Task> softDropKey{this, nullptr, nullptr, true};
};

static auto getColors(Piece type) {
    QColor color;
    switch (type) {
        case Piece::Trash: color.setRgb(115, 115, 115); break;
        case Piece::O:  color.setRgb(245, 220, 0); break;
        case Piece::I: color.setRgb(57, 195, 199); break;
        case Piece::T: color.setRgb(138, 43, 227); break;
        case Piece::L: color.setRgb(255, 166, 0); break;
        case Piece::J: color.setRgb(0, 0, 255); break;
        case Piece::S: color.setRgb(51, 204, 51); break;
        case Piece::Z: color.setRgb(255, 0, 0); break;
        default:  color.setRgb(255, 255, 255, 0); break;
    }
    return color;
}

class TetrisTetro: public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(int type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(bool tight READ tight WRITE setTight NOTIFY tightChanged)
    Q_PROPERTY(bool ghost READ ghost WRITE setGhost)
public:
    TetrisTetro(QQuickItem *parent = nullptr) : QQuickItem(parent) {
        setTransformOrigin(QQuickItem::Center);
        setFlag(ItemHasContents, true);
    }

    void fresh(TetrisNode &_node, int dropDis = 0) {
        tetroNode = &_node;
        if (dropDis == 0 && m_ghost) {
            setVisible(false);
            return;
        } else setVisible(true);
        auto const c = parentItem()->scale();
        setType(static_cast<int>(_node.type));
//       setSize(QSizeF(20 * size * c, 20 * size * c));
        setX(20 * tetroNode->pos.x * c);
        setY(20 * (tetroNode->pos.y + dropDis) * c);
        setRotation(tetroNode->rotateState * 90);
        //   updateCall();
    }

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
        QSGGeometryNode *node = nullptr;
        QSGGeometry *geometry = nullptr;
        int drawStyle = m_ghost ? 1 : 0;
        auto getStyle = [&]() {
            int count;
            switch (drawStyle) {
                case 0: count = 4 * 3 * 2; break;
                case 1: count = 4 * 8; break;
            }
            return count;
        };

        if (!oldNode) {
            node = new QSGGeometryNode;
            geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), getStyle(), 0, QSGGeometry::UnsignedIntType);
            geometry->setDrawingMode(m_ghost ? QSGGeometry::DrawLines : QSGGeometry::DrawTriangles);
            geometry->setLineWidth(2);
            node->setGeometry(geometry);
            QSGVertexColorMaterial *material = new QSGVertexColorMaterial;
            node->setMaterial(material);
            node->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);
        } else {
            node = static_cast<QSGGeometryNode *>(oldNode);
            geometry = node->geometry();
            geometry->allocate(getStyle());
        }

        QSGGeometry::ColoredPoint2D *vertices = geometry->vertexDataAsColoredPoint2D();
        auto const grid = 20 * scale(); //* parentItem()->scale();
        auto const c = 1 * scale(); //parentItem()->scale();
        //    qDebug()<<"scale"<<c;
        if (tetroNode == nullptr && !m_tight) return node;
        const auto &data = TetrisNode::rotateDatas[static_cast<Piece>(m_type)][0];
        auto size = data.size();
        auto setBlock = [&](int i, int x, int y) {
            auto color = getColors(static_cast<Piece>(m_type));
            QRectF area(0 + y * grid + 0.5 / c, 0 + x * grid + 0.5 / c, grid - 1 / c, grid - 1 / c);
            if (drawStyle == 0) {
                vertices[i * 6 + 0].set(area.topRight().x(), area.topRight().y(), color.red(), color.green(), color.blue(), color.alpha());
                vertices[i * 6 + 1].set(area.topLeft().x(), area.topLeft().y(), color.red(), color.green(), color.blue(), color.alpha());
                vertices[i * 6 + 2].set(area.bottomRight().x(), area.bottomRight().y(), color.red(), color.green(), color.blue(), color.alpha());
                vertices[i * 6 + 3].set(area.bottomRight().x(), area.bottomRight().y(), color.red(), color.green(), color.blue(), color.alpha());
                vertices[i * 6 + 4].set(area.bottomLeft().x(), area.bottomLeft().y(), color.red(), color.green(), color.blue(), color.alpha());
                vertices[i * 6 + 5].set(area.topLeft().x(), area.topLeft().y(), color.red(), color.green(), color.blue(), color.alpha());
            } else if (drawStyle == 1) {
                vertices[i * 8 + 0].set(area.topRight().x(), area.topRight().y(), color.red(), color.green(), color.blue(), color.alpha());
                vertices[i * 8 + 1].set(area.topLeft().x(), area.topLeft().y(), color.red(), color.green(), color.blue(), color.alpha());
                vertices[i * 8 + 2].set(area.bottomLeft().x(), area.bottomLeft().y(), color.red(), color.green(), color.blue(), color.alpha());
                vertices[i * 8 + 3].set(area.bottomRight().x(), area.bottomRight().y(), color.red(), color.green(), color.blue(), color.alpha());
                vertices[i * 8 + 4].set(area.topLeft().x(), area.topLeft().y(), color.red(), color.green(), color.blue(), color.alpha());
                vertices[i * 8 + 5].set(area.bottomLeft().x(), area.bottomLeft().y(), color.red(), color.green(), color.blue(), color.alpha());
                vertices[i * 8 + 6].set(area.topRight().x(), area.topRight().y(), color.red(), color.green(), color.blue(), color.alpha());
                vertices[i * 8 + 7].set(area.bottomRight().x(), area.bottomRight().y(), color.red(), color.green(), color.blue(), color.alpha());
            }
        };

        for (int x = 0, i = 0; x < size; x++)
            for (int y = 0; y < size; y++) {
                if ((data.at(x) >> y) & 1) {
                    setBlock(i, x - std::get<0>(offset), y - std::get<1>(offset));
                    i++;
                }
            }
        node->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
        return node;
    }

    int type() const { return m_type;}
    void setType(int type) {
        if (type != m_type) {
            m_type = type;
            auto const c = 1 * scale();
            const auto &data = TetrisNode::rotateDatas[static_cast<Piece>(m_type)][0];
            auto size = data.size();
            if (m_tight) {
                if (!tightNodes.contains(m_type)) {
                    const auto &data = TetrisNode::rotateDatas[static_cast<Piece>(m_type)][0];
                    auto size = data.size();
                    struct V { int tx, ty, bx, by; } v;
                    for (int x = 0, i = 0; x < size; x++)
                        for (int y = 0; y < size; y++) {
                            if ((data.at(x) >> y) & 1) {
                                if (!i) v = V{x, y, x, y};
                                {
                                    v.bx = std::min(v.bx, x);
                                    v.by = std::min(v.by, y);
                                    v.tx = std::max(v.tx, x);
                                    v.ty = std::max(v.ty, y);
                                }
                                i++;
                            }
                        }
                    tightNodes.insert(m_type, std::tuple<int, int, int, int> {v.bx, v.by, v.ty - v.by + 1, v.tx - v.bx + 1});
                }
                auto p = tightNodes.value(m_type);
                std::get<0>(offset) = std::get<0>(p);
                std::get<1>(offset) = std::get<1>(p);
                setSize(QSizeF(20 * std::get<2>(p) * c, 20 * std::get<3>(p) * c));
            } else
                setSize(QSizeF(20 * size * c, 20 * size * c));
            updateCall();
            emit typeChanged(type);
        }
    }

    int tight() const { return m_tight;}
    void setTight(bool tight) {
        if (tight != m_tight) {
            m_tight = tight;
            updateCall();
            emit tightChanged(tight);
        }
    }

    int ghost() const { return m_ghost;}
    void setGhost(bool ghost) {
        if (ghost != m_ghost) {
            m_ghost = ghost;
            updateCall();
        }
    }


    static QHash<int, std::tuple<int, int, int, int>>tightNodes;
signals:
    void typeChanged(int);
    void tightChanged(bool);

public slots:
    void updateCall() {
        if (thread() != QThread::currentThread()) {
            QMetaObject::invokeMethod(this, "updateCall", Qt::QueuedConnection);
            return;
        }
        update();
    }

private:
    TetrisNode *tetroNode = nullptr;
    int m_type = static_cast<int>(Piece::None);
    bool m_tight = false, m_ghost = false;
    std::tuple<int, int> offset{0, 0};
};



class Tetris: public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(int trash READ trash WRITE setTrash NOTIFY trashChanged)
public:
    struct gameData {
        std::size_t clear = 0;
        std::size_t b2b = 0;
        std::size_t combo = 0;
        std::size_t pieces = 0;
        bool comboState = false;
        QVector<int> trashLines;
        int trashLinesCount = 0;
    };

    Tetris(QQuickItem *parent = 0);
    ~Tetris();
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);
    void keyPressEvent(QKeyEvent *event);   //键盘按下事件
    void keyReleaseEvent(QKeyEvent *event);   //键盘释放事件

    //操作类
    Q_INVOKABLE void restart();//重开
    void hold();//暂存
    void left();//左移
    void right();//右移
    void softdrop();//软降
    void harddrop();//硬降
    void ccw();//逆时针旋转
    void cw();//顺时针旋转
    void hello();//你好

    //辅助
    void opers(Oper a);//操作
    Q_INVOKABLE void replay(const QString &str = ""); //重播
    void replayFunc(); //录像重播

    int timeRecord();//录像操作时间点
    void toFresh(Piece &, std::vector<Pos> &, std::vector<int> &, const std::tuple<bool, bool, int> &);//刷新状态信息
    QVector<Oper> caculateBot(const TetrisNode &, const int) const; //bot计算
    void botCall();//bot调用执行操作
    void ExampleMap();//使用地图;
    int sendTrash(const std::tuple<TSpinType, int, bool, int> &);//计算攻击
    void digModAdd(int, bool);//添加挖掘垃圾行

    Q_INVOKABLE void passMap();
    Q_INVOKABLE void passHold(bool ifForce = false);
    Q_INVOKABLE void passNext(bool ifForce = false);
    Q_INVOKABLE void getTrash(int trash) {
        if (trash > 0) {
            gamedata.trashLines.push_back(trash);
            setTrash(gamedata.trashLinesCount + trash);
        }
    }

    void setTrash(int a) {
        if (a != gamedata.trashLinesCount) {
            gamedata.trashLinesCount = a;
            emit trashChanged();
        }
    }

    int trash() const {
        return gamedata.trashLinesCount;
    }

    int handle = -1, botOperHandle = -1, botHandle = -1;
    QFutureWatcher<QVector<Oper>> watcher;

    static QQmlPropertyMap keyconfig;
    Task task;
    static MyThread td;
    static bool initConfig;

public slots:
    void playPath();
    void replayBotOperFunc(); //bot操作播放
    void updateCall();

private:
    QQmlPropertyMap mtest;
    bool tg = false;
    Random randSys;
    Hold holdSys{&randSys};
    gameData gamedata;
    TetrisNode tn{randSys.getOne()};
    TetrisMapEx map{10, 20};
    QMutex mutex;
    Recorder record{randSys.seed}, recordPath{randSys.seed};
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    bool isReplay = false;
    QVector<int> comboTable{ 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5, -1};
    int tableMax = 14, underAttack = 0;
    int digRows = 10, digRowsEnd = 100;
    bool deaded = false, digMod = false;
    TetrisTetro cur{this}, ghost{this};

signals:
    void whl(QVariantMap a);
    void next(QVariantMap a);
    void holdFresh(QVariantMap a);
    void harddropFresh(QVariantMap a);
    void mapFresh(QVariantMap a);
    void restartGame();
    void the(int);
    void the1(int);
    void sendAttack(int);
    void trashChanged();
    void testChanged();
};

#endif // TETRIS_H
