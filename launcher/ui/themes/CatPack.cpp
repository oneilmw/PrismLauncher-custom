// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (c) 2023 Trial97 <alexandru.tripon97@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "ui/themes/CatPack.h"
#include <qdatetime.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qobject.h>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include "FileSystem.h"
#include "Json.h"
#include "ui/themes/ThemeManager.h"

QString BasicCatPack::path()
{
    const QDateTime now = QDateTime::currentDateTime();
    const QDateTime birthday(QDate(now.date().year(), 11, 30), QTime(0, 0));
    const QDateTime xmas(QDate(now.date().year(), 12, 25), QTime(0, 0));
    const QDateTime halloween(QDate(now.date().year(), 10, 31), QTime(0, 0));

    QString cat = QString(":/backgrounds/%1").arg(m_id);
    if (std::abs(now.daysTo(xmas)) <= 4) {
        cat += "-xmas";
    } else if (std::abs(now.daysTo(halloween)) <= 4) {
        cat += "-spooky";
    } else if (std::abs(now.daysTo(birthday)) <= 12) {
        cat += "-bday";
    }
    return cat;
}

JsonCatPack::JsonCatPack(QFileInfo& manifestInfo) : BasicCatPack(manifestInfo.dir().dirName())
{
    QString path = FS::PathCombine("catpacks", m_id);

    if (!FS::ensureFolderPathExists(path)) {
        themeWarningLog() << "couldn't create folder for catpack!";
        return;
    }

    if (manifestInfo.exists() && manifestInfo.isFile()) {
        try {
            auto doc = Json::requireDocument(manifestInfo.absoluteFilePath(), "CatPack JSON file");
            const auto root = doc.object();
            m_name = Json::requireString(root, "name", "Catpack name");
            auto id = Json::ensureString(root, "id", "", "Catpack ID");
            if (!id.isEmpty())
                m_id = id;
            m_defaultPath = FS::PathCombine(path, Json::requireString(root, "default", "Deafult Cat"));
            auto variants = Json::ensureArray(root, "variants", QJsonArray(), "Catpack Variants");
            for (auto v : variants) {
                auto variant = Json::ensureObject(v, QJsonObject(), "Cat variant");
                m_variants << Variant{ FS::PathCombine(path, Json::requireString(variant, "path", "Variant path")),
                                       date(Json::requireString(variant, "startTime", "Variant startTime")),
                                       date(Json::requireString(variant, "endTime", "Variant endTime")) };
            }

        } catch (const Exception& e) {
            themeWarningLog() << "Couldn't load catpack json: " << e.cause();
            return;
        }
    } else {
        themeDebugLog() << "No catpack json present.";
    }
}

QString JsonCatPack::path()
{
    const QDate now = QDate::currentDate();
    for (auto var : m_variants) {
        QDate startDate(now.year(), var.startTime.month, var.startTime.day);
        QDate endDate(now.year(), var.endTime.month, var.endTime.day);
        if (startDate.daysTo(endDate) < 0)  // in this case end date should be next year
            endDate = endDate.addYears(1);
        if (startDate.daysTo(now) >= 0 && now.daysTo(endDate) >= 0)
            return var.path;
    }
    return m_defaultPath;
}
