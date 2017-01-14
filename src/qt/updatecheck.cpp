// Copyright (c) 2015 The Magi developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <string>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "updatecheck.h"
//#include <iostream>

UpdateCheck::UpdateCheck(QUrl updateUrl, QObject *parent) :
 QObject(parent)
{
 connect(&m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
         this, SLOT (updateDownloaded(QNetworkReply*))
        );
    QNetworkRequest request(updateUrl);
    m_WebCtrl.get(request);
}
 
UpdateCheck::~UpdateCheck() { }
 
void UpdateCheck::updateDownloaded(QNetworkReply* pReply) {
    m_DownloadedData = pReply->readAll();
    //emit a signal
    pReply->deleteLater();
    emit downloaded();
}
 
QByteArray UpdateCheck::downloadedData() const {
    return m_DownloadedData;
}

unsigned int UpdateCheck::parseClientVersion(const std::string &s, char delim)
{
<<<<<<< HEAD
    std::stringstream ss(s);
    std::string seg;
    unsigned int nVersion = 0;
    for (unsigned int i = 0; std::getline(ss, seg, delim); i += 2) {
        boost::trim(seg);
        try {
            nVersion +=10000000 / pow(10., i) * boost::lexical_cast<int>(seg);
        } catch (const boost::bad_lexical_cast &) {
            unsigned nPos;
            if (seg.find("alpha") == 0) { // release alpha1 ~ alpha3
                nVersion -= 10;
                nPos = 5;
=======
    std::vector<std::string> output;
    size_t start = 0;
    size_t end = 0;

    //DEBUG: In order to ensure we aren't having errors receiving the version info,
    //       print the string that we read in the console.
    //       This should *not* be enabled in the released wallet
    //std::cout << "Version String Received: " << input << std::endl;

    // Until we hit the end of the string, push string chunks
    // into the return array
    while (start != std::string::npos && end != std::string::npos)
    {
        start = input.find_first_not_of(delimiter, end);

        if (start != std::string::npos)
        {
            end = input.find_first_of(delimiter, start);

            if (end != std::string::npos)
            {
                output.push_back(input.substr(start, end - start));
>>>>>>> f6f7bd4ad51cfb0f67d2dce2360d91b435348610
            }
            else if (seg.find("beta") == 0) { // release beta1 ~ beta3
                nVersion -= 7;
                nPos = 4;
            }
            else if (seg.find("rc") == 0) { // rc1 ~ rc3
                nVersion -= 4;
                nPos = 2;
            }
            try {
                nVersion += boost::lexical_cast<int>(seg[nPos]);
            } catch (const boost::bad_lexical_cast &) {
            }
            break;
        }

    }
    return nVersion;
}
