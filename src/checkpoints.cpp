// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "txdb.h"
#include "main.h"
#include "uint256.h"


static const int nCheckpointSpan = 500;

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    //
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 0,	hashGenesisBlockOfficial )
	( 1999,	uint256("0x00000000221617cf173f4b7b972eb818cce4bebccf655df9b8045a1693614700"))
	( 9999,	uint256("0x000000000068f65edd06adea78ba75ce1325c1316dc31c3c3e8a82c5a2f06bf2"))
	( 19999,uint256("0x985a40a8d509121d8f633e0e05e0091435d4db549d7558915c7a8a4773130ff4"))
	( 37090,uint256("0x3a185dbcff5271d9b75b12086c064e9596db26d96503ff84439f24e720807bb1"))
	( 69999,uint256("0x00000000059b68241f8482737003cd6672298dc58e48ee961f577551b74b1604"))
	( 109999,uint256("0xba31c8b1aca84143858c4afd1ce59d9c3e327f69d272eb1bf87fe8a5a61449f6"))
	( 220000,uint256("0x000000003d1f4b82ee64d28f9b05a310f374a948ba5dd81b939e1af030c17941"))
	( 260000,uint256("0x979d5173ad642aa0f8166c9a3c2b351de0e7ec381f2465659de31287e0fb5ad7"))
	( 300000,uint256("0x0000000085d96ac62f6208a3520ced06102cef49a607a2550cd4126e82091a00"))
	( 350000,uint256("0x000000005f2959514e33e69d8a879ddb82b0f860f0f2bba5dd4cc4c9115b20c4"))
	( 380000,uint256("0x000000001eafd4b5d92620f4413487c021889ed1749718373a5bd5c4fb65c798"))
	( 400000,uint256("0x846c39d7ae5b9f9e7c1564f75fe8ef9565cd7fee4f4791a7a599c3a4f09fc6fc"))
	( 450000,uint256("0xd9b19fa6d10cf25ec5f1e2dde5561feb290b109d80f63fed0ca7adb8ba336443"))
	( 480330,uint256("0x0000000041ae89a6138179e395d4fe4e5658a3bdfe718fdb44d6253d1229b36e"))
	;

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 0,	hashGenesisBlockTestNet )
        ;

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }


    // Automatically select a suitable sync-checkpoint 
    const CBlockIndex* AutoSelectSyncCheckpoint()
    {
        const CBlockIndex *pindex = pindexBest;
        // Search backward for a block within max span and maturity window
        while (pindex->pprev && pindex->nHeight + nCheckpointSpan > pindexBest->nHeight)
            pindex = pindex->pprev;
        return pindex;
    }

    // Check against synchronized checkpoint
//    bool CheckSync(const uint256& hashBlock, const CBlockIndex* pindexPrev)
    bool CheckSync(int nHeight)
    {
        if (fTestNet) return true; // Testnet has no checkpoints

        const CBlockIndex* pindexSync = AutoSelectSyncCheckpoint();
        if (nHeight <= pindexSync->nHeight)
            return false;
        return true;
    }
}
