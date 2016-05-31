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
	( 9,	uint256("0x000000b608889c47e3f772c0a6cd7f7a6cecb27a4e1df42193fba633bd796b52"))
	( 19,	uint256("0x000001bb9ebe4331ea51c667f875c6451e42608ee3dab0072355de2745628570"))
	( 99,	uint256("0x0000000130ea8bc46c98ab87deb3bd0456692ee7ef32215b4623e2529c45e4be"))
	( 199,	uint256("0x000000021465afb8c2aa1e506a4f82f304e5cb595b5fc5e9055bd0b55a74262b"))
	( 999,	uint256("0x000000003b59a70255fd0a773416496226cc89443eb7f5dcd0fdd0678dba48c4"))
	( 1999,	uint256("0x00000000221617cf173f4b7b972eb818cce4bebccf655df9b8045a1693614700"))
	( 9999,	uint256("0x000000000068f65edd06adea78ba75ce1325c1316dc31c3c3e8a82c5a2f06bf2"))
	( 19999,uint256("0x985a40a8d509121d8f633e0e05e0091435d4db549d7558915c7a8a4773130ff4"))
	( 29999,uint256("0x0000000002c7b6ba5369a2b2e29683838facc7171c705b2aa4470bedecd5f5b3"))
	( 37090,uint256("0x3a185dbcff5271d9b75b12086c064e9596db26d96503ff84439f24e720807bb1"))
	( 49999,uint256("0xa51118e82860e059f37120e3e03d68f240483b5c9163fc46c25166bd336cd8aa"))
	( 69999,uint256("0x00000000059b68241f8482737003cd6672298dc58e48ee961f577551b74b1604"))
	( 89999,uint256("0xa7655e22de7d76e1b0b95450ef19de18d4aea28c4769f610c11c375e539c7efb"))
	( 109999,uint256("0xba31c8b1aca84143858c4afd1ce59d9c3e327f69d272eb1bf87fe8a5a61449f6"))
	( 123838,uint256("0x0f29125ecac37d1465490fad8a9f949fb76828e256017affb31406c1ba50900e"))
	( 200000,uint256("0x4d79738e5a16ba831fceed7ad784ef0b101707adbf77355192e70f82e623883f"))
	( 220000,uint256("0x000000003d1f4b82ee64d28f9b05a310f374a948ba5dd81b939e1af030c17941"))
	( 240000,uint256("0x8aa68bb57f04521971ce479cfa6140b367462116ddeb55c523b2ec29b439a5e5"))
	( 260000,uint256("0x979d5173ad642aa0f8166c9a3c2b351de0e7ec381f2465659de31287e0fb5ad7"))
	( 280000,uint256("0x9b7f2a068f29983821b562b26390b4bfd56e37ba4a3873c45b2b826c24137755"))
	( 300000,uint256("0x0000000085d96ac62f6208a3520ced06102cef49a607a2550cd4126e82091a00"))
	( 310000,uint256("0x797ee4acbef92adc9cbfdd188a68b8b0d29c8a4610742da9ee85d6e5af1a386e"))
	( 313600,uint256("0x011646fbd93d5594d35f399af68cb8e2feba8d563e37f11eda860fe8dfe60d8b"))
	( 320000,uint256("0x9dbf3c825efbcf7328287a8b4634fb02be246481e205c87fb389a0ba1ad51d59"))
	( 350000,uint256("0x000000005f2959514e33e69d8a879ddb82b0f860f0f2bba5dd4cc4c9115b20c4"))
	( 380000,uint256("0x000000001eafd4b5d92620f4413487c021889ed1749718373a5bd5c4fb65c798"))
	( 400000,uint256("0x846c39d7ae5b9f9e7c1564f75fe8ef9565cd7fee4f4791a7a599c3a4f09fc6fc"))
	( 420000,uint256("0x000000000cccddb818253378e2e4e6da433c0b1ce6f0c6f44f0e5ed616b06233"))
	( 450000,uint256("0xd9b19fa6d10cf25ec5f1e2dde5561feb290b109d80f63fed0ca7adb8ba336443"))
	( 465000,uint256("0x0000000038f6ee5ea559e7f04bee27277c65d77d12cc1092ff5f8d351b328820"))
	( 480330,uint256("0x0000000041ae89a6138179e395d4fe4e5658a3bdfe718fdb44d6253d1229b36e"))
	;

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 0,	hashGenesisBlockTestNet )
	( 999,	uint256("0x00000019d27a844914155427e0391cec5655cf2592dbdec60374ee3171578f23"))
	( 1999,	uint256("0x0000027424df19d9a2fb9f2c05c0a8bdb3f8627d9d9dfd2b4432c67747f82b46"))
	( 9999,	uint256("0x9ab990fe5574c593826357e50adf3ea213b93e5e95844dcd2d7f82c66bb7c272"))
	( 19999,uint256("0x000002565a853b2b49caf74bb9b4ca8553be397121390c91711c40bd2a9063f0"))
	( 27680,uint256("0x000005bd5400733354539b58f5889c6632e42d216ef8d74809098459820a111f"))
        ;

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
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
