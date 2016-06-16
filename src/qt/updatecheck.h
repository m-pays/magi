// Copyright (c) 2015 The Magi developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UPDATECHECK_H
#define UPDATECHECK_H

#include <vector>
 
#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

/** \def _UPDATE_INTERVAL Frequency of update checks (units: hours)
 */
#define _UPDATE_INTERVAL 12

/** \def _UPDATE_MS_TO_HOURS Conversion factor from milliseconds to hours
 */
#define _UPDATE_MS_TO_HOURS 1000*60*60

/** \def _UPDATE_VERSION_URL The URL that stores the current wallet version string
 *  \details The newest wallet version string should be the sole contents of the file
 *  to which this URL points.
 *  The string is formatted as follows (based on the defines in ../version.h):
 *  DISPLAY_VERSION_MAJOR.DISPLAY_VERSION_MINOR.DISPLAY_VERSION_REVISION.DISPLAY_VERSION_BUILD
 *  
 *  This yields strings of the form: 1.2.1.2
 * 
 *  It is necessary to change the value of _UPDATE_URL to reflect the official location
 *  of the version string file. The file can be hosted be hosted in the official
 *  repository (using the raw file URL) or the coin's website.
 */
#define _UPDATE_VERSION_URL "http://info.coinmagi.org/version"

/** \def _UPDATE_DOWNLOAD_URL The URL linked when a new wallet version is available
 *  \details This URL string should reflect the offical wallet download location, be
 *  it a web page or directory. This macro contains \ characters before quotation marks
 *  to escape them for use with QString
 */
#define _UPDATE_DOWNLOAD_URL "http://coinmagi.org/bin/"

// url (temporary) for checking price
#define BTC_PRICE_URL "https://api.coinmarketcap.com/v1/ticker/bitcoin/"
#define MAGI_TO_USD_PRICE_URL "https://api.coinmarketcap.com/v1/ticker/magi/"

class UpdateCheck : public QObject
{
    Q_OBJECT
    public:
        explicit UpdateCheck(QUrl updateUrl, QObject *parent = 0);
        virtual ~UpdateCheck();
        QByteArray downloadedData() const;
        unsigned int parseClientVersion(const std::string &s, char delim);

    signals:
        void downloaded();
 
    private slots:
        void updateDownloaded(QNetworkReply* pReply);
 
    private:
        QNetworkAccessManager m_WebCtrl;
        QByteArray m_DownloadedData;
};
 
#endif // UPDATECHECK_H
