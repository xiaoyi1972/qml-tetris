#include "tetris.h"

void Task::back(QThread *td)
{
    for (auto &id : m_intervalHash.keys()) {
        if (thread() != QThread::currentThread()) {
            QMetaObject::invokeMethod(this, "killTimer_", Qt::BlockingQueuedConnection, Q_ARG(int, id));
        } else {
            killTimer(id);
        }
    }
    for (auto &id : m_timeoutHash.keys()) {
        if (thread() != QThread::currentThread()) {
            QMetaObject::invokeMethod(this, "killTimer_", Qt::BlockingQueuedConnection, Q_ARG(int, id));
        } else {
            killTimer(id);
        }
    }
    m_intervalHash.clear();
    m_timeoutHash.clear();
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "test", Qt::BlockingQueuedConnection, Q_ARG(QThread *, td));
    } else {
        test(td);
    }
}

void Task::test(QThread *td)
{
    moveToThread(td);
}


void Task::killTimer_(int timerId)
{
    killTimer(timerId);
}

int Task::startTimer_(int delay)
{
    int timerId = startTimer(delay, Qt::PreciseTimer);
    return timerId;
}

int Task::setTimeOut(const std::function<void()> &func, int delay)
{
    int timerId;
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "startTimer_", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, timerId), Q_ARG(int, delay));
    } else {
        timerId = startTimer(delay, Qt::PreciseTimer);
    }
    m_timeoutHash.insert(timerId, func);
    return timerId;
}

void Task::clearTimeout(int &timerId)
{
    if (timerId != -1)
        if (m_timeoutHash.contains(timerId)) {
            if (thread() != QThread::currentThread()) {
                QMetaObject::invokeMethod(this, "killTimer_", Qt::BlockingQueuedConnection, Q_ARG(int, timerId));
            } else {
                killTimer(timerId);
            }
            m_timeoutHash.remove(timerId);
            timerId = -1;
        }
}

int Task::setInterval(const std::function<void()> &func, int delay)
{
    int timerId;
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "startTimer_", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, timerId), Q_ARG(int, delay));
    } else {
        timerId = startTimer(delay, Qt::PreciseTimer);
    }
    m_intervalHash.insert(timerId, func);
    return timerId;
}

void Task::clearInterval(int &timerId)
{
    if (timerId != -1)
        if (m_intervalHash.contains(timerId)) {
            if (thread() != QThread::currentThread()) {
                QMetaObject::invokeMethod(this, "killTimer_", Qt::BlockingQueuedConnection, Q_ARG(int, timerId));
            } else {
                killTimer(timerId);
            }
            m_intervalHash.remove(timerId);
            timerId = -1;
        }
}

void Task::timerEvent(QTimerEvent *event)
{
    auto id = event->timerId();
    auto func = m_intervalHash.value(id, nullptr);
    if (func == nullptr) {
        func = m_timeoutHash.value(id, nullptr);
        if (func != nullptr)
            clearTimeout(id);
    }
    if (func != nullptr) {
        func();
    }
}

MyThread Tetris::td;
Tetris::Tetris()
{
    connect(&watcher, SIGNAL(finished()),  this, SLOT(playPath()));
    task.moveToThread(&td);
    if (!td.isRunning())
        td.start(QThread::Priority::TimeCriticalPriority);
    startTime = std::chrono::high_resolution_clock::now();
}

Tetris::~Tetris()
{
    task.back(thread());
    //td.terminate();
    //td.wait();
}

void Tetris::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;
    auto _key = event->key();
    if (!deaded) {
        if (_key == keyconfig.harddropKey)
            harddrop();
        else if (_key == keyconfig.leftKey) {
            leftKey.keyDown();
            if (rightKey.press)
                rightKey.switchStop();
        } else if (_key == keyconfig.rightKey) {
            rightKey.keyDown();
            if (leftKey.press)
                leftKey.switchStop();
        } else if (_key == keyconfig.softDropKey)
            softDropKey.keyDown();
        else if (_key == keyconfig.cwKey)
            cw();
        else if (_key == keyconfig.ccwKey)
            ccw();
        else  if (_key == keyconfig.holdKey)
            hold();
    }
    if (_key == keyconfig.restartKey) {
        task.clearInterval(handle);
        isReplay = false;
        restart();
    } else if (_key == keyconfig.replay) {
        replay();
    } else if (_key == Qt::Key_W) {
        tg = !tg;
        if (tg) {
            botCall();
        }
    } else if (_key == Qt::Key_U) {
        QClipboard *clipboard = QGuiApplication::clipboard();   //获取系统剪贴板指针
        QString originalText = clipboard->text();            //获取剪贴板上文本信息
        clipboard->setText(Recorder::writeStruct(record).toBase64());
    } else if (_key == Qt::Key_V) {

        auto row = 0b1111111111;
        auto num = 0;
        num = int(randSys.rand.generate() % 10);
        row &= ~(1 << num);
        for (auto i = 0; i < 3; i++) {
            memcpy(map.data, map.data + 1, (map.height - 1) * sizeof(int));
            map.data[map.height - 1 ] = row;
            map.colorDatas.remove(0);
            map.colorDatas.push_back(QVector<Piece>(map.width).fill(Piece::Trash));
            map.colorDatas[map.height - 1][num] = Piece::None;
        }
        map.update();
        passMap();
        passPiece(true);
    } else if (_key == Qt::Key_M) {
        QTime startTime = QTime::currentTime();
        auto x = TetrisBot::search(tn, map);
        QTime stopTime = QTime::currentTime();
        int elapsed = startTime.msecsTo(stopTime);
        qDebug() << "QTime.currentTime =" << elapsed << "ms" << x.size();
    } else if (_key == Qt::Key_A) {
        TetrisBot::evalute(tn, map, 0, 0);
        qDebug() << Tool::printNode(tn);
    }
}

//键盘释放事件
void Tetris::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;
    auto _key = event->key();
    if (_key == keyconfig.leftKey) {
        leftKey.keyUp();
        if (rightKey.switchStopFlag) {
            rightKey.keyUp();
            rightKey.keyDown();
        }
    } else if (_key == keyconfig.rightKey) {
        rightKey.keyUp();
        if (leftKey.switchStopFlag) {
            leftKey.keyUp();
            leftKey.keyDown();
        }
    } else if (_key == keyconfig.softDropKey)
        softDropKey.keyUp();
}

void Tetris::hold()
{
    auto res = holdSys.exchange(tn);
    if (res > 0) {
        if (!isReplay)
            record.add(Oper::Hold, timeRecord());
        passHold();
        tn.lastRotate = false;
    }
    if (res == 2)
        passNext();
    passPiece(true);
}

void Tetris::restart()
{
    startTime = std::chrono::high_resolution_clock::now();
    deaded = false;
    digRowsEnd = 100;
    if (!isReplay) {
        randSys = Random{};
        record = Recorder{randSys.seed};
    } else
        randSys = Random{record.seed};
    holdSys = Hold {&randSys};
    map = TetrisMapEx{10, 22};
    tn =   TetrisNode{randSys.getOne()};
    gamedata = gameData{};
    emit restartGame();
    ExampleMap();
    if (digMod)
        digModAdd(digRows, true);
    passMap();
    passNext(true);
    passHold(true);
    passPiece(true);
}

void Tetris::digModAdd(int _needDigRows, bool _isInit)
{
    for (auto i = 0; i < _needDigRows; i++) {
        auto row = 0, num = 0;
        do {
            row = 0b1111111111;
            num = int(randSys.rand.generate() % 10);
            row &= ~(1 << num);
        } while (row == map.data[map.height - 1]);
        std::memcpy(map.data, map.data + 1, (map.height - 1) * sizeof(int));
        map.data[map.height - 1 ] = row;
        if (!_isInit) {
            map.colorDatas.remove(0);
            map.colorDatas.push_back(QVector<Piece>(map.width).fill(Piece::Trash));
            map.colorDatas[map.height - 1][num] = Piece::None;
        }
    }
    digRows = 10;
    map.update();
}

void Tetris::left()
{
    if (TetrisNode::shift(tn, map, -1, 0)) {
        if (!isReplay)
            record.add(Oper::Left, timeRecord());
        passPiece();
        tn.lastRotate = false;
    }
}

void Tetris::right()
{
    if (TetrisNode::shift(tn, map, 1, 0)) {
        if (!isReplay)
            record.add(Oper::Right, timeRecord());
        passPiece();
        tn.lastRotate = false;
    }
}

void Tetris::softdrop()
{
    if (TetrisNode::shift(tn, map, 0, 1)) {
        if (!isReplay)
            record.add(Oper::SoftDrop, timeRecord());
        passPiece();
        tn.lastRotate = false;
    }
}

void Tetris::harddrop()
{
    if (!isReplay)
        record.add(Oper::HardDrop, timeRecord());
    mutex.lock();

    holdSys.reset();
    auto dropDis = tn.getDrop(map);
    if (dropDis > 0)
        tn.lastRotate = false;
    tn.pos.y += dropDis;

    if (tn.type == Piece::T) {
        auto [spin, mini] = tn.corner3(map);
        if (spin) {
            tn.typeTSpin = mini ? TSpinType::TSpinMini : TSpinType::TSpin;
        }
    }
    /*   if (tn.typeTSpin == TSpinType::TspinMini)
             qDebug() << "mini";*/

    auto [pieceChanges, clear] = tn.attachs(map);
    auto isTSpin = (tn.typeTSpin != TSpinType::None && tn.lastRotate);
    if (clear.size() == 4 || (clear.size() > 0 && tn.typeTSpin != TSpinType::None))
        gamedata.b2b++;
    else if (clear.size() > 0)
        gamedata.b2b = 0;
    gamedata.combo = clear.size() > 0 ? gamedata.combo + 1 : 0;

    // qDebug() << "b2b:" << !!gamedata.b2b << " combo:" << gamedata.combo;
    gamedata.pieces++;
    gamedata.clear += clear.size();
    auto pieceType = tn.type;
    auto piece = randSys.getOne();
    passNext();
    tn = TetrisNode{piece};

    mutex.unlock();

    for (auto &row_i : clear) {//for dig mod
        if (((map.height - 11) + (10 - digRows)) < row_i && row_i < map.height) {
            digRows--;
        }
    }
    if (digMod && digRowsEnd > 0) {
        if (clear.size() == 0) {
            auto needDigRows = 10 - digRows;
            digRowsEnd -= needDigRows;
            digModAdd(needDigRows, false);
            digRows = 10;
            map.update(false);
            passMap();
            for (auto &piecePos : pieceChanges) {
                piecePos.x -= needDigRows;
            }
        }
    }

    if (!tn.check(map)) //check is dead
        deaded = true;

    passPiece(true);
    toFresh(pieceType, pieceChanges, clear, std::tuple<bool, bool, int> {isTSpin, gamedata.b2b > 1, gamedata.combo});
}

int Tetris::sendTrash(std::tuple<std::tuple<bool, bool>, int> &status)
{
    int attackTo = 0, b2b = !!gamedata.b2b;
    auto [isTSpin, clear] = status;
    auto [spin, mini] = isTSpin;
    int clears[] = {0, 0, 1, 2, 4};
    if (spin) {
        switch (clear) {
        case 1: attackTo += ((mini ? 1 : 2) + b2b); break;
        case 2: attackTo += (4 + b2b); break;
        case 3: attackTo += (b2b ? 6 : 8); break;
        default: break;
        }
    } else {
        attackTo += (clears[clear] + b2b);
    }
    attackTo += comboTable[std::min(gamedata.combo, tableMax)];
    return attackTo;
}

void Tetris::ccw()
{
    if (TetrisNode::rotate(tn, map, false)) {
        if (!isReplay)
            record.add(Oper::Ccw, timeRecord());
        passPiece();
        tn.lastRotate = true;
    }
}

void Tetris::cw()
{
    if (TetrisNode::rotate(tn, map)) {
        if (!isReplay)
            record.add(Oper::Cw, timeRecord());
        passPiece();
        tn.lastRotate = true;
    }
}

void Tetris::hello()
{
    qDebug() << "hello";
    task.setTimeOut(std::bind(&Tetris::hello, this), 100);
}

void Tetris::opers(Oper a)
{
    switch (a) {
    case Oper::Left: left(); break;
    case Oper::Right: right(); break;
    case Oper::SoftDrop: softdrop(); break;
    case Oper::HardDrop: harddrop(); break;
    case Oper::Cw: cw(); break;
    case Oper::Ccw: ccw(); break;
    case Oper::Hold: hold(); break;
    default: break;
    }
}

void Tetris::toFresh(Piece &_type, QVector<Pos> &pieceChanges, QVector<int> &clear, const std::tuple<bool, bool, int> &clearState)
{
    QVariantList list, next;
    auto [tspin, b2b, combo] = clearState;
    for (int y = 0; y < map.height; y++)
        for (int x = 0; x < map.width; x++) {
            list << static_cast<int>(map.colorDatas[y][x]);
        }
    for (auto piece : randSys.displayBag)
        next << static_cast<int>(piece);
    QVariantList changes;
    for (auto &y : pieceChanges) {
        QVariantMap m;
        m.insert("y", y.x);
        m.insert("x", y.y);
        m.insert("type", static_cast<int>(_type));
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
    mapMsg.insert("dead", deaded);
    QString specialClear = "", clearStatus = "";
    if (tspin)
        specialClear = QString::fromLocal8Bit("T旋");
    switch (clear.size()) {
    case 0: break;
    case 1: specialClear += QString::fromLocal8Bit("\n单消"); break;
    case 2: specialClear += QString::fromLocal8Bit("\n双消"); break;
    case 3: specialClear += QString::fromLocal8Bit("\n三消"); break;
    case 4: specialClear = QString::fromLocal8Bit("四消"); break;
    }
    if (clear.size() > 0 && b2b)
        specialClear += QString::fromLocal8Bit("[连续]");
    if (combo - 1 > 0)
        specialClear += QString::fromLocal8Bit(" \n%1 连击").arg(combo - 1);
    if (map.count == 0)
        specialClear += QString::fromLocal8Bit("\n完美清除");
    clearStatus += QString::fromLocal8Bit("块数 %1\n消行数 %2").arg(gamedata.pieces).arg(gamedata.clear);
    mapMsg.insert("specialClear", specialClear);
    mapMsg.insert("clearStatus", clearStatus);
    emit harddropFresh(mapMsg);
};

void Tetris::passPiece(bool newPiece)
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
    mapMsg.insert("isNewSpawn", newPiece);
    mapMsg.insert("active", active);
    mapMsg.insert("dead", deaded);
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
    for (auto piece : randSys.displayBag) {
        nexts << static_cast<int>(piece);
    }
    QVariantMap map;
    map.insert("next", nexts);
    map.insert("nextNew", nexts.back());
    map.insert("force", ifForce);
    emit next(map);
}

void Tetris::passMap()
{
    QVariantList list;
    for (int y = 0; y < map.height; y++)
        for (int x = 0; x < map.width; x++) {
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

void Tetris::replay(const QString &str)
{
    task.clearInterval(handle);
    if (str != "")
        record = Recorder::readStruct(QByteArray::fromBase64(str.toLatin1()));
    qDebug() << "seed" << record.seed;
    qDebug() << record.time;
    isReplay = true;
    restart();
    record.reset();
    handle = task.setTimeOut(std::bind(&Tetris::replayFunc, this),  record.firstTime);
}

void Tetris::replayFunc()
{
    auto [time, oper] = record.play();
    opers(oper);
    if (!record.isEnd())
        handle = task.setTimeOut(std::bind(&Tetris::replayFunc, this), time);
    else {
        task.clearTimeout(handle);
        isReplay = false;
    }
}

void Tetris::replayBotOperFunc()
{
    if (!recordPath.isEnd()) {
        auto [time, oper] = recordPath.play();
        opers(oper);
        handle = task.setTimeOut(std::bind(&Tetris::replayBotOperFunc, this), time);
    } else {
        task.clearTimeout(handle);
        // tg = false;
        if (tg)
            botHandle = task.setTimeOut(std::bind(&Tetris::botCall, this), 0);
    }
}

void Tetris::playPath()
{
    auto path  = watcher.result();
    auto interval = 0;
    for (auto i = 0; i < path.size(); i++) {
        recordPath.add(path[i],  interval); //interval
        if ((i + 1 < path.size()) &&  path[i + 1] == Oper::SoftDrop) {
            interval = 16;
        } else if (path[i] != Oper::HardDrop) {
            interval = 30;
        } else
            interval = 0;
    }
    replayBotOperFunc();
}

int Tetris::timeRecord()
{
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;
    auto now = high_resolution_clock::now();
    milliseconds timeDelta = std::chrono::duration_cast<milliseconds>(now - startTime);
    startTime = now;
    return timeDelta.count();
}

QVector<Oper> Tetris::caculateBot(TetrisNode &start, int limitTime)
{
    auto a = start;
    TreeContext ctx;
    auto dp = randSys.displayBag.mid(0, 6);
    ctx.createRoot(a, map, dp, holdSys.type, !!gamedata.b2b, gamedata.combo);
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;
    auto now = high_resolution_clock::now(), end = now + milliseconds(limitTime);
    do {
        ctx.run();
    } while ((now = high_resolution_clock::now()) < end);
    auto [best, ishold, test1] = ctx.getBest();
    QVector<Oper>path;
    if (!ishold)
        path = TetrisBot::make_path(a, best, map, true);
    else {
        auto holdNode = TetrisNode{holdSys.type == Piece::None ? randSys.displayBag[0] : holdSys.type};
        path = TetrisBot::make_path(holdNode, best, map, true);
        path.push_front(Oper::Hold);
    }
    return path;
}

void Tetris::botCall()
{
    auto limitTime = 140;
    recordPath.clear();
    QFuture<QVector<Oper>> future = QtConcurrent::run(this, &Tetris::caculateBot, tn, limitTime);
    watcher.setFuture(future);
}

void Tetris::ExampleMap()
{
    return;
    auto reverseMap = [&]() {
        for (auto j = 0; j < map.height; j++) {
            if (map.data[j] == 0)
                continue;
            auto reverse = 0;
            for (auto i = 0; i < map.width; i++) {
                auto x = (map.data[j] >> i);
                reverse |= (x & 1);
                reverse = reverse << 1;
            }
            map.data[j] = (reverse >> 1);
        }
    };


    /*  map.data[map.height - 3] = 0b1111111011;
      map.data[map.height - 2] = 0b1111110011;
      map.data[map.height - 1] = 0b1111111011;*/
    /*map.data[map.height - 3] = 0b1101111111;
    map.data[map.height - 2] = 0b1100111111;
    map.data[map.height - 1] = 0b1101111111;*/
    //tsd
    // map.data[map.height - 7] = 0b0000000000;
    // map.data[map.height - 6] = 0b0100000000;
    /* map.data[map.height - 5] = 0b0000000000;
     map.data[map.height - 4] = 0b0000000000;
     map.data[map.height - 3] = 0b0000111111;
     map.data[map.height - 2] = 0b0001111111;
     map.data[map.height - 1] = 0b0101111111;*/


    map.data[map.height - 8] = 0b1100000010;
    map.data[map.height - 7] = 0b1100111111;
    map.data[map.height - 6] = 0b1011101100;
    map.data[map.height - 5] = 0b1111101111;
    map.data[map.height - 4] = 0b0011000110;
    map.data[map.height - 3] = 0b1101111001;
    map.data[map.height - 2] = 0b1111110000;
    map.data[map.height - 1] = 0b0000000000;
    reverseMap();
    //qDebug() << 0b1111110101;

    /*  for (auto i = 1; i < 18; i++) {
          map.data[map.height - i] = 0b1111000111;
      }*/

    for (auto i = 0; i < map.height; i++)
        for (auto j = 0; j < map.width; j++) {
            if (map(i, j)) {
                map.roof = std::min(map.roof, i);
                map.count++;
                map.colorDatas[i][j] = Piece::Trash;
            }
        }
}
