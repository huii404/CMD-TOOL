#ifndef AUTOACTIONS_H
#define AUTOACTIONS_H

#include "SystemCore.h"

class AutoActions {
private:
    SystemCore &sc;
public:
    AutoActions(SystemCore &s);
    void autoClickPoint();
    void spamText();
    void autoPasteData();
};

#endif