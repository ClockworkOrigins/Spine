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
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
// Copyright 2018 Clockwork Origins

#include "widgets/RatingWidget.h"

#include <thread>

#include "SpineConfig.h"

#include "common/MessageStructs.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QSvgWidget>

namespace spine {
namespace widgets {

	RatingWidget::RatingWidget(QWidget * par) : QWidget(par), _svgs(), _username(), _modID(), _modname(), _allowedToRate(false), _editable(true), _visible(false) {
		QHBoxLayout * l = new QHBoxLayout();
		for (size_t i = 0; i < _svgs.size(); i++) {
			_svgs[i] = new QSvgWidget(":/svg/star.svg", this);
			_svgs[i]->setFixedSize(QSize(25, 25));
			l->addWidget(_svgs[i]);
		}
		setLayout(l);

		connect(this, SIGNAL(receivedRating(int32_t, int32_t, int32_t, bool)), this, SLOT(updateRating(int32_t, int32_t, int32_t, bool)));
	}

	RatingWidget::~RatingWidget() {
	}

	void RatingWidget::setEditable(bool editable) {
		_editable = editable;
		QWidget::setVisible(_visible && (!_editable || _allowedToRate));
	}

	void RatingWidget::setUsername(QString username, QString password) {
		_username = username;
		_password = password;
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

	void RatingWidget::updateRating(int32_t modID, int32_t sum, int32_t count, bool allowedToRate) {
		if (modID != _modID) {
			return;
		}
		_allowedToRate = allowedToRate;
		double rating = sum;
		if (count > 0) {
			rating /= count;
		}
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
		if (!_allowedToRate || !_editable) {
			return;
		}
		double value = std::ceil((double(evt->localPos().x()) / width()) * 5);
		for (size_t i = 0; i < _svgs.size(); i++) {
			if (value > i) {
				_svgs[i]->load(QString(":/svg/star-edit-full.svg"));
			} else {
				_svgs[i]->load(QString(":/svg/star-edit.svg"));
			}
		}
		std::thread([this, value]() {
			common::SubmitRatingMessage srm;
			srm.username = _username.toStdString();
			srm.password = _password.toStdString();
			srm.modID = _modID;
			srm.rating = value;
			std::string serialized = srm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
				sock.writePacket(serialized);
			}
		}).detach();
	}

	void RatingWidget::requestRating() {
		std::thread([this]() {
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == cErr) {
				{
					common::RequestRatingMessage rrm;
					rrm.modID = _modID;
					rrm.username = _username.toStdString();
					rrm.password = _password.toStdString();
					std::string serialized = rrm.SerializePublic();
					sock.writePacket(serialized);
					if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
						try {
							common::Message * m = common::Message::DeserializePublic(serialized);
							if (m) {
								common::SendRatingMessage * srm = dynamic_cast<common::SendRatingMessage *>(m);
								emit receivedRating(srm->modID, srm->sum, srm->voteCount, srm->allowedToRate);
							}
							delete m;
						} catch (...) {
							return;
						}
					} else {
						qDebug() << "Error occurred: " << int(cErr);
					}
				}
			}
		}).detach();
	}

} /* namespace widgets */
} /* namespace spine */
