#include <iostream>
#include "CoinTossing.h"

using namespace std;


int main(int argc, char* argv[])
{

    CoinTossingParty party(argc, argv);
    party.run();

//    BCastClique bCastClique(argc, argv);
//    bCastClique.run();

    return 0;
}