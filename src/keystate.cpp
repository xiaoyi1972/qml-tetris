#include "keystate.h"
#include"tetris.cpp"

KeyState::KeyState(Tetris *_tetris,
                   const std::function<void()> &_func, std::function<void()>_endFunc,
                   bool _isDown, bool _noDas)
{
    func = _func;
    endFunc = _endFunc;
    isDown = _isDown;
    noDas = _noDas;
    tetris = _tetris;
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
            moveCall(tetris->keyconfig.softdropDelay);
        } else {
            dasHandle = tetris->task.setTimeOut(std::bind(&KeyState::dasCall, this), tetris->keyconfig.dasDelay);
        }
        return true;
    } else {
        return false;
    }
}

void  KeyState::dasCall()
{
    das = true;
    if (dasHandle != -1)
        moveCall(tetris->keyconfig.arrDelay);
}

void KeyState::moveCall(int delay)
{
    dasHandle = -1;
    /*  if (delay ==  0)
      {
      //   endFunc();
      }
      else*/
    {
        // func();
        arrHandle = tetris->task.setInterval(std::bind(&KeyState::funcChecked, this), delay);
    }
}

int KeyState::funcChecked()
{
    if (das || isDown) {
        func();
        return 0;
    } else {
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
        tetris->task.clearTimeout(dasHandle);
        dasHandle = -1;
    }
    tetris->task.clearInterval(arrHandle);
    arrHandle = -1;
}
