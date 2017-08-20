// Copyright (c) 2012-2013 The PPCoin developers
// Copyright (c) 2014 The Magi developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp>

#include "kernel.h"
#include "txdb.h"

using namespace std;

extern int nStakeMaxAge;
extern int nStakeTargetSpacing;

// Modifier interval: time to elapse before new modifier is computed
// Set to 3-hour for production network and 20-minute for test network

unsigned int nModifierInterval = MODIFIER_INTERVAL;

typedef std::map<int, unsigned int> MapModifierCheckpoints;

// Hard checkpoints of stake modifiers to ensure they are deterministic
static std::map<int, unsigned int> mapStakeModifierCheckpoints =
    boost::assign::map_list_of
    ( 0,	0xfd11f4e7 )
    ( 9,	0x4fdba6a6 )
    ( 19,	0x8300e57b )
    ( 99,	0xb74d1791 )
    ( 199,	0x52ae43ca )
    ( 999,	0x47fecb89 )
    ( 1999,	0x256f6e94 )
    ( 9999,	0x66bb24af )
    ( 19999,	0xadc5749e )
    ( 29999,	0x839a4815 )
    ( 37090,	0x5e04a01a )
    ( 49999,	0x0d209374 )
    ( 69999,	0xa73d2057 )
    ( 89999,	0x53fd30d8 )
    ( 109999,	0xd82859e7 )
    ( 123838,	0xfb9d85a6 )
    ( 200000,	0x3d7cdf21 )
    ( 220000,	0x8c80b8d4 )
    ( 240000,	0x94656c86 )
    ( 260000,	0x9e61afaa )
    ( 280000,	0x10eb79c1 )
    ( 300000,	0x7f737a59 )
    ( 310000,	0x514d70d5 )
    ( 313600,	0x89c907ae )
    ( 320000,	0xdb078b96 )
    ( 350000,	0x66f13150 )
    ( 380000,	0xc9114f5a )
    ( 400000,	0xe6ac39b0 )
    ( 420000,	0x656256d6 )
    ( 450000,	0xdaf7197a )
    ( 465000,	0x7ba85aa4 )
    ( 480330,   0x8c5c6d17 )
    ( 1420000,   0xb6c936a9 )
    ( 1425000,   0x4961debb )
    ( 1430000,   0xd9d792b0 )
    ( 1435000,   0xfea1dbf1 )
    ( 1440000,   0x8b307a8a )
    ( 1445000,   0xd125289a )
    ( 1446000,   0x1cf74c5c )
    ( 1446770,   0x48d6b19f )
    ( 1447500,   0x386c0d85 )
    ( 1448292,   0xb92fa526 )
;

// Hard checkpoints of stake modifiers to ensure they are deterministic (TestNet)
static std::map<int, unsigned int> mapStakeModifierCheckpointsTestNet =
    boost::assign::map_list_of
        ( 0,	0x0e00670b )
        ( 999,	0x727489b2 )
        ( 1999,	0x654dcb7e )
        ( 9999,	0x2c9333e0 )
        ( 19999,0xf40a3252 )
        ( 27680,0xe7957d36 )
    ;

inline double wfa(double x)
{
    return (1 / (1 + exp_n( (x-0.03)/0.005 ))) + 1;
}

inline double wfb(double x)
{
    return (1 / (1 + exp_n( (x-0.06)/0.06 ))) + 1;
}

inline double wfc(double x)
{
    return (1 / (1 + exp_n( (x-0.6)/0.3 )));
}

inline double wfaV2(double x)
{
    return (1 / (1 + exp_n( (x-0.045)/0.0075 ))) + 1;
}

inline double wfbV2(double x)
{
    return (1 / (1 + exp_n( (x-0.08)/0.08 ))) + 1;
}

inline double wfcV2(double x)
{
    return (1 / (1 + exp_n( (x-0.25)/0.125 )));
}

// Get time weight
int64 GetMagiWeight_TestNet(int64 nValueIn, int64 nIntervalBeginning, int64 nIntervalEnd)
{
    double nWeight = 0;
    int64 nnMoneySupply = MAX_MONEY_STAKE_REF;

    if (nValueIn >= MAX_MONEY_STAKE_REF) return 0;
    
    double rStakeDays = (double)(max((int64)0, nIntervalEnd - nIntervalBeginning - GetStakeMinAge(nIntervalEnd))) / (24. * 60. * 60.);
    double rMro = (double)(nValueIn*6)/(double)nnMoneySupply, rEpf = exp_n(1/wfa(rMro)/wfb(rMro)/wfc(rMro));
    nWeight = 5.55243 * ( pow(rEpf, -0.3 * rStakeDays * 480. / 8.177) - pow(rEpf, -0.6 * rStakeDays * 480. / 8.177) ) * rStakeDays * 240.;

    return max((int64)0, min((int64)(nWeight * 24 * 60 * 60), (int64)nStakeMaxAge));
}

int64 GetMagiWeight_TestNetV2(int64 nValueIn, int64 nIntervalBeginning, int64 nIntervalEnd)
{
    double nWeight = 0;
    int64 nnMoneySupply = MAX_MONEY_STAKE_REF;

    if (nValueIn >= MAX_MONEY_STAKE_REF) return 0;
    
    double rStakeDays = (double)(max((int64)0, nIntervalEnd - nIntervalBeginning - GetStakeMinAge(nIntervalEnd))) / (24. * 60. * 60.);
    double rMro = (double)(nValueIn*6)/(double)nnMoneySupply, rEpf = exp_n(1/wfa(rMro)/wfb(rMro)/wfc(rMro));
    nWeight = 5.55243 * ( pow(rEpf, -0.3 * rStakeDays * 480. / 8.177) - pow(rEpf, -0.6 * rStakeDays * 480. / 8.177) ) * rStakeDays * 240.;

    if (fDebugMagiPoS) printf("@GetMagiWeight_TestNetV2 = %"PRI64d"\n", max((int64)0, min((int64)(nWeight * 24 * 60 * 60/2), (int64)(nStakeMaxAge))));

    return max((int64)0, min((int64)(nWeight * 24 * 60 * 60/2), (int64)(nStakeMaxAge)));
}

// Get time weight
int64 GetMagiWeight(int64 nValueIn, int64 nIntervalBeginning, int64 nIntervalEnd)
{
    double nWeight = 0;
    int64 nnMoneySupply = MAX_MONEY_STAKE_REF;

    if (nValueIn >= MAX_MONEY_STAKE_REF) return 0;
    
    double rStakeDays = (double)(max((int64)0, nIntervalEnd - nIntervalBeginning - GetStakeMinAge(nIntervalEnd))) / (24. * 60. * 60.);
    double rMro = (double)(nValueIn*6)/(double)nnMoneySupply, rEpf = exp_n(1/wfa(rMro)/wfb(rMro)/wfc(rMro));

    if (rMro/6 >= MAX_MAGI_BALANCE_in_STAKE) return 0;

    if (fTestNet) return GetMagiWeight_TestNet(nValueIn, nIntervalBeginning, nIntervalEnd);
    
    nWeight = 5.55243 * ( pow(rEpf, -0.3 * rStakeDays * 4. / 8.177) - pow(rEpf, -0.6 * rStakeDays * 4. / 8.177) ) * rStakeDays;

    return max((int64)0, min((int64)(nWeight * 24 * 60 * 60), (int64)nStakeMaxAge));
}

// Get time weight
int64 GetMagiWeightV2(int64 nValueIn, int64 nIntervalBeginning, int64 nIntervalEnd)
{
    double nWeight = 0;
    int64 nnMoneySupply = MAX_MONEY_STAKE_REF_V2;

    if (nValueIn >= MAX_MONEY_STAKE_REF_V2) return 0;
    
    double rStakeDays = (double)(max((int64)0, nIntervalEnd - nIntervalBeginning - GetStakeMinAge(nIntervalEnd))) / (24. * 60. * 60.);
    double rMro = (double)(nValueIn*6)/(double)nnMoneySupply, rEpf = exp_n(1/wfaV2(rMro)/wfbV2(rMro)/wfcV2(rMro));

    if (rMro/6 >= MAX_MAGI_BALANCE_in_STAKE) return 0;

    if (fTestNet & !fTestNetWeightV2) return GetMagiWeight_TestNet(nValueIn, nIntervalBeginning, nIntervalEnd);
    
    nWeight = 42.2474 * ( pow(rEpf, -0.55 * (rStakeDays+2.) / 0.4719) - pow(rEpf, -0.6 * (rStakeDays+2.) / 0.4719) ) * rStakeDays;

    if (fDebugMagiPoS) printf("@GetMagiWeightV2 = %"PRI64d"\n", max((int64)0, min((int64)(nWeight * 24 * 60 * 60), (int64)nStakeMaxAge)));

    return max((int64)0, min((int64)(nWeight * 24 * 60 * 60), (int64)nStakeMaxAge));
}

// Get time weight
int64 GetWeight(int64 nIntervalBeginning, int64 nIntervalEnd)
{
    // Kernel hash weight starts from 0 at the min age
    // this change increases active coins participating the hash and helps
    // to secure the network when proof-of-stake difficulty is low

    return min(nIntervalEnd - nIntervalBeginning - GetStakeMinAge(nIntervalEnd), (int64)nStakeMaxAge);
}

// Get the last stake modifier and its generation time from a given block
static bool GetLastStakeModifier(const CBlockIndex* pindex, uint64& nStakeModifier, int64& nModifierTime)
{
    if (!pindex)
        return error("GetLastStakeModifier: null pindex");
    while (pindex && pindex->pprev && !pindex->GeneratedStakeModifier())
        pindex = pindex->pprev;
    if (!pindex->GeneratedStakeModifier())
        return error("GetLastStakeModifier: no generation at genesis block");
    nStakeModifier = pindex->nStakeModifier;
    nModifierTime = pindex->GetBlockTime();
    return true;
}

// Get selection interval section (in seconds)
static int64 GetStakeModifierSelectionIntervalSection(int nSection)
{
    assert (nSection >= 0 && nSection < 64);
    int64 a = nModifierInterval * 63 / (63 + ((63 - nSection) * (MODIFIER_INTERVAL_RATIO - 1)));
	return a;
}

// Get stake modifier selection interval (in seconds)
static int64 GetStakeModifierSelectionInterval()
{
    int64 nSelectionInterval = 0;
    for (int nSection=0; nSection<64; nSection++)
	{
        nSelectionInterval += GetStakeModifierSelectionIntervalSection(nSection);
	}
    return nSelectionInterval;
}

// select a block from the candidate blocks in vSortedByTimestamp, excluding
// already selected blocks in vSelectedBlocks, and with timestamp up to
// nSelectionIntervalStop.
static bool SelectBlockFromCandidates(
    vector<pair<int64, uint256> >& vSortedByTimestamp,
    map<uint256, const CBlockIndex*>& mapSelectedBlocks,
    int64 nSelectionIntervalStop, uint64 nStakeModifierPrev,
    const CBlockIndex** pindexSelected)
{
    bool fSelected = false;
    uint256 hashBest = 0;
    *pindexSelected = (const CBlockIndex*) 0;
    BOOST_FOREACH(const PAIRTYPE(int64, uint256)& item, vSortedByTimestamp)
    {
        if (!mapBlockIndex.count(item.second))
            return error("SelectBlockFromCandidates: failed to find block index for candidate block %s", item.second.ToString().c_str());
        const CBlockIndex* pindex = mapBlockIndex[item.second];
        if (fSelected && pindex->GetBlockTime() > nSelectionIntervalStop)
            break;
        if (mapSelectedBlocks.count(pindex->GetBlockHash()) > 0)
            continue;
        // compute the selection hash by hashing its proof-hash and the
        // previous proof-of-stake modifier
        uint256 hashProof = pindex->IsProofOfStake()? pindex->hashProofOfStake : pindex->GetBlockHash();
        CDataStream ss(SER_GETHASH, 0);
        ss << hashProof << nStakeModifierPrev;
        uint256 hashSelection = Hash(ss.begin(), ss.end());
        // the selection hash is divided by 2**32 so that proof-of-stake block
        // is always favored over proof-of-work block. this is to preserve
        // the energy efficiency property
        if (pindex->IsProofOfStake())
            hashSelection >>= 32;
        if (fSelected && hashSelection < hashBest)
        {
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*) pindex;
        }
        else if (!fSelected)
        {
            fSelected = true;
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*) pindex;
        }
    }
    if (fDebug && GetBoolArg("-printstakemodifier"))
        printf("SelectBlockFromCandidates: selection hash=%s\n", hashBest.ToString().c_str());
    return fSelected;
}

// Stake Modifier (hash modifier of proof-of-stake):
// The purpose of stake modifier is to prevent a txout (coin) owner from
// computing future proof-of-stake generated by this txout at the time
// of transaction confirmation. To meet kernel protocol, the txout
// must hash with a future stake modifier to generate the proof.
// Stake modifier consists of bits each of which is contributed from a
// selected block of a given block group in the past.
// The selection of a block is based on a hash of the block's proof-hash and
// the previous stake modifier.
// Stake modifier is recomputed at a fixed time interval instead of every
// block. This is to make it difficult for an attacker to gain control of
// additional bits in the stake modifier, even after generating a chain of
// blocks.
bool ComputeNextStakeModifier(const CBlockIndex* pindexPrev, uint64& nStakeModifier, bool& fGeneratedStakeModifier)
{
    nStakeModifier = 0;
    fGeneratedStakeModifier = false;
    if (!pindexPrev)
    {
        fGeneratedStakeModifier = true;
        return true;  // genesis block's modifier is 0
    }
    // First find current stake modifier and its generation block time
    // if it's not old enough, return the same stake modifier
    int64 nModifierTime = 0;
    if (!GetLastStakeModifier(pindexPrev, nStakeModifier, nModifierTime))
        return error("ComputeNextStakeModifier: unable to get last modifier");
    if (fDebug)
    {
        printf("ComputeNextStakeModifier: prev modifier=0x%016"PRI64x" time=%s\n", nStakeModifier, DateTimeStrFormat(nModifierTime).c_str());
    }
    if (nModifierTime / nModifierInterval >= pindexPrev->GetBlockTime() / nModifierInterval)
        return true;

    // Sort candidate blocks by timestamp
    vector<pair<int64, uint256> > vSortedByTimestamp;
    vSortedByTimestamp.reserve(64 * nModifierInterval / nStakeTargetSpacing);
    int64 nSelectionInterval = GetStakeModifierSelectionInterval();
    int64 nSelectionIntervalStart = (pindexPrev->GetBlockTime() / nModifierInterval) * nModifierInterval - nSelectionInterval;
    const CBlockIndex* pindex = pindexPrev;
    while (pindex && pindex->GetBlockTime() >= nSelectionIntervalStart)
    {
        vSortedByTimestamp.push_back(make_pair(pindex->GetBlockTime(), pindex->GetBlockHash()));
        pindex = pindex->pprev;
    }
    int nHeightFirstCandidate = pindex ? (pindex->nHeight + 1) : 0;
    reverse(vSortedByTimestamp.begin(), vSortedByTimestamp.end());
    sort(vSortedByTimestamp.begin(), vSortedByTimestamp.end());

    // Select 64 blocks from candidate blocks to generate stake modifier
    uint64 nStakeModifierNew = 0;
    int64 nSelectionIntervalStop = nSelectionIntervalStart;
    map<uint256, const CBlockIndex*> mapSelectedBlocks;
    for (int nRound=0; nRound<min(64, (int)vSortedByTimestamp.size()); nRound++)
    {
        // add an interval section to the current selection round
        nSelectionIntervalStop += GetStakeModifierSelectionIntervalSection(nRound);
        // select a block from the candidates of current round
        if (!SelectBlockFromCandidates(vSortedByTimestamp, mapSelectedBlocks, nSelectionIntervalStop, nStakeModifier, &pindex))
            return error("ComputeNextStakeModifier: unable to select block at round %d", nRound);
        // write the entropy bit of the selected block
        nStakeModifierNew |= (((uint64)pindex->GetStakeEntropyBit()) << nRound);
        // add the selected block from candidates to selected list
        mapSelectedBlocks.insert(make_pair(pindex->GetBlockHash(), pindex));
        if (fDebug && GetBoolArg("-printstakemodifier"))
            printf("ComputeNextStakeModifier: selected round %d stop=%s height=%d bit=%d\n",
                nRound, DateTimeStrFormat(nSelectionIntervalStop).c_str(), pindex->nHeight, pindex->GetStakeEntropyBit());
    }

    // Print selection map for visualization of the selected blocks
    if (fDebug && GetBoolArg("-printstakemodifier"))
    {
        string strSelectionMap = "";
        // '-' indicates proof-of-work blocks not selected
        strSelectionMap.insert(0, pindexPrev->nHeight - nHeightFirstCandidate + 1, '-');
        pindex = pindexPrev;
        while (pindex && pindex->nHeight >= nHeightFirstCandidate)
        {
            // '=' indicates proof-of-stake blocks not selected
            if (pindex->IsProofOfStake())
                strSelectionMap.replace(pindex->nHeight - nHeightFirstCandidate, 1, "=");
            pindex = pindex->pprev;
        }
        BOOST_FOREACH(const PAIRTYPE(uint256, const CBlockIndex*)& item, mapSelectedBlocks)
        {
            // 'S' indicates selected proof-of-stake blocks
            // 'W' indicates selected proof-of-work blocks
            strSelectionMap.replace(item.second->nHeight - nHeightFirstCandidate, 1, item.second->IsProofOfStake()? "S" : "W");
        }
        printf("ComputeNextStakeModifier: selection height [%d, %d] map %s\n", nHeightFirstCandidate, pindexPrev->nHeight, strSelectionMap.c_str());
    }
    if (fDebug)
    {
        printf("ComputeNextStakeModifier: new modifier=0x%016"PRI64x" time=%s\n", nStakeModifierNew, DateTimeStrFormat(pindexPrev->GetBlockTime()).c_str());
    }

    nStakeModifier = nStakeModifierNew;
    fGeneratedStakeModifier = true;
    return true;
}

// The stake modifier used to hash for a stake kernel is chosen as the stake
// modifier about a selection interval later than the coin generating the kernel
static bool GetKernelStakeModifier(uint256 hashBlockFrom, uint64& nStakeModifier, int& nStakeModifierHeight, int64& nStakeModifierTime, bool fPrintProofOfStake)
{
    nStakeModifier = 0;
    if (!mapBlockIndex.count(hashBlockFrom))
        return error("GetKernelStakeModifier() : block not indexed");
    const CBlockIndex* pindexFrom = mapBlockIndex[hashBlockFrom];
    nStakeModifierHeight = pindexFrom->nHeight;
    nStakeModifierTime = pindexFrom->GetBlockTime();
    int64 nStakeModifierSelectionInterval = GetStakeModifierSelectionInterval();
    const CBlockIndex* pindex = pindexFrom;

    // loop to find the stake modifier later by a selection interval
    while (nStakeModifierTime < pindexFrom->GetBlockTime() + nStakeModifierSelectionInterval)
    {
        if (!pindex->pnext)
        {   // reached best block; may happen if node is behind on block chain
            if (fPrintProofOfStake || (pindex->GetBlockTime() + GetStakeMinAge(pindexFrom->GetBlockTime()) - nStakeModifierSelectionInterval > GetAdjustedTime()))
                return error("GetKernelStakeModifier() : reached best block %s at height %d from block %s",
                    pindex->GetBlockHash().ToString().c_str(), pindex->nHeight, hashBlockFrom.ToString().c_str());
            else
			{
			if(fDebug)
				 printf(">> nStakeModifierTime = %"PRI64d", pindexFrom->GetBlockTime() = %"PRI64d", nStakeModifierSelectionInterval = %"PRI64d"\n",
				 	nStakeModifierTime, pindexFrom->GetBlockTime(), nStakeModifierSelectionInterval);
                return false;
			}
        }
        pindex = pindex->pnext;
        if (pindex->GeneratedStakeModifier())
        {
            nStakeModifierHeight = pindex->nHeight;
            nStakeModifierTime = pindex->GetBlockTime();
        }
    }
    nStakeModifier = pindex->nStakeModifier;
    return true;
}

// ppcoin kernel protocol
// coinstake must meet hash target according to the protocol:
// kernel (input 0) must meet the formula
//     hash(nStakeModifier + txPrev.block.nTime + txPrev.offset + txPrev.nTime + txPrev.vout.n + nTime) < bnTarget * nCoinDayWeight
// this ensures that the chance of getting a coinstake is proportional to the
// amount of coin age one owns.
// The reason this hash is chosen is the following:
//   nStakeModifier:
//       (v0.3) scrambles computation to make it very difficult to precompute
//              future proof-of-stake at the time of the coin's confirmation
//       (v0.2) nBits (deprecated): encodes all past block timestamps
//   txPrev.block.nTime: prevent nodes from guessing a good timestamp to
//                       generate transaction for future advantage
//   txPrev.offset: offset of txPrev inside block, to reduce the chance of
//                  nodes generating coinstake at the same time
//   txPrev.nTime: reduce the chance of nodes generating coinstake at the same
//                 time
//   txPrev.vout.n: output number of txPrev, to reduce the chance of nodes
//                  generating coinstake at the same time
//   block/tx hash should not be used here as they can be generated in vast
//   quantities so as to generate blocks faster, degrading the system back into
//   a proof-of-work situation.
//
bool CheckStakeKernelHash(CBlockIndex* pindexPrev, unsigned int nBits, const CBlock& blockFrom, unsigned int nTxPrevOffset, const CTransaction& txPrev, const COutPoint& prevout, unsigned int nTimeTx, uint256& hashProofOfStake, bool fPrintProofOfStake)
{
    if (nTimeTx < txPrev.nTime)  // Transaction timestamp violation
        return error("CheckStakeKernelHash() : nTime violation");

    unsigned int nTimeBlockFrom = blockFrom.GetBlockTime();
    if (nTimeBlockFrom + GetStakeMinAge(nTimeTx) > nTimeTx) // Min age requirement
        return error("CheckStakeKernelHash() : min age violation");

    if(!pindexPrev && fDebugMagi) printf("ERROR: CheckStakeKernelHash() - pindexPrev is null");

    CBigNum bnTargetPerCoinDay;
    bnTargetPerCoinDay.SetCompact(nBits);
    int64 nValueIn = txPrev.vout[prevout.n].nValue;

        unsigned int nTimeTxPrev = txPrev.nTime;

    
    // v0.3 protocol kernel hash weight starts from 0 at the min age
    // this change increases active coins participating the hash and helps
    // to secure the network when proof-of-stake difficulty is low
//    int64 nTimeWeight = min((int64)nTimeTx - txPrev.nTime, (int64)nStakeMaxAge) - nStakeMinAge;
    int64 nTimeWeight = (IsPoSIIProtocolV2(pindexPrev->nHeight+1)) ?
			GetMagiWeightV2(nValueIn, txPrev.nTime, nTimeTx) : 
			GetMagiWeight(nValueIn, txPrev.nTime, nTimeTx);
    CBigNum bnCoinDayWeight = CBigNum(nValueIn) * nTimeWeight / COIN / (24 * 60 * 60);

	// printf(">>> CheckStakeKernelHash: nTimeWeight = %"PRI64d"\n", nTimeWeight);
    // Calculate hash
    CDataStream ss(SER_GETHASH, 0);
    uint64 nStakeModifier = 0;
    int nStakeModifierHeight = 0;
    int64 nStakeModifierTime = 0;

    if (!GetKernelStakeModifier(blockFrom.GetHash(), nStakeModifier, nStakeModifierHeight, nStakeModifierTime, fPrintProofOfStake))
	{
		if(fDebug)
			printf(">>> CheckStakeKernelHash: GetKernelStakeModifier return false\n");
        return false;
	}
    if(fDebug && fPrintProofOfStake)
		printf(">>> CheckStakeKernelHash: passed GetKernelStakeModifier\n");
    ss << nStakeModifier;

    ss << nTimeBlockFrom << nTxPrevOffset << txPrev.nTime << prevout.n << nTimeTx;
    hashProofOfStake = Hash(ss.begin(), ss.end());
    if (fPrintProofOfStake)
    {
        printf("CheckStakeKernelHash() : using modifier 0x%016"PRI64x" at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
            nStakeModifier, nStakeModifierHeight,
            DateTimeStrFormat(nStakeModifierTime).c_str(),
            mapBlockIndex[blockFrom.GetHash()]->nHeight,
            DateTimeStrFormat(blockFrom.GetBlockTime()).c_str());
        printf("CheckStakeKernelHash() : check protocol=%s modifier=0x%016"PRI64x" nTimeBlockFrom=%u nTxPrevOffset=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
            "0.3",
            nStakeModifier,
            nTimeBlockFrom, nTxPrevOffset, txPrev.nTime, prevout.n, nTimeTx,
            hashProofOfStake.ToString().c_str());
    }

    
    
    // Now check if proof-of-stake hash meets target protocol
    if (CBigNum(hashProofOfStake) > bnCoinDayWeight * bnTargetPerCoinDay)
	{
		if(fDebug)
		{
			printf(">>> nValueIn = %"PRI64d", nTimeWeight = %"PRI64d", nPrevMoneySupply = %"PRI64d"\n", nValueIn, nTimeWeight, pindexPrev->nMoneySupply);
			CBigNum hashTargett_ = bnCoinDayWeight * bnTargetPerCoinDay;
			printf(">>> bnCoinDayWeight = %s, bnTargetPerCoinDay = %s\n",
			bnCoinDayWeight.ToString().c_str(), bnTargetPerCoinDay.ToString().c_str());
//			printf(">>> hashTarget = %s, hashProof = %s\n",
//			hashTargett_.ToString().c_str(), CBigNum(hashProofOfStake).ToString().c_str());
//			printf(">>> bnCoinDayWeight = %s, bnTargetPerCoinDay = %s\n",
//			bnCoinDayWeight.getuint256().ToString().c_str(), bnTargetPerCoinDay.getuint256().ToString().c_str());
			printf(">>> hashTarget = %s, hashProof = %s\n",
			hashTargett_.getuint256().ToString().c_str(), hashProofOfStake.ToString().c_str());
			printf(">>> CheckStakeKernelHash - hashProofOfStake too much\n");
		}
        return false;
	}


    if (fDebug && !fPrintProofOfStake)
    {
        printf("CheckStakeKernelHash() : using modifier 0x%016"PRI64x" at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
            nStakeModifier, nStakeModifierHeight,
            DateTimeStrFormat(nStakeModifierTime).c_str(),
            mapBlockIndex[blockFrom.GetHash()]->nHeight,
            DateTimeStrFormat(blockFrom.GetBlockTime()).c_str());
        printf("CheckStakeKernelHash() : pass protocol=%s modifier=0x%016"PRI64x" nTimeBlockFrom=%u nTxPrevOffset=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
            "0.3",
            nStakeModifier,
            nTimeBlockFrom, nTxPrevOffset, txPrev.nTime, prevout.n, nTimeTx,
            hashProofOfStake.ToString().c_str());
    }
		if(fDebug)
		{
			printf(">>> nValueIn = %"PRI64d", nTimeWeight = %"PRI64d"\n", nValueIn, nTimeWeight);
			CBigNum hashTargett_ = bnCoinDayWeight * bnTargetPerCoinDay;
			printf(">>> bnCoinDayWeight = %s, bnTargetPerCoinDay = %s\n",
			bnCoinDayWeight.ToString().c_str(), bnTargetPerCoinDay.ToString().c_str());
//			printf(">>> hashTarget = %s, hashProof = %s\n",
//			hashTargett_.ToString().c_str(), CBigNum(hashProofOfStake).ToString().c_str());
//			printf(">>> bnCoinDayWeight = %s, bnTargetPerCoinDay = %s\n",
//			bnCoinDayWeight.getuint256().ToString().c_str(), bnTargetPerCoinDay.getuint256().ToString().c_str());
			printf(">>> hashTarget = %s, hashProof = %s\n",
			hashTargett_.getuint256().ToString().c_str(), hashProofOfStake.ToString().c_str());
			printf(">>< Found a kernel\n");
		}
    return true;
}

// Check kernel hash target and coinstake signature
bool CheckProofOfStake(CBlockIndex* pindexPrev, const CTransaction& tx, unsigned int nBits, uint256& hashProofOfStake)
{
    if (!tx.IsCoinStake())
        return error("CheckProofOfStake() : called on non-coinstake %s", tx.GetHash().ToString().c_str());

    // Kernel (input 0) must match the stake hash target per coin age (nBits)
    const CTxIn& txin = tx.vin[0];

    // First try finding the previous transaction in database
    CTxDB txdb("r");
    CTransaction txPrev;
    CTxIndex txindex;
    if (!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
        return tx.DoS(1, error("CheckProofOfStake() : INFO: read txPrev failed"));  // previous transaction not in main chain, may occur during initial download

    // Verify signature
    if (!VerifySignature(txPrev, tx, 0, true, 0))
        return tx.DoS(100, error("CheckProofOfStake() : VerifySignature failed on coinstake %s", tx.GetHash().ToString().c_str()));

    // Read block header
    CBlock block;
    if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
        return fDebug? error("CheckProofOfStake() : read block failed") : false; // unable to read block of previous transaction

    if (!CheckStakeKernelHash(pindexPrev, nBits, block, txindex.pos.nTxPos - txindex.pos.nBlockPos, txPrev, txin.prevout, tx.nTime, hashProofOfStake, fDebug))
        return tx.DoS(1, error("CheckProofOfStake() : INFO: check kernel failed on coinstake %s, hashProof=%s", tx.GetHash().ToString().c_str(), hashProofOfStake.ToString().c_str())); // may occur during initial download or if behind on block chain sync

    return true;
}

// Check whether the coinstake timestamp meets protocol
bool CheckCoinStakeTimestamp(int64 nTimeBlock, int64 nTimeTx)
{
    // v0.3 protocol
    return (nTimeBlock == nTimeTx);
}

// Get stake modifier checksum
unsigned int GetStakeModifierChecksum(const CBlockIndex* pindex)
{
    assert (pindex->pprev || pindex->GetBlockHash() == (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet));
    // Hash previous checksum with flags, hashProofOfStake and nStakeModifier
    CDataStream ss(SER_GETHASH, 0);
    if (pindex->pprev)
        ss << pindex->pprev->nStakeModifierChecksum;
    ss << pindex->nFlags << (pindex->IsProofOfStake() ? pindex->hashProofOfStake : 0) << pindex->nStakeModifier;
    uint256 hashChecksum = Hash(ss.begin(), ss.end());
    hashChecksum >>= (256 - 32);
    return hashChecksum.Get64();
}

// Check stake modifier hard checkpoints
bool CheckStakeModifierCheckpoints(int nHeight, unsigned int nStakeModifierChecksum)
{
    MapModifierCheckpoints& checkpoints = (fTestNet ? mapStakeModifierCheckpointsTestNet : mapStakeModifierCheckpoints);
    if (checkpoints.count(nHeight))
        return nStakeModifierChecksum == checkpoints[nHeight];
    return true;
}
