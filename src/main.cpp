#include "ConsoleMenu.h"

#include <cstdio>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <clocale>
#endif

int main() {
#ifdef _WIN32
    // Перенаправленный ввод и вывод используют UTF-8.
    const int input = _fileno(stdin);
    const int output = _fileno(stdout);
    if (!_isatty(input)) {
        _setmode(input, _O_U8TEXT);
    }
    _setmode(output, _isatty(output) ? _O_U16TEXT : _O_U8TEXT);
#else
    std::setlocale(LC_ALL, "");
#endif
    ConsoleMenu menu;
    menu.run();
}
