//
// Created by hama on 20. 2. 23..
//
#include "fabric.h"
#include <memory>
#include <string>
#include "sha256.h"
#include "string_format.h"

//LevelDB
string LevelDB::getValue(string key){

    std::lock_guard<std::mutex> lock(_vMtx);
    return db[key];
}

void LevelDB::setValue(string key, string value){

    std::lock_guard<std::mutex> lock(_vMtx);
    db[key] = value;
};

// LEDGER
void Ledger::createGenesisBlock(){
    time( &timeCur );
    shared_ptr<_Block>  genesisBlock = std::make_shared<_Block>();
    genesisBlock->Index = 0;
    genesisBlock->Timestamp = 0;
    genesisBlock->Hash = calculateHash(genesisBlock);
    genesisBlock->PrevHash = "";

    //std::lock_guard<std::mutex> lock(_vMtx);
    blockchain.push_back(genesisBlock);
}

void Ledger::addBlock(Block block){
    //std::lock_guard<std::mutex> lock(_vMtx);
    shared_ptr<_Block> prevBlock = blockchain.back();
    shared_ptr<_Block> newBlock = generateBlock(prevBlock, block);
    blockchain.push_back(newBlock);
}
shared_ptr<_Block> Ledger::generateBlock(shared_ptr<_Block> oldBlock , Block block ){
    auto newBlock = std::make_shared<_Block>();
    time( &timeCur );
    newBlock->Index = oldBlock->Index + 1;
    newBlock->Timestamp = timeCur;
    newBlock->PrevHash = oldBlock->Hash;
    newBlock->Hash = calculateHash(newBlock);
    return newBlock;
}
void Ledger::setState(_Transaction trans ){
    db->setValue(trans.key, trans.value);
}

string Ledger::getState(Transaction trans ){
    return db->getValue(trans.key);
}

string Ledger::calculateHash(shared_ptr<_Block> block) {

//    string trans_concated;
//    for (_Transaction trans : block.Trans ) {
//        trans_concated.append(trans.value);
//    }
    //string record = hama::string_format("%d%lf%s%s\n", block.Index, block.Timestamp, trans_concated, block.PrevHash);
    string record = "afefwfwef2323f2332f23f23wfwefweref23423423sdvsvsr423423432432432f23f23f23432234322f";
    string hashed = sha256(record);
    return hashed;
}


// PEER
void Peer::start(){

    ledger->createGenesisBlock();

    if (peer_type == 1) {
        _endorser = std::thread([&]() { endorsing(); });
    }
    _committer = std::thread([&]() { committing(); });
}


void Peer::endorsing(){

    std::unique_lock<std::mutex> translock(_transMtx, std::defer_lock);
    std::unique_lock<std::mutex> rwlock(_rwsetMtx , std::defer_lock);

    while (!_stop) {
        translock.lock();

        if (_transactionList.empty()) {
            _transCond.wait(translock);
        }

        if (_transactionList.empty()) {
            translock.unlock();
            continue;
        }

        Transaction trans = _transactionList.front();
        translock.unlock();

        if (trans.client_msp == fabric->MSP_org1) {
            //
            //execute chain code !!!
            //
            RWSet rwset(trans.key,trans.value,msp.id, {});

            rwlock.lock();
            _rwsetList.push_back(rwset);
            rwlock.unlock();
            _rwsetCond.notify_all();

        }

        translock.lock();
        _transactionList.pop_front();
        translock.unlock();

    }
}
void Peer::committing(){

    std::unique_lock<std::mutex> rlock(_blockMtx, std::defer_lock);

    while (!_stop) {
        rlock.lock();

        while (_blockList.empty()) {
            _blockCond.wait(rlock);
        }

        Block block = _blockList.front();
        _blockList.pop_front();


        bool ok = validating(block);
        if (ok == false) {
            continue;
        }

        for (_Transaction trans : block.Trans) {
            ledger->setState(trans);
        }

        ledger->addBlock(block);
        rlock.unlock();
    }

}

RWSet Peer::addTrans(Transaction trans){
    std::unique_lock<std::mutex> translock(_transMtx, std::defer_lock);
    translock.lock();
    _transactionList.push_back(trans);
    translock.unlock();
    _transCond.notify_all();

    std::unique_lock<std::mutex> rwlock(_rwsetMtx );
    while (_rwsetList.empty()) {
        _rwsetCond.wait(rwlock);

    }
    RWSet rwset = _rwsetList.front();
    _rwsetList.pop_front();
    return rwset;
}

void Peer::addBlock(Block block){
    std::unique_lock<std::mutex> blocklock(_blockMtx, std::defer_lock);
    blocklock.lock();
    _blockList.push_back(block);
    blocklock.unlock();

    _blockCond.notify_all();
}

bool Peer::validating(Block block){
    if (block.endorsers[0] == fabric->MSP_peer1 &&
        block.endorsers[1] == fabric->MSP_peer2) {
        return true;
    }

    return false;
}
string Peer::getData(string key){
    return ledger->db->getValue(key);
}


Orderer::Orderer(MSP _msp, shared_ptr<Kafaka> _kafka, vector<shared_ptr<Peer>> _committer, Fabric* _fabric){
    msp = _msp;
    fabric = _fabric;
    committer = _committer;
}
Orderer::~Orderer(){

}

void Orderer::addCommitter(shared_ptr<Peer> peer){
    committer.push_back(peer);
}

void Orderer::start(){

  //  _producer = std::thread([&]() { producer(); });
 //   _consumer = std::thread([&]() { consumer(); });

}

void Orderer::addRWSet(RWSet rwset){
    //_rwsetList.push(rwset);
    vector<RWSet> rwsets;
    rwsets.push_back(rwset);
    auto newBlock = createBlock(rwsets);

    for(shared_ptr<Peer> peer : committer){
        peer->addBlock(newBlock);
    }
}

void Orderer::producer(){
    while (!_stop){
        kafka->push(_rwsetList.pop());
    }
}

void Orderer::consumer(){

    while (!_stop) {
       vector<RWSet> rwsets;
       RWSet rwset = _rwsetList.pop();
       rwsets.push_back(rwset);

       auto newBlock = createBlock(rwsets);

       for(shared_ptr<Peer> peer : committer){
           peer->addBlock(newBlock);
       }
    }

}

Block Orderer::createBlock(vector<RWSet> _rwsets){

    Block newBlock;
    for (RWSet rwset : _rwsets){
        _Transaction _trans(rwset.key, rwset.value);
        newBlock.Trans.push_back(_trans);
        newBlock.endorsers.push_back(rwset.peers_msp[0]);
        newBlock.endorsers.push_back(rwset.peers_msp[1]);
    }
    return newBlock;
}

void Kafaka::push(RWSet rwset) {
    channel.push(rwset);
}

vector<RWSet> Kafaka::pull(){

    RWSet rwset1 = channel.pop();
    RWSet rwset2 = channel.pop();
    RWSet rwset3 = channel.pop();

    vector<RWSet> rwsets;
    rwsets.push_back(rwset1);
    rwsets.push_back(rwset2);
    rwsets.push_back(rwset3);

    return rwsets;
}

// FABRIC
void Fabric::start(){

    // 1. three peer simulator start (two endorsing peer, one committing only peer)
    shared_ptr<Ledger> ledger1 = std::make_shared<Ledger>();
    shared_ptr<Ledger> ledger2 = std::make_shared<Ledger>();
    shared_ptr<Ledger> ledger3 = std::make_shared<Ledger>();


    MSP_org1 = "org1";
    MSP_peer1 = "peer1";
    MSP_peer2 = "peer2";
    MSP_peer3 = "peer3";
    MSP_orderer1 = "orderer1";
    MSP_orderer2 = "orderer2";


    MSP msp_peer1;
    msp_peer1.id = MSP_peer1;

    endorser1 = std::make_shared<Peer>(1, msp_peer1, this, ledger1);
    endorser1->start();

    MSP msp_peer2;
    msp_peer2.id = MSP_peer2;

    endorser2 = std::make_shared<Peer>(1, msp_peer2, this, ledger2);
    endorser2->start();

    MSP msp_peer3;
    msp_peer3.id = MSP_peer3;

    committer = std::make_shared<Peer>(0, msp_peer3, this, ledger3);
    committer->start();

    // 2. kafka simulator start
    kafka = std::make_shared<Kafaka>();

    // 3. two orderer simulator start (first is input, second is ordering)
    MSP msp_orderer1;
    msp_orderer1.id = MSP_orderer1;

    vector<shared_ptr<Peer>> _committer;
    _committer.push_back(endorser1);
    _committer.push_back(endorser2);
    _committer.push_back(committer);

    orderer1 = std::make_shared<Orderer>(msp_orderer1, kafka, _committer, this);
    orderer1->start();
}

void Fabric::stop(){

}

std::tuple<RWSet, RWSet> Fabric::writeTransaction(string key , string value, string auth){

    Transaction t (auth,key,value);

    RWSet rwset1 = endorser1->addTrans(t);
    RWSet rwset2 = endorser2->addTrans(t);

    return std::tie(rwset1,rwset2);
}

string Fabric::readTranaction( string key , string auth ){
    return committer->getData(key);
}

void Fabric::sendToOrderer(RWSet rwset){
//    if (roundrobin) {
//        orderer1->addRWSet(rwset);
//        roundrobin = false;
//    } else {
//        orderer2->addRWSet(rwset);
//        roundrobin = true;
//    }

    orderer1->addRWSet(rwset);
}

