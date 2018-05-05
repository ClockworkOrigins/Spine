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

#include "widgets/management/AchievementWidget.h"

#include <fstream>

#include "Config.h"
#include "Conversion.h"

#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filter/zlib.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCryptographicHash>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	AchievementWidget::AchievementWidget(QWidget * par) : QWidget(par), _unlockedImagePath(nullptr) {
		QGridLayout * gl = new QGridLayout();

		QStringList languages;
		languages << QApplication::tr("German") << QApplication::tr("English") << QApplication::tr("Polish") << QApplication::tr("Russian");

		gl->addWidget(new QLabel(QApplication::tr("Name"), this), 0, 0);
		_nameEdit = new QLineEdit(this);
		_nameLanguageBox = new QComboBox(this);
		_nameLanguageBox->setEditable(false);
		_nameLanguageBox->addItems(languages);
		_nameLanguageBox->setCurrentText(QApplication::tr("German"));
		connect(_nameLanguageBox, &QComboBox::currentTextChanged, this, &AchievementWidget::nameLanguageChanged);
		_currentNameLanguage = QApplication::tr("German");

		gl->addWidget(_nameEdit, 0, 1);
		gl->addWidget(_nameLanguageBox, 0, 2);

		gl->addWidget(new QLabel(QApplication::tr("Description"), this), 1, 0);
		_descriptionEdit = new QLineEdit(this);
		_descriptionLanguageBox = new QComboBox(this);
		_descriptionLanguageBox->setEditable(false);
		_descriptionLanguageBox->addItems(languages);
		_descriptionLanguageBox->setCurrentText(QApplication::tr("German"));
		connect(_descriptionLanguageBox, &QComboBox::currentTextChanged, this, &AchievementWidget::descriptionLanguageChanged);
		_currentDescriptionLanguage = QApplication::tr("German");

		gl->addWidget(_descriptionEdit, 1, 1);
		gl->addWidget(_descriptionLanguageBox, 1, 2);

		gl->addWidget(new QLabel(QApplication::tr("Hidden"), this), 2, 0);
		_hiddenBox = new QCheckBox(this);
		gl->addWidget(_hiddenBox, 2, 1);

		gl->addWidget(new QLabel(QApplication::tr("MaxProgress"), this), 3, 0);
		_progressBox = new QSpinBox(this);
		_progressBox->setMinimum(0);
		_progressBox->setMaximum(1000000);
		gl->addWidget(_progressBox, 3, 1);

		{
			QHBoxLayout * hl = new QHBoxLayout();
			gl->addWidget(new QLabel(QApplication::tr("LockedImage"), this), 4, 0);
			_lockedImagePath = new QLineEdit(this);
			connect(_lockedImagePath, &QLineEdit::textChanged, this, &AchievementWidget::changedLockedImage);
			QPushButton * pb = new QPushButton("...", this);
			connect(pb, &QPushButton::released, this, &AchievementWidget::openLockedImage);
			hl->addWidget(_lockedImagePath, 1);
			hl->addWidget(pb);
			gl->addLayout(hl, 4, 1);

			_lockedImage = new QLabel(this);
			_lockedImage->setFixedSize(64, 64);

			changedLockedImage("");

			gl->addWidget(_lockedImage, 4, 2);
		}

		{
			QHBoxLayout * hl = new QHBoxLayout();
			gl->addWidget(new QLabel(QApplication::tr("UnlockedImage"), this), 5, 0);
			_unlockedImagePath = new QLineEdit(this);
			connect(_unlockedImagePath, &QLineEdit::textChanged, this, &AchievementWidget::changedUnlockedImage);
			QPushButton * pb = new QPushButton("...", this);
			connect(pb, &QPushButton::released, this, &AchievementWidget::openUnlockedImage);
			hl->addWidget(_unlockedImagePath, 1);
			hl->addWidget(pb);
			gl->addLayout(hl, 5, 1);

			_unlockedImage = new QLabel(this);
			_unlockedImage->setFixedSize(64, 64);

			changedUnlockedImage("");

			gl->addWidget(_unlockedImage, 5, 2);
		}

		setLayout(gl);
	}

	AchievementWidget::~AchievementWidget() {
	}

	void AchievementWidget::setAchievement(int32_t modID, common::SendModManagementMessage::ModManagement::Achievement achievement) {
		_currentNameLanguage.clear();
		_currentDescriptionLanguage.clear();

		_achievement = achievement;

		_newAchievement.names = achievement.names;
		_newAchievement.descriptions = achievement.descriptions;
		_newAchievement.hidden = achievement.hidden;
		_newAchievement.maxProgress = achievement.maxProgress;
		_newAchievement.lockedImageName = achievement.lockedImageName;
		_newAchievement.lockedImageHash = achievement.lockedImageHash;
		_newAchievement.unlockedImageName = achievement.unlockedImageName;
		_newAchievement.unlockedImageHash = achievement.unlockedImageHash;

		nameLanguageChanged(QApplication::tr("German"));
		descriptionLanguageChanged(QApplication::tr("German"));

		_hiddenBox->setChecked(_newAchievement.hidden);
		_progressBox->setValue(_newAchievement.maxProgress);

		QString lockedImage = s2q(_newAchievement.lockedImageName);
		lockedImage.chop(2);
		lockedImage.prepend(Config::MODDIR + "/mods/" + QString::number(modID) + "/achievements/");

		changedLockedImage(lockedImage);

		QString unlockedImage = s2q(_newAchievement.unlockedImageName);
		unlockedImage.chop(2);
		unlockedImage.prepend(Config::MODDIR + "/mods/" + QString::number(modID) + "/achievements/");

		changedUnlockedImage(unlockedImage);
	}

	common::UpdateAchievementsMessage::Achievement AchievementWidget::getAchievement() {
		_newAchievement.hidden = _hiddenBox->isChecked();
		_newAchievement.maxProgress = _progressBox->value();

		nameLanguageChanged(_currentNameLanguage);
		descriptionLanguageChanged(_currentDescriptionLanguage);

		return _newAchievement;
	}

	void AchievementWidget::nameLanguageChanged(QString language) {
		if (_currentNameLanguage == QApplication::tr("German")) {
			bool found = false;
			for (auto it = _newAchievement.names.begin(); it != _newAchievement.names.end(); ++it) {
				if (it->language == "Deutsch") {
					if (_nameEdit->text().isEmpty()) {
						_newAchievement.names.erase(it);
					} else {
						it->text = q2s(_nameEdit->text());
					}
					found = true;
					break;
				}
			}
			if (!found && !_nameEdit->text().isEmpty()) {
				common::TranslatedText tt;
				tt.language = "Deutsch";
				tt.text = q2s(_nameEdit->text());
				_newAchievement.names.push_back(tt);
			}
		} else if (_currentNameLanguage == QApplication::tr("English")) {
			bool found = false;
			for (auto it = _newAchievement.names.begin(); it != _newAchievement.names.end(); ++it) {
				if (it->language == "English") {
					if (_nameEdit->text().isEmpty()) {
						_newAchievement.names.erase(it);
					} else {
						it->text = q2s(_nameEdit->text());
					}
					found = true;
					break;
				}
			}
			if (!found && !_nameEdit->text().isEmpty()) {
				common::TranslatedText tt;
				tt.language = "English";
				tt.text = q2s(_nameEdit->text());
				_newAchievement.names.push_back(tt);
			}
		} else if (_currentNameLanguage == QApplication::tr("Polish")) {
			bool found = false;
			for (auto it = _newAchievement.names.begin(); it != _newAchievement.names.end(); ++it) {
				if (it->language == "Polish") {
					if (_nameEdit->text().isEmpty()) {
						_newAchievement.names.erase(it);
					} else {
						it->text = q2s(_nameEdit->text());
					}
					found = true;
					break;
				}
			}
			if (!found && !_nameEdit->text().isEmpty()) {
				common::TranslatedText tt;
				tt.language = "Polish";
				tt.text = q2s(_nameEdit->text());
				_newAchievement.names.push_back(tt);
			}
		} else if (_currentNameLanguage == QApplication::tr("Russian")) {
			bool found = false;
			for (auto it = _newAchievement.names.begin(); it != _newAchievement.names.end(); ++it) {
				if (it->language == "Russian") {
					if (_nameEdit->text().isEmpty()) {
						_newAchievement.names.erase(it);
					} else {
						it->text = q2s(_nameEdit->text());
					}
					found = true;
					break;
				}
			}
			if (!found && !_nameEdit->text().isEmpty()) {
				common::TranslatedText tt;
				tt.language = "Russian";
				tt.text = q2s(_nameEdit->text());
				_newAchievement.names.push_back(tt);
			}
		}
		bool found = false;
		for (auto it = _newAchievement.names.begin(); it != _newAchievement.names.end(); ++it) {
			if (it->language == "Deutsch" && language == QApplication::tr("German")) {
				_nameEdit->setText(s2q(it->text));
				found = true;
				break;
			} else if (it->language == "English" && language == QApplication::tr("English")) {
				_nameEdit->setText(s2q(it->text));
				found = true;
				break;
			} else if (it->language == "Polish" && language == QApplication::tr("Polish")) {
				_nameEdit->setText(s2q(it->text));
				found = true;
				break;
			} else if (it->language == "Russian" && language == QApplication::tr("Russian")) {
				_nameEdit->setText(s2q(it->text));
				found = true;
				break;
			}
		}
		if (!found) {
			_nameEdit->setText("");
		}
		_currentNameLanguage = language;
	}

	void AchievementWidget::descriptionLanguageChanged(QString language) {
		if (_currentDescriptionLanguage == QApplication::tr("German")) {
			bool found = false;
			for (auto it = _newAchievement.descriptions.begin(); it != _newAchievement.descriptions.end(); ++it) {
				if (it->language == "Deutsch") {
					if (_descriptionEdit->text().isEmpty()) {
						_newAchievement.descriptions.erase(it);
					} else {
						it->text = q2s(_descriptionEdit->text());
					}
					found = true;
					break;
				}
			}
			if (!found && !_descriptionEdit->text().isEmpty()) {
				common::TranslatedText tt;
				tt.language = "Deutsch";
				tt.text = q2s(_descriptionEdit->text());
				_newAchievement.descriptions.push_back(tt);
			}
		} else if (_currentDescriptionLanguage == QApplication::tr("English")) {
			bool found = false;
			for (auto it = _newAchievement.descriptions.begin(); it != _newAchievement.descriptions.end(); ++it) {
				if (it->language == "English") {
					if (_descriptionEdit->text().isEmpty()) {
						_newAchievement.descriptions.erase(it);
					} else {
						it->text = q2s(_descriptionEdit->text());
					}
					found = true;
					break;
				}
			}
			if (!found && !_descriptionEdit->text().isEmpty()) {
				common::TranslatedText tt;
				tt.language = "English";
				tt.text = q2s(_descriptionEdit->text());
				_newAchievement.descriptions.push_back(tt);
			}
		} else if (_currentDescriptionLanguage == QApplication::tr("Polish")) {
			bool found = false;
			for (auto it = _newAchievement.descriptions.begin(); it != _newAchievement.descriptions.end(); ++it) {
				if (it->language == "Polish") {
					if (_descriptionEdit->text().isEmpty()) {
						_newAchievement.descriptions.erase(it);
					} else {
						it->text = q2s(_descriptionEdit->text());
					}
					found = true;
					break;
				}
			}
			if (!found && !_descriptionEdit->text().isEmpty()) {
				common::TranslatedText tt;
				tt.language = "Polish";
				tt.text = q2s(_descriptionEdit->text());
				_newAchievement.descriptions.push_back(tt);
			}
		} else if (_currentDescriptionLanguage == QApplication::tr("Russian")) {
			bool found = false;
			for (auto it = _newAchievement.descriptions.begin(); it != _newAchievement.descriptions.end(); ++it) {
				if (it->language == "Russian") {
					if (_descriptionEdit->text().isEmpty()) {
						_newAchievement.descriptions.erase(it);
					} else {
						it->text = q2s(_descriptionEdit->text());
					}
					found = true;
					break;
				}
			}
			if (!found && !_descriptionEdit->text().isEmpty()) {
				common::TranslatedText tt;
				tt.language = "Russian";
				tt.text = q2s(_descriptionEdit->text());
				_newAchievement.descriptions.push_back(tt);
			}
		}
		bool found = false;
		for (auto it = _newAchievement.descriptions.begin(); it != _newAchievement.descriptions.end(); ++it) {
			if (it->language == "Deutsch" && language == QApplication::tr("German")) {
				_descriptionEdit->setText(s2q(it->text));
				found = true;
				break;
			} else if (it->language == "English" && language == QApplication::tr("English")) {
				_descriptionEdit->setText(s2q(it->text));
				found = true;
				break;
			} else if (it->language == "Polish" && language == QApplication::tr("Polish")) {
				_descriptionEdit->setText(s2q(it->text));
				found = true;
				break;
			} else if (it->language == "Russian" && language == QApplication::tr("Russian")) {
				_descriptionEdit->setText(s2q(it->text));
				found = true;
				break;
			}
		}
		if (!found) {
			_descriptionEdit->setText("");
		}
		_currentDescriptionLanguage = language;
	}

	void AchievementWidget::openLockedImage() {
		QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectLockedAchievementImage"), _lockedImagePath->text(), "Image (*.png *.jpg)");
		if (!path.isEmpty()) {
			changedLockedImage(path);
		}
	}

	void AchievementWidget::openUnlockedImage() {
		QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectUnlockedAchievementImage"), _unlockedImagePath->text(), "Image (*.png *.jpg)");
		if (!path.isEmpty()) {
			changedUnlockedImage(path);
		}
	}

	void AchievementWidget::changedLockedImage(QString path) {
		QPixmap achievementPixmap(path);
		if (path.isEmpty() && _unlockedImagePath && !_unlockedImagePath->text().isEmpty()) {
			achievementPixmap = QPixmap(_unlockedImagePath->text());
			if (!achievementPixmap.isNull()) {
				QImage img = achievementPixmap.toImage();
				unsigned int * d = reinterpret_cast<unsigned int*>(img.bits());
				const int pixelCount = img.width() * img.height();

				// Convert each pixel to grayscale
				for (int i = 0; i < pixelCount; ++i) {
					const int val = qGray(*d);
					*d = qRgba(val, val, val, qAlpha(*d));
					++d;
				}
				achievementPixmap = QPixmap::fromImage(img);
			}
		}
		if (achievementPixmap.isNull()) {
			achievementPixmap = QPixmap(":/Achievement_Locked.png");
		}
		const QPixmap pixmap = achievementPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
		_lockedImage->setPixmap(pixmap);

		if (!path.isEmpty() && !QPixmap(path).isNull()) {
			if (!path.contains(s2q(_newAchievement.lockedImageName) + ".z")) {
				// compress => add to data => delete
				QFile uncompressedFile(path);
				if (uncompressedFile.open(QIODevice::ReadOnly)) {
					QCryptographicHash hash(QCryptographicHash::Sha512);
					hash.addData(&uncompressedFile);
					QString hashSum = QString::fromLatin1(hash.result().toHex());
					uncompressedFile.close();

					{
						std::ifstream fileToCompress(q2s(path), std::ios_base::in | std::ios_base::binary);
						boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
						in.push(boost::iostreams::zlib_compressor(boost::iostreams::zlib::best_compression));
						in.push(fileToCompress);
						std::ofstream compressedFile(q2s(path) + ".z", std::ios_base::out | std::ios_base::binary);
						boost::iostreams::copy(in, compressedFile);
					}

					QFile compressedFile(path + ".z");
					if (compressedFile.open(QIODevice::ReadOnly)) {
						QByteArray byteArr = compressedFile.readAll();
						std::vector<uint8_t> buffer(byteArr.length());
						memcpy(&buffer[0], byteArr.data(), byteArr.length());
						_newAchievement.lockedImageName = q2s(QFileInfo(path).fileName()) + ".z";
						_newAchievement.lockedImageHash = q2s(hashSum);
						_newAchievement.lockedImageData = buffer;
					}
					compressedFile.close();
					compressedFile.remove();
				}
			}
		} else {
			_newAchievement.lockedImageName = "";
			_newAchievement.lockedImageHash = "";
			_newAchievement.lockedImageData.clear();
		}
	}

	void AchievementWidget::changedUnlockedImage(QString path) {
		if (_lockedImagePath->text().isEmpty() && !path.isEmpty()) {
			QPixmap lockedAchievementPixmap = QPixmap(path);
			if (!lockedAchievementPixmap.isNull()) {
				QImage img = lockedAchievementPixmap.toImage();
				unsigned int * d = reinterpret_cast<unsigned int*>(img.bits());
				const int pixelCount = img.width() * img.height();

				// Convert each pixel to grayscale
				for (int i = 0; i < pixelCount; ++i) {
					const int val = qGray(*d);
					*d = qRgba(val, val, val, qAlpha(*d));
					++d;
				}
				lockedAchievementPixmap = QPixmap::fromImage(img);
			}
			_lockedImage->setPixmap(lockedAchievementPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
		}
		QPixmap achievementPixmap(path);
		if (achievementPixmap.isNull()) {
			achievementPixmap = QPixmap(":/Achievement_Unlocked.png");
		}
		const QPixmap pixmap = achievementPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
		_unlockedImage->setPixmap(pixmap);

		if (!path.isEmpty() && !QPixmap(path).isNull()) {
			if (!path.contains(s2q(_newAchievement.unlockedImageName) + ".z")) {
				// compress => add to data => delete
				QFile uncompressedFile(path);
				if (uncompressedFile.open(QIODevice::ReadOnly)) {
					QCryptographicHash hash(QCryptographicHash::Sha512);
					hash.addData(&uncompressedFile);
					QString hashSum = QString::fromLatin1(hash.result().toHex());
					uncompressedFile.close();

					{
						std::ifstream fileToCompress(q2s(path), std::ios_base::in | std::ios_base::binary);
						boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
						in.push(boost::iostreams::zlib_compressor(boost::iostreams::zlib::best_compression));
						in.push(fileToCompress);
						std::ofstream compressedFile(q2s(path) + ".z", std::ios_base::out | std::ios_base::binary);
						boost::iostreams::copy(in, compressedFile);
					}

					QFile compressedFile(path + ".z");
					if (compressedFile.open(QIODevice::ReadOnly)) {
						QByteArray byteArr = compressedFile.readAll();
						std::vector<uint8_t> buffer(byteArr.length());
						memcpy(&buffer[0], byteArr.data(), byteArr.length());
						_newAchievement.unlockedImageName = q2s(QFileInfo(path).fileName()) + ".z";
						_newAchievement.unlockedImageHash = q2s(hashSum);
						_newAchievement.unlockedImageData = buffer;
					}
					compressedFile.close();
					compressedFile.remove();
				}
			}
		} else {
			_newAchievement.unlockedImageName = "";
			_newAchievement.unlockedImageHash = "";
			_newAchievement.unlockedImageData.clear();
		}
	}

} /* namespace widgets */
} /* namespace spine */
