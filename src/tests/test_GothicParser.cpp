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

#ifdef WITH_TRANSLATOR

#include "translator/GothicParser.h"

#include "utils/Conversion.h"

#include "translator/common/TranslationModel.h"
#include "translator/api/TranslatorAPI.h"

#include "gtest/gtest.h"

#include <QFileInfo>
#include <QTextStream>

class GothicParserTest : public ::testing::Test {
protected:
	void SetUp() override {
		model = translator::api::TranslatorAPI::createModel("", "", "");
	}

	void TearDown() override {
		delete model;
		model = nullptr;
	}

	void loadData(QString folder) {
		{
			ASSERT_TRUE(QFileInfo::exists("tests/" + folder + "/names.txt"));
			QFile namesFile("tests/" + folder + "/names.txt");
			const bool b = namesFile.open(QIODevice::ReadOnly);
			ASSERT_TRUE(b);
			QTextStream nameStream(&namesFile);
			while (!nameStream.atEnd()) {
				const QString line = nameStream.readLine();
				names.push_back(q2s(line));
			}
		}
		{
			ASSERT_TRUE(QFileInfo::exists("tests/" + folder + "/texts.txt"));
			QFile textsFile("tests/" + folder + "/texts.txt");
			const bool b = textsFile.open(QIODevice::ReadOnly);
			ASSERT_TRUE(b);
			QTextStream textStream(&textsFile);
			while (!textStream.atEnd()) {
				const QString line = textStream.readLine();
				texts.push_back(q2s(line));
			}
		}
		{
			ASSERT_TRUE(QFileInfo::exists("tests/" + folder + "/dialogs.txt"));
			QFile dialogsFile("tests/" + folder + "/dialogs.txt");
			const bool b = dialogsFile.open(QIODevice::ReadOnly);
			ASSERT_TRUE(b);
			QTextStream dialogStream(&dialogsFile);
			std::vector<std::string> dialog;
			while (!dialogStream.atEnd()) {
				QString line = dialogStream.readLine();
				if (line.isEmpty() && !dialog.empty()) {
					dialogs.push_back(dialog);
					dialog.clear();
				} else {
					dialog.push_back(q2s(line));
				}
			}
			if (!dialog.empty()) {
				dialogs.push_back(dialog);
			}
		}
	}

	void parseScript(QString folder) {
		spine::translation::GothicParser gp(nullptr);
		gp.parseFile("tests/" + folder + "/test.d", model);
	}

	void compare() {
		ASSERT_EQ(names.size(), model->getNames().size());
		if (texts.size() != model->getTexts().size()) {
			for (size_t i = 0; i < std::min(texts.size(), model->getTexts().size()); i++) {
				std::cout << texts[i] << " vs. " << model->getTexts()[i] << std::endl;
			}
		}
		ASSERT_EQ(texts.size(), model->getTexts().size());
		ASSERT_EQ(dialogs.size(), model->getDialogs().size());

		for (size_t i = 0; i < names.size(); i++) {
			EXPECT_EQ(names[i], model->getNames()[i]);
		}

		for (size_t i = 0; i < texts.size(); i++) {
			EXPECT_EQ(texts[i], model->getTexts()[i]);
		}

		for (size_t i = 0; i < dialogs.size(); i++) {
			ASSERT_EQ(dialogs[i].size(), model->getDialogs()[i].size());
			for (size_t j = 0; j < dialogs[i].size(); j++) {
				EXPECT_EQ(dialogs[i][j], model->getDialogs()[i][j]);
			}
		}
	}

	void performTest(QString folder) {
		loadData(folder);
		parseScript(folder);
		compare();
	}

	translator::common::TranslationModel * model = nullptr;
	std::vector<std::string> names;
	std::vector<std::string> texts;
	std::vector<std::vector<std::string>> dialogs;
};

TEST_F(GothicParserTest, Test1) {
	performTest("test1");
}

TEST_F(GothicParserTest, Test2) {
	performTest("test2");
}

TEST_F(GothicParserTest, Test3) {
	performTest("test3");
}

TEST_F(GothicParserTest, Test4) {
	performTest("test4");
}

TEST_F(GothicParserTest, Test5) {
	performTest("test5");
}

#endif /* WITH_TRANSLATOR */
