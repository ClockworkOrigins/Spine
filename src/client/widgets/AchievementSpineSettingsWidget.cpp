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

#include "widgets/AchievementSpineSettingsWidget.h"

#include "Config.h"
#include "UpdateLanguage.h"

#include "models/SpineEditorModel.h"

#include "widgets/AchievementOrientationPreview.h"
#include "widgets/GeneralSettingsWidget.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	AchievementSpineSettingsWidget::AchievementSpineSettingsWidget(GeneralSettingsWidget * generalSettingsWidget, models::SpineEditorModel * model, QWidget * par) : QWidget(par), _model(model), _generalSettingsWidget(generalSettingsWidget), _orientationLabel(nullptr), _orientationComboBox(nullptr), _displayDurationLabel(nullptr), _displayDurationSpinBox(nullptr), _achievements() {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		{
			QHBoxLayout * hl = new QHBoxLayout();
			_orientationLabel = new QLabel(QApplication::tr("AchievementOrientation"), this);
			UPDATELANGUAGESETTEXT(generalSettingsWidget, _orientationLabel, "AchievementOrientation");
			_orientationComboBox = new QComboBox(this);
			_orientationComboBox->addItem(QApplication::tr("TopLeft"));
			UPDATELANGUAGESETITEMTEXT(generalSettingsWidget, _orientationComboBox, 0, "TopLeft");
			_orientationComboBox->addItem(QApplication::tr("TopRight"));
			UPDATELANGUAGESETITEMTEXT(generalSettingsWidget, _orientationComboBox, 1, "TopRight");
			_orientationComboBox->addItem(QApplication::tr("BottomLeft"));
			UPDATELANGUAGESETITEMTEXT(generalSettingsWidget, _orientationComboBox, 2, "BottomLeft");
			_orientationComboBox->addItem(QApplication::tr("BottomRight"));
			UPDATELANGUAGESETITEMTEXT(generalSettingsWidget, _orientationComboBox, 3, "BottomRight");

			QPushButton * orientationPreviewButton = new QPushButton(QApplication::tr("Preview"), this);
			connect(orientationPreviewButton, &QPushButton::released, this, &AchievementSpineSettingsWidget::showOrientationPreview);

			hl->addWidget(_orientationLabel);
			hl->addWidget(_orientationComboBox);
			hl->addWidget(orientationPreviewButton);

			l->addLayout(hl);
		}

		{
			QHBoxLayout * hl = new QHBoxLayout();
			_displayDurationLabel = new QLabel(QApplication::tr("DisplayDuration"), this);
			UPDATELANGUAGESETTEXT(generalSettingsWidget, _displayDurationLabel, "DisplayDuration");
			_displayDurationSpinBox = new QSpinBox(this);
			_displayDurationSpinBox->setMinimum(1000);
			_displayDurationSpinBox->setMaximum(60000);
			_displayDurationSpinBox->setValue(5000);

			hl->addWidget(_displayDurationLabel);
			hl->addWidget(_displayDurationSpinBox);

			l->addLayout(hl);
		}

		QScrollArea * achievementScrollArea = new QScrollArea(this);
		QWidget * achievementsWidget = new QWidget(achievementScrollArea);
		_scrollLayout = new QVBoxLayout();
		_scrollLayout->setAlignment(Qt::AlignTop);
		achievementsWidget->setLayout(_scrollLayout);
		achievementScrollArea->setWidget(achievementsWidget);
		achievementScrollArea->setWidgetResizable(true);
		achievementsWidget->setProperty("achievementEditor", true);

		l->addWidget(achievementScrollArea);

		setLayout(l);

		addNewAchievement();
	}

	AchievementSpineSettingsWidget::~AchievementSpineSettingsWidget() {
	}

	void AchievementSpineSettingsWidget::save() {
		_model->setAchievementOrientation(models::AchievementOrientation(_orientationComboBox->currentIndex()));
		_model->setAchievementDisplayDuration(_displayDurationSpinBox->value());
		QList<models::AchievementModel> achievements;
		for (Achievement a : _achievements) {
			models::AchievementModel am;
			am.name = a.nameLineEdit->text();
			am.description = a.descriptionLineEdit->text();
			am.lockedImage = QFileInfo(a.lockedImageLineEdit->text()).fileName();
			am.unlockedImage = QFileInfo(a.unlockedImageLineEdit->text()).fileName();
			if (am.unlockedImage.isEmpty()) {
				am.unlockedImage = "SPINE_ACHIEVEMENT_DEFAULT.TGA";
			} else {
				am.unlockedImage = am.unlockedImage.replace(".tga", ".TGA", Qt::CaseSensitivity::CaseInsensitive);
			}
			am.hidden = a.hiddenCheckBox->isChecked();
			am.maxProgress = a.progressBox->value();
			achievements.append(am);
		}
		if (!achievements.empty() && achievements.back().name.isEmpty()) {
			achievements.pop_back(); // last one is always empty
		}
		_model->setAchievements(achievements);
	}

	void AchievementSpineSettingsWidget::addNewAchievement() {
		QPushButton * pb = qobject_cast<QPushButton *>(sender());
		Achievement s;
		s.layout = new QVBoxLayout();
		s.nameLineEdit = new QLineEdit(this);
		s.nameLineEdit->setPlaceholderText(QApplication::tr("Name"));
		UPDATELANGUAGESETPLACEHOLDERTEXT(_generalSettingsWidget, s.nameLineEdit, "Name");
		s.descriptionLineEdit = new QLineEdit(this);
		s.descriptionLineEdit->setPlaceholderText(QApplication::tr("Description"));
		UPDATELANGUAGESETPLACEHOLDERTEXT(_generalSettingsWidget, s.descriptionLineEdit, "Description");
		s.lockedImageLineEdit = new QLineEdit(this);
		s.lockedImageLineEdit->setPlaceholderText(QApplication::tr("LockedImage"));
		UPDATELANGUAGESETPLACEHOLDERTEXT(_generalSettingsWidget, s.lockedImageLineEdit, "LockedImage");
		s.lockedImageButton = new QPushButton("...", this);
		s.unlockedImageLineEdit = new QLineEdit(this);
		s.unlockedImageLineEdit->setPlaceholderText(QApplication::tr("UnlockedImage"));
		UPDATELANGUAGESETPLACEHOLDERTEXT(_generalSettingsWidget, s.unlockedImageLineEdit, "UnlockedImage");
		s.unlockedImageButton = new QPushButton("...", this);
		s.hiddenCheckBox = new QCheckBox(QApplication::tr("Hidden"), this);
		UPDATELANGUAGESETTEXT(_generalSettingsWidget, s.hiddenCheckBox, "Hidden");
		s.addButton = new QPushButton("+", this);
		s.removeButton = new QPushButton("-", this);
		s.previewLockedImage = new QLabel(this);
		s.previewLockedImage->setFixedSize(64, 64);
		s.previewUnlockedImage = new QLabel(this);
		s.previewUnlockedImage->setFixedSize(64, 64);
		s.progressLabel = new QLabel(QApplication::tr("ProgressCounter"), this);
		UPDATELANGUAGESETTEXT(_generalSettingsWidget, s.progressLabel, "ProgressCounter");
		s.progressLabel->setToolTip(QApplication::tr("ProgressCounterTooltip"));
		UPDATELANGUAGESETTOOLTIP(_generalSettingsWidget, s.progressLabel, "ProgressCounterTooltip");
		s.progressBox = new QSpinBox(this);
		s.progressBox->setMinimumWidth(100);
		s.progressBox->setToolTip(QApplication::tr("ProgressCounterTooltip"));
		UPDATELANGUAGESETTOOLTIP(_generalSettingsWidget, s.progressBox, "ProgressCounterTooltip");
		s.progressBox->setMaximum(1000000);

		const QString iconUnlocked = ":/Achievement_Unlocked.png";
		const QString iconLocked = ":/Achievement_Locked.png";

		{
			QPixmap achievementPixmap(iconLocked);
			if (achievementPixmap.isNull()) {
				achievementPixmap = QPixmap(":/Achievement_Locked.png");
			}
			const QPixmap pixmap = achievementPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
			s.previewLockedImage->setPixmap(pixmap);

			s.previewLockedImage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		}
		{
			QPixmap achievementPixmap(iconUnlocked);
			if (achievementPixmap.isNull()) {
				achievementPixmap = QPixmap(":/Achievement_Unlocked.png");
			}
			const QPixmap pixmap = achievementPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
			s.previewUnlockedImage->setPixmap(pixmap);

			s.previewUnlockedImage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		}

		QHBoxLayout * hl1 = new QHBoxLayout();
		hl1->addWidget(s.nameLineEdit);
		hl1->addWidget(s.descriptionLineEdit);

		QHBoxLayout * hl2 = new QHBoxLayout();
		hl2->addWidget(s.lockedImageLineEdit);
		hl2->addWidget(s.lockedImageButton);
		hl2->addWidget(s.unlockedImageLineEdit);
		hl2->addWidget(s.unlockedImageButton);

		QHBoxLayout * hl3 = new QHBoxLayout();
		hl3->addWidget(s.previewLockedImage);
		hl3->addWidget(s.previewUnlockedImage);

		QHBoxLayout * hl4 = new QHBoxLayout();
		hl4->addWidget(s.hiddenCheckBox);
		hl4->addWidget(s.progressLabel);
		hl4->addWidget(s.progressBox);
		hl4->addWidget(s.removeButton, 1, Qt::AlignRight);
		hl4->addWidget(s.addButton, 0, Qt::AlignRight);

		s.layout->addLayout(hl1);
		s.layout->addLayout(hl2);
		s.layout->addLayout(hl3);
		s.layout->addLayout(hl4);

		connect(s.addButton, SIGNAL(released()), this, SLOT(addNewAchievement()));
		connect(s.removeButton, SIGNAL(released()), this, SLOT(removeAchievement()));
		connect(s.lockedImageButton, SIGNAL(released()), this, SLOT(selectLockedImage()));
		connect(s.unlockedImageButton, SIGNAL(released()), this, SLOT(selectUnlockedImage()));
		connect(s.lockedImageLineEdit, SIGNAL(textChanged(QString)), this, SLOT(changedLockedImagePath()));
		connect(s.unlockedImageLineEdit, SIGNAL(textChanged(QString)), this, SLOT(changedUnlockedImagePath()));

		if (pb) {
			int index = 0;
			for (Achievement oldS : _achievements) {
				if (oldS.addButton == pb) {
					_scrollLayout->insertLayout(index + 4, s.layout);
					_achievements.insert(index + 1, s);
					break;
				} else {
					index++;
				}
			}
		} else {
			_scrollLayout->addLayout(s.layout);
			_achievements.append(s);
		}

		if (_achievements.size() == 1) {
			s.removeButton->setDisabled(true);
		} else {
			_achievements[0].removeButton->setEnabled(true);
		}
	}

	void AchievementSpineSettingsWidget::removeAchievement() {
		QPushButton * pb = qobject_cast<QPushButton *>(sender());
		if (pb) {
			int index = 0;
			for (Achievement s : _achievements) {
				if (s.removeButton == pb) {
					_scrollLayout->removeItem(s.layout);
					_achievements.removeAt(index);
					s.addButton->deleteLater();
					s.layout->deleteLater();
					s.removeButton->deleteLater();
					s.nameLineEdit->deleteLater();
					s.descriptionLineEdit->deleteLater();
					s.lockedImageLineEdit->deleteLater();
					s.lockedImageButton->deleteLater();
					s.unlockedImageLineEdit->deleteLater();
					s.unlockedImageButton->deleteLater();
					s.previewLockedImage->deleteLater();
					s.previewUnlockedImage->deleteLater();
					s.hiddenCheckBox->deleteLater();
					s.progressLabel->deleteLater();
					s.progressBox->deleteLater();
					break;
				} else {
					index++;
				}
			}
		}

		if (_achievements.size() == 1) {
			_achievements[0].removeButton->setDisabled(true);
		}
	}

	void AchievementSpineSettingsWidget::selectLockedImage() {
		QPushButton * pb = qobject_cast<QPushButton *>(sender());
		for (Achievement a : _achievements) {
			if (a.lockedImageButton == pb) {
				const QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectLockedAchievementImage"), a.lockedImageLineEdit->text(), "*.tga");
				a.lockedImageLineEdit->setText(path);
				updateAchievementImages(a);
				break;
			}
		}
	}

	void AchievementSpineSettingsWidget::selectUnlockedImage() {
		QPushButton * pb = qobject_cast<QPushButton *>(sender());
		for (Achievement a : _achievements) {
			if (a.unlockedImageButton == pb) {
				const QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectUnlockedAchievementImage"), a.unlockedImageLineEdit->text(), "*.tga");
				a.unlockedImageLineEdit->setText(path);
				updateAchievementImages(a);
				break;
			}
		}
	}

	void AchievementSpineSettingsWidget::changedLockedImagePath() {
		QLineEdit * le = qobject_cast<QLineEdit *>(sender());
		for (Achievement a : _achievements) {
			if (a.lockedImageLineEdit == le) {
				updateAchievementImages(a);
				break;
			}
		}
	}

	void AchievementSpineSettingsWidget::changedUnlockedImagePath() {
		QLineEdit * le = qobject_cast<QLineEdit *>(sender());
		for (Achievement a : _achievements) {
			if (a.unlockedImageLineEdit == le) {
				updateAchievementImages(a);
				break;
			}
		}
	}

	void AchievementSpineSettingsWidget::updateAchievementImages(Achievement a) {
		QString iconLocked = a.lockedImageLineEdit->text();
		QString iconUnlocked = a.unlockedImageLineEdit->text();

		{
			QPixmap achievementPixmap(iconLocked);
			if (iconLocked.isEmpty() && !iconUnlocked.isEmpty()) {
				achievementPixmap = QPixmap(iconUnlocked);
				if (!achievementPixmap.isNull()) {
					QImage img = achievementPixmap.toImage();
					unsigned int * d = reinterpret_cast<unsigned int *>(img.bits());
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
			a.previewLockedImage->setPixmap(pixmap);
		}
		{
			QPixmap achievementPixmap(iconUnlocked);
			if (achievementPixmap.isNull()) {
				achievementPixmap = QPixmap(":/Achievement_Unlocked.png");
			}
			const QPixmap pixmap = achievementPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
			a.previewUnlockedImage->setPixmap(pixmap);
		}
	}

	void AchievementSpineSettingsWidget::updateFromModel() {
		_orientationComboBox->setCurrentIndex(_model->getAchievementOrientation());
		_displayDurationSpinBox->setValue(_model->getAchievementDisplayDuration());

		clear();
		for (const models::AchievementModel am : _model->getAchievements()) {
			addNewAchievement();
			_achievements.back().nameLineEdit->setText(am.name);
			_achievements.back().descriptionLineEdit->setText(am.description);
			_achievements.back().lockedImageLineEdit->setText(am.lockedImage);
			_achievements.back().unlockedImageLineEdit->setText(am.unlockedImage);
			_achievements.back().hiddenCheckBox->setChecked(am.hidden);
			_achievements.back().progressBox->setValue(am.maxProgress);
			updateAchievementImages(_achievements.back());
		}
		if (_achievements.empty()) {
			addNewAchievement();
		}
	}

	void AchievementSpineSettingsWidget::showOrientationPreview() {
		AchievementOrientationPreview aop(_orientationComboBox->currentIndex(), this);
		aop.exec();
	}

	void AchievementSpineSettingsWidget::clear() {
		for (Achievement a : _achievements) {
			_scrollLayout->removeItem(a.layout);
			delete a.addButton;
			delete a.layout;
			delete a.removeButton;
			delete a.nameLineEdit;
			delete a.descriptionLineEdit;
			delete a.lockedImageLineEdit;
			delete a.lockedImageButton;
			delete a.unlockedImageLineEdit;
			delete a.unlockedImageButton;
			delete a.previewUnlockedImage;
			delete a.previewLockedImage;
			delete a.hiddenCheckBox;
			delete a.progressLabel;
			delete a.progressBox;
		}
		_achievements.clear();
	}

} /* namespace widgets */
} /* namespace spine */
