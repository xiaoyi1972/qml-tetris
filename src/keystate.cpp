#include "keystate.h"
#include"tetris.cpp"

KeyState::KeyState(Task*_task,
        const std::function<void()> &_func, std::function<void()>_endFunc,
        bool _isDown, bool _noDas)
{
    func = _func;
    endFunc = _endFunc;
    isDown = _isDown;
    noDas = _noDas;
    task=_task;
}

bool KeyState::keyDown()
{
    if (!press) {
        press = true;
        func();
        if (noDas) {
            return true;
        }
        if (isDown) {
            moveCall(Tetris::keyconfig["softdropDelay"].value<int>());
        } else {
            dasHandle = task->setTimeOut(std::bind(&KeyState::dasCall, this), Tetris::keyconfig["dasDelay"].value<int>());
        }
        return true;
    } else {
        return false;
    }
}

void  KeyState::dasCall()
{
    das = true;
    dasHandle = -1;
    moveCall(Tetris::keyconfig["arrDelay"].value<int>());
}

void KeyState::moveCall(int delay)
{

    /*  if (delay ==  0)
      {
      //   endFunc();
      }
      else*/
    {
        // func();
        arrHandle = task->setInterval(std::bind(&KeyState::funcChecked, this), delay);
    }
}

int KeyState::funcChecked()
{
    if (das || isDown) {
        func();
        return 0;
    } else {
        qDebug() << "bad";
        return -1;
    }
}

void KeyState::keyUp()
{
    switchStopFlag = false;
    press = false;
    if (!noDas) {
        das = false;
        stop();
    }
}

void KeyState::switchStop()
{
    switchStopFlag = true;
    if (!noDas) {
        das = false;
        stop();
    }
}

void KeyState::stop()
{
    if (!isDown) {
        task->clearTimeout(dasHandle);
        dasHandle = -1;
    }
    task->clearInterval(arrHandle);
    arrHandle = -1;
}
