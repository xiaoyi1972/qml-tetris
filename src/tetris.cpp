#include "tetris.h"

int Task::setTimeOut(const std::function<void()> &func, int delay)
{
    int timerId = startTimer(delay, Qt::PreciseTimer);
    m_timeoutHash.insert(timerId, func);
    return timerId;
}

void Task::clearTimeout(int &timerId)
{
    if (m_timeoutHash.contains(timerId))
    {
    killTimer(timerId);
    m_timeoutHash.remove(timerId);
    timerId = -1;
    }
}

int Task::setInterval(const std::function<void()> &func, int delay)
{
    int timerId = startTimer(delay, Qt::PreciseTimer);
    m_intervalHash.insert(timerId, func);
    return timerId;
}

void Task::clearInterval(int &timerId)
{
    if (m_intervalHash.contains(timerId))
    {
    killTimer(timerId);
    m_intervalHash.remove(timerId);
    timerId = -1;
    }
}

void Task::timerEvent(QTimerEvent *event)
{
    auto id = event->timerId();
    auto x = m_intervalHash.value(id, nullptr);
    if (x == nullptr)
    {
    x = m_timeoutHash.value(id, nullptr);
    if (x != nullptr)
        clearTimeout(id);
    }
    if (x != nullptr)
    {
    x();
    }
}

Tetris::Tetris()
{
    task.start(QThread::Priority::TimeCriticalPriority);
    startTime = QDateTime::currentDateTime();
}

Tetris::~Tetris()
{
    task.terminate();
}

void Tetris::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
    return;
    auto _key = event->key();
    if (_key == keyconfig.harddropKey)
    harddrop();
    else if (_key == keyconfig.leftKey)
    {
    leftKey.keyDown();
    if (rightKey.press)
        rightKey.switchStop();
    }
    else if (_key == keyconfig.rightKey)
    {
    rightKey.keyDown();
    if (leftKey.press)
        leftKey.switchStop();
    }
    else if (_key == keyconfig.softDropKey)
    softDropKey.keyDown();
    else if (_key == keyconfig.cwKey)
    cw();
    else if (_key == keyconfig.ccwKey)
    ccw();
    else  if (_key == keyconfig.restartKey)
    {
    task.clearInterval(handle);
    isReplay = false;
    restart();
    }
    else  if (_key == keyconfig.holdKey)
    hold();
    else if (_key == keyconfig.replay)
    {
    replay();
    }
    else if (_key == Qt::Key_W)
    {
    tg = !tg;
    if (tg)
    {
        botCall();
    }
    }
}

//键盘释放事件
void Tetris::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
    return;
    auto _key = event->key();
    if (_key == keyconfig.leftKey)
    {
    leftKey.keyUp();
    if (rightKey.switchStopFlag)
    {
        rightKey.keyUp();
        rightKey.keyDown();
    }
    }
    else if (_key == keyconfig.rightKey)
    {
    rightKey.keyUp();
    if (leftKey.switchStopFlag)
    {
        leftKey.keyUp();
        leftKey.keyDown();
    }
    }
    else if (_key == keyconfig.softDropKey)
    softDropKey.keyUp();
}

void Tetris::hold()
{
    auto res = holdSys.exchange(tn);
    if (res > 0)
    {
    passHold();
    }
    if (res == 2)
    passNext();
    if (!isReplay)
    record.add(Oper::Hold, timeRecord());
    update();
}

void Tetris::restart()
{
    startTime = QDateTime::currentDateTime();
    if (!isReplay)
    {
    rand = Random{};
    record = Recorder{rand.seed, std::bind(&Tetris::opers, this, std::placeholders::_1)};
    }
    else
    rand = Random{record.seed};
    holdSys = Hold {&rand};
    map = TetrisMap{10, 22};
    tn =   TetrisNode{rand.getOne()};
    emit restartGame();
    //ExampleMap();
    //passMap();
    passNext(true);
    passHold(true);
    update();
}

void Tetris::left()
{
    if (TetrisNode::shift(tn, map, -1, 0))
    update();
    if (!isReplay)
    record.add(Oper::Left, timeRecord());
}

void Tetris::right()
{
    if (TetrisNode::shift(tn, map, 1, 0))
    update();
    if (!isReplay)
    record.add(Oper::Right, timeRecord());
}

void Tetris::softdrop()
{
    if (TetrisNode::shift(tn, map, 0, 1))
    update();
    if (!isReplay)
    record.add(Oper::SoftDrop, timeRecord());
}

void Tetris::harddrop()
{
    mutex.lock();
    holdSys.reset();
    tn.pos.y += tn.getDrop(map);
   /* if(tn.type==Piece::T){
    auto [spin,mini]=tn.corner3(map);
            qDebug()<<"spin "<<spin<<"mini "<<mini;
    }*/
    auto [pieceChanges,clear] = tn.attachs(map);
    //auto clear = map.clear();
    if (!isReplay)
    record.add(Oper::HardDrop, timeRecord());
    toFresh(pieceChanges, clear);
    auto piece = rand.getOne();
    passNext();
    tn = TetrisNode{piece, Pos(3, 0), 0};
    mutex.unlock();
    update();
}

void Tetris::ccw()
{
    if (TetrisNode::rotate(tn, map, false))
    update();
    if (!isReplay)
    record.add(Oper::Ccw, timeRecord());
}

void Tetris::cw()
{
    if (TetrisNode::rotate(tn, map))
    update();
    if (!isReplay)
    record.add(Oper::Cw, timeRecord());
}

void Tetris::hello()
{
    qDebug() << "hello";
}

void Tetris::opers(Oper a)
{
    switch (a)
    {
    case Oper::Left: left(); break;
    case Oper::Right: right(); break;
    case Oper::SoftDrop: softdrop(); break;
    case Oper::HardDrop: harddrop(); break;
    case Oper::Cw: cw(); break;
    case Oper::Ccw: ccw(); break;
    case Oper::Hold: hold(); break;
    }
}

void Tetris::update()
{
    passPiece();
}

void Tetris::toFresh(QVector<Pos> &pieceChanges, QVector<int> &clear)
{
    QVariantList list, next;
    for (int y = 0; y < map.height; y++)
    for (int x = 0; x < map.width; x++)
    {
        list << static_cast<int>(map.colorDatas[y][x]);
    }
    for (auto piece : rand.displayBag)
    next << static_cast<int>(piece);
    QVariantList changes;
    for (auto &y : pieceChanges)
    {
    QVariantMap m;
    m.insert("y", y.x);
    m.insert("x", y.y);
    m.insert("type", static_cast<int>(tn.type));
    changes << m;
    }
    QVariantMap clearMsg;
    QVariantList clearArr;
    clearMsg.insert("num", clear.size());
    for (auto &y : clear)
    clearArr << y;
    clearMsg.insert("clearArr", clearArr);
    QVariantMap mapMsg;
    mapMsg.insert("changes", changes);
    mapMsg.insert("clear", clearMsg);
    emit harddropFresh(mapMsg);
};

void Tetris::passPiece()
{
    QVariantList list, next;
    QVariantMap mapMsg;
    QVariantMap active;
    auto row = tn.getDrop(map);
    active.insert("type", static_cast<int>(tn.type));
    active.insert("x", tn.pos.x);
    active.insert("y", tn.pos.y);
    active.insert("rs", tn.rotateState);
    active.insert("drop", row);
    mapMsg.insert("active", active);
    emit whl(mapMsg);
}

void Tetris::passHold(bool ifForce)
{
    QVariantMap mapMsg;
    mapMsg.insert("type", static_cast<int>(holdSys.type));
    mapMsg.insert("able", holdSys.able);
    mapMsg.insert("force", ifForce);
    emit holdFresh(mapMsg);
}

void Tetris::passNext(bool ifForce)
{
    QVariantList nexts;
    for (auto piece : rand.displayBag)
    {
//    qDebug() << Tool::printType(piece);
    nexts << static_cast<int>(piece);
    }
    QVariantMap map;
    map.insert("next", nexts);
    map.insert("nextNew", nexts.back());
    map.insert("force", ifForce);
    emit next(map);
}

void Tetris::PassMap()
{
    QVariantList list;
    for (int y = 0; y < map.height; y++)
    for (int x = 0; x < map.width; x++)
    {
        list << static_cast<int>(map.colorDatas[y][x]);
    }
    QVariantMap mapMsg;
    mapMsg.insert("map", list);
    emit mapFresh(mapMsg);
}

void Tetris::setKeyboard(QVariantMap a)
{
    keyconfig.leftKey = a["left_move"].value<int>();
    keyconfig.rightKey = a["right_move"].value<int>();
    keyconfig.harddropKey = a["hard_drop"].value<int>();
    keyconfig.softDropKey = a["soft_drop"].value<int>();
    keyconfig.ccwKey = a["rotate_normal"].value<int>();
    keyconfig.cwKey = a["rotate_reverse"].value<int>();
    keyconfig.dasDelay = a["das_delay"].value<int>();
    keyconfig.arrDelay = a["das_repeat"].value<int>();
    keyconfig.softdropDelay = a["softDropSpeed"].value<int>();
    keyconfig.replay = a["replay"].value<int>();
}

void Tetris::replay()
{
    task.clearInterval(handle);
    isReplay = true;
    restart();
    delta = 0;
    record.playIndex = 0;
    qDebug() << record.time;
// record=Recorder::readStruct(Recorder::writeStruct(record));
    handle = task.setTimeOut(std::bind(&Tetris::replayFunc, this),  record.firstTime);
}

void Tetris::replayFunc()
{
    auto i = 0;
    if (!record.isEnd())
    {
    i = record.time[record.playIndex];
    record.play1();
    handle = task.setTimeOut(std::bind(&Tetris::replayFunc, this), i);
    }
    else
    {
    task.clearTimeout(handle);
    isReplay = false;
    }
}

qint64 Tetris::timeRecord()
{
    auto now = QDateTime::currentDateTime();
    auto timeDelta = startTime.msecsTo(now);
    startTime = now;
    return timeDelta;
}

QVector<Oper> Tetris::caculateBot(TetrisNode &start, int limitTime)
{
    auto a = start;
    TreeContext ctx;
    auto dp = rand.displayBag.mid(0, 6);
    ctx.createRoot(a, map, dp, holdSys.type);
    auto now = QDateTime::currentDateTime();
    do
    {
    ctx.run();
    }
    while ((now.msecsTo(QDateTime::currentDateTime())) <= limitTime);
    auto [best, ishold, test1] = ctx.getBest();
    QVector<Oper>path;
    if (!ishold)
    path = TetrisBot::make_path(a, best, map);
    else
    {
    auto holdNode = TetrisNode{holdSys.type == Piece::None ? rand.displayBag[0] : holdSys.type};
    path = TetrisBot::make_path(holdNode, best, map);
    path.push_front(Oper::Hold);
    }
    return path;
}

void Tetris::botCall()
{
    auto limitTime = 100;
    QEventLoop eventloop;
    QFutureWatcher<QVector<Oper>> watcher;
    connect(&watcher, SIGNAL(finished()),  &eventloop, SLOT(quit()));
    QFuture<QVector<Oper>> future = QtConcurrent::run(this, &Tetris::caculateBot, tn, limitTime);
    watcher.setFuture(future);
    eventloop.exec();
    auto path  = future.result();
    for (auto i = 0; i < path.size(); i++)
    {
    if (path[i] == Oper::DropToBottom)
    {
        auto softdrops = tn.getDrop(map);
        path.remove(i);
        path.insert(i, softdrops, Oper::SoftDrop);
    }
    opers(path[i]);
    if ((i + 1 < path.size()) &&  path[i + 1] == Oper::SoftDrop)
        sleepTo(6);
    else if (path[i] != Oper::HardDrop)
        sleepTo(30);
    }
    if (tg)
    botHandle = task.setTimeOut(std::bind(&Tetris::botCall, this), 0);
}

void Tetris::ExampleMap()
{
    map.data[map.height - 3] = 0b0000000111;
    map.data[map.height - 2] = 0b0000001111;
    map.data[map.height - 1] = 0b0010001111;
    for (auto i = 0; i < map.height; i++)
    for (auto j = 0; j < map.width; j++)
    {
        if (map(i, j))
        {
        map.roof = std::min(map.roof, i);
        map.colorDatas[i][j] = Piece::Trash;
        }
    }
}
