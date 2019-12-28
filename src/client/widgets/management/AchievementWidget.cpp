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

#include "widgets/management/AchievementWidget.h"

#include "Config.h"
#include "FileDownloader.h"

#include "utils/Compression.h"
#include "utils/Hashing.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace spine {
namespace client {
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

	void AchievementWidget::setAchievement(int32_t modID, ManagementAchievement achievement) {
		_currentNameLanguage.clear();
		_currentDescriptionLanguage.clear();

		_achievement = achievement;
		_newAchievement = achievement;

		nameLanguageChanged(QApplication::tr("German"));
		descriptionLanguageChanged(QApplication::tr("German"));

		_hiddenBox->setChecked(_newAchievement.hidden);
		_progressBox->setValue(_newAchievement.maxProgress);

		QString lockedImage = _newAchievement.lockedImageName;
		lockedImage.chop(2);
		lockedImage.prepend(Config::MODDIR + "/mods/" + QString::number(modID) + "/achievements/");

		if (QFileInfo::exists(lockedImage) || _newAchievement.lockedImageName.isEmpty()) {
			changedLockedImage(lockedImage);
		} else {
			const QString fileName = _newAchievement.lockedImageName;
			FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/mods/" + QString::number(modID) + "/achievements/" + fileName), Config::MODDIR + "/mods/" + QString::number(modID) + "/achievements/", fileName, _newAchievement.lockedImageHash, this);
			connect(fd, &FileDownloader::downloadSucceeded, [=]() {
				changedLockedImage(lockedImage);
				fd->deleteLater();
			});
			connect(fd, &FileDownloader::downloadFailed, fd, &FileDownloader::deleteLater);
			fd->startDownload();
		}

		QString unlockedImage = _newAchievement.unlockedImageName;
		unlockedImage.chop(2);
		unlockedImage.prepend(Config::MODDIR + "/mods/" + QString::number(modID) + "/achievements/");

		if (QFileInfo::exists(unlockedImage) || _newAchievement.unlockedImageName.isEmpty()) {
			changedUnlockedImage(unlockedImage);
		} else {
			const QString fileName = _newAchievement.unlockedImageName;
			FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/mods/" + QString::number(modID) + "/achievements/" + fileName), Config::MODDIR + "/mods/" + QString::number(modID) + "/achievements/", fileName, _newAchievement.unlockedImageHash, this);
			connect(fd, &FileDownloader::downloadSucceeded, [=]() {
				changedUnlockedImage(unlockedImage);
				fd->deleteLater();
			});
			connect(fd, &FileDownloader::downloadFailed, fd, &FileDownloader::deleteLater);
			fd->startDownload();
		}
	}

	ManagementAchievement AchievementWidget::getAchievement() {
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
						it->text = _nameEdit->text();
					}
					found = true;
					break;
				}
			}
			if (!found && !_nameEdit->text().isEmpty()) {
				ManagementTranslation tt;
				tt.language = "Deutsch";
				tt.text = _nameEdit->text();
				_newAchievement.names.push_back(tt);
			}
		} else if (_currentNameLanguage == QApplication::tr("English")) {
			bool found = false;
			for (auto it = _newAchievement.names.begin(); it != _newAchievement.names.end(); ++it) {
				if (it->language == "English") {
					if (_nameEdit->text().isEmpty()) {
						_newAchievement.names.erase(it);
					} else {
						it->text = _nameEdit->text();
					}
					found = true;
					break;
				}
			}
			if (!found && !_nameEdit->text().isEmpty()) {
				ManagementTranslation tt;
				tt.language = "English";
				tt.text = _nameEdit->text();
				_newAchievement.names.push_back(tt);
			}
		} else if (_currentNameLanguage == QApplication::tr("Polish")) {
			bool found = false;
			for (auto it = _newAchievement.names.begin(); it != _newAchievement.names.end(); ++it) {
				if (it->language == "Polish") {
					if (_nameEdit->text().isEmpty()) {
						_newAchievement.names.erase(it);
					} else {
						it->text = _nameEdit->text();
					}
					found = true;
					break;
				}
			}
			if (!found && !_nameEdit->text().isEmpty()) {
				ManagementTranslation tt;
				tt.language = "Polish";
				tt.text = _nameEdit->text();
				_newAchievement.names.push_back(tt);
			}
		} else if (_currentNameLanguage == QApplication::tr("Russian")) {
			bool found = false;
			for (auto it = _newAchievement.names.begin(); it != _newAchievement.names.end(); ++it) {
				if (it->language == "Russian") {
					if (_nameEdit->text().isEmpty()) {
						_newAchievement.names.erase(it);
					} else {
						it->text = _nameEdit->text();
					}
					found = true;
					break;
				}
			}
			if (!found && !_nameEdit->text().isEmpty()) {
				ManagementTranslation tt;
				tt.language = "Russian";
				tt.text = _nameEdit->text();
				_newAchievement.names.push_back(tt);
			}
		}
		bool found = false;
		for (auto & name : _newAchievement.names) {
			if (name.language == "Deutsch" && language == QApplication::tr("German")) {
				_nameEdit->setText(name.text);
				found = true;
				break;
			} else if (name.language == "English" && language == QApplication::tr("English")) {
				_nameEdit->setText(name.text);
				found = true;
				break;
			} else if (name.language == "Polish" && language == QApplication::tr("Polish")) {
				_nameEdit->setText(name.text);
				found = true;
				break;
			} else if (name.language == "Russian" && language == QApplication::tr("Russian")) {
				_nameEdit->setText(name.text);
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
						it->text = _descriptionEdit->text();
					}
					found = true;
					break;
				}
			}
			if (!found && !_descriptionEdit->text().isEmpty()) {
				ManagementTranslation tt;
				tt.language = "Deutsch";
				tt.text = _descriptionEdit->text();
				_newAchievement.descriptions.push_back(tt);
			}
		} else if (_currentDescriptionLanguage == QApplication::tr("English")) {
			bool found = false;
			for (auto it = _newAchievement.descriptions.begin(); it != _newAchievement.descriptions.end(); ++it) {
				if (it->language == "English") {
					if (_descriptionEdit->text().isEmpty()) {
						_newAchievement.descriptions.erase(it);
					} else {
						it->text = _descriptionEdit->text();
					}
					found = true;
					break;
				}
			}
			if (!found && !_descriptionEdit->text().isEmpty()) {
				ManagementTranslation tt;
				tt.language = "English";
				tt.text = _descriptionEdit->text();
				_newAchievement.descriptions.push_back(tt);
			}
		} else if (_currentDescriptionLanguage == QApplication::tr("Polish")) {
			bool found = false;
			for (auto it = _newAchievement.descriptions.begin(); it != _newAchievement.descriptions.end(); ++it) {
				if (it->language == "Polish") {
					if (_descriptionEdit->text().isEmpty()) {
						_newAchievement.descriptions.erase(it);
					} else {
						it->text = _descriptionEdit->text();
					}
					found = true;
					break;
				}
			}
			if (!found && !_descriptionEdit->text().isEmpty()) {
				ManagementTranslation tt;
				tt.language = "Polish";
				tt.text = _descriptionEdit->text();
				_newAchievement.descriptions.push_back(tt);
			}
		} else if (_currentDescriptionLanguage == QApplication::tr("Russian")) {
			bool found = false;
			for (auto it = _newAchievement.descriptions.begin(); it != _newAchievement.descriptions.end(); ++it) {
				if (it->language == "Russian") {
					if (_descriptionEdit->text().isEmpty()) {
						_newAchievement.descriptions.erase(it);
					} else {
						it->text = _descriptionEdit->text();
					}
					found = true;
					break;
				}
			}
			if (!found && !_descriptionEdit->text().isEmpty()) {
				ManagementTranslation tt;
				tt.language = "Russian";
				tt.text = _descriptionEdit->text();
				_newAchievement.descriptions.push_back(tt);
			}
		}
		bool found = false;
		for (const auto & description : _newAchievement.descriptions) {
			if (description.language == "Deutsch" && language == QApplication::tr("German")) {
				_descriptionEdit->setText(description.text);
				found = true;
				break;
			} else if (description.language == "English" && language == QApplication::tr("English")) {
				_descriptionEdit->setText(description.text);
				found = true;
				break;
			} else if (description.language == "Polish" && language == QApplication::tr("Polish")) {
				_descriptionEdit->setText(description.text);
				found = true;
				break;
			} else if (description.language == "Russian" && language == QApplication::tr("Russian")) {
				_descriptionEdit->setText(description.text);
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
		const QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectLockedAchievementImage"), _lockedImagePath->text(), "Image (*.png *.jpg)");
		if (!path.isEmpty()) {
			changedLockedImage(path);
		}
	}

	void AchievementWidget::openUnlockedImage() {
		const QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectUnlockedAchievementImage"), _unlockedImagePath->text(), "Image (*.png *.jpg)");
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
			if (!path.contains(_newAchievement.lockedImageName + ".z")) {
				// compress => add to data => delete
				QString hashSum;
				const bool b = utils::Hashing::hash(path, hashSum);
				if (b) {
					utils::Compression::compress(path, false);

					QFile compressedFile(path + ".z");
					if (compressedFile.open(QIODevice::ReadOnly)) {
						QByteArray byteArr = compressedFile.readAll();
						std::vector<uint8_t> buffer(byteArr.length());
						memcpy(&buffer[0], byteArr.data(), byteArr.length());
						_newAchievement.lockedImageName = QFileInfo(path).fileName() + ".z";
						_newAchievement.lockedImageHash = hashSum;
						_newAchievement.lockedImageData = buffer;
					}
					compressedFile.close();
					compressedFile.remove();
				}
			}

			_lockedImagePath->setText(path);
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
			if (!path.contains(_newAchievement.unlockedImageName + ".z")) {
				// compress => add to data => delete
				QString hashSum;
				const bool b = utils::Hashing::hash(path, hashSum);
				if (b) {
					utils::Compression::compress(path, false);

					QFile compressedFile(path + ".z");
					if (compressedFile.open(QIODevice::ReadOnly)) {
						QByteArray byteArr = compressedFile.readAll();
						std::vector<uint8_t> buffer(byteArr.length());
						memcpy(&buffer[0], byteArr.data(), byteArr.length());
						_newAchievement.unlockedImageName = QFileInfo(path).fileName() + ".z";
						_newAchievement.unlockedImageHash = hashSum;
						_newAchievement.unlockedImageData = buffer;
					}
					compressedFile.close();
					compressedFile.remove();
				}
			}

			_unlockedImagePath->setText(path);
		} else {
			_newAchievement.unlockedImageName = "";
			_newAchievement.unlockedImageHash = "";
			_newAchievement.unlockedImageData.clear();
		}
	}

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
