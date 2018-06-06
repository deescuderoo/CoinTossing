//
// Created by moriya on 04/01/17.
// Edited by Eduardo and Daniel on 07/06/18
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
    iotape.resize(m_numberOfParties);
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
        if (i != m_partyId) {// This party will be the receiver in the protocol

            me = SocketPartyData(boost_ip::address::from_string(ips[m_partyId]), ports[m_partyId] + i);
            cout<<"my port = "<<ports[m_partyId] + i<<endl;
            other = SocketPartyData(boost_ip::address::from_string(ips[i]), ports[i] + m_partyId);
            cout<<"other port = "<<ports[i] + m_partyId<<endl;

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

void CoinTossingParty::broadcastExchange(vector<data256>& buffer)
{
    assert (buffer.size() == m_numberOfParties);

    for (int idx = 0; idx < m_numberOfParties; idx++)
    {
        if (idx < m_partyId) // other (idx) sends to me (m_partyId)
        {
            m_channels[idx].get()->getChannel().get()->read(buffer[idx].bytes, 32);
            m_channels[idx].get()->getChannel().get()->write(buffer[m_partyId].bytes, 32);

        }
        else if (idx > m_partyId) // I (m_partyId) send to other (idx)
        {
            m_channels[idx-1].get()->getChannel().get()->write(buffer[m_partyId].bytes, 32);
            m_channels[idx-1].get()->getChannel().get()->read(buffer[idx].bytes, 32);
        }
    }
}

void CoinTossingParty::run()
{

    iotape[m_partyId] = std::move(data256(string("Hello world from party ") + to_string(m_partyId)));

    broadcastExchange(iotape);

    cout << "Contents of party " << m_partyId << ": " << endl;
    for (int i = 0; i < m_numberOfParties; i++){
    cout << iotape[i].bytes << ", ";
    }
}
