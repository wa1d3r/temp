#include "Stockfish.h"
#include <iostream>
#include <sstream>

Stockfish::Stockfish(std::string path)
    : exePath(path)
{
}

Stockfish::~Stockfish()
{
    stop();
}

bool Stockfish::start()
{
    SECURITY_ATTRIBUTES saAttr; // параметры безопасности объекта
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); // требование WinAPI
    saAttr.bInheritHandle = TRUE; // разришить наследование дочернему процессу
    saAttr.lpSecurityDescriptor = NULL; // Дескриптор безопасности по умолчанию

    // труба для вывода данных от дочернего процесса
    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0))
        return false;

    // дескриптор для чтения не наследуется, чтобы stockfish не читал сам себя
    if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
        return false;

    // ввод данных в дочерний процесс
    if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0))
        return false;

    // дескриптор для записи не наследуется, чтобы stockfish не писал сам себе
    if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
        return false;

    STARTUPINFOA siStartInfo; // параметры запуска процесса
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO)); // Обнуляем память структуры
    siStartInfo.cb = sizeof(STARTUPINFO); // Указываем размер

    // Подмена стандартных потоков, на созданные
    siStartInfo.hStdError = hChildStd_OUT_Wr; 
    siStartInfo.hStdOutput = hChildStd_OUT_Wr;
    siStartInfo.hStdInput = hChildStd_IN_Rd;

    // указание использовать прописанные дескрипторы
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo; // Информация о созданном процессе
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // запуск процесса
    if (!CreateProcessA(NULL, const_cast<char*>(exePath.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo))
    {
        return false;
    }

    // закрытие дескрипторов процесса и потока
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(hChildStd_OUT_Wr);
    CloseHandle(hChildStd_IN_Rd);

    // Инициализация протокола UCI
    sendCommand("uci");
    sendCommand("isready");
    return true;
}

void Stockfish::stop()
{
    sendCommand("quit");

    // закрытие труб
    if (hChildStd_IN_Wr)
    {
        CloseHandle(hChildStd_IN_Wr);
        hChildStd_IN_Wr = NULL;
    }
    if (hChildStd_OUT_Rd)
    {
        CloseHandle(hChildStd_OUT_Rd);
        hChildStd_OUT_Rd = NULL;
    }
}

void Stockfish::sendCommand(std::string cmd)
{
    if (!hChildStd_IN_Wr)
        return;
    DWORD dwWritten; // количество записанных байт (требование)
    cmd += "\n";

    // запись в ввод движка
    WriteFile(hChildStd_IN_Wr, cmd.c_str(), cmd.length(), &dwWritten, NULL);
}

std::string Stockfish::readResponse()
{
    if (!hChildStd_OUT_Rd)
        return "";
    DWORD dwRead;  // количество прочитанных байт
    CHAR chBuf[4096]; 
    std::string result;

    while (true)
    {
        // Если чтение не удалось — выход
        if (!ReadFile(hChildStd_OUT_Rd, chBuf, 4096, &dwRead, NULL) || dwRead == 0)
            break;

        // останавливаемся на найденном ходе
        result.append(chBuf, dwRead);
        if (result.find("bestmove") != std::string::npos)
            break;
    }
    return result;
}

std::string Stockfish::moveToString(const Move& move)
{
    std::string res = "";
    res += (char)('a' + move.getFrom().getX());
    res += (char)('1' + move.getFrom().getY());
    res += (char)('a' + move.getTo().getX());
    res += (char)('1' + move.getTo().getY());
    if (!move.getPromotionPiece().empty())
    {
        if (move.getPromotionPiece() == "knight")
            res += 'n';
        else
            res += move.getPromotionPiece()[0];
    }
    return res;
}

std::string Stockfish::getBestMove(const std::string& fen)
{
    std::string cmd = "position fen " + fen;

    sendCommand(cmd);
    sendCommand("go movetime 1000"); // 1 секунда на поиск хода

    std::string output = readResponse(); // ожидание ответа

    size_t pos = output.find("bestmove");
    if (pos != std::string::npos)
    {
        std::string moveStr = output.substr(pos + 9, 5);
        size_t space = moveStr.find(' ');
        if (space != std::string::npos)
            moveStr = moveStr.substr(0, space);
        moveStr.erase(std::remove(moveStr.begin(), moveStr.end(), '\n'), moveStr.end());
        moveStr.erase(std::remove(moveStr.begin(), moveStr.end(), '\r'), moveStr.end());
        return moveStr;
    }
    return "";
}