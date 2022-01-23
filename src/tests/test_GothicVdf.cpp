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
// Copyright 2022 Clockwork Origins

#include <QApplication>
#include <QFile>

#include "utils/GothicVdf.h"

#include "gtest/gtest.h"

#include <QTemporaryFile>

using namespace spine::utils;

class GothicVdfTest : public ::testing::Test {
protected:
	
};

TEST_F(GothicVdfTest, parse) {
	GothicVdf vdf(VDF_PATH);
	const auto b = vdf.parse();

	ASSERT_TRUE(b);

	vdf.close();
}

TEST_F(GothicVdfTest, getFiles) {
	GothicVdf vdf(VDF_PATH);
	const auto b = vdf.parse();

	ASSERT_TRUE(b);

	const auto files = vdf.getFiles();

	ASSERT_EQ(2, files.count());

	ASSERT_EQ("_WORK/DATA/TEXTURES/_COMPILED/ACHIEVEMENT_BACKGROUND-C.TEX", files[0]);
	ASSERT_EQ("_WORK/DATA/TEXTURES/_COMPILED/SPINE_ACHIEVEMENT_DEFAULT-C.TEX", files[1]);

	vdf.close();
}

TEST_F(GothicVdfTest, getHash) {
	GothicVdf vdf(VDF_PATH);
	const auto b = vdf.parse();

	ASSERT_TRUE(b);

	const auto files = vdf.getFiles();

	ASSERT_EQ(2, files.count());

	ASSERT_EQ("c749220d8d4869dd01a50e7df38b51ffe299056b140b6e5ea2b8ccad1fd091206323d476a74f1866159e7ac0519b94a4677ec6fbd48a09254500cfe3a450ba73", vdf.getHash(0));
	ASSERT_EQ("cd4aca152019f2f77b8dd052c6a2bc3f816b28f0183315c823968ff734750a8251622d074909f8ed334b9706375ab9c7b9f137e96437889fdfc7d4a278dc8c0e", vdf.getHash(1));

	vdf.close();
}

TEST_F(GothicVdfTest, write) {
	GothicVdf vdf(VDF_PATH);
	vdf.parse();

	vdf.write("export.vdf");

	GothicVdf vdf2("export.vdf");
	const auto b = vdf2.parse();

	ASSERT_TRUE(b);

	const auto files = vdf2.getFiles();

	ASSERT_EQ(2, files.count());

	ASSERT_EQ("_WORK/DATA/TEXTURES/_COMPILED/ACHIEVEMENT_BACKGROUND-C.TEX", files[0]);
	ASSERT_EQ("_WORK/DATA/TEXTURES/_COMPILED/SPINE_ACHIEVEMENT_DEFAULT-C.TEX", files[1]);

	ASSERT_EQ("c749220d8d4869dd01a50e7df38b51ffe299056b140b6e5ea2b8ccad1fd091206323d476a74f1866159e7ac0519b94a4677ec6fbd48a09254500cfe3a450ba73", vdf2.getHash(0));
	ASSERT_EQ("cd4aca152019f2f77b8dd052c6a2bc3f816b28f0183315c823968ff734750a8251622d074909f8ed334b9706375ab9c7b9f137e96437889fdfc7d4a278dc8c0e", vdf2.getHash(1));

	vdf2.close();

	QFile::remove("export.vdf");
}

TEST_F(GothicVdfTest, remove) {
	GothicVdf vdf(VDF_PATH);
	vdf.parse();

	vdf.remove("_WORK/DATA/TEXTURES/_COMPILED/SPINE_ACHIEVEMENT_DEFAULT-C.TEX");

	vdf.write("export.vdf");

	GothicVdf vdf2("export.vdf");
	const auto b = vdf2.parse();

	ASSERT_TRUE(b);

	const auto files = vdf2.getFiles();

	ASSERT_EQ(1, files.count());

	ASSERT_EQ("_WORK/DATA/TEXTURES/_COMPILED/ACHIEVEMENT_BACKGROUND-C.TEX", files[0]);

	ASSERT_EQ("c749220d8d4869dd01a50e7df38b51ffe299056b140b6e5ea2b8ccad1fd091206323d476a74f1866159e7ac0519b94a4677ec6fbd48a09254500cfe3a450ba73", vdf2.getHash(0));

	vdf2.close();

	QFile::remove("export.vdf");
}

TEST_F(GothicVdfTest, writeMoreComplex) {
	GothicVdf vdf(QString(TEST_RESOURCES_PATH) + "/SpineTest.mod");
	vdf.parse();

	vdf.write("export.vdf");

	GothicVdf vdf2("export.vdf");
	const auto b = vdf2.parse();

	ASSERT_TRUE(b);

	const auto files = vdf2.getFiles();

	ASSERT_EQ(3, files.count());

	ASSERT_EQ("_WORK/DATA/SCRIPTS/CONTENT/CUTSCENE/OU.BIN", files[0]);
	ASSERT_EQ("_WORK/DATA/SCRIPTS/_COMPILED/GOTHIC.DAT", files[1]);
	ASSERT_EQ("_WORK/DATA/SCRIPTS/_COMPILED/MENU.DAT", files[2]);

	vdf2.close();

	QFile::remove("export.vdf");
}
