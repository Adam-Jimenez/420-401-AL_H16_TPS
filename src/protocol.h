#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <string>

struct Protocol
{
    enum Constants
    {
        MsgCommandNumClients  = 0x6579f100,
        MsgCommandClientId    = 0x6b4a6200,
        MsgCommandClientReady = 0x28b5ac00,
        MsgCommandServerReady = 0x55f54d63,
        MsgMessageAck         = 0x12d8a9c7
    };

    static const std::string labelServer()
    {
        static std::string label("s");

        return label;
    }

    static const std::string labelClient()
    {
        static std::string label("c");

        return label;
    }

    static std::string clientIdToLabel(int clientId)
    {
        std::string label = labelClient();
        label += static_cast<char>('0' + clientId);

        return label;
    }

    static int clientLabelToId(const std::string &clientLabel)
    {
        int clientId = clientLabel[clientLabel.size() - 1];

        return static_cast<int>(clientId - '0');
    }

    static const std::string & endCommand()
    {
        static std::string cmd("end");

        return cmd;
    }

    static const std::string & spawnCommand()
    {
        static std::string cmd("spawn");

        return cmd;
    }

    static const std::string & moveCommand()
    {
        static std::string cmd("move");

        return cmd;
    }

    static const std::string & eatCommand()
    {
        static std::string cmd("eat");

        return cmd;
    }

    static const std::string & attackCommand()
    {
        static std::string cmd("attack");

        return cmd;
    }

    static const std::string & colorCommand()
    {
        static std::string cmd("color");

        return cmd;
    }

    static const std::string & speciesCommand()
    {
        static std::string cmd("species");

        return cmd;
    }

    static const std::string & turnCommand()
    {
        static std::string cmd("turn");

        return cmd;
    }

    static const std::string & statusCommand()
    {
        static std::string cmd("status");

        return cmd;
    }

    static const std::string & neighboorCommand()
    {
        static std::string cmd("neighboor");

        return cmd;
    }

    static const std::string & foodCommand()
    {
        static std::string cmd("food");

        return cmd;
    }

    static const std::string & sleepCommand()
    {
        static std::string cmd("sleep");

        return cmd;
    }

    static const std::string & wakeupCommand()
    {
        static std::string cmd("wakeup");

        return cmd;
    }

    static const std::string & mateCommand()
    {
        static std::string cmd("mate");

        return cmd;
    }

    static const std::string & winCommand()
    {
        static std::string cmd("win");

        return cmd;
    }

    static const std::string & loseCommand()
    {
        static std::string cmd("lose");

        return cmd;
    }
};
#endif // PROTOCOL_H
