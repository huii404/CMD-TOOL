#ifndef EXTENSION_H
#define EXTENSION_H

#include "SystemCore.h"
#include <string>

class Extension {
private:
    SystemCore &sc;
public:
    Extension(SystemCore &s);
    bool text_processing(const std::string &text);
    void ShowQR(std::string text);
    void ShowN_QR(int number);
    void uninstallBloatware();
};

#endif