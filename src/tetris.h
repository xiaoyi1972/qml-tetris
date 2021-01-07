#ifndef TETRIS_H
#define TETRIS_H
#include<QtQuick>
#include<QDebug>
#include<QMap>
#include<QVector>
#include<QThread>
#include<functional>
#include<keystate.h>
#include<tetrisBase.h>
#include <QMutex>
#include <tetrisBot.h>
#include<algorithm>
#include<chrono>
#include<limits>
#include <QtConcurrent>
class Task: public QObject
{
    Q_OBJECT
public slots:
    int setTimeOut(const std::function<void()> &, int delay);
    int setInterval(const std::function<void()> &, int delay);
    void clearTimeout(int &);
    void clearInterval(int &);
    void killTimer_(int );
    int startTimer_(int );
    void timerEvent(QTimerEvent *);

public:
    Task() = default;
    QHash<int, std::function<void()>> m_intervalHash;
    QHash<int, std::function<void()>> m_timeoutHash;
};

class Tetris: public QQuickItem
{
    Q_OBJECT
public:
    struct keyConfig {
        int leftKey = Qt::Key_Left;
        int rightKey = Qt::Key_Right;
        int softDropKey = Qt::Key_Down;
        int harddropKey = Qt::Key_Space;
        int cwKey = Qt::Key_Z;
        int ccwKey = Qt::Key_X;
        int holdKey = Qt::Key_C;
        int restartKey = Qt::Key_R;
        int dasDelay = 65;
        int arrDelay = 16;
        int softdropDelay = 8;
        int replay = Qt::Key_Q;
    };

    struct gameData {
        int clear = 0;
        int b2b = 0;
        int combo = 0;
        bool comboState = false;
    };

    Tetris();
    ~Tetris();

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
    void replayFunc();
    int timeRecord();//录像操作时间点
    void toFresh(Piece &, QVector<Pos> &, QVector<int> &);
    QVector<Oper> caculateBot(TetrisNode &, int); //bot计算
    void botCall();//bot调用执行操作
    void ExampleMap();//使用地图;

    void sleepTo(int msec)
    {
        QEventLoop eventloop;
        QTimer::singleShot(msec, Qt::PreciseTimer, &eventloop, SLOT(quit()));
        eventloop.exec();
    }

    void Tetris::replayFunc1()
    {
        if (!recordPath.isEnd()) {
            auto [time, oper] = recordPath.play();
            opers(oper);
            handle = task.setTimeOut(std::bind(&Tetris::replayFunc1, this), time);
        } else {
            task.clearTimeout(handle);
            if (tg)
                botHandle = task.setTimeOut(std::bind(&Tetris::botCall, this), 0);
        }
    }

    Q_INVOKABLE void passPiece(bool newPiece = false);
    Q_INVOKABLE void passMap();
    Q_INVOKABLE void passHold(bool ifForce = false);
    Q_INVOKABLE void passNext(bool ifForce = false);
    Q_INVOKABLE void setKeyboard(QVariantMap a);

    Task task;
    keyConfig keyconfig;
    int handle, botHandle = -1;
    int digRows = 10;
    bool digMod = false;
    bool deaded = false;
    QFutureWatcher<QVector<Oper>> watcher;
    QThread thread;
public slots:
    void testPath()
    {
        auto path  = watcher.result();
        auto interval = 0;
        for (auto i = 0; i < path.size(); i++) {
            recordPath.add(path[i],  interval); //interval
            if ((i + 1 < path.size()) &&  path[i + 1] == Oper::SoftDrop) {
                interval = 6;
            } else if (path[i] != Oper::HardDrop) {
                interval = 30;
            } else
                interval = 0;
        }
        replayFunc1();
    }
private:
    bool tg = false;
    Random rand;
    gameData gamedata;
    TetrisNode tn{rand.getOne()};
    TetrisMap map{10, 20};
    Hold holdSys{&rand};
    Recorder record{rand.seed}, recordPath{rand.seed};
    KeyState leftKey{this, std::bind(&Tetris::left, this)};
    KeyState rightKey{this, std::bind(&Tetris::right, this)};
    KeyState softDropKey{this, std::bind(&Tetris::softdrop, this), nullptr, true};
    QMutex mutex;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    bool isReplay = false;

signals:
    void whl(QVariantMap a);
    void next(QVariantMap a);
    void holdFresh(QVariantMap a);
    void restartGame();
    void harddropFresh(QVariantMap a);
    void activeFresh(QVariantMap a);
    void mapFresh(QVariantMap a);
};

#endif // TETRIS_H
