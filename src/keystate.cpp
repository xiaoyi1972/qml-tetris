#include "keystate.h"
#include"tetris.cpp"

KeyState::KeyState(
        const std::function<void()> &_func, std::function<void()>_endFunc,
        bool _isDown, bool _noDas)
{
    func = _func;
    endFunc = _endFunc;
    isDown = _isDown;
    noDas = _noDas;
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
            moveCall(Tetris::keyconfig.softdropDelay);
        } else {
            dasHandle = Tetris::task.setTimeOut(std::bind(&KeyState::dasCall, this), Tetris::keyconfig.dasDelay);
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
    moveCall(Tetris::keyconfig.arrDelay);
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
        arrHandle = Tetris::task.setInterval(std::bind(&KeyState::funcChecked, this), delay);
    }
}

int KeyState::funcChecked()
{
    if (das || isDown) {
        func();
        return 0;
    } else {
        qDebug() << "haole";
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
        Tetris::task.clearTimeout(dasHandle);
        dasHandle = -1;
    }
    Tetris::task.clearInterval(arrHandle);
    arrHandle = -1;
}
