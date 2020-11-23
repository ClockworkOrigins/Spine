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
#include "utils/Conversion.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QPushButton>
#include <QSvgWidget>
#include <QtConcurrentRun>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

RatingWidget::RatingWidget(RatingMode mode, QWidget * par) : QWidget(par), _svgs(), _projectID(), _allowedToRate(false), _editable(true), _visible(false), _mode(mode) {
	auto * vl = new QVBoxLayout();
	
	auto * l = new QHBoxLayout();
	for (auto & svg : _svgs) {
		svg = new QSvgWidget(":/svg/star.svg", this);
		svg->setFixedSize(QSize(25, 25));
		l->addWidget(svg);
	}
	
	vl->addLayout(l);

	_reviewButton = new QPushButton(this);
	_reviewButton->setVisible(false);

	vl->addWidget(_reviewButton);
	
	setLayout(vl);

	connect(this, &RatingWidget::receivedRating, this, &RatingWidget::updateRating);
	connect(_reviewButton, &QPushButton::released, this, [this]() {
		emit editReview(_projectID, _review);
	});
}

void RatingWidget::setEditable(bool editable) {
	_editable = editable;
	QWidget::setVisible(_visible && (!_editable || _allowedToRate));
}

void RatingWidget::loginChanged() {
	requestRating();
}

void RatingWidget::setProjectID(int32_t modID) {
	_projectID = modID;
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
	if (modID != _projectID) return;
	
	_allowedToRate = allowedToRate;
	for (size_t i = 0; i < _svgs.size(); i++) {
		if (std::floor(rating) > static_cast<qreal>(i)) {
			if (_editable) {
				_svgs[i]->load(QString(":/svg/star-edit-full.svg"));
			} else {
				_svgs[i]->load(QString(":/svg/star-full.svg"));
			}
		} else if (rating - static_cast<qreal>(i) >= 0.5) {
			_svgs[i]->load(QString(":/svg/star-half.svg"));
		} else {
			if (_editable) {
				_svgs[i]->load(QString(":/svg/star-edit.svg"));
			} else {
				_svgs[i]->load(QString(":/svg/star.svg"));
			}
		}
	}

	_reviewButton->setText(QApplication::tr(_review.isEmpty() ? "WriteReview" : "EditReview"));
	_reviewButton->setVisible(allowedToRate && rating > 0);
	
	QWidget::setVisible(_visible && (!_editable || _allowedToRate));
	if (_editable) {
		setToolTip(QApplication::tr("OwnRatingTooltip").arg(_modname));
	} else {
		setToolTip(QApplication::tr("AvgRatingTooltip").arg(rating, 0, 'f', 1).arg(i2s(count)).arg(_modname));
	}
}

void RatingWidget::mousePressEvent(QMouseEvent * evt) {
	if (!_allowedToRate || !_editable)  return;

	double value = std::ceil((static_cast<double>(evt->localPos().x()) / width()) * 5);
	for (size_t i = 0; i < _svgs.size(); i++) {
		if (value > static_cast<qreal>(i)) {
			_svgs[i]->load(QString(":/svg/star-edit-full.svg"));
		} else {
			_svgs[i]->load(QString(":/svg/star-edit.svg"));
		}
	}
	
	_reviewButton->setVisible(true);

	emit reviewEnabled();
	
	QtConcurrent::run([this, value]() {
		common::SubmitRatingMessage srm;
		srm.username = Config::Username.toStdString();
		srm.password = Config::Password.toStdString();
		srm.modID = _projectID;
		srm.rating = static_cast<int32_t>(value);
		const std::string serialized = srm.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
			sock.writePacket(serialized);
		}
	});
}

void RatingWidget::requestRating() {
	if (_projectID < 1) return;

	QWidget::setVisible(false);
	
	switch (_mode) {
	case RatingMode::Overall: {
		QJsonObject requestData;
		requestData["ProjectID"] = _projectID;

		int projectID = _projectID;
		
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
		requestData["ProjectID"] = _projectID;

		int projectID = _projectID;
		
		https::Https::postAsync(DATABASESERVER_PORT, "getOwnRating", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this, projectID](const QJsonObject & json, int statusCode) {
			if (statusCode != 200) return;

			if (!json.contains("Rating")) return;
			
			if (!json.contains("AllowedToRate")) return;
			
			const int rating = json["Rating"].toString().toInt();
			const bool allowedToRate = json["AllowedToRate"].toString().toInt() == 1;

			if (json.contains("Review")) {
				const auto review = decodeString(json["Review"].toString());
			}

			emit receivedRating(projectID, rating, 1, allowedToRate);

			if (allowedToRate && rating > 0) {
				emit reviewEnabled();
			}
		});
		break;
	}
	}
}
