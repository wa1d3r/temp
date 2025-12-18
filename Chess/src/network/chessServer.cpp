#include "chessServer.h" 

ChessServer::ChessServer(unsigned short port)
    : port(port)
{
}

void ChessServer::run()
{
    if (listener.listen(port) != sf::Socket::Status::Done)
    {
        std::cerr << "Error: Could not listen to port " << port << std::endl;
        return;
    }

    std::cout << "Server is listening on port " << port << "..." << std::endl;
    selector.add(listener);

    while (true)
    {
        if (selector.wait())
        {
            if (selector.isReady(listener))
            {
                handleNewConnection();
            }
            handleClientActivity();
        }
    }
}

void ChessServer::handleNewConnection()
{
    if (clients.size() >= 2)
    {
        // Если уже 2 игрока, отклоняем 
        sf::TcpSocket temp;
        if (listener.accept(temp) == sf::Socket::Status::Done)
        {
            temp.disconnect();
        }
        return;
    }

    auto client = std::make_unique<sf::TcpSocket>();

    if (listener.accept(*client) == sf::Socket::Status::Done)
    {
        if (auto addr = client->getRemoteAddress())
            std::cout << "New client connected: " << addr->toString() << std::endl;
        else
            std::cout << "New client connected (unknown IP)" << std::endl;

        selector.add(*client);
        clients.push_back(std::move(client));

        if (clients.size() == 2)
        {
            startGame();
        }
        else
        {
            std::cout << "Waiting for second player..." << std::endl;
        }
    }
}

void ChessServer::startGame()
{
    std::cout << "Match started! Applying host settings..." << std::endl;
    std::cout << "Settings: Host=" << (hostColorInt == 0 ? "White" : "Black")
              << ", Time=" << timeMinutes << "+" << incrementSeconds << std::endl;

    // 1. Формируем пакеты конфигурации

    // Для Хоста (Client 0) - настройки как есть
    sf::Packet p1;
    p1 << static_cast<int>(PacketType::GameConfig)
       << hostColorInt
       << timeMinutes
       << incrementSeconds
       << gameTypeInt
       << seed;
    clients[0]->send(p1);

    // Для Гостя (Client 1)
    sf::Packet p2;
    int guestColorInt = (hostColorInt == 0) ? 1 : 0;
    p2 << static_cast<int>(PacketType::GameConfig)
       << guestColorInt
       << timeMinutes
       << incrementSeconds
       << gameTypeInt
       << seed;
    clients[1]->send(p2);

    sf::Packet pStart;
    pStart << static_cast<int>(PacketType::StartGame);

    for (auto& client : clients)
        client->send(pStart);
}

void ChessServer::handleClientActivity()
{
    for (auto it = clients.begin(); it != clients.end();)
    {
        sf::TcpSocket& socket = **it;

        if (selector.isReady(socket))
        {
            sf::Packet packet;
            sf::Socket::Status status = socket.receive(packet);

            if (status == sf::Socket::Status::Done)
            {
                processPacket(socket, packet);
                ++it;
            }
            else if (status == sf::Socket::Status::Disconnected)
            {
                std::cout << "Client disconnected" << std::endl;
                selector.remove(socket);
                it = clients.erase(it);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }
}

void ChessServer::processPacket(sf::TcpSocket& sender, sf::Packet& packet)
{
    int typeInt;
    if (packet >> typeInt)
    {
        PacketType type = static_cast<PacketType>(typeInt);

        if (type == PacketType::Move)
        {
            Move move;
            if (packet >> move)
            {
                sf::Packet relayPacket;
                relayPacket << static_cast<int>(PacketType::Move) << move;
                relayPacketToOthers(sender, relayPacket);
            }
        }
        else if (type == PacketType::GameOver)
        {
            sf::Packet relayPacket;
            relayPacket << static_cast<int>(PacketType::GameOver);
            relayPacketToOthers(sender, relayPacket);
        }
        else if (type == PacketType::GameConfig)
        {
            if (clients.size() > 0 && clients[0].get() == &sender)
            {
                int c, t, i, g, s;
                if (packet >> c >> t >> i >> g >> s)
                {
                    hostColorInt = c;
                    timeMinutes = t;
                    incrementSeconds = i;
                    gameTypeInt = g;
                    seed = s;
                    std::cout << "Host updated settings: Mode=" << g << ", Seed=" << s << std::endl;
                }
            }
        }
    }
}

void ChessServer::relayPacketToOthers(sf::TcpSocket& sender, sf::Packet& packet)
{
    for (auto& client : clients)
    {
        if (client.get() != &sender)
        {
            client->send(packet);
        }
    }
}