//
// Created by moriya on 04/01/17.
//

#include "CoinTossing.h"

using namespace std;

CoinTossing::CoinTossing(int argc, char* argv []):
        Protocol("CoinTossing", argc, argv)
{
    vector<string> names{"TestComm"};
    m_measure = new Measurement(*this, names);

    m_partyId = stoi(this->getParser().getValueByKey(arguments, "partyID"));
    m_numberOfParties = stoi(this->getParser().getValueByKey(arguments, "partiesNumber"));
    m_partiesFilePath = this->getParser().getValueByKey(arguments, "partiesFile");
    m_d = stoi(this->getParser().getValueByKey(arguments, "D"));
    setCommunication();

}

int CoinTossing::getID() { return m_partyId; }

void CoinTossing::setCommunication()
{
    //open file
    ConfigFile cf(m_partiesFilePath);

    string portString, ipString;
    vector<int> ports(m_numberOfParties);
    vector<string> ips(m_numberOfParties);

    int counter = 0;

    for (int i = 0; i < m_numberOfParties; i++)
    {
        portString = "party_" + to_string(i) + "_port";
        ipString = "party_" + to_string(i) + "_ip";

        //get partys IPs and ports data
        ports[i] = stoi(cf.Value("", portString));
        ips[i] = cf.Value("", ipString);
    }

    SocketPartyData me, other;

    for (int i=0; i<m_numberOfParties; i++)
    {
        if (i < m_partyId) {// This party will be the receiver in the protocol

            me = SocketPartyData(boost_ip::address::from_string(ips[m_partyId]), ports[m_partyId] + i);
            cout<<"my port = "<<ports[m_partyId] + i<<endl;
            other = SocketPartyData(boost_ip::address::from_string(ips[i]), ports[i] + m_partyId - 1);
            cout<<"other port = "<<ports[i] + m_partyId - 1<<endl;

            shared_ptr<CommParty> channel = make_shared<CommPartyTCPSynced>(m_ioService, me, other);
            // connect to party one
            channel->join(500, 5000);
            cout<<"channel established"<<endl;

            m_channels.emplace_back(make_shared<ProtocolPartyData>(i, channel));
        }

        // This party will be the sender in the protocol
        else if (i>m_partyId)
        {
            me = SocketPartyData(boost_ip::address::from_string(ips[m_partyId]), ports[m_partyId] + i - 1);
            cout<<"my port = "<<ports[m_partyId] + i - 1<<endl;
            other = SocketPartyData(boost_ip::address::from_string(ips[i]), ports[i] + m_partyId);
            cout<<"other port = "<< ports[i] + m_partyId<<endl;

            shared_ptr<CommParty> channel = make_shared<CommPartyTCPSynced>(m_ioService, me, other);
            // connect to party one
            channel->join(500, 5000);
            cout<<"channel established"<<endl;

            m_channels.emplace_back(make_shared<ProtocolPartyData>(i, channel));
        }
    }
}

void CoinTossingParty::createData()
{
    random_bytes_engine rbe;
    m_data.resize(m_d);
    generate(begin(m_data), end(m_data), ref(rbe));
}

void CoinTossingParty::run()
{
    if (m_partyId == 0)
    {
        vector<byte*> data(m_numberOfParties - 1);
        m_measure->startSubTask("TestComm",1);
        for (int idx = 0; idx < m_channels.size(); ++idx)
        {
            int dataSize =  m_d;
            data[idx] = new byte[dataSize];
            m_channels[idx].get()->getChannel().get()->read(data[idx], dataSize);

            cout << "DATA SENT: " << endl;
            for (int i = 0; i < m_d; i++)
            {
                cout << "Party " << this->getID() << " receives " << (int)data[idx][i] << " from party " << idx + 1 << endl;

            }
        }
        m_measure->endSubTask("TestComm",1);
    }

    else
    {
        m_channels[0].get()->getChannel().get()->write(m_data.data(), m_data.size());
    }
}

void BCastClique::run()
{
    int numberOfRounds = (int)log2(m_numberOfParties);
    cout << "Number of rounds: " << numberOfRounds << endl;
    int data[2];
    data[0] = 41;
    data[1] = 42;
    for (int phaseIdx = 0; phaseIdx < numberOfRounds; ++phaseIdx)
    {
        for (int partyIdx = 0; partyIdx < m_numberOfParties; ++partyIdx)
        {
            byte dataForRead[2];
            if (m_partyId < partyIdx)
            {
                int channelId1 = m_channels[partyIdx + (phaseIdx % m_numberOfParties)].get()->getID();
                cout << "Party id : " << channelId1 << " using channel with party id for w/r : "
                     << partyIdx + phaseIdx % m_numberOfParties <<endl;
                m_channels[partyIdx + (phaseIdx % m_numberOfParties)].get()->getChannel().get()->write((byte*)data,2*
                                                                                                                   sizeof(int));
                cout << "Party id : " << partyIdx + phaseIdx % m_numberOfParties << " using channel with party id for r/w : "
                     << channelId1 <<endl;
                m_channels[partyIdx + (phaseIdx % m_numberOfParties)].get()->getChannel().get()->read(dataForRead,2*
                                                                                                                  sizeof(int));
            }

            else if (m_partyId > partyIdx)
            {
                int channelId1 = m_channels[partyIdx + (phaseIdx % m_numberOfParties)].get()->getID();
                cout << "Party id : " << channelId1 << " using channel with party id for r/w : "
                     << partyIdx + phaseIdx % m_numberOfParties <<endl;
                m_channels[partyIdx + (phaseIdx % m_numberOfParties)].get()->getChannel().get()->read(dataForRead,2*
                                                                                                                  sizeof(int));
                cout << "Party id : " << partyIdx + phaseIdx % m_numberOfParties << " using channel with party id for w/r : "
                     << channelId1 <<endl;
                m_channels[partyIdx + (phaseIdx % m_numberOfParties)].get()->getChannel().get()->write((byte*)data,2*
                                                                                                                   sizeof(int));
            }
        }
        break;
    }
}