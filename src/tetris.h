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
#include<tetrisBot.h>
#include<QMutex>
#include<algorithm>
#include<chrono>
#include<QtConcurrent>
class MyThread: public QThread
{
    using QThread::run;
public: MyThread(QObject *parent = 0): QThread(parent) {}
    ~MyThread() {quit(); wait();};
};

class Task: public QObject
{
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
public:
    Task() = default;
    void back(QThread *);
    QHash<int, std::function<int()>> m_intervalHash;
    QHash<int, std::function<void()>> m_timeoutHash;
    KeyState leftKey{this, nullptr};
    KeyState rightKey{this, nullptr};
    KeyState softDropKey{this, nullptr, nullptr, true};
};

class Tetris: public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int trash READ trash WRITE setTrash NOTIFY trashChanged)
public:
    struct gameData {
        int clear = 0;
        int b2b = 0;
        int combo = 0;
        int pieces = 0;
        bool comboState = false;
        QVector<int> trashLines;
        int trashLinesCount = 0;
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
    void replayFunc(); //录像重播
    void replayBotOperFunc(); //bot操作播放
    int timeRecord();//录像操作时间点
    void toFresh(Piece &, QVector<Pos> &, QVector<int> &, const std::tuple<bool, bool, int> &);//刷新状态信息
    QVector<Oper> caculateBot(TetrisNode &, int); //bot计算
    void botCall();//bot调用执行操作
    void ExampleMap();//使用地图;
    int sendTrash(const std::tuple<TSpinType, int> &);//计算攻击
    void digModAdd(int, bool);//添加挖掘垃圾行

    Q_INVOKABLE void passPiece(bool newPiece = false);
    Q_INVOKABLE void passMap();
    Q_INVOKABLE void passHold(bool ifForce = false);
    Q_INVOKABLE void passNext(bool ifForce = false);
    Q_INVOKABLE void getTrash(int trash)
    {
        if (trash > 0) {
            gamedata.trashLines.push_back(trash);
            setTrash(gamedata.trashLinesCount + trash);
            //gamedata.trashLinesCount += trash;
        }
    }

    void setTrash(int a)
    {
        if (a != gamedata.trashLinesCount) {
            gamedata.trashLinesCount = a;
            emit trashChanged();
        }
    }

    int trash() const
    {
        return gamedata.trashLinesCount;
    }

    int handle, botHandle = -1;
    QFutureWatcher<QVector<Oper>> watcher;

    static QQmlPropertyMap keyconfig;
    Task task;
    static MyThread td;
    static bool initConfig;

public slots:
    void playPath();

private:
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

signals:
    void whl(QVariantMap a);
    void next(QVariantMap a);
    void holdFresh(QVariantMap a);
    void harddropFresh(QVariantMap a);
    void activeFresh(QVariantMap a);
    void mapFresh(QVariantMap a);
    void restartGame();
    void the(int);
    void the1(int);
    void sendAttack(int);
    void trashChanged();
};

#endif // TETRIS_H
