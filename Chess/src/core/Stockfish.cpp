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
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0))
        return false;
    if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
        return false;
    if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0))
        return false;
    if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
        return false;

    STARTUPINFOA siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = hChildStd_OUT_Wr;
    siStartInfo.hStdInput = hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    if (!CreateProcessA(NULL, const_cast<char*>(exePath.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo))
    {
        return false;
    }

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(hChildStd_OUT_Wr);
    CloseHandle(hChildStd_IN_Rd);

    sendCommand("uci");
    sendCommand("isready");
    return true;
}

void Stockfish::stop()
{
    sendCommand("quit");
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


bool Stockfish::start()
{
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0))
        return false;
    if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
        return false;
    if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0))
        return false;
    if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
        return false;

    STARTUPINFOA siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = hChildStd_OUT_Wr;
    siStartInfo.hStdInput = hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    if (!CreateProcessA(NULL, const_cast<char*>(exePath.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo))
    {
        return false;
    }

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(hChildStd_OUT_Wr);
    CloseHandle(hChildStd_IN_Rd);

    sendCommand("uci");
    sendCommand("isready");
    return true;
}

void Stockfish::stop()
{
    sendCommand("quit");
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
    DWORD dwWritten;
    cmd += "\n";
    WriteFile(hChildStd_IN_Wr, cmd.c_str(), cmd.length(), &dwWritten, NULL);
}

std::string Stockfish::readResponse()
{
    if (!hChildStd_OUT_Rd)
        return "";
    DWORD dwRead;
    CHAR chBuf[4096];
    std::string result;

    while (true)
    {
        if (!ReadFile(hChildStd_OUT_Rd, chBuf, 4096, &dwRead, NULL) || dwRead == 0)
            break;
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