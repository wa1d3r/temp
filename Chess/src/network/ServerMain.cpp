#include "chessServer.h"
#include <iostream>

int main()
{
    unsigned short port = 53000;

    std::cout << "--- Chess Server (SFML 3.0) ---" << std::endl;
    std::cout << "Initializing on port " << port << "..." << std::endl;

    try
    {
        ChessServer server(port);
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Critical Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}