// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "main.h"
#include "magirpc.h"
#include "checkpoints.h"

using namespace json_spirit;
using namespace std;

extern void TxToJSON(const CTransaction& tx, const uint256 hashBlock, json_spirit::Object& entry);

double GetDifficulty(const CBlockIndex* blockindex)
{
    // Floating point number that is a multiple of the minimum difficulty,
    // minimum difficulty = 1.0.
    if (blockindex == NULL)
    {
        if (pindexBest == NULL)
            return 1.0;
        else
            blockindex = GetLastBlockIndex(pindexBest, false);	// PoW
    }

    int nShift = (blockindex->nBits >> 24) & 0xff;

    double dDiff =
        (double)0x0000ffff / (double)(blockindex->nBits & 0x00ffffff);

    while (nShift < 29)
    {
        dDiff *= 256.0;
        nShift++;
    }
    while (nShift > 29)
    {
        dDiff /= 256.0;
        nShift--;
    }

    return dDiff;
}


double GetPoSKernelPS(const CBlockIndex* blockindex, int lookup)
{
    int nPoSInterval = lookup;
    double dStakeKernelsTriedAvg = 0;
    int nStakesHandled = 0, nStakesTime = 0;

    const CBlockIndex* pindex = ((blockindex == NULL) ? GetLastBlockIndex(pindexBest, true) : blockindex);
    const CBlockIndex* pindexPrevStake = NULL;

    while (pindex && nStakesHandled < nPoSInterval)
    {
        if (pindex->IsProofOfStake())
        {
            dStakeKernelsTriedAvg += GetDifficulty(pindex) * 4294967296.0;
            nStakesTime += pindexPrevStake ? (pindexPrevStake->nTime - pindex->nTime) : 0;
            pindexPrevStake = pindex;
            nStakesHandled++;
        }

        pindex = pindex->pprev;
    }
    if (fDebugMagi)
	printf("@GetPoSKernelPS -> stake blocks for average = %d\n", nStakesHandled);

    return nStakesTime ? dStakeKernelsTriedAvg / nStakesTime : 0;
}

// hashrate = diff * 2^32/blocktime = diff * 4 294 967 296 / blocktime
double GetPoSKernelPSV2(const CBlockIndex* blockindex, int lookup)
{
    int nPoSInterval = lookup;
    double diffTot = 0.;

    const CBlockIndex* pindex0 = ((blockindex == NULL) ? GetLastBlockIndex(pindexBest, true) : blockindex);
    const CBlockIndex* pindexPrev = GetLastPoSBlockIndex(pindex0);
    if (pindexPrev == NULL || !pindexPrev->nHeight) return 0;
    const CBlockIndex* pindexPrevPrev = GetLastPoSBlockIndex(pindexPrev->pprev);
    if (pindexPrevPrev == NULL || !pindexPrevPrev->nHeight) return 0;

    int nActualBlockTime = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime(), nActualBlockTimeTot = nActualBlockTime;
    int nStakesHandled = 1;
//    if (nActualBlockTime <= 0) {
//	nActualBlockTimeTot = 0;
//	nStakesHandled = 0;
//    }
//    else {
      diffTot = GetDifficulty(pindexPrev);
//    }
    for(int i = 1; i < nPoSInterval; i++)
    {
	pindexPrev = pindexPrevPrev;
	pindexPrevPrev = GetLastPoSBlockIndex(pindexPrev->pprev);
	if (pindexPrevPrev == NULL || !pindexPrevPrev->nHeight) break;
	nActualBlockTime = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
//	if (nActualBlockTime > 0)
//	{
	    diffTot += GetDifficulty(pindexPrev);
	    nActualBlockTimeTot += nActualBlockTime;
	    nStakesHandled++;
//	}
    }
    if (nActualBlockTimeTot == 0 || nStakesHandled == 0) return 0;
    if (fDebugMagi)
	printf("@GetPoSKernelPSV2 -> aver diff = %f, block time = %f\n", diffTot / (double)nStakesHandled, (double)nActualBlockTimeTot / (double)nStakesHandled);

    return diffTot*4294967296.0/double(nActualBlockTimeTot);
}

double GetPoSKernelPSV3(const CBlockIndex* blockindex)
{
    int nPoSInterval = 72;
    double dStakeKernelsTriedAvg = 0., diff = 0.;

    const CBlockIndex* pindex0 = ((blockindex == NULL) ? GetLastBlockIndex(pindexBest, true) : blockindex);
    const CBlockIndex* pindexPrev = GetLastPoSBlockIndex(pindex0);
    if (pindexPrev == NULL || !pindexPrev->nHeight) return 0;
    const CBlockIndex* pindexPrevPrev = GetLastPoSBlockIndex(pindexPrev->pprev);
    if (pindexPrevPrev == NULL || !pindexPrevPrev->nHeight) return 0;

    int nActualBlockTime = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime(), nActualBlockTimeTot = nActualBlockTime;
    int nStakesHandled = 1;
    if (nActualBlockTime <= 0) {
	nActualBlockTimeTot = 0;
	nStakesHandled = 0;
    }
    else {
      diff = GetDifficulty(pindexPrev);
      dStakeKernelsTriedAvg = GetDifficulty(pindexPrev) * 4294967296.0 / double(nActualBlockTime);
    }
    for(int i = 1; i < nPoSInterval; i++)
    {
	pindexPrev = pindexPrevPrev;
	pindexPrevPrev = GetLastPoSBlockIndex(pindexPrev->pprev);
	if (pindexPrevPrev == NULL || !pindexPrevPrev->nHeight) break;
	nActualBlockTime = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
	if (nActualBlockTime > 0)
	{
	    diff += GetDifficulty(pindexPrev);
	    dStakeKernelsTriedAvg += GetDifficulty(pindexPrev) * 4294967296.0 / double(nActualBlockTime);
	    nActualBlockTimeTot += nActualBlockTime;
	    nStakesHandled++;
	}
    }
    if (nActualBlockTimeTot == 0 || nStakesHandled == 0) return 0;
    if (fDebugMagi)
	printf("@GetPoSKernelPSV2 -> aver diff = %f, block time = %f\n", diff / (double)nStakesHandled, (double)nActualBlockTimeTot / (double)nStakesHandled);

    return dStakeKernelsTriedAvg / double(nStakesHandled);
}

Object blockToJSON(const CBlock& block, const CBlockIndex* blockindex, bool fPrintTransactionDetail)
{
    Object result;
    result.push_back(Pair("hash", block.GetHash().GetHex()));
    CMerkleTx txGen(block.vtx[0]);
    txGen.SetMerkleBranch(&block);
    result.push_back(Pair("confirmations", (int)txGen.GetDepthInMainChain()));
    result.push_back(Pair("size", (int)::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION)));
    result.push_back(Pair("height", blockindex->nHeight));
    result.push_back(Pair("version", block.nVersion));
    result.push_back(Pair("merkleroot", block.hashMerkleRoot.GetHex()));
    result.push_back(Pair("mint", ValueFromAmount(blockindex->nMint)));
    result.push_back(Pair("time", (boost::int64_t)block.GetBlockTime()));
    result.push_back(Pair("nonce", (boost::uint64_t)block.nNonce));
    result.push_back(Pair("bits", HexBits(block.nBits)));
    result.push_back(Pair("difficulty", GetDifficulty(blockindex)));

    if (blockindex->pprev)
        result.push_back(Pair("previousblockhash", blockindex->pprev->GetBlockHash().GetHex()));
    if (blockindex->pnext)
        result.push_back(Pair("nextblockhash", blockindex->pnext->GetBlockHash().GetHex()));

    result.push_back(Pair("flags", strprintf("%s%s", blockindex->IsProofOfStake()? "proof-of-stake" : "proof-of-work", blockindex->GeneratedStakeModifier()? " stake-modifier": "")));
    result.push_back(Pair("proofhash", blockindex->IsProofOfStake()? blockindex->hashProofOfStake.GetHex() : blockindex->GetBlockHash().GetHex()));
    result.push_back(Pair("entropybit", (int)blockindex->GetStakeEntropyBit()));
    result.push_back(Pair("modifier", strprintf("%016" PRI64x, blockindex->nStakeModifier)));
    result.push_back(Pair("modifierchecksum", strprintf("%08x", blockindex->nStakeModifierChecksum)));
    Array txinfo;
    BOOST_FOREACH (const CTransaction& tx, block.vtx)
    {
        if (fPrintTransactionDetail)
        {
            Object entry;

            entry.push_back(Pair("txid", tx.GetHash().GetHex()));
            TxToJSON(tx, 0, entry);

            txinfo.push_back(entry);
        }
        else
            txinfo.push_back(tx.GetHash().GetHex());
    }

    result.push_back(Pair("tx", txinfo));
    result.push_back(Pair("signature", HexStr(block.vchBlockSig.begin(), block.vchBlockSig.end())));

    return result;
}


Value getblockcount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getblockcount\n"
            "Returns the number of blocks in the longest block chain.");

    return nBestHeight;
}


Value getdifficulty(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getdifficulty\n"
            "Returns the difficulty as a multiple of the minimum difficulty.");

    Object obj;
    obj.push_back(Pair("proof-of-work",        GetDifficulty()));
    obj.push_back(Pair("proof-of-stake",       GetDifficulty(GetLastBlockIndex(pindexBest, true))));
    obj.push_back(Pair("search-interval",      (int)nLastCoinStakeSearchInterval));
    return obj;
}


Value getdifficultym(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getdifficultym\n"
            "Returns the proof-of-work difficulty as a multiple of the minimum difficulty.");

    return GetDifficulty();
}

Value settxfee(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 1 || AmountFromValue(params[0]) < MIN_TX_FEE)
        throw runtime_error(
            "settxfee <amount>\n"
            "<amount> is a real and is rounded to the nearest 0.01");

    nTransactionFee = AmountFromValue(params[0]);
    nTransactionFee = (nTransactionFee / CENT) * CENT;  // round to cent

    return true;
}

Value getrawmempool(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getrawmempool\n"
            "Returns all transaction ids in memory pool.");

    vector<uint256> vtxid;
    mempool.queryHashes(vtxid);

    Array a;
    BOOST_FOREACH(const uint256& hash, vtxid)
        a.push_back(hash.ToString());

    return a;
}

Value getblockhash(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getblockhash <index>\n"
            "Returns hash of block in best-block-chain at <index>.");

    int nHeight = params[0].get_int();
    if (nHeight < 0 || nHeight > nBestHeight)
        throw runtime_error("Block number out of range.");

    CBlockIndex* pblockindex = FindBlockByHeight(nHeight);
    return pblockindex->phashBlock->GetHex();
}

Value getblock(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getblock <hash> [txinfo]\n"
            "txinfo optional to print more detailed tx info\n"
            "Returns details of a block with given block-hash.");

    std::string strHash = params[0].get_str();
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    block.ReadFromDisk(pblockindex, true);

    return blockToJSON(block, pblockindex, params.size() > 1 ? params[1].get_bool() : false);
}

Value getblockbynumber(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getblock <number> [txinfo]\n"
            "txinfo optional to print more detailed tx info\n"
            "Returns details of a block with given block-number.");

    int nHeight = params[0].get_int();
    if (nHeight < 0 || nHeight > nBestHeight)
        throw runtime_error("Block number out of range.");

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hashBestChain];
    while (pblockindex->nHeight > nHeight)
        pblockindex = pblockindex->pprev;

    uint256 hash = *pblockindex->phashBlock;

    pblockindex = mapBlockIndex[hash];
    block.ReadFromDisk(pblockindex, true);

    return blockToJSON(block, pblockindex, params.size() > 1 ? params[1].get_bool() : false);
}

unsigned int MagiQuantumWave(const CBlockIndex* pindexLast, bool fProofOfStake);
unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake);
Value getdebuginfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getdebuginfobyheight <height>\n");

    int nHeight = params[0].get_int();
    if (nHeight < 1 || nHeight > nBestHeight)
        throw runtime_error("Block number out of range.");

    Object obj;

    const CBlockIndex* pblockindex = FindBlockByHeight(nHeight);
    obj.push_back(Pair("flags", strprintf("%s", pblockindex->IsProofOfStake()? "proof-of-stake" : "proof-of-work")));
    obj.push_back(Pair("height", pblockindex->nHeight));
    obj.push_back(Pair("hash", pblockindex->phashBlock->GetHex()));

    const CBlockIndex* pblockindexprev = GetLastBlockIndex(pblockindex->pprev, pblockindex->IsProofOfStake());
    obj.push_back(Pair("hashPrev", pblockindexprev->phashBlock->GetHex()));
    obj.push_back(Pair("difficulty", GetDifficulty(pblockindex)));

    int bnBitsMQW = MagiQuantumWave(pblockindexprev, pblockindexprev->IsProofOfStake());
    int bnBitsTarget = GetNextTargetRequired(pblockindexprev, pblockindexprev->IsProofOfStake());

    obj.push_back(Pair("nBits", HexBits(pblockindex->nBits)));
    obj.push_back(Pair("nBitsMQW", HexBits(bnBitsMQW)));
    obj.push_back(Pair("nBitsTarget", HexBits(bnBitsTarget)));

    obj.push_back(Pair("diff", GetDifficultyFromBits(pblockindex->nBits)));
    obj.push_back(Pair("diffMQW", GetDifficultyFromBits(bnBitsMQW)));
    obj.push_back(Pair("diffTarget", GetDifficultyFromBits(bnBitsTarget)));

    CBlockIndex* pcheckpoint = Checkpoints::GetLastSyncCheckpoint();
    int64 deltaTime = pblockindex->GetBlockTime() - pcheckpoint->nTime;

    CBigNum bnNewBlock;
    bnNewBlock.SetCompact(pblockindex->nBits);
    CBigNum bnRequired;
    unsigned int nRequired;

    const CBlockIndex* pcheckpointLast;
    if (pblockindex->IsProofOfStake()) {
        bnRequired.SetCompact(ComputeMinStake(GetLastBlockIndex(pcheckpoint, true)->nBits, deltaTime, pblockindex->nTime));
        nRequired = ComputeMinStake(GetLastBlockIndex(pcheckpoint, true)->nBits, deltaTime, pblockindex->nTime);
        pcheckpointLast = GetLastBlockIndex(pcheckpoint, true);
    } else {
        bnRequired.SetCompact(ComputeMinWork(GetLastBlockIndex(pcheckpoint, false)->nBits, deltaTime));
        nRequired = ComputeMinWork(GetLastBlockIndex(pcheckpoint, false)->nBits, deltaTime);
        pcheckpointLast = GetLastBlockIndex(pcheckpoint, false);
    }

    obj.push_back(Pair("checkpoint-height", pcheckpoint->nHeight));
    obj.push_back(Pair("deltaTime", (int) deltaTime));
    obj.push_back(Pair("pcheckpointLast-height", pcheckpointLast->nHeight));

    obj.push_back(Pair("bnNewBlock", HexBits(pblockindex->nBits)));
    obj.push_back(Pair("bnRequired", HexBits(nRequired)));

    return obj;
}

int64 GetProofOfWorkRewardV2(const CBlockIndex* pindexPrev, int64 nFees, bool fLastBlock);
//double GetDifficultyFromBitsV2(const CBlockIndex* pindex0);
Value getnewblockvaluebynumber(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getnewblockvaluebynumber <number>\n"
            "Returns a block reward with given block-number.");

    int nHeight = params[0].get_int();
    if (nHeight < 1 || nHeight > nBestHeight)
        throw runtime_error("Block number out of range.");

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hashBestChain];
    while (pblockindex->nHeight > nHeight)
        pblockindex = pblockindex->pprev;
    Object obj;
    obj.push_back(Pair("flags", strprintf("%s", pblockindex->IsProofOfStake()? "proof-of-stake" : "proof-of-work")));
    if (pblockindex->IsProofOfStake()) {
	obj.push_back(Pair("difficulty", GetDifficulty(pblockindex)));
	obj.push_back(Pair("difficulty for Blockvalue (PoW)", GetDifficultyFromBitsV2(pblockindex)));
	obj.push_back(Pair("blockvalue (PoW)", ((double)GetProofOfWorkRewardV2(pblockindex, 0, false))/((double)COIN)));
    }
    else {
	obj.push_back(Pair("difficulty", GetDifficulty(pblockindex)));
	obj.push_back(Pair("difficulty for Blockvalue", GetDifficultyFromBitsV2(pblockindex)));
	obj.push_back(Pair("blockvalue", ((double)GetProofOfWorkRewardV2(pblockindex, 0, false))/((double)COIN)));
    }
    return obj;
}

Value getautocheckpoint(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getautocheckpoint\n"
            "Show info of auto synchronized checkpoint.\n");

    Object result;
    const CBlockIndex* pindexCheckpoint = Checkpoints::AutoSelectSyncCheckpoint();

    result.push_back(Pair("synccheckpoint", pindexCheckpoint->GetBlockHash().ToString().c_str()));
    result.push_back(Pair("height", pindexCheckpoint->nHeight));
    result.push_back(Pair("timestamp", DateTimeStrFormat(pindexCheckpoint->GetBlockTime()).c_str()));

    return result;
}

Value getsynccheckpoint(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getsynccheckpoint\n"
            "Show info of synchronized checkpoint.\n");

    Object result;
    result.push_back(Pair("checkpoint", Checkpoints::hashSyncCheckpoint.ToString().c_str()));
    if (mapBlockIndex.count(Checkpoints::hashSyncCheckpoint))
    {
        CBlockIndex* pindexCheckpoint = mapBlockIndex[Checkpoints::hashSyncCheckpoint];
        result.push_back(Pair("height", pindexCheckpoint->nHeight));
        result.push_back(Pair("timestamp", DateTimeStrFormat(pindexCheckpoint->GetBlockTime()).c_str()));
    }

    if (mapArgs.count("-checkpointkey"))
        result.push_back(Pair("checkpointmaster", true));

    return result;
}

Value sendcheckpoint(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "sendcheckpoint <blockhash>\n"
            "Send a synchronized checkpoint.\n");

    if (!mapArgs.count("-checkpointkey") || CSyncCheckpoint::strMasterPrivKey.empty())
        throw runtime_error("Not a checkpointmaster node, first set checkpointkey in configuration and restart client. ");

    std::string strHash = params[0].get_str();
    uint256 hash(strHash);

    if (!Checkpoints::SendSyncCheckpoint(hash))
        throw runtime_error("Failed to send checkpoint, check log. ");

    Object result;
    CBlockIndex* pindexCheckpoint;

    result.push_back(Pair("synccheckpoint", Checkpoints::hashSyncCheckpoint.ToString().c_str()));
    if (mapBlockIndex.count(Checkpoints::hashSyncCheckpoint))
    {
        pindexCheckpoint = mapBlockIndex[Checkpoints::hashSyncCheckpoint];
        result.push_back(Pair("height", pindexCheckpoint->nHeight));
        result.push_back(Pair("timestamp", (boost::int64_t) pindexCheckpoint->GetBlockTime()));
    }
    result.push_back(Pair("subscribemode", Checkpoints::IsSyncCheckpointEnforced()? "enforce" : "advisory"));
    if (mapArgs.count("-checkpointkey"))
        result.push_back(Pair("checkpointmaster", true));

    return result;
}

Value enforcecheckpoint(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "enforcecheckpoint <enforce>\n"
            "<enforce> is true or false to enable or disable enforcement of broadcasted checkpoints by developer.");

    bool fEnforceCheckpoint = params[0].get_bool();
    if (mapArgs.count("-checkpointkey") && !fEnforceCheckpoint)
        throw runtime_error(
            "checkpoint master node must enforce synchronized checkpoints.");
    if (fEnforceCheckpoint)
        Checkpoints::strCheckpointWarning = "";
    mapArgs["-checkpointenforce"] = (fEnforceCheckpoint ? "1" : "0");
    return Value::null;
}
