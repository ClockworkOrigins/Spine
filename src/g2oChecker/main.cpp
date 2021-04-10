/*
	This file is part of Spine.

    Spine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Spine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Spine.  If not, see <http://www.gnu.org/licenses/>.
 */
// Copyright 2021 Clockwork Origins

#include <iostream>
#include <thread>

#include <QApplication>
#include <QDirIterator>
#include <QEventLoop>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

#include "https/Https.h"

#include "zipper/unzipper.h"

#include <Windows.h>

using namespace spine::https;

struct Version {
    int major;
    int minor;
    int patch;
    int build;
};

enum G2OCode {
	G2O_UP_TO_DATE,
	G2O_OUTDATED,
	G2O_NOT_SUPPORTED,
	G2O_ERROR
};

typedef Version(__cdecl * G2O_Version)();

int main(int argc, char ** argv) {
	if (!QFileInfo::exists(G2OFOLDER)) {
        std::cout << "G2O folder doesn't exist" << std::endl;
        return 1;
	}

    const auto proxyDll = QStringLiteral(G2OFOLDER) + "/G2O_Proxy.dll";

	if (!QFileInfo::exists(proxyDll)) {
        std::cout << "G2O_Proxy.dll doesn't exist" << std::endl;
        return 1;
	}

    int code;

    QApplication app(argc,argv);

    do {
        const auto dll = LoadLibrary(QDir::toNativeSeparators(proxyDll).toStdString().c_str());

        if (!dll) {
            std::cout << "G2O_Proxy.dll couldn't be loaded" << std::endl;
            std::cout << GetLastError() << std::endl;
            return 1;
        }

        const auto func = reinterpret_cast<G2O_Version>(GetProcAddress(dll, "G2O_Version"));

        if (!func) {
            std::cout << "Function G2O_Version couldn't be loaded" << std::endl;
            return 1;
        }

        const auto version = func();

        FreeLibrary(dll);

        QJsonObject json;
        json["Major"] = version.major;
        json["Minor"] = version.minor;
        json["Patch"] = version.patch;
        json["Build"] = version.build;

        const auto payload = QJsonDocument(json).toJson(QJsonDocument::Compact);

        QString url;

        Http::post("api.gothic-online.com.pl", 80, "update/version", payload, [&code, &url](const QJsonObject & json, int statusCode) {
            if (statusCode != 200) {
                code = G2O_ERROR;
                return;
            }

            code = json["code"].toInt();

        	if (code == G2O_OUTDATED) {
                url = json["link"].toString();
        	}
        });

    	if (code == G2O_OUTDATED) {
            QEventLoop loop;
            QNetworkAccessManager networkManager;
            QNetworkRequest request(url);
            request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
            auto * reply = networkManager.get(request);
    		
    		while (!reply->isFinished()) {
                loop.processEvents();
    		}
    		
            const auto filename = QFileInfo(url).fileName();
            const auto zipname = filename + ".zip";
    		
            QFile tf(QDir::homePath() + "/" + zipname);
            tf.open(QIODevice::WriteOnly);
            tf.write(reply->readAll());
            tf.close();

            const auto path = tf.fileName();

    		if (!QFileInfo::exists(path)) {
                std::cout << "Download from " << url.toStdString() << " failed" << std::endl;
                return 1;
    		}
    		
            const auto outfolder = QFileInfo(path).absolutePath() + "/" + filename;
    		
            {
                zipper::Unzipper zip(path.toStdString());
                zip.extract(outfolder.toStdString());
            }

            QDirIterator dirIt(outfolder + "/", QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);
    		while (dirIt.hasNext()) {
                const auto file = dirIt.next();
                auto fn = file;
                fn.remove(outfolder + "/");

                const auto resultFile = QStringLiteral(G2OFOLDER) + "/" + fn;

    			if (QFileInfo::exists(resultFile)) {
                    QFile::remove(resultFile);
    			}
    			
                QFile::copy(file, resultFile);
    		}

            QFile::remove(path);
            QDir(outfolder).removeRecursively();
    	}
    } while (code == G2O_OUTDATED);
	
	return 0;
}
