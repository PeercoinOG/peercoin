// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2019 The Peercoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "db.h"
#include "main.h"
#include "txdb.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;   // hardened checkpoints

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 0, hashGenesisBlockOfficial )
        ( 19080, uint256("0x000000000000bca54d9ac17881f94193fd6a270c1bb21c3bf0b37f588a40dbd7"))
        ( 30583, uint256("0xd39d1481a7eecba48932ea5913be58ad3894c7ee6d5a8ba8abeb772c66a6696e"))
        ( 99999, uint256("0x27fd5e1de16a4270eb8c68dee2754a64da6312c7c3a0e99a7e6776246be1ee3f"))
        (219999, uint256("0xab0dad4b10d2370f009ed6df6effca1ba42f01d5070d6b30afeedf6463fbe7a2"))
        (336000, uint256("0x4d261cef6e61a5ed8325e560f1d6e36f4698853a4c7134677f47a1d1d842bdf6"))
        (371850, uint256("0x6b18adcb0a6e080dae85b74eee2b83fabb157bbea64fab0ed2192b2f6d5b89f3"))
        (407813, uint256("0x00000000000000012730b0f48bed8afbeb08164c9d63597afb082e82ea05cec9"))
        (419368, uint256("0xa6c25717d4f11bed297115a517d20ea66ec76a9edbc6e6731f50d00a26754e20"))
        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1552392838, // * UNIX timestamp of last checkpoint block
        1604408,   // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        305.0     // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet = 
        boost::assign::map_list_of
        ( 0, hashGenesisBlockTestNet )
        ( 19080, uint256("0xb054d63d41852d71b611eaa8eca37d9fddca69b5013cf0966d453402ec8005ce"))
        ( 30583, uint256("0x5179c0c496b5d25ab81ffe14273ea6928c6ff81c0a0d6a83b5d7d41d64886300"))
        ( 99999, uint256("0xa7b03b14b8673683d972ab81775f3e85fea4fe689874b5956183466535dc651c"))
        (219999, uint256("0x0691bb86c92762c5c4c5a3723585ebeb7ec59310bbb0bdb6666551ab24ad919e"))
        (336000, uint256("0xf07adf61615c529f7c282b858d13d3e037b197324cb12e0669c461947494c4e3"))
        (372751, uint256("0x000000000000148db599b217c117b5104f5043c55f6ca2a8a065d9fab9f9bba1"))
        (382019, uint256("0x3ab75769d7957d9bf0857b5019d0a0e41044fa9ecf30b2f9c32aa457b0864ce5"))
        (394259, uint256("0xa8ca5f2112bb419803c791b8b03ae419efd72e7a33846f6049d6b41f830fd3e4"))
        (400000, uint256("0x2f4e6f9b98ad4190b2a7f8a7c54c3848591d88ade6f6338bf094366b516a8cc8"))
        (408500, uint256("0x0c897bc0bc2aad17f6bc8f3c7f7414d14e5dc7a5ca2fcb58d6bbd90232b64a57"))
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1563293806,
        799367,
        300
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    uint256 GetLatestHardenedCheckpoint()
    {
        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;
        return (checkpoints.rbegin()->second);
    }
}
