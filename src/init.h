// Copyright (c) 2009-2016 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Developers
// Copyright (c) 2015-2016 Silk Network
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef DARKSILK_INIT_H
#define DARKSILK_INIT_H

#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#endif

namespace boost {
    class thread_group;
} // namespace boost

#ifdef ENABLE_WALLET
extern CWallet* pwalletMain;
#endif

void StartShutdown();
bool ShutdownRequested();
void Shutdown();
bool AppInit2(boost::thread_group& threadGroup);
std::string HelpMessage();
extern bool fOnlyTor;

#endif
