//
// Created by lior on 26/03/18.
//

#ifndef MPCCOMMUNICATION_H
#define MPCCOMMUNICATION_H

#include <gmp.h>
#include <gmpxx.h>
#include <random>
#include <climits>
#include <algorithm>
#include <functional>
#include <libscapi/include/comm/Comm.hpp>
#include <libscapi/include/infra/ConfigFile.hpp>
#include <libscapi/include/infra/Measurement.hpp>
#include <libscapi/include/cryptoInfra/SecurityLevel.hpp>
#include <libscapi/include/cryptoInfra/Protocol.hpp>

using namespace std;
using namespace boost::asio;
using random_bytes_engine = std::independent_bits_engine<
        std::default_random_engine, CHAR_BIT, unsigned char>;


class ProtocolPartyData {
private:
    int id;
    shared_ptr<CommParty> channel;  // Channel between this party to every other party in the protocol.

public:
    ProtocolPartyData() {}
    ProtocolPartyData(int id, shared_ptr<CommParty> channel)
            : id (id), channel(channel){
    }

    int getID() { return id; }
    shared_ptr<CommParty> getChannel() { return channel; }
};

class CoinTossing :public Protocol, public HonestMajority, public MultiParty
{

public:
    CoinTossing(int argc, char* argv []);
    ~CoinTossing()
    {
        m_ioService.stop();
        delete m_measure;
    }
    bool hasOnline() override { return false; }
    bool hasOffline() override { return false; }

    void setCommunication();
    int m_partyId;


protected:
    Measurement *m_measure;
    int m_numberOfParties;
    string m_partiesFilePath;
    int m_d;
    io_service  m_ioService;
    vector<shared_ptr<ProtocolPartyData>> m_channels;
    vector<byte> m_data;

    virtual void createData() = 0;
};


//OpoF = One Point of Failure
class CoinTossingParty: public CoinTossing
{
public:
    CoinTossingParty(int argc, char* argv []): CoinTossing(argc, argv){ /*m_data = new byte[m_d];*/ createData();}
    void run() override;
    virtual void createData();
};


class BCastClique: public CoinTossing
{
public:
    BCastClique(int argc, char* argv []): CoinTossing(argc, argv){}
    void run() override;
    virtual void createData(){}
};

#endif //MPCCOMMUNICATION_H
