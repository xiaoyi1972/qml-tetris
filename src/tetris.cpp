﻿#include "tetris.h"

void Task::back(QThread *td) {
    for (auto &id : m_intervalHash.keys()) {
        if (thread() != QThread::currentThread())
            QMetaObject::invokeMethod(this, "killTimer_", Qt::BlockingQueuedConnection, Q_ARG(int, id));
        else
            killTimer(id);
    }
    for (auto &id : m_timeoutHash.keys()) {
        if (thread() != QThread::currentThread())
            QMetaObject::invokeMethod(this, "killTimer_", Qt::BlockingQueuedConnection, Q_ARG(int, id));
        else
            killTimer(id);
    }
    m_intervalHash.clear();
    m_timeoutHash.clear();
    if (thread() != QThread::currentThread())
        QMetaObject::invokeMethod(this, "test", Qt::BlockingQueuedConnection, Q_ARG(QThread *, td));
    else
        test(td);
}

void Task::pressHandle(int e) {
    auto &_key = e;
    if (_key == Tetris:: keyconfig["leftKey"]) {
        if (rightKey.press)
            rightKey.switchStop();
        else
            rightKey.keyUp();
        leftKey.keyDown();
    } else if (_key == Tetris:: keyconfig["rightKey"]) {
        if (leftKey.press)
            leftKey.switchStop();
        else
            leftKey.keyUp();
        rightKey.keyDown();
    } else if (_key == Tetris:: keyconfig["softDropKey"])
        softDropKey.keyDown();
}

void Task::releaseHandle(int e) {
    auto &_key = e;
    if (_key == Tetris:: keyconfig["leftKey"]) {
        leftKey.keyUp();
        if (rightKey.switchStopFlag) {
            rightKey.keyUp();
            rightKey.keyDown();
        }
    } else if (_key == Tetris:: keyconfig["rightKey"]) {
        rightKey.keyUp();
        if (leftKey.switchStopFlag) {
            leftKey.keyUp();
            leftKey.keyDown();
        }
    } else if (_key == Tetris:: keyconfig["softDropKey"])
        softDropKey.keyUp();
}

void Task::test(QThread *td) {
    moveToThread(td);
}

void Task::killTimer_(int timerId) {
    killTimer(timerId);
}

int Task::startTimer_(int delay) {
    int timerId = startTimer(delay, Qt::PreciseTimer);
    return timerId;
}

int Task::setTimeOut(const std::function<void()> &func, int delay) {
    int timerId;
    if (thread() != QThread::currentThread())
        QMetaObject::invokeMethod(this, "startTimer_", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, timerId), Q_ARG(int, delay));
    else
        timerId = startTimer(delay, Qt::PreciseTimer);
    m_timeoutHash.insert(timerId, func);
    return timerId;
}

void Task::clearTimeout(int timerId) {
    if (thread() != QThread::currentThread()) {
        //qDebug() << "no same";
        QMetaObject::invokeMethod(this, "clearTimeout", Qt::QueuedConnection, Q_ARG(int, timerId));
        return;
    }
    if (timerId != -1)
        if (m_timeoutHash.contains(timerId)) {
            m_timeoutHash.remove(timerId);
            if (thread() != QThread::currentThread())
                QMetaObject::invokeMethod(this, "killTimer_", Qt::BlockingQueuedConnection, Q_ARG(int, timerId));
            else
                killTimer(timerId);
        }
}

int Task::setInterval(const std::function<int()> &func, int delay) {
    int timerId;
    if (thread() != QThread::currentThread())
        QMetaObject::invokeMethod(this, "startTimer_", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, timerId), Q_ARG(int, delay));
    else
        timerId = startTimer(delay, Qt::PreciseTimer);
    m_intervalHash.insert(timerId, func);
    return timerId;
}

void Task::clearInterval(int timerId) {
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "clearInterval", Qt::QueuedConnection, Q_ARG(int, timerId));
        return;
    }
    if (timerId != -1)
        if (m_intervalHash.contains(timerId)) {
            m_intervalHash.remove(timerId);
            if (thread() != QThread::currentThread())
                QMetaObject::invokeMethod(this, "killTimer_", Qt::BlockingQueuedConnection, Q_ARG(int, timerId));
            else
                killTimer(timerId);
        }
}

void Task::timerEvent(QTimerEvent *event) {
    auto id = event->timerId();
    auto intervalFunc = m_intervalHash.value(id, nullptr);
    if (intervalFunc == nullptr) {
        auto timeFunc = m_timeoutHash.value(id, nullptr);
        if (timeFunc != nullptr) {
            clearTimeout(id);
            timeFunc();
        }
    } else {
        auto res = intervalFunc();
        if (res == -1)
            clearInterval(id);
    }
}

QHash<int, std::tuple<int, int, int, int>> TetrisTetro::tightNodes;
MyThread Tetris::td;

QQmlPropertyMap Tetris:: keyconfig;
bool initConfig = std::invoke([]() {
    Tetris::keyconfig.insert("leftKey", QVariant(int(Qt::Key_Left)));
    Tetris::keyconfig.insert("rightKey", QVariant(int(Qt::Key_Right)));
    Tetris::keyconfig.insert("softDropKey", QVariant(int(Qt::Key_Down)));
    Tetris::keyconfig.insert("harddropKey", QVariant(int(Qt::Key_Space)));
    Tetris::keyconfig.insert("cwKey", QVariant(int(Qt::Key_Z)));
    Tetris::keyconfig.insert("ccwKey", QVariant(int(Qt::Key_X)));
    Tetris::keyconfig.insert("holdKey", QVariant(int(Qt::Key_C)));
    Tetris::keyconfig.insert("restartKey", QVariant(int(Qt::Key_R)));
    Tetris::keyconfig.insert("dasDelay", QVariant(int(69)));
    Tetris::keyconfig.insert("arrDelay", QVariant(int(17)));
    Tetris::keyconfig.insert("softdropDelay", QVariant(int(8)));
    Tetris::keyconfig.insert("replay", QVariant(int(Qt::Key_Q)));
    Tetris::keyconfig.insert("bot", QVariant(int(Qt::Key_W)));
    Tetris::keyconfig.insert("botTimeCost", QVariant(int(100)));
    return true;
});

Tetris::Tetris(QQuickItem *parent) : QQuickItem(parent) {
    setFlag(ItemHasContents, true);
    cur.setParentItem(this);
    ghost.setParentItem(this);
    ghost.setProperty("ghost", true);
    connect(&watcher, SIGNAL(finished()),  this, SLOT(playPath()));
    connect(this, SIGNAL(the(int)),  &task, SLOT(pressHandle(int)));
    connect(this, SIGNAL(the1(int)),  &task, SLOT(releaseHandle(int)));
    task.leftKey.func = std::bind(&Tetris::left, this);
    task.rightKey.func = std::bind(&Tetris::right, this);
    task.softDropKey.func = std::bind(&Tetris::softdrop, this);
    task.moveToThread(&td);
    if (!td.isRunning())
        td.start(QThread::Priority::TimeCriticalPriority);
    startTime = std::chrono::high_resolution_clock::now();

    mtest.insert("x", QVariant(int(0)));
    mtest.insert("y", QVariant(int(0)));
    mtest.insert("rs", QVariant(int(0)));
    mtest.insert("minoType", QVariant(int(-1)));
    mtest.insert("newSpawn", QVariant(bool(true)));
}

Tetris::~Tetris() {
    task.back(thread());
}

QSGNode *Tetris::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    QSGGeometryNode *node = nullptr;
    QSGGeometry *geometry = nullptr;
    constexpr int drawStyle = 0;
    auto getStyle = [&]() {
        int count;
        switch (drawStyle) {
            case 0: count = map.count * 3 * 2; break;
            case 1: count = map.count * 8; break;
        }
        return count;
    };

    if (!oldNode) {
        node = new QSGGeometryNode;
        geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), getStyle(), 0, QSGGeometry::UnsignedIntType);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        geometry->setLineWidth(1);
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
    auto const grid = 20 * this->scale();
    auto const c = this->scale();
    for (int y = map.roof, i = 0; y < map.height; y++)
        for (int x = 0; x < map.width; x++) {
            if (map.full(y, x)) {
                auto color = getColors(map.colorDatas[y][x]);
                QRectF area(0 + x * grid + 0.5 / c, 0 + y * grid + 0.5 / c, grid - 1 / c, grid - 1 / c);
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
                i++;
            }
        }
    node->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
    return node;
}

void Tetris::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat())
        return;
    auto _key = event->key();
    if (!deaded) {
        if (_key == keyconfig["harddropKey"])
            harddrop();
        else if (_key == keyconfig["cwKey"])
            cw();
        else if (_key == keyconfig["ccwKey"])
            ccw();
        else  if (_key == keyconfig["holdKey"])
            hold();
    }
    if (_key == keyconfig["restartKey"]) {
        task.clearInterval(handle);
        isReplay = false;
        restart();
    } else if (_key == keyconfig["replay"])
        replay();
    else if (_key == keyconfig["bot"]) {
        tg = !tg;
        if (tg)
            botCall();
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
            map.colorDatas.erase(map.colorDatas.begin());
            map.colorDatas.push_back(std::vector<Piece>(map.width, Piece::Trash));
            //    map.colorDatas.push_back(QVector<Piece>(map.width).fill(Piece::Trash));
            map.colorDatas[map.height - 1][num] = Piece::None;
        }
        map.update();
    } else if (_key == Qt::Key_A) {
        TetrisBot::evalute(tn, map, 0, 0);
        // TetrisBot::search(tn, map);
        //qDebug() <<
        //   sendAttack(4);
    } else if (_key == Qt::Key_B) {
        //  getTrash(3);
        // getTrash(1);
        for (auto i = 0; i < 4; i++)
            emit sendAttack(1);
    } else if (_key == Qt::Key_H) {
        getTrash(4);
        /* getTrash(1);
         for (auto i = 0; i < 4; i++) {
             emit sendAttack(1);
         }*/
    } else
        emit the(_key);
}

//键盘释放事件
void Tetris::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat())
        return;
    auto _key = event->key();
    emit the1(_key);
    return;
}

void Tetris::hold() {
    auto res = holdSys.exchange(tn);
    if (res > 0) {
        if (!isReplay)
            record.add(Oper::Hold, timeRecord());
        passHold();
        tn.lastRotate = false;
    }
    if (res == 2)
        passNext();
    updateCall();
}

void Tetris::restart() {
    startTime = std::chrono::high_resolution_clock::now();
    deaded = false;
    digRowsEnd = 100;
    gamedata.trashLines.clear();
    if (!isReplay) {
        randSys = Random{};
        record = Recorder{randSys.seed};
    } else
        randSys = Random{record.seed};
    holdSys = Hold {&randSys};
    map = TetrisMapEx{10, 22};
    tn =   TetrisNode::spawn(randSys.getOne());
    setTrash(0);
    gamedata = gameData{};
    emit restartGame();
    ExampleMap();
    if (digMod)
        digModAdd(digRows, true);
    passNext(true);
    passHold(true);
    updateCall();
}

void Tetris::digModAdd(int _needDigRows, bool _isInit) {
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
            /* map.colorDatas.remove(0);
             map.colorDatas.push_back(QVector<Piece>(map.width).fill(Piece::Trash));*/
            map.colorDatas.erase(map.colorDatas.begin());
            map.colorDatas.push_back(std::vector<Piece>(map.width, Piece::Trash));
            map.colorDatas[map.height - 1][num] = Piece::None;
        }
    }
    digRows = 10;
    map.update();
}

void Tetris::left() {
    if (TetrisNode::shift(tn, map, -1, 0)) {
        if (!isReplay)
            record.add(Oper::Left, timeRecord());
        tn.lastRotate = false;
        updateCall();
    }
}

void Tetris::right() {
    if (TetrisNode::shift(tn, map, 1, 0)) {
        if (!isReplay)
            record.add(Oper::Right, timeRecord());
        tn.lastRotate = false;
        updateCall();
    }
}

void Tetris::softdrop() {
    if (TetrisNode::shift(tn, map, 0, 1)) {
        if (!isReplay)
            record.add(Oper::SoftDrop, timeRecord());
        tn.lastRotate = false;
        updateCall();
    }
}

void Tetris::harddrop() {
    auto addLines = [&](int rows) {
        auto row = 0b1111111111;
        auto num = 0;
        num = int(randSys.rand.generate() % 10);
        row &= ~(1 << num);
        for (auto i = 0; i < rows; i++) {
            memcpy(map.data, map.data + 1, (map.height - 1) * sizeof(int));
            map.data[map.height - 1 ] = row;
            map.colorDatas.erase(map.colorDatas.begin());
            map.colorDatas.push_back(std::vector<Piece>(map.width, Piece::Trash));
            map.colorDatas[map.height - 1][num] = Piece::None;
        }
    };

    if (!isReplay)
        record.add(Oper::HardDrop, timeRecord());
    holdSys.reset();

    auto dropDis = tn.getDrop(map);
    if (dropDis > 0)
        tn.lastRotate = false;
    tn.pos.y += dropDis;
    if (tn.type == Piece::T) {
        auto [spin, mini] = tn.corner3(map);
        if (spin)
            tn.typeTSpin = mini ? TSpinType::TSpinMini : TSpinType::TSpin;
    }
    auto [pieceChanges, clear] = tn.attachs(map);
    auto isTSpin = (tn.typeTSpin != TSpinType::None && tn.lastRotate);
    if (clear.size() == 4 || (clear.size() > 0 && tn.typeTSpin != TSpinType::None))
        gamedata.b2b++;
    else if (clear.size() > 0)
        gamedata.b2b = 0;
    gamedata.combo = clear.size() > 0 ? gamedata.combo + 1 : 0;
    auto attack = sendTrash(std::tuple<TSpinType, int, bool, int> {tn.typeTSpin, clear.size(), gamedata.b2b > 1, gamedata.combo});
    gamedata.pieces++;
    gamedata.clear += clear.size();
    auto pieceType = tn.type;
    auto piece = randSys.getOne();
    passNext();
    tn = TetrisNode::spawn(piece);
    if (digMod && digRowsEnd > 0) {//for dig mod
        for (auto &row_i : clear) {
            if (((map.height - 11) + (10 - digRows)) < row_i && row_i < map.height)
                digRows--;
        }
        if (clear.size() == 0) {
            auto needDigRows = 10 - digRows;
            digRowsEnd -= needDigRows;
            digModAdd(needDigRows, false);
            digRows = 10;
            map.update(false);
            passMap();
            for (auto &piecePos : pieceChanges)
                piecePos.x -= needDigRows;
        }
    }
    mutex.lock();
    auto noClear = (clear.size() == 0);
    auto addCount = 0;
    while (!gamedata.trashLines.empty()) {
        auto &first = gamedata.trashLines.front();
        if (!noClear) {
            if (first < attack && (attack -= first))
                setTrash(gamedata.trashLinesCount -  gamedata.trashLines.takeFirst());
            else {
                setTrash(gamedata.trashLinesCount - attack);
                attack = (first -= attack) ? 0 : gamedata.trashLines.takeFirst();
                if (!attack)
                    break;
            }
        } else {
            auto every = gamedata.trashLines.takeFirst();
            addCount += every;
            addLines(every);
        }
    }
    if (noClear &&  addCount > 0) {
        setTrash(0);
        map.update(false);
        passMap();
        for (auto &piecePos : pieceChanges)
            piecePos.x -= addCount;
    }
    if (attack > 0)
        emit sendAttack(attack);
    mutex.unlock();
    if (!tn.check(map)) //check is dead
        deaded = true;
    toFresh(pieceType, pieceChanges, clear, std::tuple<bool, bool, int> {isTSpin, gamedata.b2b > 1, gamedata.combo});
    updateCall();
}

int Tetris::sendTrash(const std::tuple<TSpinType, int, bool, int> &status) {
    int attackTo = 0;
    auto [spin, clear, b2b, combo] = status;
    int clears[] = {0, 0, 1, 2, 4};
    if (spin != TSpinType::None) {
        switch (clear) {
            case 1: attackTo += ((spin == TSpinType::TSpinMini ? 1 : 2) + b2b); break;
            case 2: attackTo += (4 + b2b); break;
            case 3: attackTo += (b2b ? 6 : 8); break;
            default: break;
        }
    } else
        attackTo += (clears[clear] + (clear == 4 ? b2b : 0));
    if (map.count == 0)
        attackTo += 6;
    if (combo > 0)
        attackTo += comboTable[std::min(combo - 1, tableMax - 1)];
    return attackTo;
}

void Tetris::ccw() {
    if (TetrisNode::rotate(tn, map, false)) {
        if (!isReplay)
            record.add(Oper::Ccw, timeRecord());
        tn.lastRotate = true;
        updateCall();
    }
}

void Tetris::cw() {
    if (TetrisNode::rotate(tn, map)) {
        if (!isReplay)
            record.add(Oper::Cw, timeRecord());
        tn.lastRotate = true;
        updateCall();
    }
}

void Tetris::hello() {
    qDebug() << "hello";
    task.setTimeOut(std::bind(&Tetris::hello, this), 100);
}

void Tetris::opers(Oper a) {
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

void Tetris::toFresh(Piece &_type, std::vector<Pos> &pieceChanges, std::vector<int> &clear, const std::tuple<bool, bool, int> &clearState) {
    QVariantList list, next;
    auto [tspin, b2b, combo] = clearState;
    for (int y = 0; y < map.height; y++)
        for (int x = 0; x < map.width; x++)
            list << static_cast<int>(map.colorDatas[y][x]);
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

void Tetris::passHold(bool ifForce) {
    QVariantMap mapMsg;
    mapMsg.insert("type", static_cast<int>(holdSys.type));
    mapMsg.insert("able", holdSys.able);
    mapMsg.insert("force", ifForce);
    emit holdFresh(mapMsg);
}

void Tetris::passNext(bool ifForce) {
    QVariantList nexts;
    for (auto piece : randSys.displayBag)
        nexts << static_cast<int>(piece);
    QVariantMap map;
    map.insert("next", nexts);
    map.insert("nextNew", nexts.back());
    map.insert("force", ifForce);
    emit next(map);
}

void Tetris::passMap() {
    QVariantList list;
    for (int y = 0; y < map.height; y++)
        for (int x = 0; x < map.width; x++)
            list << static_cast<int>(map.colorDatas[y][x]);
    QVariantMap mapMsg;
    mapMsg.insert("map", list);
    emit mapFresh(mapMsg);
}

void Tetris::replay(const QString &str) {
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

void Tetris::replayFunc() {
    auto [time, oper] = record.play();
    opers(oper);
    if (!record.isEnd())
        handle = task.setTimeOut(std::bind(&Tetris::replayFunc, this), time);
    else {
        task.clearTimeout(handle);
        isReplay = false;
    }
}

void Tetris::updateCall() {
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "updateCall", Qt::QueuedConnection);
        return;
    }
   // if (!tg)
    {
        cur.fresh(tn);
        ghost.fresh(tn,tn.getDrop(map));
        update();
    }
}

void Tetris::replayBotOperFunc() {
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "replayBotOperFunc", Qt::QueuedConnection);
        return;
    }
    if (!recordPath.isEnd()) {
        auto [time, oper] = recordPath.play();
        opers(oper);
        botOperHandle = task.setTimeOut(std::bind(&Tetris::replayBotOperFunc, this), time);
    } else {
        task.clearTimeout(handle);
        // tg = false;
        if (tg)
            botHandle = task.setTimeOut(std::bind(&Tetris::botCall, this), 0);
    }
}

void Tetris::playPath() {
    auto path  = watcher.result();
    auto interval = 0;
    if (path.empty())
        path.append(Oper::HardDrop);
    for (auto i = 0; i < path.size(); i++) {
        recordPath.add(path[i],  interval); //interval
        if ((i + 1 < path.size()) &&  path[i + 1] == Oper::SoftDrop)
            interval = 8;
        else if (path[i] != Oper::HardDrop)
            interval = 20;
        else
            interval = 0;
    }
    replayBotOperFunc();
}

int Tetris::timeRecord() {
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;
    auto now = high_resolution_clock::now();
    milliseconds timeDelta = std::chrono::duration_cast<milliseconds>(now - startTime);
    startTime = now;
    return timeDelta.count();
}

QVector<Oper> Tetris::caculateBot(const TetrisNode &start, const int limitTime) const {
    auto a = start;
    TreeContext ctx;
    auto dp = randSys.displayBag.mid(0, 6);
    auto &mapRef = const_cast<TetrisMapEx &>(map);
    ctx.createRoot(a, mapRef, dp, holdSys.type, !!gamedata.b2b, gamedata.combo, gamedata.trashLinesCount);
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;
    auto now = high_resolution_clock::now(), end = now + milliseconds(limitTime);
    do {
        ctx.run();
    } while ((now = high_resolution_clock::now()) < end);
    auto [best, ishold, test1] = ctx.getBest();
    QVector<Oper>path;
    if (!ishold)
        path = TetrisBot::make_path(a, best, mapRef, true);
    else {
        auto holdNode = TetrisNode::spawn(holdSys.type == Piece::None ? randSys.displayBag[0] : holdSys.type);
        path = TetrisBot::make_path(holdNode, best, mapRef, true);
        path.push_front(Oper::Hold);
    }
    return path;
}

void Tetris::botCall() {
    auto limitTime = keyconfig["botTimeCost"].value<int>();
    recordPath.clear();
    QFuture<QVector<Oper>> future = QtConcurrent::run(this, &Tetris::caculateBot, tn, limitTime);
    watcher.setFuture(future);
}

void Tetris::ExampleMap() {
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

    auto TestStyle = [&](int style) {
        if (style == 0) {//huopin
            for (auto i = 1; i < 19; i++) {
                auto row = 0b1010101010;
                map.data[map.height - i] = (i % 2 ? row : (row >> 1));
            }
        } else if (style == 1) {//more wells
            auto n = 0;
            auto m = (randSys.rand.generate() % 10);
            for (auto i = 1; i < 19 - 1; i++) {
                auto row = 0b1111111111;
                n++;
                if (n == 4) {
                    n = 0;
                    m = (randSys.rand.generate() % 10);
                }
                row &= ~(1 << (m % 10));
                map.data[map.height - i] = row;//(i % 2 ? row : (row >> 1));
            }
        } else if (style == 2) { //t3 slot
            map.data[map.height - 5] = 0b1000000000;
            map.data[map.height - 4] = 0b1000000000;
            map.data[map.height - 3] = 0b1011111111;
            map.data[map.height - 2] = 0b1001111111;
            map.data[map.height - 1] = 0b1011111111;
            reverseMap();
        } else if (style == 3) {//t2 slot
            map.data[map.height - 4] = 0b1100111111;
            map.data[map.height - 3] = 0b1100011111;
            map.data[map.height - 2] = 0b1110111111;
            map.data[map.height - 1] = 0b1110111111;
            reverseMap();
        }
    };
    TestStyle(3);
    map.update();
}


