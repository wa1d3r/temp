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
    HANDLE hChildStd_IN_Rd = NULL;
    HANDLE hChildStd_IN_Wr = NULL;
    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;

    void sendCommand(std::string cmd);
    std::string readResponse();
    std::string moveToString(const Move& move);
};