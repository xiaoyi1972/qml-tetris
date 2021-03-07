#include "keystate.h"
#include"tetris.cpp"

KeyState::KeyState(Task *_task,
                   const std::function<void()> &_func, std::function<void()>_endFunc,
                   bool _isDown)
{
    func = _func;
    endFunc = _endFunc;
    isDown = _isDown;
    task = _task;
}

bool KeyState::keyDown()
{
    if (!press) {
        press = true;
        func();
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
       /* if (isDown) {
            startTime = std::chrono::high_resolution_clock::now();
        }*/
        arrHandle = task->setInterval(std::bind(&KeyState::funcChecked, this), delay);
    }
}

int KeyState::funcChecked()
{
    if (das || isDown) {
      /*  if (isDown) {
            auto delta = test();
            if (std::abs(delta - Tetris::keyconfig["softdropDelay"].value<int>()) > 3)
                qDebug() << delta;
        }*/
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
    das = false;
    stop();
}

void KeyState::switchStop()
{
    switchStopFlag = true;
    das = false;
    stop();
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
