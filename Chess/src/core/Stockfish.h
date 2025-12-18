#pragma once
#include "move.h"
#include <string>
#include <vector>
#include <windows.h>

class Stockfish
{
public:
    Stockfish(std::string path);
    ~Stockfish();

    bool start();
    std::string getBestMove(const std::string& fen);
    void stop();

private:
    std::string exePath;

    // Указатели на ресурсы
    HANDLE hChildStd_IN_Rd = NULL; // Чтение из трубы ввода (для дочернего процесса)
    HANDLE hChildStd_IN_Wr = NULL; // Запись в трубу ввода (пишет приложение)
    HANDLE hChildStd_OUT_Rd = NULL; // Чтение из трубы вывода (читает приложение)
    HANDLE hChildStd_OUT_Wr = NULL; // Запись в трубу вывода (пишет дочерний процесс)

    void sendCommand(std::string cmd);
    std::string readResponse();
    std::string moveToString(const Move& move);
};