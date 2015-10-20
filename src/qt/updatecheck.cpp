// Copyright (c) 2015 The Magi developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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

std::vector<std::string> UpdateCheck::splitString(std::string input, std::string delimiter)
{
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
            }
            else
            {
                output.push_back(input.substr(start));
            }
        }
    }

    return output;
}