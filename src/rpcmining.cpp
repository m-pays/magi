// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "main.h"
#include "db.h"
#include "txdb.h"
#include "init.h"
#include "magirpc.h"

using namespace json_spirit;
using namespace std;

Value getgenerate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getgenerate\n"
            "Returns true or false.");

    return GetBoolArg("-gen");
}


Value setgenerate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "setgenerate <generate> [genproclimit]\n"
            "<generate> is true or false to turn generation on or off.\n"
            "Generation is limited to [genproclimit] processors, -1 is unlimited.");

    bool fGenerate = true;
    if (params.size() > 0)
        fGenerate = params[0].get_bool();

    if (params.size() > 1)
    {
        int nGenProcLimit = params[1].get_int();
        mapArgs["-genproclimit"] = itostr(nGenProcLimit);
        if (nGenProcLimit == 0)
            fGenerate = false;
    }
    mapArgs["-gen"] = (fGenerate ? "1" : "0");

    assert(pwalletMain != NULL);
    GenerateMagi(fGenerate, pwalletMain);
    return Value::null;
}


Value gethashespersec(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "gethashespersec\n"
            "Returns a recent hashes per second performance measurement while generating.");

    if (GetTimeMillis() - nHPSTimerStart > 8000)
        return (boost::int64_t)0;
    return (boost::int64_t)dHashesPerSec;
}

Value getmininginfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getmininginfo\n"
            "Returns an object containing mining-related information.");
    uint64 nWeight = 0, nMinWeight = 0, nMaxWeight = 0;
    pwalletMain->GetStakeWeight(nMinWeight, nMaxWeight, nWeight);
    int64_t nNetWorkWeit = GetPoSKernelPS();
    uint64 nEstimateTime = 90 * GetPoSKernelPS() / nWeight;
    double bvalue = (IsPoWIIRewardProtocolV2(pindexBest->nTime)) ? 
		  ((double)GetProofOfWorkRewardV2(pindexBest, 0, true))/((double)COIN) : 
		  (double)((uint64_t)(GetProofOfWorkReward(pindexBest->nBits, pindexBest->nHeight, 0)/COIN));
    double rAPR = (IsPoSIIProtocolV2(pindexBest->nHeight+1)) ? 
		  GetAnnualInterestV2(nNetWorkWeit, MAX_MAGI_PROOF_OF_STAKE, pindexBest) : 
		  GetAnnualInterest(nNetWorkWeit, MAX_MAGI_PROOF_OF_STAKE);

    Object obj, diff, blockvalue, weight;
    obj.push_back(Pair("blocks",           (int)nBestHeight));
    obj.push_back(Pair("currentblocksize", (uint64_t)nLastBlockSize));
    obj.push_back(Pair("currentblocktx",   (uint64_t)nLastBlockTx));

    diff.push_back(Pair("proof-of-work",   GetDifficulty()));
    diff.push_back(Pair("proof-of-stake",  GetDifficulty(GetLastBlockIndex(pindexBest, true))));
    diff.push_back(Pair("search-interval", (int)nLastCoinStakeSearchInterval));
    obj.push_back(Pair("difficulty",       diff));

//    obj.push_back(Pair("blockvalue",       (uint64_t)(GetProofOfWorkReward(pindexBest->nBits, pindexBest->nHeight, 0)/COIN)));
    blockvalue.push_back(Pair("difficulty-V2",  GetDifficultyFromBitsV2(pindexBest)));
    blockvalue.push_back(Pair("blockvalue",  bvalue));
    obj.push_back(Pair("blockvalue",       blockvalue));

    obj.push_back(Pair("netmhashps",       GetPoWMHashPS()));
    obj.push_back(Pair("netstakeweight",   GetPoSKernelPS()));
//    obj.push_back(Pair("netstakeweightV2", GetPoSKernelPSV2()));
//    obj.push_back(Pair("netstakeweightV3", GetPoSKernelPSV3()));
    obj.push_back(Pair("errors",           GetWarnings("statusbar")));
    obj.push_back(Pair("pooledtx",         (uint64_t)mempool.size()));

    weight.push_back(Pair("minimum",       (uint64_t)nMinWeight));
    weight.push_back(Pair("maximum",       (uint64_t)nMaxWeight));
    weight.push_back(Pair("combined",      (uint64_t)nWeight));
    obj.push_back(Pair("stakeweight",      weight));
    if (nEstimateTime < 60)
    {
	obj.push_back(Pair("Expected PoS (seconds)", (uint64_t)nEstimateTime));
    }
    else if (nEstimateTime < 60*60)
    {
	obj.push_back(Pair("Expected PoS (minutes)", (uint64_t)nEstimateTime/60));
    }
    else if (nEstimateTime < 24*60*60)
    {
	obj.push_back(Pair("Expected PoS (hours)", (uint64_t)nEstimateTime/(60*60)));
    }
    else
    {
	obj.push_back(Pair("Expected PoS (days)", (uint64_t)nEstimateTime/(60*60*24)));
    }
    obj.push_back(Pair("stakeinterest",    rAPR));
    obj.push_back(Pair("testnet",          fTestNet));

    obj.push_back(Pair("generate",         GetBoolArg("-gen")));
    obj.push_back(Pair("genproclimit",     (int)GetArg("-genproclimit", -1)));
    obj.push_back(Pair("hashespersec",     gethashespersec(params, false)));
    obj.push_back(Pair("networkhashps",    getnetworkhashps(params, false)));
    obj.push_back(Pair("testnet",          fTestNet));
    return obj;
}

// hashrate = diff * 2^32/180 = diff * 4 294 967 296 / 180 (180 - PoW block time)
double GetPoWHashPS(int lookup, int height)
{
    if (pindexBest == NULL)
        return 0;
    if (pindexBest->nHeight >= MAX_MAGI_POW_HEIGHT)
        return 0;

    const CBlockIndex *pindex0 = pindexBest;

    if (height > 0 && height < nBestHeight)
        pindex0 = FindBlockByHeight(height);

    if (pindex0 == NULL || !pindex0->nHeight)
        return 0;

    if (lookup > pindex0->nHeight)
        lookup = pindex0->nHeight;

    const CBlockIndex* pindexPrev = GetLastPoWBlockIndex(pindex0);
    if (pindexPrev == NULL || !pindexPrev->nHeight) return 0;
    const CBlockIndex* pindexPrevPrev = GetLastPoWBlockIndex(pindexPrev->pprev);
    if (pindexPrevPrev == NULL || !pindexPrevPrev->nHeight) return 0;

    int64_t nActualBlockTime = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime(), nActualBlockTimeTot = nActualBlockTime;
    int64_t nBlockWeight = 1;
    if (nActualBlockTime < 0) 
    {
	nActualBlockTimeTot = 0;
	nBlockWeight = 0;
    }

    for(int i = 1; i < lookup; i++)
    {
	pindexPrev = pindexPrevPrev;
	pindexPrevPrev = GetLastPoWBlockIndex(pindexPrev->pprev);
	if (pindexPrevPrev == NULL || !pindexPrevPrev->nHeight) break;
	nActualBlockTime = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
	if (nActualBlockTime >= 0)
	{
	    nActualBlockTimeTot += nActualBlockTime;
	    nBlockWeight++;
	}
    }
    if (nActualBlockTimeTot == 0 || nBlockWeight == 0) return 0;
    if (fDebugMagi)
	printf("@GetPoWHashPS -> diff = %f, block time = %f\n", GetDifficulty(GetLastPoWBlockIndex(pindex0)), (double)nActualBlockTimeTot / (double)nBlockWeight);
    
    return GetDifficulty(GetLastPoWBlockIndex(pindex0)) * pow(2.0, 32) / ((double)nActualBlockTimeTot / (double)nBlockWeight);
}

double GetPoWMHashPS() {
    return GetPoWHashPS(120, -1) / 1.e6;
}

double GetPoWMHashPS_old()
{
    if (pindexBest->nHeight >= MAX_MAGI_POW_HEIGHT)
        return 0;

    int nPoWInterval = 72;
    int64_t nTargetSpacingWorkMin = 30, nTargetSpacingWork = 30;

    CBlockIndex* pindex = pindexGenesisBlock;
    CBlockIndex* pindexPrevWork = pindexGenesisBlock;

    while (pindex)
    {
        if (pindex->IsProofOfWork())
        {
            int64_t nActualSpacingWork = pindex->GetBlockTime() - pindexPrevWork->GetBlockTime();
            nTargetSpacingWork = ((nPoWInterval - 1) * nTargetSpacingWork + nActualSpacingWork + nActualSpacingWork) / (nPoWInterval + 1);
            nTargetSpacingWork = max(nTargetSpacingWork, nTargetSpacingWorkMin);
            pindexPrevWork = pindex;
        }

        pindex = pindex->pnext;
    }

    return GetDifficulty() * 4294.967296 / nTargetSpacingWork;
}

Value GetNetworkHashPS(int lookup, int height) {
    return (boost::int64_t)(GetPoWHashPS(lookup, height));
}

// Litecoin: Return average network hashes per second based on last number of blocks despite PoW or PoS
Value GetNetworkHashPS_old(int lookup) {
    if (pindexBest == NULL)
        return 0;

    // If lookup is -1, then use blocks since last difficulty change.
//    if (lookup <= 0)
//        lookup = pindexBest->nHeight % 2016 + 1;

    // If lookup is larger than chain, then set it to chain length.
    if (lookup > pindexBest->nHeight)
        lookup = pindexBest->nHeight;

    CBlockIndex* pindexPrev = pindexBest;
    for (int i = 0; i < lookup; i++)
        pindexPrev = pindexPrev->pprev;

    double timeDiff = pindexBest->GetBlockTime() - pindexPrev->GetBlockTime();
    double timePerBlock = timeDiff / lookup;

    return (boost::int64_t)(((double)GetDifficulty() * pow(2.0, 32)) / timePerBlock);
}


Value getnetworkhashps(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "getnetworkhashps [blocks] [height]\n"
            "Returns the estimated network PoW hashes per second.\n"
            "Pass in [blocks] to override default number (120) of blocks to average block time.\n"
            "Pass in [height] to estimate the network speed at the time when a certain block was found.\n"
	    "Always return the hash rate of the nearest PoW block.");
    int height = params.size() > 1 ? params[1].get_int() : -1;
    int lookup = params.size() > 0 ? params[0].get_int() : 120;
    if (lookup < 1)
        throw runtime_error(
            "The number of blocks to be averaged must be greater than 1.");
    if (height != -1 && height < 1)
        throw runtime_error(
            "The block height must be greater than 1.");
    return GetNetworkHashPS(lookup, height);
}


Value getnetstakeweight(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "getnetstakeweight [blocks] [height]\n"
            "Returns the estimated net stake weight (PoS).\n"
            "Pass in [blocks] to override default number (72) of blocks for the average.\n"
            "Pass in [height] to estimate the net stake weight at the time when a certain block was found.");
    if (pindexBest == NULL)
	return 0;

    int height = params.size() > 1 ? params[1].get_int() : -1;
    int lookup = params.size() > 0 ? params[0].get_int() : 72;
    if (lookup < 1)
        throw runtime_error(
            "The number of blocks to be averaged must be greater than 1.");
    if (height != -1 && height < 10080)
        throw runtime_error(
            "PoS block height must be greater than 10080.");

    const CBlockIndex *pindex0 = pindexBest;
    if (height > 0 && height < nBestHeight)
	pindex0 = FindBlockByHeight(height);
    if (pindex0 == NULL || !pindex0->nHeight)
	return 0;
    
    if (lookup > pindex0->nHeight)
	lookup = pindex0->nHeight;

    return GetPoSKernelPS(pindex0, lookup);
}


//double GetDifficultyFromBitsV2(const CBlockIndex* pindex0);
double GetDifficultyFromBitsAver(const CBlockIndex* pindex0, int nBlocksAver0);
Value getminingbykhps(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "getminingbykhps [hashrate in kh/s] [blocks]\n"
            "Returns the estimated mining earnings in XMG.\n"
            "Pass in [hashrate] in kh/s, default: 1kh/s.\n"
            "Pass in [blocks] to average difficulty, default: 50.");

    double khps = params.size() > 0 ? params[0].get_real() : 1.;
    int blocks = params.size() > 1 ? params[1].get_int() : 50;

    const CBlockIndex* pblockindex = GetLastBlockIndex(pindexBest, false); // PoW
    double blockvalue = ((double)GetProofOfWorkRewardV2(pblockindex, 0, true))/((double)COIN); 
    double difficultyaver = GetDifficultyFromBitsAver(pblockindex, blocks); 
    double timetofindablock = difficultyaver * pow(2.0, 32) / (khps * 1000.); // in s

    Object obj, mining;
    obj.push_back(Pair("hashrate (kh/s)", khps));
    obj.push_back(Pair("difficulty", GetDifficulty(pblockindex)));
    obj.push_back(Pair("difficulty(aver)", difficultyaver));
    obj.push_back(Pair("blockvalue", blockvalue));
    mining.push_back(Pair("1 hour", 60.*60./timetofindablock*blockvalue));
    mining.push_back(Pair("1 day", 24.*60.*60./timetofindablock*blockvalue));
    mining.push_back(Pair("1 week", 7.*24.*60.*60./timetofindablock*blockvalue));
    obj.push_back(Pair("mining (XMG)", mining));

    return obj;
}

Value getworkex(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "getworkex [data, coinbase]\n"
            "If [data, coinbase] is not specified, returns extended work data.\n"
        );

    if (vNodes.empty())
        throw JSONRPCError(-9, "Magi is not connected!");

    if (IsInitialBlockDownload())
        throw JSONRPCError(-10, "Magi is downloading blocks...");

    typedef map<uint256, pair<CBlock*, CScript> > mapNewBlock_t;
    static mapNewBlock_t mapNewBlock;
    static vector<CBlock*> vNewBlock;
    static CReserveKey reservekey(pwalletMain);

    if (params.size() == 0)
    {
        // Update block
        static unsigned int nTransactionsUpdatedLast;
        static CBlockIndex* pindexPrev;
        static int64 nStart;
        static CBlock* pblock;
        if (pindexPrev != pindexBest ||
            (nTransactionsUpdated != nTransactionsUpdatedLast && GetTime() - nStart > 60))
        {
            if (pindexPrev != pindexBest)
            {
                // Deallocate old blocks since they're obsolete now
                mapNewBlock.clear();
                BOOST_FOREACH(CBlock* pblock, vNewBlock)
                    delete pblock;
                vNewBlock.clear();
            }
            nTransactionsUpdatedLast = nTransactionsUpdated;
            pindexPrev = pindexBest;
            nStart = GetTime();

            // Create new block
            pblock = CreateNewBlock(pwalletMain);
            if (!pblock)
                throw JSONRPCError(-7, "Out of memory");
            vNewBlock.push_back(pblock);
        }

        // Update nTime
        pblock->nTime = max(pindexPrev->GetMedianTimePast()+1, GetAdjustedTime());
        pblock->nNonce = 0;

        // Update nExtraNonce
        static unsigned int nExtraNonce = 0;
        IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

        // Save
        mapNewBlock[pblock->hashMerkleRoot] = make_pair(pblock, pblock->vtx[0].vin[0].scriptSig);

        // Prebuild hash buffers
        char pmidstate[32];
        char pdata[128];
        char phash1[64];
        FormatHashBuffers(pblock, pmidstate, pdata, phash1);

        uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();

        CTransaction coinbaseTx = pblock->vtx[0];
        std::vector<uint256> merkle = pblock->GetMerkleBranch(0);

        Object result;
        result.push_back(Pair("data",     HexStr(BEGIN(pdata), END(pdata))));
        result.push_back(Pair("target",   HexStr(BEGIN(hashTarget), END(hashTarget))));

        CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
        ssTx << coinbaseTx;
        result.push_back(Pair("coinbase", HexStr(ssTx.begin(), ssTx.end())));

        Array merkle_arr;

        BOOST_FOREACH(uint256 merkleh, merkle) {
            merkle_arr.push_back(HexStr(BEGIN(merkleh), END(merkleh)));
        }

        result.push_back(Pair("merkle", merkle_arr));


        return result;
    }
    else
    {
        // Parse parameters
        vector<unsigned char> vchData = ParseHex(params[0].get_str());
        vector<unsigned char> coinbase;

        if(params.size() == 2)
            coinbase = ParseHex(params[1].get_str());

        if (vchData.size() != 128)
            throw JSONRPCError(-8, "Invalid parameter");

        CBlock* pdata = (CBlock*)&vchData[0];

        // Byte reverse
        for (int i = 0; i < 128/4; i++)
            ((unsigned int*)pdata)[i] = ByteReverse(((unsigned int*)pdata)[i]);

        // Get saved block
        if (!mapNewBlock.count(pdata->hashMerkleRoot))
            return false;
        CBlock* pblock = mapNewBlock[pdata->hashMerkleRoot].first;

        pblock->nTime = pdata->nTime;
        pblock->nNonce = pdata->nNonce;

        if(coinbase.size() == 0)
            pblock->vtx[0].vin[0].scriptSig = mapNewBlock[pdata->hashMerkleRoot].second;
        else
            CDataStream(coinbase, SER_NETWORK, PROTOCOL_VERSION) >> pblock->vtx[0]; // FIXME - HACK!

        pblock->hashMerkleRoot = pblock->BuildMerkleTree();

        if (!pblock->SignBlock(*pwalletMain))
            throw JSONRPCError(-100, "Unable to sign block, wallet locked?");

        return CheckWork(pblock, *pwalletMain, reservekey);
    }
}


Value getwork(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getwork [data]\n"
            "If [data] is not specified, returns formatted hash data to work on:\n"
            "  \"midstate\" : precomputed hash state after hashing the first half of the data (DEPRECATED)\n" // deprecated
            "  \"data\" : block data\n"
            "  \"hash1\" : formatted hash buffer for second hash (DEPRECATED)\n" // deprecated
            "  \"target\" : little endian hash target\n"
            "If [data] is specified, tries to solve the block and returns true if it was successful.");

    if (vNodes.empty())
        throw JSONRPCError(RPC_CLIENT_NOT_CONNECTED, "Magi is not connected!");

    if (IsInitialBlockDownload())
        throw JSONRPCError(RPC_CLIENT_IN_INITIAL_DOWNLOAD, "Magi is downloading blocks...");

    typedef map<uint256, pair<CBlock*, CScript> > mapNewBlock_t;
    static mapNewBlock_t mapNewBlock;    // FIXME: thread safety
    static vector<CBlock*> vNewBlock;
    static CReserveKey reservekey(pwalletMain);

    if (params.size() == 0)
    { 
        // Update block
        static unsigned int nTransactionsUpdatedLast;
        static CBlockIndex* pindexPrev;
        static int64 nStart;
        static CBlock* pblock;
        if (pindexPrev != pindexBest ||
            (nTransactionsUpdated != nTransactionsUpdatedLast && GetTime() - nStart > 60))
        {
            if (pindexPrev != pindexBest)
            {
                // Deallocate old blocks since they're obsolete now
                mapNewBlock.clear();
                BOOST_FOREACH(CBlock* pblock, vNewBlock)
                    delete pblock;
                vNewBlock.clear();
            }

            // Clear pindexPrev so future getworks make a new block, despite any failures from here on
            pindexPrev = NULL;

            // Store the pindexBest used before CreateNewBlock, to avoid races
            nTransactionsUpdatedLast = nTransactionsUpdated;
            CBlockIndex* pindexPrevNew = pindexBest;
            nStart = GetTime();

            // Create new block
            pblock = CreateNewBlock(pwalletMain);
            if (!pblock)
                throw JSONRPCError(RPC_OUT_OF_MEMORY, "Out of memory");
            vNewBlock.push_back(pblock);

            // Need to update only after we know CreateNewBlock succeeded
            pindexPrev = pindexPrevNew;
        }

        // Update nTime
        pblock->UpdateTime(pindexPrev);
        pblock->nNonce = 0;

        // Update nExtraNonce
        static unsigned int nExtraNonce = 0;
        IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

        // Save
        mapNewBlock[pblock->hashMerkleRoot] = make_pair(pblock, pblock->vtx[0].vin[0].scriptSig);

        // Pre-build hash buffers
        char pmidstate[32];
        char pdata[128];
        char phash1[64];
        FormatHashBuffers(pblock, pmidstate, pdata, phash1);

        uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();

if (fDebug && fDebugMagi)
{
    std::string cdata = HexStr(BEGIN(pdata), END(pdata));
    std::string chashTarget = HexStr(BEGIN(hashTarget), END(hashTarget));
    printf(">>$ work sent: \n");
    printf("nVersion:       %i\n", pblock->nVersion);
    printf("hashPrevBlock:  0x%s\n", pblock->hashPrevBlock.GetHex().c_str());
    printf("hashMerkleRoot: 0x%s\n", pblock->hashMerkleRoot.GetHex().c_str());
    printf("nTime:          %i\n", pblock->nTime);
    printf("nBits:          %i\n", pblock->nBits);
    printf("nNonce:         %i\n", pblock->nNonce);
//    printf("nPrevMoneySupply: %"PRI64d"\n", pblock->nPrevMoneySupply);
    printf("-----------------\n");
    printf("hashTarget:     0x%s\n", hashTarget.GetHex().c_str());
    printf(">>$ work sent (bytes swapped): \n");
    printf("all:            0x%s\n", cdata.c_str());
    printf("nVersion:       0x%s\n", cdata.substr(0, 8).c_str());
    printf("hashPrevBlock:  0x%s\n", cdata.substr(8, 64).c_str());
    printf("hashMerkleRoot: 0x%s\n", cdata.substr(72, 64).c_str());
    printf("nTime:          0x%s\n", cdata.substr(136, 8).c_str());
    printf("nBits:          0x%s\n", cdata.substr(144, 8).c_str());
    printf("nNonce:         0x%s\n", cdata.substr(152, 8).c_str());
//    printf("nPrevMoneySupply: 0x%s\n", cdata.substr(160, 16).c_str());
    printf("-----------------\n");
    printf("hashTarget:     0x%s\n\n", chashTarget.c_str());
}

        Object result; // HexStr: inverst bytes
        result.push_back(Pair("midstate", HexStr(BEGIN(pmidstate), END(pmidstate)))); // deprecated
        result.push_back(Pair("data",     HexStr(BEGIN(pdata), END(pdata))));
        result.push_back(Pair("hash1",    HexStr(BEGIN(phash1), END(phash1)))); // deprecated
        result.push_back(Pair("target",   HexStr(BEGIN(hashTarget), END(hashTarget))));
        return result;
    }
    else
    {
        // Parse parameters
        vector<unsigned char> vchData = ParseHex(params[0].get_str());
        if (vchData.size() != 128)
	{
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter");
	}
        CBlock* pdata = (CBlock*)&vchData[0];

        // Byte reverse
        for (int i = 0; i < 128/4; i++)
            ((unsigned int*)pdata)[i] = ByteReverse(((unsigned int*)pdata)[i]);
if (fDebug && fDebugMagi)
{
    printf("<<$ work received: \n");
    uint256 hashTarget_rc = CBigNum().SetCompact(pdata->nBits).getuint256();
    printf("nVersion:       %i\n", pdata->nVersion);
    printf("hashPrevBlock:  0x%s\n", pdata->hashPrevBlock.GetHex().c_str());
    printf("hashMerkleRoot: 0x%s\n", pdata->hashMerkleRoot.GetHex().c_str());
    printf("nTime:          %i\n", pdata->nTime);
    printf("nBits:          %i\n", pdata->nBits);
    printf("nNonce:         %i\n", pdata->nNonce);
    printf("-----------------\n");
    printf("hashTarget:     0x%s\n", hashTarget_rc.GetHex().c_str());
    printf("hash:           0x%s\n\n", pdata->GetHash().GetHex().c_str());
}

        // Get saved block
        if (!mapNewBlock.count(pdata->hashMerkleRoot))
            return false;
        CBlock* pblock = mapNewBlock[pdata->hashMerkleRoot].first;

        pblock->nTime = pdata->nTime;
        pblock->nNonce = pdata->nNonce;
        pblock->vtx[0].vin[0].scriptSig = mapNewBlock[pdata->hashMerkleRoot].second;
        pblock->hashMerkleRoot = pblock->BuildMerkleTree();


        if (!pblock->SignBlock(*pwalletMain))
	{
            throw JSONRPCError(-100, "Unable to sign block, wallet locked?");
	}

        return CheckWork(pblock, *pwalletMain, reservekey);
    }
}


Value getblocktemplate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getblocktemplate [params]\n"
            "Returns data needed to construct a block to work on:\n"
            "  \"version\" : block version\n"
            "  \"previousblockhash\" : hash of current highest block\n"
            "  \"transactions\" : contents of non-coinbase transactions that should be included in the next block\n"
            "  \"coinbaseaux\" : data that should be included in coinbase\n"
            "  \"coinbasevalue\" : maximum allowable input to coinbase transaction, including the generation award and transaction fees\n"
            "  \"target\" : hash target\n"
            "  \"mintime\" : minimum timestamp appropriate for next block\n"
            "  \"curtime\" : current timestamp\n"
            "  \"mutable\" : list of ways the block template may be changed\n"
            "  \"noncerange\" : range of valid nonces\n"
            "  \"sigoplimit\" : limit of sigops in blocks\n"
            "  \"sizelimit\" : limit of block size\n"
            "  \"bits\" : compressed target of next block\n"
            "  \"height\" : height of the next block\n"
            "See https://en.bitcoin.it/wiki/BIP_0022 for full specification.");

    std::string strMode = "template";
    if (params.size() > 0)
    {
        const Object& oparam = params[0].get_obj();
        const Value& modeval = find_value(oparam, "mode");
        if (modeval.type() == str_type)
            strMode = modeval.get_str();
        else if (modeval.type() == null_type)
        {
            /* Do nothing */
        }
        else
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid mode");
    }

    if (strMode != "template")
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid mode");

    if (vNodes.empty())
        throw JSONRPCError(RPC_CLIENT_NOT_CONNECTED, "Magi is not connected!");

    if (IsInitialBlockDownload())
        throw JSONRPCError(RPC_CLIENT_IN_INITIAL_DOWNLOAD, "Magi is downloading blocks...");

    static CReserveKey reservekey(pwalletMain);

    // Update block
    static unsigned int nTransactionsUpdatedLast;
    static CBlockIndex* pindexPrev;
    static int64 nStart;
    static CBlock* pblock;
    if (pindexPrev != pindexBest ||
        (nTransactionsUpdated != nTransactionsUpdatedLast && GetTime() - nStart > 5))
    {
        // Clear pindexPrev so future calls make a new block, despite any failures from here on
        pindexPrev = NULL;

        // Store the pindexBest used before CreateNewBlock, to avoid races
        nTransactionsUpdatedLast = nTransactionsUpdated;
        CBlockIndex* pindexPrevNew = pindexBest;
        nStart = GetTime();

        // Create new block
        if(pblock)
        {
            delete pblock;
            pblock = NULL;
        }
        pblock = CreateNewBlock(pwalletMain);
        if (!pblock)
            throw JSONRPCError(RPC_OUT_OF_MEMORY, "Out of memory");

        // Need to update only after we know CreateNewBlock succeeded
        pindexPrev = pindexPrevNew;
    }

    // Update nTime
    pblock->UpdateTime(pindexPrev);
    pblock->nNonce = 0;

    Array transactions;
    map<uint256, int64_t> setTxIndex;
    int i = 0;
    CTxDB txdb("r");
    BOOST_FOREACH (CTransaction& tx, pblock->vtx)
    {
        uint256 txHash = tx.GetHash();
        setTxIndex[txHash] = i++;

        if (tx.IsCoinBase() || tx.IsCoinStake())
            continue;

        Object entry;

        CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
        ssTx << tx;
        entry.push_back(Pair("data", HexStr(ssTx.begin(), ssTx.end())));

        entry.push_back(Pair("hash", txHash.GetHex()));

        MapPrevTx mapInputs;
        map<uint256, CTxIndex> mapUnused;
        bool fInvalid = false;
        if (tx.FetchInputs(txdb, mapUnused, false, false, mapInputs, fInvalid))
        {
            entry.push_back(Pair("fee", (int64_t)(tx.GetValueIn(mapInputs) - tx.GetValueOut())));

            Array deps;
            BOOST_FOREACH (MapPrevTx::value_type& inp, mapInputs)
            {
                if (setTxIndex.count(inp.first))
                    deps.push_back(setTxIndex[inp.first]);
            }
            entry.push_back(Pair("depends", deps));

            int64_t nSigOps = tx.GetLegacySigOpCount();
            nSigOps += tx.GetP2SHSigOpCount(mapInputs);
            entry.push_back(Pair("sigops", nSigOps));
        }

        transactions.push_back(entry);
    }

    Object aux;
    aux.push_back(Pair("flags", HexStr(COINBASE_FLAGS.begin(), COINBASE_FLAGS.end())));

    uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();

    static Array aMutable;
    if (aMutable.empty())
    {
        aMutable.push_back("time");
        aMutable.push_back("transactions");
        aMutable.push_back("prevblock");
    }

    Object result;
    result.push_back(Pair("version", pblock->nVersion));
    result.push_back(Pair("previousblockhash", pblock->hashPrevBlock.GetHex()));
    result.push_back(Pair("transactions", transactions));
    result.push_back(Pair("coinbaseaux", aux));
    result.push_back(Pair("coinbasevalue", (int64_t)pblock->vtx[0].vout[0].nValue));
    result.push_back(Pair("target", hashTarget.GetHex()));
    result.push_back(Pair("mintime", (int64_t)pindexPrev->GetMedianTimePast()+1));
    result.push_back(Pair("mutable", aMutable));
    result.push_back(Pair("noncerange", "00000000ffffffff"));
    result.push_back(Pair("sigoplimit", (int64_t)MAX_BLOCK_SIGOPS));
    result.push_back(Pair("sizelimit", (int64_t)MAX_BLOCK_SIZE));
    result.push_back(Pair("curtime", (int64_t)pblock->nTime));
    result.push_back(Pair("bits", HexBits(pblock->nBits)));
    result.push_back(Pair("height", (int64_t)(pindexPrev->nHeight+1)));

    return result;
}

Value submitblock(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "submitblock <hex data> [optional-params-obj]\n"
            "[optional-params-obj] parameter is currently ignored.\n"
            "Attempts to submit new block to network.\n"
            "See https://en.bitcoin.it/wiki/BIP_0022 for full specification.");

    vector<unsigned char> blockData(ParseHex(params[0].get_str()));
    CDataStream ssBlock(blockData, SER_NETWORK, PROTOCOL_VERSION);
    CBlock block;
    try {
        ssBlock >> block;
    }
    catch (std::exception &e) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Block decode failed");
    }

    if (!block.SignBlock(*pwalletMain))
        throw JSONRPCError(-100, "Unable to sign block, wallet locked?");

    bool fAccepted = ProcessBlock(NULL, &block);
    if (!fAccepted)
        return "rejected";

    return Value::null;
}

