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
// Copyright 2018 Clockwork Origins

#include "widgets/RatingWidget.h"

#include "SpineConfig.h"

#include "common/MessageStructs.h"

#include "https/Https.h"

#include "utils/Config.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QSvgWidget>
#include <QtConcurrentRun>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

RatingWidget::RatingWidget(RatingMode mode, QWidget * par) : QWidget(par), _svgs(), _modID(), _allowedToRate(false), _editable(true), _visible(false), _mode(mode) {
	QHBoxLayout * l = new QHBoxLayout();
	for (size_t i = 0; i < _svgs.size(); i++) {
		_svgs[i] = new QSvgWidget(":/svg/star.svg", this);
		_svgs[i]->setFixedSize(QSize(25, 25));
		l->addWidget(_svgs[i]);
	}
	setLayout(l);

	connect(this, &RatingWidget::receivedRating, this, &RatingWidget::updateRating);
}

void RatingWidget::setEditable(bool editable) {
	_editable = editable;
	QWidget::setVisible(_visible && (!_editable || _allowedToRate));
}

void RatingWidget::loginChanged() {
	requestRating();
}

void RatingWidget::setModID(int32_t modID) {
	_modID = modID;
	requestRating();
}

void RatingWidget::setVisible(bool visible) {
	_visible = visible;
	QWidget::setVisible(visible && (!_editable || _allowedToRate));
}

void RatingWidget::setModName(QString name) {
	_modname = name;
}

void RatingWidget::updateRating(int32_t modID, qreal rating, int32_t count, bool allowedToRate) {
	if (modID != _modID) return;
	
	_allowedToRate = allowedToRate;
	for (size_t i = 0; i < _svgs.size(); i++) {
		if (std::floor(rating) > i) {
			if (_editable) {
				_svgs[i]->load(QString(":/svg/star-edit-full.svg"));
			} else {
				_svgs[i]->load(QString(":/svg/star-full.svg"));
			}
		} else if (rating - i >= 0.5) {
			_svgs[i]->load(QString(":/svg/star-half.svg"));
		} else {
			if (_editable) {
				_svgs[i]->load(QString(":/svg/star-edit.svg"));
			} else {
				_svgs[i]->load(QString(":/svg/star.svg"));
			}
		}
	}
	QWidget::setVisible(_visible && (!_editable || _allowedToRate));
	if (_editable) {
		setToolTip(QApplication::tr("OwnRatingTooltip").arg(_modname));
	} else {
		setToolTip(QApplication::tr("AvgRatingTooltip").arg(rating, 0, 'f', 1).arg(count).arg(_modname));
	}
}

void RatingWidget::mousePressEvent(QMouseEvent * evt) {
	if (!_allowedToRate || !_editable)  return;

	double value = std::ceil((static_cast<double>(evt->localPos().x()) / width()) * 5);
	for (size_t i = 0; i < _svgs.size(); i++) {
		if (value > i) {
			_svgs[i]->load(QString(":/svg/star-edit-full.svg"));
		} else {
			_svgs[i]->load(QString(":/svg/star-edit.svg"));
		}
	}
	QtConcurrent::run([this, value]() {
		common::SubmitRatingMessage srm;
		srm.username = Config::Username.toStdString();
		srm.password = Config::Password.toStdString();
		srm.modID = _modID;
		srm.rating = value;
		const std::string serialized = srm.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
			sock.writePacket(serialized);
		}
	});
}

void RatingWidget::requestRating() {
	if (_modID < 1) return;

	QWidget::setVisible(false);
	
	switch (_mode) {
	case RatingMode::Overall: {
		QJsonObject requestData;
		requestData["ProjectID"] = _modID;

		int projectID = _modID;
		
		https::Https::postAsync(DATABASESERVER_PORT, "getWeightedRating", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this, projectID](const QJsonObject & json, int statusCode) {
			if (statusCode != 200) return;

			if (!json.contains("Rating")) return;
			
			if (!json.contains("Count")) return;
			
			if (!json.contains("RealCount")) return;

			const int rating = json["Rating"].toString().toInt();
			const int count = json["Count"].toString().toInt();
			const int realCount = json["RealCount"].toString().toInt();

			qreal average = rating;
			if (count > 0) {
				average /= count;
			}

			emit receivedRating(projectID, average, realCount, false);
		});
		break;
	}
	case RatingMode::User: {
		QJsonObject requestData;
		requestData["Username"] = Config::Username;
		requestData["Password"] = Config::Password;
		requestData["ProjectID"] = _modID;

		int projectID = _modID;
		
		https::Https::postAsync(DATABASESERVER_PORT, "getOwnRating", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this, projectID](const QJsonObject & json, int statusCode) {
			if (statusCode != 200) return;

			if (!json.contains("Rating")) return;
			
			if (!json.contains("AllowedToRate")) return;
			
			const int rating = json["Rating"].toString().toInt();
			const bool allowedToRate = json["AllowedToRate"].toString().toInt() == 1;

			emit receivedRating(projectID, rating, 1, allowedToRate);
		});
		break;
	}
	default: {
		break;
	}
	}
}
