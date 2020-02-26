//
// Created by hama on 20. 2. 23..
//

#ifndef HYPERLEDGER_FABRIC_400_CPP_FABRIC_H
#define HYPERLEDGER_FABRIC_400_CPP_FABRIC_H

#endif //HYPERLEDGER_FABRIC_400_CPP_FABRIC_H
#include <ctime>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <condition_variable>
#include <thread>

#include "concurrent_queue.h"
using namespace std;

//================================ TRANACTION
struct Transaction {

    Transaction(string _auth, string _key,string _value):client_msp(_auth),
                                                        key(_key),
                                                        value(_value){}

    string client_msp;
    string key;
    string value;
};

struct _Transaction {
    _Transaction(string _key,string _value):key(_key),value(_value){}
    string key;
    string value;
};

//================================= RWSet
struct RWSet {
    RWSet(string key, string value, string msp):key(key),value(value),msp(msp) {}
    string msp;
    string key;
    string value;
    vector<string> peers_msp;
};

//=================================== LEVEL DB
struct LevelDB {
    unordered_map<string,string> db;
    string getValue(string key);
    void setValue(string key, string value);

    std::mutex                        _vMtx;
};


//=================================== BLOCK
struct Block  {
    vector<string> endorsers;
    vector<_Transaction> Trans;
};

struct _Block  {
    int Index;
    long int  Timestamp;
    vector<_Transaction> Trans;
    string Hash;
    string PrevHash;
};

//==================================== LEDGER
struct Ledger  {

    Ledger(){
        db = std::make_shared<LevelDB>();
    }

    vector<_Block> blockchain;
    shared_ptr<LevelDB> db;
    std::mutex _vMtx;
    time_t timeCur;

    void createGenesisBlock();
    void addBlock(Block block);
    _Block generateBlock(_Block oldBlock , Block block );
    void  setState(_Transaction trans );
    string getState(Transaction trans );
    string calculateHash(_Block block);
};

//==================================  Fabric-CA & MSP
struct MSP  {
    string pubKey;
    string priKey;
    string id;

    bool validating(string _id){
        return id == _id;
    }
};

//================================== PEER
class Fabric;
class Peer  {
public:
    Peer(int _peer_type, MSP _msp, Fabric*  _fabric, shared_ptr<Ledger> _ledger){
        peer_type = _peer_type;
        msp = _msp;
        fabric = _fabric;
        ledger = _ledger;
    }
    ~Peer(){}

private:
    int peer_type; // 0 commit only peer, 1 endorse + commit peer
    shared_ptr<Ledger> ledger;
    Fabric* fabric;
    MSP msp;

    std::thread _endorser;
    std::thread _committer;

    list<Transaction> _transactionList;
    list<RWSet> _rwsetList;
    list<Block> _blockList;

    std::mutex _transMtx;
    std::condition_variable _transCond;
    std::mutex _rwsetMtx;
    std::condition_variable _rwsetCond;
    std::mutex _blockMtx;
    std::condition_variable _blockCond;

    bool _stop;
public:
    void start();
    RWSet addTrans(Transaction trans);
    void addBlock(Block block);
    void endorsing();
    void committing();

    bool validating(Block block);
    string getData(string key);
};


//================================== Orderer
class Fabric;
class Kafaka;
class Orderer  {
public:
    Orderer(MSP _msp, shared_ptr<Kafaka> _kafka, vector<shared_ptr<Peer>> _committer, Fabric* _fabric);
    ~Orderer();

private:
    std::thread _producer;
    std::thread _consumer;

    MSP msp;
    list<RWSet> _rwsetList;
    std::mutex _rwsetMtx;
    std::condition_variable _rwsetCond;
    vector<shared_ptr<Peer>> committer;
    Fabric* fabric;
    shared_ptr<Kafaka> kafka;

    bool _stop;
public:
    void start();
    void addRWSet(RWSet rwset);
    void producer();
    void consumer();

    void addCommitter(shared_ptr<Peer> peer);
    Block createBlock(vector<RWSet> _rwsets);
};

//==================================  KAFKA
struct Kafaka  {

    hama::ConcurrentQueue<RWSet> channel;
    void push(RWSet rwset);
    bool pull(vector<RWSet> & rwsets);

};

//==================================  FABRIC  =================================//
class Fabric {

private:
    shared_ptr<Kafaka>   kafka;
    shared_ptr<Orderer>  orderer1;
    shared_ptr<Orderer>  orderer2;
    shared_ptr<Peer> endorser1;
    shared_ptr<Peer> endorser2;
    shared_ptr<Peer> committer;

    bool roundrobin;

public:
    string MSP_org1;
    string MSP_peer1;
    string MSP_peer2;
    string MSP_peer3;
    string MSP_orderer1;
    string MSP_orderer2;

public:

    void start();
    void stop();
    std::tuple<RWSet, RWSet> writeTranaction(string key , string value, string auth);
    string readTranaction( string key , string auth );
    void sendToOrderer(RWSet rwset);

};